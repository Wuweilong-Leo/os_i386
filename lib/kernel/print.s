TI_GDT equ 0
RPL0 equ 0
SELECTOR_VIDEO equ (0x0003<<3) + TI_GDT + RPL0

[bits 32]
section .text

global put_char ; 表示全局符号

put_char:
    pushad ;压入所有双字长寄存器，一共8个
    mov ax, SELECTOR_VIDEO

    mov gs, ax

    ; 使用显卡的寄存器
    ; 往寄存器 0x03d4 里写入 0x0e, 读取高8位
    mov dx, 0x03d4 
    mov al, 0x0e
    out dx, al
    ; 把寄存器 0x03d4 读入 al
    mov dx, 0x03d5
    in al, dx
    ; 这是高8位
    mov ah, al;
    
    ; 往寄存器 0x03d4 里写入 0x0f, 读取低8位
    mov dx, 0x03d4
    mov al, 0x0f
    out dx, al
    mov dx, 0x03d5
    in al, dx
    
    mov bx, ax
    mov ecx, [esp + 36] ; esp + 36 里是调用者压入的想打印的字符

    cmp cl, 0xd ; CR是 0x0d , LF 是 0x0a
    jz .is_carriage_return 
    cmp cl, 0xa
    jz .is_line_feed

    cmp cl, 0x8
    jz .is_backspace
    jmp .put_other

.is_backspace:
    dec bx
    shl bx, 1 ; (bx-1)*2 是退后一格应该写的偏移
    ; 写入空格
    mov byte [gs:bx], 0x20 
    inc bx
    mov byte [gs:bx], 0x07
    ; 把光标应该出现的位置存在bx
    shr bx, 1
    jmp .set_cursor 

.put_other:
    shl bx, 1
    mov [gs:bx], cl
    inc bx
    mov byte [gs:bx], 0x07
    shr bx, 1
    inc bx
    ;判断是不是超过一个屏幕了
    cmp bx, 2000
    jl .set_cursor

.is_line_feed:
.is_carriage_return:
    xor dx, dx
    mov ax, bx
    mov si, 80
    div si
    sub bx, dx

.is_carriage_return_end:
    add bx, 80
    cmp bx, 2000
.is_line_feed_end:
    jl .set_cursor

.roll_screen:
    cld
    mov ecx, 960

    mov esi, 0xc00b80a0
    mov edi, 0xc00b8000
    rep movsd

    mov ebx, 3840
    mov ecx, 80

.cls:
    mov word [gs:ebx], 0x0720
    add ebx, 2
    loop .cls
    mov bx, 1920

.set_cursor:
    mov dx, 0x03d4
    mov al, 0x0e
    out dx, al
    mov dx, 0x03d5
    mov al, bh
    out dx, al

    mov dx, 0x03d4
    mov al, 0x0f
    out dx, al
    mov dx, 0x03d5
    mov al, bl
    out dx, al
.put_char_done:
    popad
    ret


global put_str
put_str:
    push ebx
    push ecx
    xor ecx, ecx
    mov ebx, [esp + 12]
.goon:
    mov cl, [ebx]
    cmp cl, 0
    jz .str_over
    push ecx
    call put_char
    add esp, 4
    inc ebx
    jmp .goon

.str_over:
    pop ecx
    pop ebx
    ret

section .data
put_int_buffer dq 0 ; 存放答案，一共 8B

global put_int
put_int:
    pushad
    mov ebp, esp
    mov eax, [ebp + 4*9] ;获取参数
    mov edx, eax
    mov edi, 7
    mov ecx, 8
    mov ebx, put_int_buffer

.16based_4bits:
    and edx, 0xf ;取最低的4位
    cmp edx, 9
    jg .isA2F
    add edx, '0'
    jmp .store
.isA2F:
    sub edx, 10
    add edx, 'A'
.store:
    mov [ebx + edi], dl
    dec edi
    shr eax, 4 
    mov edx, eax
    loop .16based_4bits

.ready_to_print:
    inc edi      
.skip_prefix_0: 
    cmp edi, 8 
    je .full0

.go_on_skip:
    mov cl, [put_int_buffer + edi]
    inc edi
    cmp cl, '0'
    je .skip_prefix_0
    dec edi
    jmp .put_each_num

.full0:
    mov cl, '0'
.put_each_num:
    push ecx
    call put_char
    add esp, 4
    inc edi
    mov cl, [put_int_buffer + edi]
    cmp edi, 8
    jl .put_each_num
    popad
    ret
