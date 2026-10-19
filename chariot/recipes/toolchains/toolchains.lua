--------------------------------------------------------------------------------------------
-- Copyright (c) 2026-present, SkillerRaptor
--
-- SPDX-License-Identifier: MIT
--------------------------------------------------------------------------------------------

local toolchains = Source {
    name = "toolchains",
    Local("toolchains"),
    prepare = string.format ([[
        PREFIX="%s"
        sed -i "s|\${SYSROOT_DIR}|${SYSROOT_DIR}|g" ./*
        sed -i "s|\${PREFIX}|${PREFIX}|g" ./*
    ]], chariot.target_prefix)
}

return toolchains
