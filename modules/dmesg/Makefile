# NURNware Ray kernel module dmesg
# (For use with Cold iGnition operating system)
# Copyright (C) 2007 - NURNware Technologies


CC=gcc
C_ARGS= -I../../kernel/include -O2 -m32 -c -nostartfiles -fno-common -ffreestanding -Wall -Werror -fno-ident -fno-stack-protector
LD=ld


all: dmesg


clean:
	@-rm `find . -iname '*.o'`>/dev/null


rebuild: clean dmesg


dmesg.o: dmesg.c ../../kernel/include/raykernel.h \
 ../../kernel/include/ray/typedefs.h ../../kernel/include/standard.h \
 ../../kernel/include/threads/sleep.h \
 ../../kernel/include/memory/memory.h ../../kernel/include/rmi/rmi.h \
 ../../kernel/include/threads/thread_types.h \
 ../../kernel/include/threads/threads.h \
 ../../kernel/include/ray/arguments.h \
 ../../kernel/include/threads/thread_types.h \
 ../../kernel/include/pid/pids.h ../../kernel/include/string.h exports.h \
 dmesgerr.h
	@echo -e "	 SUBPART OF MODULE [dmesg.c]"
	@$(CC) $(C_ARGS) dmesg.c -o dmesg.o


dmesg:  dmesg.o
	@echo -e "Building module dmesg"
	@$(LD) -z max-page-size=4096 -o dmesg  dmesg.o  ../../kcall_static/memory/memory.o ../../kcall_static/rmi/rmi.o ../../kcall_static/string/string.o ../../kcall_static/threads/sleep.o ../../kcall_static/threads/threads.o ../../kcall_static/standard/standard.o -T raymodule.ld
