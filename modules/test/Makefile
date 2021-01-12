# NURNware Ray kernel module test
# (For use with Cold iGnition operating system)
# Copyright (C) 2007 - NURNware Technologies


CC=gcc
C_ARGS= -I../../kernel/include -O2 -m32 -c -nostartfiles -fno-common -ffreestanding -Wall -Werror -fno-ident -fno-stack-protector
LD=ld


all: test


clean:
	@-rm `find . -iname '*.o'`>/dev/null


rebuild: clean test


elf.o: elf.c ../../kernel/include/raykernel.h \
 ../../kernel/include/ray/typedefs.h ../../kernel/include/standard.h \
 ../../kernel/include/string.h ../../kernel/include/threads/threads.h \
 ../../kernel/include/ray/arguments.h ../../kernel/include/rmi/rmi.h \
 ../../kernel/include/threads/thread_types.h \
 ../../kernel/include/threads/thread_types.h \
 ../../kernel/include/threads/process.h \
 ../../kernel/include/memory/memory.h elftypes.h elf_common.h
	@echo -e "	 SUBPART OF MODULE [elf.c]"
	@$(CC) $(C_ARGS) elf.c -o elf.o


test.o: test.c ../../kernel/include/raykernel.h \
 ../../kernel/include/ray/typedefs.h ../../kernel/include/standard.h \
 ../../kernel/include/kdisplay/kprintf.h \
 ../../kernel/include/threads/sleep.h \
 ../../kernel/include/threads/threads.h \
 ../../kernel/include/ray/arguments.h ../../kernel/include/rmi/rmi.h \
 ../../kernel/include/threads/thread_types.h \
 ../../kernel/include/threads/thread_types.h \
 ../../kernel/include/memory/memory.h ../../kernel/include/string.h \
 ../../kernel/include/debug/debug.h ../../kernel/include/pid/pids.h \
 ../i8042/exports.h ../dmesg/exports.h ../fs/charexports.h \
 ../fs/fsregister.h ../../kernel/include/unicode.h ../fs/charerr.h \
 ../i8042/usrkbd.h ../pci/pcitypes.h ../vfs/exports.h ../vfs/vfstypes.h \
 vendors.h elf.h ../../kernel/include/drivers/keyboard/raykeybind.h
	@echo -e "	 SUBPART OF MODULE [test.c]"
	@$(CC) $(C_ARGS) test.c -o test.o


udiv3.o: udiv3.c ../../kernel/include/raykernel.h \
 ../../kernel/include/ray/typedefs.h ../../kernel/include/standard.h \
 udiv3.h
	@echo -e "	 SUBPART OF MODULE [udiv3.c]"
	@$(CC) $(C_ARGS) udiv3.c -o udiv3.o


test:  elf.o test.o udiv3.o
	@echo -e "Building module test"
	@$(LD) -z max-page-size=4096 -o test  elf.o test.o udiv3.o  ../../kcall_static/debug/debug.o ../../kcall_static/kdisplay/kprintf.o ../../kcall_static/memory/memory.o ../../kcall_static/rmi/rmi.o ../../kcall_static/threads/threads.o ../../kcall_static/threads/mutexes.o ../../kcall_static/threads/process.o ../../kcall_static/string/string.o ../../kcall_static/threads/sleep.o ../../kcall_static/string/unicode.o ../../kcall_static/standard/standard.o -T raymodule.ld
