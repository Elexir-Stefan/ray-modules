# NURNware Ray kernel module pci
# (For use with Cold iGnition operating system)
# Copyright (C) 2007 - NURNware Technologies


CC=gcc
C_ARGS= -I../../kernel/include -O2 -m32 -c -nostartfiles -fno-common -ffreestanding -Wall -Werror -fno-ident -fno-stack-protector
LD=ld


all: pci


clean:
	@-rm `find . -iname '*.o'`>/dev/null


rebuild: clean pci


pci.o: pci.c ../../kernel/include/raykernel.h \
 ../../kernel/include/ray/typedefs.h ../../kernel/include/standard.h \
 ../../kernel/include/tdm/io.h ../../kernel/include/tdm/tdm.h \
 ../../kernel/include/rmi/rmi.h \
 ../../kernel/include/threads/thread_types.h \
 ../../kernel/include/threads/threads.h \
 ../../kernel/include/ray/arguments.h \
 ../../kernel/include/threads/thread_types.h \
 ../../kernel/include/memory/memory.h \
 ../../kernel/include/threads/sleep.h \
 ../../kernel/include/drivers/dmesg/dmesg.h ../../kernel/include/string.h \
 ../../kernel/include/pid/pids.h pcitypes.h pci.h pciio.h
	@echo -e "	 SUBPART OF MODULE [pci.c]"
	@$(CC) $(C_ARGS) pci.c -o pci.o


pciio.o: pciio.c ../../kernel/include/raykernel.h \
 ../../kernel/include/ray/typedefs.h ../../kernel/include/standard.h \
 ../../kernel/include/tdm/io.h ../../kernel/include/tdm/tdm.h \
 ../../kernel/include/memory/memory.h ../../kernel/include/string.h \
 pcitypes.h pciio.h hwmakros.h
	@echo -e "	 SUBPART OF MODULE [pciio.c]"
	@$(CC) $(C_ARGS) pciio.c -o pciio.o


pci:  pci.o pciio.o
	@echo -e "Building module pci"
	@$(LD) -z max-page-size=4096 -o pci  pci.o pciio.o  ../../kcall_static/string/string.o ../../kcall_static/drivers/dmesg/dmesg.o ../../kcall_static/tdm/io.o ../../kcall_static/rmi/rmi.o ../../kcall_static/memory/memory.o ../../kcall_static/threads/sleep.o ../../kcall_static/threads/threads.o ../../kcall_static/standard/standard.o -T raymodule.ld
