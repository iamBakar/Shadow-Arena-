#pragma once

// Per-character sprite layout is now stored in CharacterDef itself,
// since each downloaded character has different frame counts per animation.
// These constants are kept only for backward-compat compile checks.
namespace Sheet {
    constexpr int MaxCols = 11;   // maximum frames in any single animation row
}
