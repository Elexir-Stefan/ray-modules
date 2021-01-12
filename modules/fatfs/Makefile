# NURNware Ray kernel module fatfs
# (For use with Cold iGnition operating system)
# Copyright (C) 2007 - NURNware Technologies


CC=gcc
C_ARGS= -I../../kernel/include -O2 -m32 -c -nostartfiles -fno-common -ffreestanding -Wall -Werror -fno-ident -fno-stack-protector
LD=ld


all: fatfs


clean:
	@-rm `find . -iname '*.o'`>/dev/null


rebuild: clean fatfs


fatfs.o: fatfs.c ../../kernel/include/raykernel.h \
 ../../kernel/include/ray/typedefs.h ../../kernel/include/standard.h \
 ../../kernel/include/rmi/rmi.h \
 ../../kernel/include/threads/thread_types.h \
 ../../kernel/include/threads/threads.h \
 ../../kernel/include/ray/arguments.h \
 ../../kernel/include/threads/thread_types.h \
 ../../kernel/include/threads/sleep.h \
 ../../kernel/include/kdisplay/kprintf.h \
 ../../kernel/include/memory/memory.h ../fs/fs.h \
 ../../kernel/include/pid/pids.h ../../kernel/include/string.h \
 ../../kernel/include/miscellaneous/hash.h ../../kernel/include/unicode.h \
 ../fs/../fs/fsregister.h ../fs/../fs/charerr.h ../fs/../fs/charexports.h \
 ../fs/../fs/chardev.h ../fs/../vfs/vfstypes.h ../fs/../vfs/exports.h \
 ../fs/exports.h ../fs/fserr.h fat16.h fatfs.h myfs.h
	@echo -e "	 SUBPART OF MODULE [fatfs.c]"
	@$(CC) $(C_ARGS) fatfs.c -o fatfs.o


fatfs:  fatfs.o
	@echo -e "Building module fatfs"
	@$(LD) -z max-page-size=4096 -o fatfs  fatfs.o  ../../kcall_static/rmi/rmi.o ../../kcall_static/string/string.o ../../kcall_static/string/unicode.o ../../kcall_static/memory/memory.o ../../kcall_static/kdisplay/kprintf.o ../../kcall_static/threads/sleep.o ../../kcall_static/threads/threads.o ../../kcall_static/threads/mutexes.o ../../kcall_static/hash/hash.o ../../kcall_static/math/math.o ../../kcall_static/standard/standard.o -T raymodule.ld
