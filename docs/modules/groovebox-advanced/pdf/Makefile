PANDOC      ?= pandoc
PDF_ENGINE  ?= xelatex
BASE_DIR    := $(abspath $(CURDIR)/..)
MASTER      := $(BASE_DIR)/master.md
OUTPUT      := $(CURDIR)/groovebox-advanced-manual.pdf
LUA_FILTER  := $(CURDIR)/tables.lua

PANDOC_OPTS = \
    --from=markdown+raw_tex+pipe_tables+yaml_metadata_block+autolink_bare_uris-implicit_figures \
    --pdf-engine=$(PDF_ENGINE) \
    --top-level-division=chapter \
    --toc \
    --toc-depth=2 \
    --number-sections \
    --resource-path=$(BASE_DIR) \
    --lua-filter=$(LUA_FILTER) \
    --listings \
    -V mainfont="DejaVu Serif" \
    -V monofont="DejaVu Sans Mono" \
    -V sansfont="DejaVu Sans"

.PHONY: all build pdf clean check

all: pdf

build: $(MASTER)

$(MASTER): build.py $(BASE_DIR)/user_manual.md $(wildcard $(BASE_DIR)/modules/ga*.md)
	@python3 build.py

pdf: $(OUTPUT)

$(OUTPUT): $(MASTER) $(LUA_FILTER)
	@$(PANDOC) $(PANDOC_OPTS) -o $(OUTPUT) $(MASTER)
	@echo "Built $(OUTPUT)"

check:
	@command -v $(PANDOC) >/dev/null 2>&1 || { echo "ERROR: pandoc not installed"; exit 1; }
	@command -v $(PDF_ENGINE) >/dev/null 2>&1 || { echo "ERROR: $(PDF_ENGINE) not installed"; exit 1; }
	@echo "OK: $(PANDOC) and $(PDF_ENGINE) are installed"

clean:
	@rm -f $(MASTER) $(OUTPUT)
