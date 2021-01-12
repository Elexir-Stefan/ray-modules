# NURNware Ray kernel module readtsc
# (For use with Cold iGnition operating system)
# Copyright (C) 2007 - NURNware Technologies


CC=gcc
C_ARGS= -I../../kernel/include -O2 -m32 -c -nostartfiles -fno-common -ffreestanding -Wall -Werror -fno-ident -fno-stack-protector
LD=ld


all: readtsc


clean:
	@-rm `find . -iname '*.o'`>/dev/null


rebuild: clean readtsc


readtsc.o: readtsc.c ../../kernel/include/raykernel.h \
 ../../kernel/include/ray/typedefs.h ../../kernel/include/standard.h \
 ../../kernel/include/rmi/rmi.h \
 ../../kernel/include/threads/thread_types.h \
 ../../kernel/include/threads/threads.h \
 ../../kernel/include/ray/arguments.h \
 ../../kernel/include/threads/thread_types.h \
 ../../kernel/include/threads/sleep.h
	@echo -e "	 SUBPART OF MODULE [readtsc.c]"
	@$(CC) $(C_ARGS) readtsc.c -o readtsc.o


readtsc:  readtsc.o
	@echo -e "Building module readtsc"
	@$(LD) -z max-page-size=4096 -o readtsc  readtsc.o  ../../kcall_static/rmi/rmi.o ../../kcall_static/threads/sleep.o ../../kcall_static/standard/standard.o -T raymodule.ld
