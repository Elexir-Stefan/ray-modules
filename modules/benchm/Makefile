# NURNware Ray kernel module benchm
# (For use with Cold iGnition operating system)
# Copyright (C) 2007 - NURNware Technologies


CC=gcc
C_ARGS= -I../../kernel/include -O2 -m32 -c -nostartfiles -fno-common -ffreestanding -Wall -Werror -fno-ident -fno-stack-protector
LD=ld


all: benchm


clean:
	@-rm `find . -iname '*.o'`>/dev/null


rebuild: clean benchm


benchm.o: benchm.c ../../kernel/include/raykernel.h \
 ../../kernel/include/ray/typedefs.h ../../kernel/include/standard.h \
 ../../kernel/include/kdisplay/kprintf.h \
 ../../kernel/include/threads/sleep.h ../../kernel/include/rmi/rmi.h \
 ../../kernel/include/threads/thread_types.h \
 ../../kernel/include/threads/threads.h \
 ../../kernel/include/ray/arguments.h \
 ../../kernel/include/threads/thread_types.h \
 ../../kernel/include/string.h ../../kernel/include/pid/pids.h
	@echo -e "	 SUBPART OF MODULE [benchm.c]"
	@$(CC) $(C_ARGS) benchm.c -o benchm.o


udiv3.o: udiv3.c ../../kernel/include/raykernel.h \
 ../../kernel/include/ray/typedefs.h ../../kernel/include/standard.h \
 udiv3.h
	@echo -e "	 SUBPART OF MODULE [udiv3.c]"
	@$(CC) $(C_ARGS) udiv3.c -o udiv3.o


benchm:  benchm.o udiv3.o
	@echo -e "Building module benchm"
	@$(LD) -z max-page-size=4096 -o benchm  benchm.o udiv3.o  ../../kcall_static/rmi/rmi.o ../../kcall_static/threads/sleep.o ../../kcall_static/threads/threads.o ../../kcall_static/string/string.o ../../kcall_static/kdisplay/kprintf.o ../../kcall_static/standard/standard.o -T raymodule.ld
