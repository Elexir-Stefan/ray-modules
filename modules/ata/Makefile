# NURNware Ray kernel module ata
# (For use with Cold iGnition operating system)
# Copyright (C) 2007 - NURNware Technologies


CC=gcc
C_ARGS= -I../../kernel/include -O2 -m32 -c -nostartfiles -fno-common -ffreestanding -Wall -Werror -fno-ident -fno-stack-protector
LD=ld


all: ata


clean:
	@-rm `find . -iname '*.o'`>/dev/null


rebuild: clean ata


ata.o: ata.c ../../kernel/include/raykernel.h \
 ../../kernel/include/ray/typedefs.h ../../kernel/include/standard.h \
 ../../kernel/include/drivers/dmesg/dmesg.h ../../kernel/include/tdm/io.h \
 ../../kernel/include/tdm/tdm.h ../../kernel/include/tdm/int.h \
 ../../kernel/include/threads/sleep.h ../../kernel/include/rmi/rmi.h \
 ../../kernel/include/threads/thread_types.h \
 ../../kernel/include/threads/threads.h \
 ../../kernel/include/ray/arguments.h \
 ../../kernel/include/threads/thread_types.h \
 ../../kernel/include/pid/pids.h ../../kernel/include/memory/memory.h \
 ../../kernel/include/string.h ataio.h ../fs/charerr.h drives.h \
 partitions.h functions.h lowlevel.h ../fs/charexports.h \
 ../fs/charimports.h ../fs/fsregister.h ../../kernel/include/unicode.h \
 ../fs/chardev.h
	@echo -e "	 SUBPART OF MODULE [ata.c]"
	@$(CC) $(C_ARGS) ata.c -o ata.o


filesystems.o: filesystems.c ../../kernel/include/raykernel.h \
 ../../kernel/include/ray/typedefs.h ../../kernel/include/standard.h \
 ../../kernel/include/ray/arguments.h \
 ../../kernel/include/drivers/dmesg/dmesg.h ../../kernel/include/tdm/io.h \
 ../../kernel/include/tdm/tdm.h ../../kernel/include/tdm/int.h \
 ../../kernel/include/threads/sleep.h \
 ../../kernel/include/threads/threads.h ../../kernel/include/rmi/rmi.h \
 ../../kernel/include/threads/thread_types.h \
 ../../kernel/include/threads/thread_types.h \
 ../../kernel/include/pid/pids.h ../../kernel/include/memory/memory.h \
 ../../kernel/include/string.h ataio.h drives.h partitions.h \
 ../fs/charimports.h ../fs/fsregister.h ../../kernel/include/unicode.h \
 ../fs/fserr.h ../fs/charerr.h ../fs/exports.h
	@echo -e "	 SUBPART OF MODULE [filesystems.c]"
	@$(CC) $(C_ARGS) filesystems.c -o filesystems.o


functions.o: functions.c ../../kernel/include/raykernel.h \
 ../../kernel/include/ray/typedefs.h ../../kernel/include/standard.h \
 ../../kernel/include/drivers/dmesg/dmesg.h \
 ../../kernel/include/rmi/rmi.h \
 ../../kernel/include/threads/thread_types.h \
 ../../kernel/include/threads/threads.h \
 ../../kernel/include/ray/arguments.h \
 ../../kernel/include/threads/thread_types.h \
 ../../kernel/include/threads/sleep.h \
 ../../kernel/include/memory/memory.h ../../kernel/include/string.h \
 ../../kernel/include/pid/pids.h ../fs/charerr.h ../fs/fsregister.h \
 ../../kernel/include/unicode.h ../fs/chardev.h drives.h partitions.h \
 lowlevel.h
	@echo -e "	 SUBPART OF MODULE [functions.c]"
	@$(CC) $(C_ARGS) functions.c -o functions.o


list.o: list.c ../../kernel/include/raykernel.h \
 ../../kernel/include/ray/typedefs.h ../../kernel/include/standard.h \
 ../../kernel/include/memory/memory.h ../../kernel/include/rmi/rmi.h \
 ../../kernel/include/threads/thread_types.h \
 ../../kernel/include/threads/threads.h \
 ../../kernel/include/ray/arguments.h \
 ../../kernel/include/threads/thread_types.h ../fs/fsregister.h \
 ../../kernel/include/unicode.h ../fs/chardev.h drives.h partitions.h \
 list.h
	@echo -e "	 SUBPART OF MODULE [list.c]"
	@$(CC) $(C_ARGS) list.c -o list.o


lowlevel.o: lowlevel.c ../../kernel/include/raykernel.h \
 ../../kernel/include/ray/typedefs.h ../../kernel/include/standard.h \
 ../../kernel/include/tdm/io.h ../../kernel/include/tdm/tdm.h \
 ../../kernel/include/rmi/rmi.h \
 ../../kernel/include/threads/thread_types.h \
 ../../kernel/include/threads/threads.h \
 ../../kernel/include/ray/arguments.h \
 ../../kernel/include/threads/thread_types.h \
 ../../kernel/include/threads/sleep.h \
 ../../kernel/include/drivers/dmesg/dmesg.h \
 ../../kernel/include/memory/memory.h ../../kernel/include/tdm/int.h \
 drives.h partitions.h ataio.h
	@echo -e "	 SUBPART OF MODULE [lowlevel.c]"
	@$(CC) $(C_ARGS) lowlevel.c -o lowlevel.o


ata:  ata.o filesystems.o functions.o list.o lowlevel.o
	@echo -e "Building module ata"
	@$(LD) -z max-page-size=4096 -o ata  ata.o filesystems.o functions.o list.o lowlevel.o  ../../kcall_static/rmi/rmi.o ../../kcall_static/tdm/io.o ../../kcall_static/tdm/int.o ../../kcall_static/string/string.o ../../kcall_static/drivers/dmesg/dmesg.o ../../kcall_static/threads/threads.o ../../kcall_static/memory/memory.o ../../kcall_static/threads/sleep.o ../../kcall_static/threads/mutexes.o ../../kcall_static/standard/standard.o -T raymodule.ld
