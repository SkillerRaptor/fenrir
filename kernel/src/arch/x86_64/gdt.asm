;
; Copyright (c) 2026-present, SkillerRaptor
;
; SPDX-License-Identifier: MIT
;

bits 64

section .text

global load_gdt
global load_tss
global reload_segments

load_gdt:
    lgdt [rdi]
    ret

load_tss:
    mov ax, 0x48
    ltr ax
    ret

reload_segments:
    push 0x28
    lea rax, [rel reload_cs]
    push rax
    retfq

reload_cs:
    mov ax, 0x30
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ret
