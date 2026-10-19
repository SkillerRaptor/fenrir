--------------------------------------------------------------------------------------------
-- Copyright (c) 2026-present, SkillerRaptor
--
-- SPDX-License-Identifier: MIT
--------------------------------------------------------------------------------------------

local mlibc_source = Source {
    name = "mlibc",
    Git("https://github.com/SkillerRaptor/mlibc.git", "86ddc2a14ddacba41ec12f854289b3a929133ace"),
};

return mlibc_source
