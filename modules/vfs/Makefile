# NURNware Ray kernel module vfs
# (For use with Cold iGnition operating system)
# Copyright (C) 2007 - NURNware Technologies


CC=gcc
C_ARGS= -I../../kernel/include -O2 -m32 -c -nostartfiles -fno-common -ffreestanding -Wall -Werror -fno-ident -fno-stack-protector
LD=ld


all: vfs


clean:
	@-rm `find . -iname '*.o'`>/dev/null


rebuild: clean vfs


vfs.o: vfs.c ../../kernel/include/raykernel.h \
 ../../kernel/include/ray/typedefs.h ../../kernel/include/standard.h \
 ../../kernel/include/string.h ../../kernel/include/unicode.h \
 ../../kernel/include/rmi/rmi.h \
 ../../kernel/include/threads/thread_types.h \
 ../../kernel/include/threads/threads.h \
 ../../kernel/include/ray/arguments.h \
 ../../kernel/include/threads/thread_types.h \
 ../../kernel/include/memory/memory.h \
 ../../kernel/include/threads/sleep.h ../../kernel/include/pid/pids.h \
 ../../kernel/include/miscellaneous/hash.h \
 ../../kernel/include/miscellaneous/stringhash.h vfs.h vfstypes.h \
 vfsstructure.h vfstree.h exports.h ../fs/exports.h
	@echo -e "	 SUBPART OF MODULE [vfs.c]"
	@$(CC) $(C_ARGS) vfs.c -o vfs.o


vfstree.o: vfstree.c ../../kernel/include/raykernel.h \
 ../../kernel/include/ray/typedefs.h ../../kernel/include/standard.h \
 ../../kernel/include/string.h ../../kernel/include/unicode.h \
 ../../kernel/include/memory/memory.h ../../kernel/include/rmi/rmi.h \
 ../../kernel/include/threads/thread_types.h \
 ../../kernel/include/threads/threads.h \
 ../../kernel/include/ray/arguments.h \
 ../../kernel/include/threads/thread_types.h vfstree.h vfsstructure.h
	@echo -e "	 SUBPART OF MODULE [vfstree.c]"
	@$(CC) $(C_ARGS) vfstree.c -o vfstree.o


vfs:  vfs.o vfstree.o
	@echo -e "Building module vfs"
	@$(LD) -z max-page-size=4096 -o vfs  vfs.o vfstree.o  ../../kcall_static/string/string.o ../../kcall_static/memory/memory.o ../../kcall_static/miscellaneous/stringhash.o ../../kcall_static/string/unicode.o ../../kcall_static/rmi/rmi.o ../../kcall_static/threads/sleep.o ../../kcall_static/threads/threads.o ../../kcall_static/threads/mutexes.o ../../kcall_static/hash/hash.o ../../kcall_static/math/math.o ../../kcall_static/standard/standard.o -T raymodule.ld
