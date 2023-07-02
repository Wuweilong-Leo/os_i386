; 和中断不通，因为switch_to 是个函数，编译器约定被调用者只要保持 
; esi edi ebx ebp esp 不变就行了。因此即使切换线程，只用上下文
; 其实就是这几个寄存器。

[bits 32]
section .text
global switch_to
switch_to:
    
    
    ;前后线程的esi,edi,ebx,ebp 不同，因此要保证先存起来，再出栈。
    push esi
    push edi
    push ebx
    push ebp

    mov eax, [esp + 20] ;eax 里存放 当前线程tcb首地址
    mov [eax], esp ; 把当前esp地址存入tcb中

    mov eax, [esp + 24];eax 里存放 将执行线程tcb首地址
    mov esp, [eax] ; 把下一个线程的栈地址写入esp,完成栈的切换

    pop ebp
    pop ebx
    pop edi
    pop esi

    ret


