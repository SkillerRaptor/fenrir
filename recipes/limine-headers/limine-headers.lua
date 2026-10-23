--------------------------------------------------------------------------------------------
-- Copyright (c) 2026-present, SkillerRaptor
--
-- SPDX-License-Identifier: MIT
--------------------------------------------------------------------------------------------

local limine_headers = Source {
    name = "limine-headers",
    Git("https://github.com/Limine-Bootloader/limine-protocol.git", "3a0526b700e356f0eac1b71a77697b3fd1c707a3"),
};

return limine_headers
