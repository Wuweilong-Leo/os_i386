[bits 32]
%define ERROR_CODE nop
%define ZERO push 0
%define ALL_SAVE_FLAG 0
%define FAST_SAVE_FLGA 1

extern idt_table
extern scheduler
extern hwi_tail_handle

section .data
global intr_entry_table

intr_entry_table:
%macro VECTOR 2
section .text
intr%1entry:
    %2 ;压入0, 或者不压
    ;压入上下文
    push ds
    push es
    push fs
    push gs
    pushad
    push ALL_SAVE_FLAG

    mov eax, [scheduler]
    mov [eax], esp

    ; clear int 
    mov al, 0x20
    out 0xa0, al
    out 0x20, al
    
    push %1 ; push int num 
    call [idt_table + %1 * 4]
    add esp, 4 ; skip int num

    call hwi_tail_handle
    
section .data
    dd intr%1entry
%endmacro

section .text
global hwi_ret

hwi_ret:
    add esp, 4 ; skip save_flag
    popad
    pop gs
    pop fs
    pop es
    pop ds
    add esp, 4 ; skip error_code
    iret

VECTOR 0x00, ZERO
VECTOR 0x01, ZERO
VECTOR 0x02, ZERO
VECTOR 0x03, ZERO
VECTOR 0x04, ZERO
VECTOR 0x05, ZERO
VECTOR 0x06, ZERO
VECTOR 0x07, ZERO
VECTOR 0x08, ERROR_CODE
VECTOR 0x09, ZERO
VECTOR 0x0a, ERROR_CODE
VECTOR 0x0b, ERROR_CODE
VECTOR 0x0c, ERROR_CODE
VECTOR 0x0d, ERROR_CODE
VECTOR 0x0e, ERROR_CODE
VECTOR 0x0f, ZERO
VECTOR 0x10, ZERO
VECTOR 0x11, ERROR_CODE
VECTOR 0x12, ZERO
VECTOR 0x13, ZERO
VECTOR 0x14, ZERO
VECTOR 0x15, ZERO
VECTOR 0x16, ZERO
VECTOR 0x17, ZERO
VECTOR 0x18, ZERO
VECTOR 0x19, ZERO
VECTOR 0x1a, ZERO
VECTOR 0x1b, ZERO
VECTOR 0x1c, ZERO
VECTOR 0x1d, ZERO
VECTOR 0x1e, ERROR_CODE
VECTOR 0x1f, ZERO
VECTOR 0x20, ZERO
VECTOR 0x21, ZERO
VECTOR 0x22, ZERO
VECTOR 0x23, ZERO
VECTOR 0x24, ZERO
VECTOR 0x25, ZERO
VECTOR 0x26, ZERO
VECTOR 0x27, ZERO
VECTOR 0x28, ZERO
VECTOR 0x29, ZERO
VECTOR 0x2a, ZERO
VECTOR 0x2b, ZERO
VECTOR 0x2c, ZERO
VECTOR 0x2d, ZERO
VECTOR 0x2e, ZERO
VECTOR 0x2f, ZERO

[bits 32]
extern syscall_table

section .text

global syscall_entry

syscall_entry:
    push 0
    push ds
    push es
    push fs
    push gs
    pushad
    push ALL_SAVE_FLAG
    push 0x80
; 压入系统调用参数
    push edx
    push ecx
    push ebx

    call [syscall_table + eax * 4]
; 跳过参数
    add esp, 12
; 把系统调用返回值传入栈中，中断返回时又会返回到eax里
    mov [esp + 9 * 4], eax

    add esp, 4 ; skip int num 0x80

    jmp hwi_ret




