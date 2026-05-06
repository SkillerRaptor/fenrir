//
// Copyright (c) 2026-present, SkillerRaptor
//
// SPDX-License-Identifier: MIT
//

fn octal_to_binary(data: &[u8]) -> u64 {
    let mut result = 0;

    for byte in data {
        result *= 8;
        result += (byte - b'0') as u64;
    }

    result
}

pub fn lookup<'a>(archive: &'a [u8], file_name: &str) -> Option<&'a [u8]> {
    let mut offset = 0;

    while archive.get(offset + 257..offset + 262) == Some(b"ustar") {
        let file_size = octal_to_binary(&archive[offset + 0x7c..offset + 0x7c + 11]);

        if archive.get(offset..offset + file_name.len()) == Some(file_name.as_bytes()) {
            return Some(&archive[offset + 512..offset + 512 + file_size as usize]);
        }

        offset += (((file_size as usize + 511) / 512) + 1) * 512;
    }

    None
}
