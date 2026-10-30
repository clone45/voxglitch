#!/usr/bin/env python3
"""
Find ModuleWidget constructors that touch `module` without a null check.

Rack's module browser builds every ModuleWidget with module == NULL to draw
its preview. A constructor that reaches through the pointer before checking
it -- even only to take a member's address, `&module->page` -- is undefined
behaviour, and a recent clang uses that to assume the pointer is non-null,
deletes the `if (module)` guard that follows, and crashes Rack the moment the
browser opens (ArpSeq, nightly of 2026-10-30, macOS). The library's older
toolchain keeps the guard, so the library build never showed it; neither
cppcheck, GCC's analyzer nor clang's analyzer reports it. Hence this check.

It is lexical, not a compiler: for every struct deriving from a *ModuleWidget
it finds the constructor taking the module pointer and flags each `name->`
(or `(*name)`) whose statement is not inside a guard. A guard is any of

    if (module) { ... }            if (module != NULL) stmt;
    if (!module) return;  ...      if (module == NULL) { return; }  ...
    module ? &module->x : NULL     [captures](...) { ... }   (deferred)

for `module`, the constructor's parameter, and any alias made from it
(`ArpSeq* m = dynamic_cast<ArpSeq*>(module);`). Code after an early return
on a negative guard counts as guarded for the rest of that scope.

    python3 scripts/check_null_module.py            # whole src/modules tree
    python3 scripts/check_null_module.py FILE...    # just these files

Exit code 1 if anything is flagged. release_check.py imports find_issues().
"""

import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
MODULES_DIR = REPO_ROOT / "src" / "modules"

# `struct Foo : ModuleWidget`, `: public VoxglitchModuleWidget`, `: app::ModuleWidget`
WIDGET_RE = re.compile(r"\bstruct\s+(\w+)\s*:\s*(?:public\s+)?(?:\w+::)*(\w*ModuleWidget)\b")


def strip_comments_and_strings(text):
    """Blank out comments and string/char literals, keeping every newline so
    line numbers survive."""
    out = []
    i, n = 0, len(text)
    while i < n:
        c = text[i]
        if text.startswith("//", i):
            j = text.find("\n", i)
            j = n if j < 0 else j
            out.append(" " * (j - i))
            i = j
        elif text.startswith("/*", i):
            j = text.find("*/", i + 2)
            j = n if j < 0 else j + 2
            out.append("".join("\n" if ch == "\n" else " " for ch in text[i:j]))
            i = j
        elif c in "\"'":
            j = i + 1
            while j < n and text[j] != c:
                j += 2 if text[j] == "\\" else 1
            j = min(j + 1, n)
            out.append(c + " " * (j - i - 2) + c if j - i >= 2 else c)
            i = j
        else:
            out.append(c)
            i += 1
    return "".join(out)


def matching_brace(text, open_pos):
    depth = 0
    for i in range(open_pos, len(text)):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return i
    return -1


def find_constructor_body(text, class_name, class_pos):
    """(param_name, body_start, body_end) of `Class(Type* param) [: init] {`,
    searched after the class head; None if the class has no such constructor."""
    ctor_re = re.compile(r"\b" + re.escape(class_name) + r"\s*\(\s*(?:const\s+)?\w+\s*\*\s*(\w+)\s*\)\s*(?::[^{;]*)?\{")
    m = ctor_re.search(text, class_pos)
    if not m:
        return None
    open_pos = m.end() - 1
    close_pos = matching_brace(text, open_pos)
    if close_pos < 0:
        return None
    return m.group(1), open_pos + 1, close_pos


class ScopeWalker:
    """Marks which character ranges of a constructor body are guarded."""

    def __init__(self, body, names):
        self.body = body
        self.names = names
        alt = "|".join(re.escape(n) for n in sorted(names, key=len, reverse=True))
        self.name_re = r"(?:this\s*->\s*)?(?:" + alt + r")\b"
        # if (module) / if (module != NULL) / if (module && ...) / if (x && module)
        self.positive_if = re.compile(
            r"\bif\s*\((?:[^()]*&&\s*)?\s*" + self.name_re + r"\s*(?:!=\s*(?:NULL|nullptr|0)\b)?\s*(?:\)|&&)")
        # if (!module) / if (module == NULL) / if (!module || ...)
        self.negative_if = re.compile(
            r"\bif\s*\(\s*(?:!\s*" + self.name_re + r"|" + self.name_re + r"\s*==\s*(?:NULL|nullptr|0)\b)\s*(?:\)|\|\|)")
        self.ternary = re.compile(r"(?:!\s*)?\(?\s*" + self.name_re + r"\s*\)?\s*\?")   # module ? / (module) ? / !module ?
        self.lambda_head = re.compile(r"\]\s*(?:\([^)]*\))?\s*(?:mutable\s*)?(?:->\s*[\w:<>*&\s]+?)?\s*$")
        self.deref = re.compile(r"(?<![\w.])" + self.name_re + r"\s*->|\(\s*\*\s*" + self.name_re + r"\s*\)")
        self.guarded = [False] * len(body)

    def mark(self, start, end, value=True):
        for i in range(start, end):
            self.guarded[i] = value

    def walk(self, start, end, guarded):
        text = self.body
        i = start
        stmt_start = start
        paren = 0
        last_if_negative = False
        while i < end:
            c = text[i]
            if c == "(":
                paren += 1
            elif c == ")":
                paren = max(0, paren - 1)
            elif c == "{":
                close = matching_brace(text, i)
                if close < 0 or close > end:
                    close = end
                preceding = text[stmt_start:i]
                self.mark(stmt_start, i, guarded or self.statement_is_guarded(preceding))
                is_else = re.search(r"\belse\s*$", preceding) is not None
                negative = self.negative_if.search(preceding) is not None
                child_guarded = (guarded
                                 or self.positive_if.search(preceding) is not None
                                 or self.lambda_head.search(preceding) is not None
                                 or (is_else and last_if_negative))
                if negative:
                    child_guarded = guarded
                self.mark(i, i + 1, child_guarded)
                self.walk(i + 1, close, child_guarded)
                self.mark(close, close + 1, child_guarded)
                # `if (!module) { ... return; }` guards the rest of this scope
                if negative and re.search(r"\breturn\b", self.top_level_text(i + 1, close)):
                    guarded = True
                last_if_negative = negative if not is_else else False
                i = close + 1
                stmt_start = i
                paren = 0
                continue
            elif c == ";" and paren == 0:
                stmt = text[stmt_start:i + 1]
                self.mark(stmt_start, i + 1, guarded or self.statement_is_guarded(stmt))
                # `if (!module) return;` guards the rest of this scope
                if self.negative_if.search(stmt) and re.search(r"\breturn\b", stmt):
                    guarded = True
                last_if_negative = False
                stmt_start = i + 1
            i += 1
        self.mark(stmt_start, end, guarded)

    def top_level_text(self, start, end):
        """The text of a block with nested blocks blanked, so a `return`
        inside an inner lambda or loop does not count as an early return."""
        text = self.body[start:end]
        out, depth = [], 0
        for ch in text:
            if ch == "{":
                depth += 1
            elif ch == "}":
                depth -= 1
            out.append(ch if depth == 0 else " ")
        return "".join(out)

    def statement_is_guarded(self, stmt):
        if self.positive_if.search(stmt):       # if (module) stmt;
            return True
        if self.ternary.search(stmt):           # module ? &module->x : NULL
            return True
        return False

    def issues(self):
        found = []
        for m in self.deref.finditer(self.body):
            if not self.guarded[m.start()]:
                found.append(m.start())
        return found


def aliases_of(body, names):
    """Names assigned from the module pointer inside the body."""
    alt = "|".join(re.escape(n) for n in names)
    found = set(names)
    for m in re.finditer(r"\b(\w+)\s*=\s*(?:(?:dynamic|static|reinterpret)_cast\s*<[^>]+>\s*\(\s*(?:" + alt + r")\s*\)|(?:" + alt + r")\s*;)", body):
        found.add(m.group(1))
    return found


def check_file(path):
    raw = Path(path).read_text(encoding="utf-8", errors="replace")
    text = strip_comments_and_strings(raw)
    issues = []
    for wm in WIDGET_RE.finditer(text):
        class_name = wm.group(1)
        ctor = find_constructor_body(text, class_name, wm.end())
        if not ctor:
            continue
        param, start, end = ctor
        body = text[start:end]
        names = aliases_of(body, {param, "module"})
        walker = ScopeWalker(body, names)
        walker.walk(0, len(body), False)
        for pos in walker.issues():
            line_no = text.count("\n", 0, start + pos) + 1
            issues.append({
                "file": str(Path(path).resolve().relative_to(REPO_ROOT)) if Path(path).resolve().is_relative_to(REPO_ROOT) else str(path),
                "line": line_no,
                "widget": class_name,
                "content": raw.splitlines()[line_no - 1].strip(),
            })
    return issues


def find_issues(paths=None):
    if not paths:
        paths = sorted(set(list(MODULES_DIR.rglob("*.hpp")) + list(MODULES_DIR.rglob("*.cpp"))))
    issues = []
    for p in paths:
        issues.extend(check_file(p))
    return issues


def main(argv):
    issues = find_issues([Path(a) for a in argv[1:]] or None)
    for it in issues:
        print(f"{it['file']}:{it['line']}: {it['widget']} constructor reaches through the module pointer unguarded: {it['content']}")
    if issues:
        print(f"\n{len(issues)} unguarded use(s). The module browser constructs every ModuleWidget with module == NULL.")
    else:
        print("OK: no ModuleWidget constructor reaches through a possibly-NULL module pointer.")
    return 1 if issues else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
