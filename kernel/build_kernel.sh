gcc -m32 -c -o main.o main.c -std=c99 &&
gcc -m32 -c -o init.o init.c -std=c99 &&
gcc -m32 -c -o interrput.o interrupt.c -std=c99 &&
gcc -m32 -c -o print.o print.c -std=c99 && 
gcc -m32 -c -o timer.o timer.c -std=c99 &&
nasm -f elf -o kernel.o kernel.s &&
ld main.o init.o interrput.o kernel.o print.o timer.o -Ttext 0xc0001500 -e main -m elf_i386 -o kernel.bin && 
dd if=./kernel.bin of=/os_i386/bochs/hd60M.img bs=512 count=200 seek=9 conv=notrunc 

