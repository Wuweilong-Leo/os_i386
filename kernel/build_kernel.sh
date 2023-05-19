gcc -m32 -c -o main.o main.c  && ld main.o -Ttext 0xc0001500 -e main -m elf_i386 -o kernel.bin && dd if=./kernel.bin of=/os_i386/bochs/hd60M.img bs=512 count=200 seek=9 conv=notrunc 
