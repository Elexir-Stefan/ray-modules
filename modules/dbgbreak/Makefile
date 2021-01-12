# NURNware Ray kernel module dbgbreak
# (For use with Cold iGnition operating system)
# Copyright (C) 2007 - NURNware Technologies


CC=gcc
C_ARGS= -I../../kernel/include -O2 -m32 -c -nostartfiles -fno-common -ffreestanding -Wall -Werror -fno-ident -fno-stack-protector
LD=ld


all: dbgbreak


clean:
	@-rm `find . -iname '*.o'`>/dev/null


rebuild: clean dbgbreak


dbgbreak.o: dbgbreak.c ../../kernel/include/raykernel.h \
 ../../kernel/include/ray/typedefs.h ../../kernel/include/standard.h \
 ../../kernel/include/kdisplay/kprintf.h \
 ../../kernel/include/threads/sleep.h ../../kernel/include/rmi/rmi.h \
 ../../kernel/include/threads/thread_types.h \
 ../../kernel/include/threads/threads.h \
 ../../kernel/include/ray/arguments.h \
 ../../kernel/include/threads/thread_types.h \
 ../../kernel/include/debug/debug.h ../../kernel/include/pid/pids.h \
 ../../kernel/include/drivers/keyboard/raykeybind.h ../i8042/exports.h \
 ../i8042/usrkbd.h
	@echo -e "	 SUBPART OF MODULE [dbgbreak.c]"
	@$(CC) $(C_ARGS) dbgbreak.c -o dbgbreak.o


dbgbreak:  dbgbreak.o
	@echo -e "Building module dbgbreak"
	@$(LD) -z max-page-size=4096 -o dbgbreak  dbgbreak.o  ../../kcall_static/debug/debug.o ../../kcall_static/kdisplay/kprintf.o ../../kcall_static/rmi/rmi.o ../../kcall_static/threads/sleep.o ../../kcall_static/threads/threads.o ../../kcall_static/threads/mutexes.o ../../kcall_static/standard/standard.o -T raymodule.ld
