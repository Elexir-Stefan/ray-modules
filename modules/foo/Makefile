# NURNware Ray kernel module foo
# (For use with Cold iGnition operating system)
# Copyright (C) 2007 - NURNware Technologies


CC=gcc
C_ARGS= -I../../kernel/include -O2 -m32 -c -nostartfiles -fno-common -ffreestanding -Wall -Werror -fno-ident -fno-stack-protector
LD=ld


all: foo


clean:
	@-rm `find . -iname '*.o'`>/dev/null


rebuild: clean foo


foo.o: foo.c ../../kernel/include/raykernel.h \
 ../../kernel/include/ray/typedefs.h ../../kernel/include/standard.h \
 ../../kernel/include/kdisplay/kprintf.h \
 ../../kernel/include/memory/memory.h \
 ../../kernel/include/threads/sleep.h
	@echo -e "	 SUBPART OF MODULE [foo.c]"
	@$(CC) $(C_ARGS) foo.c -o foo.o


foo:  foo.o
	@echo -e "Building module foo"
	@$(LD) -z max-page-size=4096 -o foo  foo.o  ../../kcall_static/kdisplay/kprintf.o ../../kcall_static/threads/sleep.o ../../kcall_static/memory/memory.o ../../kcall_static/standard/standard.o -T raymodule.ld
