/**
 * system (kernel) call 
 */

#define _FASTCALL_

#ifdef _FASTCALL_

#define SysCall(functionNumber, argumentPointer, returnValue) __asm__ __volatile__ ( \
"\t/* Save EIP (next label) */\n" \
"\tmovl $1f, %%edx\n" \
"\t/* rescue ebp */\n" \
"\tpushl %%ebp\n" \
"\t/* ebp is used as a pointer to the stack (esp will be overwritten by MSR setting) */\n" \
"\tmovl %%esp, %%ebp\n" \
"\tsysenter\n" \
"\t1:\n" \
"\t/* back HERE after sysexit in kernel, set new stack-pointer accordingly */\n" \
"\tmovl %%ebp, %%esp\n" \
"\tpopl %%ebp\n" :"=a"(returnValue): "0"(functionNumber), "b"(argumentPointer) : "ecx","edx", "memory")

#define SysCallN(functionNumber, argumentPointer) __asm__ __volatile__ ( \
"\t/* Save EIP (next label) */\n" \
"\tmovl $1f, %%edx\n" \
"\t/* rescue ebp */\n" \
"\tpushl %%ebp\n" \
"\t/* ebp is used as a pointer to the stack (esp will be overwritten by MSR setting) */\n" \
"\tmovl %%esp, %%ebp\n" \
"\tsysenter\n" \
"\t1:\n" \
"\t/* back HERE after sysexit in kernel, set new stack-pointer accordingly */\n" \
"\tmovl %%ebp, %%esp\n" \
"\tpopl %%ebp\n" :: "a"(functionNumber), "b"(argumentPointer) : "ecx","edx", "memory")

#else

#define SysCall(functionNumber, argumentPointer, returnValue) __asm__ __volatile__ ( \
"\tint $0xC1\n" \
:"=a"(returnValue): "0"(functionNumber), "b"(argumentPointer) :  "memory" )
 
 // Syscall without expected return value
 #define SysCallN(functionNumber, argumentPointer) __asm__ __volatile__ ( \
 "\tint $0xC1\n" \
 :: "a"(functionNumber), "b"(argumentPointer) : "memory")

#endif
