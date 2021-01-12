# NURNware Ray kernel modules
# (For use with Cold iGnition operating system)
# Copyright (C) 2007 - NURNware Technologies


CC=gcc
C_ARGS= -I../kernel/include -O2 -m32 -c -nostartfiles -ffreestanding -fno-common -Wall -Wswitch-enum -Werror -fno-ident -fno-stack-protector -mpreferred-stack-boundary=2
LD=ld


all: modules


clean:
	@-rm `find . -iname '*.o'`


rebuild: clean modules


standard/standard.o: standard/standard.c ../kernel/include/raykernel.h \
 ../kernel/include/ray/typedefs.h ../kernel/include/standard.h \
 ../kernel/include/syscall.h ../kernel/include/sct/standard.h
	@echo -e "MODULE [standard/standard.c]"
	@$(CC) $(C_ARGS) standard/standard.c -o standard/standard.o


debug/debug.o: debug/debug.c ../kernel/include/raykernel.h \
 ../kernel/include/ray/typedefs.h ../kernel/include/standard.h \
 ../kernel/include/syscall.h ../kernel/include/debug/debug.h \
 ../kernel/include/sct/debug.h
	@echo -e "MODULE [debug/debug.c]"
	@$(CC) $(C_ARGS) debug/debug.c -o debug/debug.o


string/string.o: string/string.c ../kernel/include/raykernel.h \
 ../kernel/include/ray/typedefs.h ../kernel/include/standard.h \
 ../kernel/include/string.h
	@echo -e "MODULE [string/string.c]"
	@$(CC) $(C_ARGS) string/string.c -o string/string.o


string/unicode.o: string/unicode.c ../kernel/include/raykernel.h \
 ../kernel/include/ray/typedefs.h ../kernel/include/standard.h \
 ../kernel/include/unicode.h ../kernel/include/memory/memory.h \
 ../kernel/include/string.h
	@echo -e "MODULE [string/unicode.c]"
	@$(CC) $(C_ARGS) string/unicode.c -o string/unicode.o


memory/memory.o: memory/memory.c ../kernel/include/raykernel.h \
 ../kernel/include/ray/typedefs.h ../kernel/include/standard.h \
 ../kernel/include/syscall.h ../kernel/include/memory/memory.h \
 ../kernel/include/sct/memory.h
	@echo -e "MODULE [memory/memory.c]"
	@$(CC) $(C_ARGS) memory/memory.c -o memory/memory.o


kdisplay/kprintf.o: kdisplay/kprintf.c ../kernel/include/raykernel.h \
 ../kernel/include/ray/typedefs.h ../kernel/include/standard.h \
 ../kernel/include/syscall.h ../kernel/include/kdisplay/kprintf.h \
 ../kernel/include/sct/console.h ../kernel/include/c/stdarg.h
	@echo -e "MODULE [kdisplay/kprintf.c]"
	@$(CC) $(C_ARGS) kdisplay/kprintf.c -o kdisplay/kprintf.o


miscellaneous/stringhash.o: miscellaneous/stringhash.c \
 ../kernel/include/raykernel.h ../kernel/include/ray/typedefs.h \
 ../kernel/include/standard.h ../kernel/include/unicode.h \
 ../kernel/include/miscellaneous/stringhash.h
	@echo -e "MODULE [miscellaneous/stringhash.c]"
	@$(CC) $(C_ARGS) miscellaneous/stringhash.c -o miscellaneous/stringhash.o


rmi/rmi.o: rmi/rmi.c ../kernel/include/raykernel.h \
 ../kernel/include/ray/typedefs.h ../kernel/include/standard.h \
 ../kernel/include/syscall.h ../kernel/include/rmi/rmi.h \
 ../kernel/include/threads/thread_types.h \
 ../kernel/include/threads/threads.h ../kernel/include/ray/arguments.h \
 ../kernel/include/threads/thread_types.h ../kernel/include/sct/ipc.h
	@echo -e "MODULE [rmi/rmi.c]"
	@$(CC) $(C_ARGS) rmi/rmi.c -o rmi/rmi.o


math/math.o: math/math.c ../kernel/include/raykernel.h \
 ../kernel/include/ray/typedefs.h ../kernel/include/standard.h
	@echo -e "MODULE [math/math.c]"
	@$(CC) $(C_ARGS) math/math.c -o math/math.o


hash/hash.o: hash/hash.c ../kernel/include/raykernel.h \
 ../kernel/include/ray/typedefs.h ../kernel/include/standard.h \
 ../kernel/include/miscellaneous/hash.h \
 ../kernel/include/miscellaneous/math.h ../kernel/include/memory/memory.h \
 ../kernel/include/string.h hash/hash.h
	@echo -e "MODULE [hash/hash.c]"
	@$(CC) $(C_ARGS) hash/hash.c -o hash/hash.o


tdm/int.o: tdm/int.c ../kernel/include/raykernel.h \
 ../kernel/include/ray/typedefs.h ../kernel/include/standard.h \
 ../kernel/include/syscall.h ../kernel/include/tdm/int.h \
 ../kernel/include/tdm/tdm.h ../kernel/include/sct/tdm.h
	@echo -e "MODULE [tdm/int.c]"
	@$(CC) $(C_ARGS) tdm/int.c -o tdm/int.o


tdm/io.o: tdm/io.c ../kernel/include/raykernel.h \
 ../kernel/include/ray/typedefs.h ../kernel/include/standard.h \
 ../kernel/include/syscall.h ../kernel/include/tdm/io.h \
 ../kernel/include/tdm/tdm.h ../kernel/include/sct/tdm.h
	@echo -e "MODULE [tdm/io.c]"
	@$(CC) $(C_ARGS) tdm/io.c -o tdm/io.o


tdm/tdm.o: tdm/tdm.c ../kernel/include/raykernel.h \
 ../kernel/include/ray/typedefs.h ../kernel/include/standard.h \
 ../kernel/include/syscall.h ../kernel/include/tdm/tdm.h \
 ../kernel/include/sct/tdm.h
	@echo -e "MODULE [tdm/tdm.c]"
	@$(CC) $(C_ARGS) tdm/tdm.c -o tdm/tdm.o


drivers/dmesg/dmesg.o: drivers/dmesg/dmesg.c \
 ../kernel/include/raykernel.h ../kernel/include/ray/typedefs.h \
 ../kernel/include/standard.h ../kernel/include/rmi/rmi.h \
 ../kernel/include/threads/thread_types.h \
 ../kernel/include/threads/threads.h ../kernel/include/ray/arguments.h \
 ../kernel/include/threads/thread_types.h ../kernel/include/pid/pids.h \
 ../kernel/include/string.h ../kernel/include/drivers/dmesg/dmesg.h \
 ../kernel/include/../../modules/dmesg/exports.h
	@echo -e "MODULE [drivers/dmesg/dmesg.c]"
	@$(CC) $(C_ARGS) drivers/dmesg/dmesg.c -o drivers/dmesg/dmesg.o


threads/mutexes.o: threads/mutexes.c ../kernel/include/raykernel.h \
 ../kernel/include/ray/typedefs.h ../kernel/include/standard.h \
 ../kernel/include/syscall.h ../kernel/include/threads/threads.h \
 ../kernel/include/ray/arguments.h ../kernel/include/rmi/rmi.h \
 ../kernel/include/threads/thread_types.h \
 ../kernel/include/threads/thread_types.h ../kernel/include/sct/threads.h
	@echo -e "MODULE [threads/mutexes.c]"
	@$(CC) $(C_ARGS) threads/mutexes.c -o threads/mutexes.o


threads/process.o: threads/process.c ../kernel/include/raykernel.h \
 ../kernel/include/ray/typedefs.h ../kernel/include/standard.h \
 ../kernel/include/syscall.h ../kernel/include/threads/threads.h \
 ../kernel/include/ray/arguments.h ../kernel/include/rmi/rmi.h \
 ../kernel/include/threads/thread_types.h \
 ../kernel/include/threads/thread_types.h \
 ../kernel/include/threads/process.h ../kernel/include/sct/process.h
	@echo -e "MODULE [threads/process.c]"
	@$(CC) $(C_ARGS) threads/process.c -o threads/process.o


threads/sleep.o: threads/sleep.c ../kernel/include/raykernel.h \
 ../kernel/include/ray/typedefs.h ../kernel/include/standard.h \
 ../kernel/include/syscall.h ../kernel/include/threads/sleep.h \
 ../kernel/include/sct/tasks.h
	@echo -e "MODULE [threads/sleep.c]"
	@$(CC) $(C_ARGS) threads/sleep.c -o threads/sleep.o


threads/threads.o: threads/threads.c ../kernel/include/raykernel.h \
 ../kernel/include/ray/typedefs.h ../kernel/include/standard.h \
 ../kernel/include/syscall.h ../kernel/include/threads/threads.h \
 ../kernel/include/ray/arguments.h ../kernel/include/rmi/rmi.h \
 ../kernel/include/threads/thread_types.h \
 ../kernel/include/threads/thread_types.h ../kernel/include/sct/threads.h
	@echo -e "MODULE [threads/threads.c]"
	@$(CC) $(C_ARGS) threads/threads.c -o threads/threads.o


modules:  standard/standard.o debug/debug.o string/string.o string/unicode.o memory/memory.o kdisplay/kprintf.o miscellaneous/stringhash.o rmi/rmi.o math/math.o hash/hash.o tdm/int.o tdm/io.o tdm/tdm.o drivers/dmesg/dmesg.o threads/mutexes.o threads/process.o threads/sleep.o threads/threads.o
