# NURNware Ray kernel module i8042
# (For use with Cold iGnition operating system)
# Copyright (C) 2007 - NURNware Technologies


CC=gcc
C_ARGS= -I../../kernel/include -O2 -m32 -c -nostartfiles -fno-common -ffreestanding -Wall -Werror -fno-ident -fno-stack-protector
LD=ld


all: i8042


clean:
	@-rm `find . -iname '*.o'`>/dev/null


rebuild: clean i8042


i8042.o: i8042.c ../../kernel/include/raykernel.h \
 ../../kernel/include/ray/typedefs.h ../../kernel/include/standard.h \
 ../../kernel/include/tdm/io.h ../../kernel/include/tdm/tdm.h \
 ../../kernel/include/tdm/int.h \
 ../../kernel/include/drivers/dmesg/dmesg.h \
 ../../kernel/include/memory/memory.h ../../kernel/include/rmi/rmi.h \
 ../../kernel/include/threads/thread_types.h \
 ../../kernel/include/threads/threads.h \
 ../../kernel/include/ray/arguments.h \
 ../../kernel/include/threads/thread_types.h \
 ../../kernel/include/threads/sleep.h 8042.h i8042io.h keymap_german.h \
 ../../kernel/include/drivers/keyboard/raykeybind.h \
 ../../kernel/include/pid/pids.h exports.h keyboard.h
	@echo -e "	 SUBPART OF MODULE [i8042.c]"
	@$(CC) $(C_ARGS) i8042.c -o i8042.o


i8042:  i8042.o
	@echo -e "Building module i8042"
	@$(LD) -z max-page-size=4096 -o i8042  i8042.o  ../../kcall_static/rmi/rmi.o ../../kcall_static/drivers/dmesg/dmesg.o ../../kcall_static/string/string.o ../../kcall_static/tdm/io.o ../../kcall_static/tdm/int.o ../../kcall_static/threads/sleep.o ../../kcall_static/threads/threads.o ../../kcall_static/threads/mutexes.o ../../kcall_static/memory/memory.o ../../kcall_static/standard/standard.o -T raymodule.ld
