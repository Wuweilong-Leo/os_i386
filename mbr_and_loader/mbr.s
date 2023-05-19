%include "boot.inc"

SECTION MBR vstart=0x7c00
    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov sp, 0x7c00
    mov ax, 0xb800
    mov gs, ax

; 一个中断可以有多个功能号，完成不同功能。

;ah 功能号=0x06 表示清屏功能
;al 上卷的行数, 0 表示全部
;bh 表示上卷行属性，可忽略
;(cl,ch),(dl,dh)表示窗口的左上角坐标和窗口右上角坐标

    mov ax, 0x600
    mov bx, 0x700
    mov cx, 0
    mov dx, 0x184f
    int 0x10

    mov byte [gs:0x00], '1'
    mov byte [gs:0x01], 0xA4
    mov byte [gs:0x02], ' '
    mov byte [gs:0x03], 0xA4
    mov byte [gs:0x04], 'M'
    mov byte [gs:0x05], 0xA4
    mov byte [gs:0x06], 'B'
    mov byte [gs:0x07], 0xA4
    mov byte [gs:0x08], 'R'
    mov byte [gs:0x09], 0xA4

    mov eax, LOADER_START_SECTOR
    mov bx, LOADER_BASE_ADDR
    mov cx, 4
    call rd_disk_m_16
    jmp LOADER_BASE_ADDR+0x300 ;跳过loader 的数据部分，直接跑代码段

rd_disk_m_16:
    ;之后的操作要更改eax, cx, 先保存起来后面恢复
    mov esi, eax
    mov di, cx

    ; 写入要读取的扇区数目
    mov dx, 0x1f2
    mov al, cl
    out dx, al
    mov eax, esi

    
    ;
    mov dx, 0x1f3
    out dx, al

    mov cl, 8
    shr eax, cl
    mov dx, 0x1f4
    out dx, al

    shr eax, cl
    mov dx, 0x1f5
    out dx, al

    shr eax, cl
    and al, 0x0f
    or al, 0xe0
    mov dx, 0x1f6
    out dx, al

    mov dx, 0x1f7
    mov al, 0x20
    out dx, al

.not_ready:
    nop
    in al, dx
    and al, 0x88
    cmp al, 0x08
    jnz .not_ready

    mov ax, di
    mov dx, 256
    mul dx
    mov cx, ax
    mov dx, 0x1f0

.go_on_read:
    in ax, dx
    mov [bx], ax
    add bx, 2
    loop .go_on_read
    ret

    times 510-($-$$) db 0
    db 0x55, 0xaa


