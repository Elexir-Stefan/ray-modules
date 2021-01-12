# NURNware Ray kernel module deferredq
# (For use with Cold iGnition operating system)
# Copyright (C) 2007 - NURNware Technologies


CC=gcc
C_ARGS= -I../../kernel/include -O2 -m32 -c -nostartfiles -fno-common -ffreestanding -Wall -Werror -fno-ident -fno-stack-protector
LD=ld


all: deferredq


clean:
	@-rm `find . -iname '*.o'`>/dev/null


rebuild: clean deferredq


deferredq.o: deferredq.c ../../kernel/include/raykernel.h \
 ../../kernel/include/ray/typedefs.h ../../kernel/include/standard.h \
 ../../kernel/include/threads/sleep.h ../../kernel/include/rmi/rmi.h \
 ../../kernel/include/threads/thread_types.h \
 ../../kernel/include/threads/threads.h \
 ../../kernel/include/ray/arguments.h \
 ../../kernel/include/threads/thread_types.h \
 ../../kernel/include/string.h
	@echo -e "	 SUBPART OF MODULE [deferredq.c]"
	@$(CC) $(C_ARGS) deferredq.c -o deferredq.o


deferredq:  deferredq.o
	@echo -e "Building module deferredq"
	@$(LD) -z max-page-size=4096 -o deferredq  deferredq.o  ../../kcall_static/threads/sleep.o ../../kcall_static/threads/threads.o ../../kcall_static/rmi/rmi.o ../../kcall_static/string/string.o ../../kcall_static/standard/standard.o -T raymodule.ld
