# NURNware Ray kernel module reboot
# (For use with Cold iGnition operating system)
# Copyright (C) 2007 - NURNware Technologies


CC=gcc
C_ARGS= -I../../kernel/include -O2 -m32 -c -nostartfiles -fno-common -ffreestanding -Wall -Werror -fno-ident -fno-stack-protector
LD=ld


all: reboot


clean:
	@-rm `find . -iname '*.o'`>/dev/null


rebuild: clean reboot


reboot.o: reboot.c ../../kernel/include/raykernel.h \
 ../../kernel/include/ray/typedefs.h ../../kernel/include/standard.h \
 ../../kernel/include/kdisplay/kprintf.h \
 ../../kernel/include/threads/sleep.h ../../kernel/include/rmi/rmi.h \
 ../../kernel/include/threads/thread_types.h \
 ../../kernel/include/threads/threads.h \
 ../../kernel/include/ray/arguments.h \
 ../../kernel/include/threads/thread_types.h \
 ../../kernel/include/pid/pids.h \
 ../../kernel/include/drivers/keyboard/raykeybind.h ../i8042/exports.h \
 ../i8042/usrkbd.h
	@echo -e "	 SUBPART OF MODULE [reboot.c]"
	@$(CC) $(C_ARGS) reboot.c -o reboot.o


reboot:  reboot.o
	@echo -e "Building module reboot"
	@$(LD) -z max-page-size=4096 -o reboot  reboot.o  ../../kcall_static/kdisplay/kprintf.o ../../kcall_static/rmi/rmi.o ../../kcall_static/threads/sleep.o ../../kcall_static/threads/threads.o ../../kcall_static/standard/standard.o -T raymodule.ld
