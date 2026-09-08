#pragma once
// Tiny assertion harness shared by the VX Drum Sequencer tests (tests/timeline idiom).
#include <cstdio>
#include <cstdlib>

static int g_checks = 0;
static int g_failures = 0;

inline void check(const char* what, bool ok)
{
    g_checks++;
    if (!ok)
    {
        g_failures++;
        std::printf("  FAIL: %s\n", what);
    }
}

inline void checkEq(const char* what, long long got, long long want)
{
    g_checks++;
    if (got != want)
    {
        g_failures++;
        std::printf("  FAIL: %s (got %lld, want %lld)\n", what, got, want);
    }
}

inline int finish(const char* suite)
{
    std::printf("%s: %d checks, %d failures\n", suite, g_checks, g_failures);
    return g_failures ? 1 : 0;
}
