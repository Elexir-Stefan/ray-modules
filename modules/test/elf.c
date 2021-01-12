#include <raykernel.h>
#include <string.h>
#include <threads/threads.h>
#include <threads/process.h>
#include <memory/memory.h>

#include "elftypes.h"

#define READTSC(var) __asm__ __volatile__("rdtsc":"=a"(var)::"edx")


BOOL IsELF32(UINT8 *image, UINT32 length) {

	// cannot be possible if file is smaller than header
	if (length < sizeof(Elf32_Ehdr)) {
		return FALSE;
	}

	Elf32_Ehdr* elf_header = (Elf32_Ehdr*) image;

	// check for valid ELF header
	if (elf_header->e_magic != ELF_MAGIC) {
		return FALSE;
	}

	// check for 32 bit elf file
	if (elf_header->e_ident[EI_CLASS] != ELFCLASS32) {
		return FALSE;
	}

	return TRUE;
}

static UINT32 countLoadableSections(Elf32_Phdr *program_header, UINT32 numHeaders) {
	UINT32 i, count = 0;
	for (i = 0; i < numHeaders; i++) {
			// if loadable, only
		if (program_header->p_type == PT_LOAD) {
			count++;
		}
		program_header++;
	}
	return count;
}

BOOL ELF32LoadFile(UINT8 *image, UINT32 length, char *procName, char *procArguments) {
	UINT32 i;
	
	if (IsELF32(image, length)) {
		Elf32_Ehdr* elf_header = (Elf32_Ehdr*) image;
		
		LOAD_PROCESS *lProcess = malloc(sizeof(LOAD_PROCESS));
	
		if (!lProcess) {
			return FALSE;
		}
	
		strcpy(lProcess->processInfo.name, procName);
		lProcess->processInfo.privilegeLevel = 255; // no rights at all
		lProcess->processInfo.priority = RAY_PRIO_NORMAL;
		// set entry point
		lProcess->processInfo.entryPoint = elf_header->e_entry;
		
		// set pointer to first program headers. Others will follow
		Elf32_Phdr* program_header = (Elf32_Phdr*) (image + elf_header->e_phoff);
		UINT32 numLoadable = countLoadableSections(program_header, elf_header->e_phnum);
		
		// allocate structures
		lProcess->loadableSegments = malloc(numLoadable * sizeof(LOAD_MEM_SEGMENT));
		lProcess->numMemSegments = numLoadable;
		
		lProcess->processInfo.arguments.count = 0;
		lProcess->processInfo.arguments.values = NULL;
		
		// pass arguments
		lProcess->numAddMemory = 1;
		lProcess->additionalMemory = malloc(sizeof(LOAD_ADD_MEM));
		if (!lProcess->additionalMemory) {
			return FALSE;
		}
		
		UINT32 argStringSize = strlen(procArguments);
		char *argString;
		argString = malloc(argStringSize + 1);	// including 0-termination
		if (!argString) {
			return FALSE;
		}
		strcpy(argString, procArguments);
		lProcess->additionalMemory->allocPointer = (UINT32)argString;
		lProcess->additionalMemory->memoryAttributes = MEM_ATTRIB_READABLE;
		
		
		LOAD_MEM_SEGMENT *lSeg = lProcess->loadableSegments;
		
		UINT32* unusedData[16] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
		UINT32** freeData = unusedData;


		// load each program header (if neccessary)
		for (i = 0; i < elf_header->e_phnum; i++) {
			// if loadable, only
			if (program_header->p_type == PT_LOAD) {
				lSeg->segmentStart = (UINT32*)(image + program_header->p_offset);
				lSeg->segmentLength = program_header->p_memsz;
				lSeg->loadAddress = program_header->p_vaddr;
				lSeg->memoryAttributes = MEM_ATTRIB_READABLE;
				if (program_header->p_flags & PF_X) lSeg->memoryAttributes |= MEM_ATTRIB_EXECUTABLE;
				if (program_header->p_flags & PF_W) lSeg->memoryAttributes |= MEM_ATTRIB_WRITABLE;
				
				if (program_header->p_memsz > program_header->p_filesz) {
					// Allocate additional data, and clear it...
					char* tempData = malloc(program_header->p_memsz);
					memcpy(tempData, image + program_header->p_offset, program_header->p_filesz);
					memset(tempData + program_header->p_filesz, 0, program_header->p_memsz - program_header->p_filesz);
					
					// change pointer accordingly
					lSeg->segmentStart = (UINT32*)tempData;
					
					*freeData = (UINT32*)tempData;
					freeData++;
				}
				lSeg++;
			}
			program_header++;
		}

		BOOL result = ProcessLoad(lProcess);
		free(lProcess->loadableSegments);
		free(lProcess->additionalMemory);
		free(lProcess);
		
		// free unused
		while(freeData > unusedData) {
			freeData--;
			free(*freeData);
		}
		return result;
	} else {
		return FALSE;
	}

}

UINT32 ELF32LoadFile2(UINT8 *image, UINT32 length, char *procName, char *procArguments) {
	UINT32 i;
	
	UINT32 processCreationTime = 0;
	
	if (IsELF32(image, length)) {
		Elf32_Ehdr* elf_header = (Elf32_Ehdr*) image;
		
		LOAD_PROCESS *lProcess = malloc(sizeof(LOAD_PROCESS));
		
		if (!lProcess) {
			return 0;
		}
		
		strcpy(lProcess->processInfo.name, procName);
		lProcess->processInfo.privilegeLevel = 255; // no rights at all
		lProcess->processInfo.priority = RAY_PRIO_NORMAL;
		// set entry point
		lProcess->processInfo.entryPoint = elf_header->e_entry;
		
		// set pointer to first program headers. Others will follow
		Elf32_Phdr* program_header = (Elf32_Phdr*) (image + elf_header->e_phoff);
		UINT32 numLoadable = countLoadableSections(program_header, elf_header->e_phnum);
		
		// allocate structures
		lProcess->loadableSegments = malloc(numLoadable * sizeof(LOAD_MEM_SEGMENT));
		lProcess->numMemSegments = numLoadable;
		
		lProcess->processInfo.arguments.count = 0;
		lProcess->processInfo.arguments.values = NULL;
		
		// pass arguments
		lProcess->numAddMemory = 1;
		lProcess->additionalMemory = malloc(sizeof(LOAD_ADD_MEM));
		if (!lProcess->additionalMemory) {
			return FALSE;
		}
		
		UINT32 argStringSize = strlen(procArguments);
		char *argString;
		argString = malloc(argStringSize + 1);	// including 0-termination
		if (!argString) {
			return FALSE;
		}
		strcpy(argString, procArguments);
		lProcess->additionalMemory->allocPointer = (UINT32)argString;
		lProcess->additionalMemory->memoryAttributes = MEM_ATTRIB_READABLE;
		
		
		LOAD_MEM_SEGMENT *lSeg = lProcess->loadableSegments;
		
		UINT32* unusedData[16] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
		UINT32** freeData = unusedData;
		
		
		// load each program header (if neccessary)
		for (i = 0; i < elf_header->e_phnum; i++) {
			// if loadable, only
			if (program_header->p_type == PT_LOAD) {
				lSeg->segmentStart = (UINT32*)(image + program_header->p_offset);
				lSeg->segmentLength = program_header->p_memsz;
				lSeg->loadAddress = program_header->p_vaddr;
				lSeg->memoryAttributes = MEM_ATTRIB_READABLE;
				if (program_header->p_flags & PF_X) lSeg->memoryAttributes |= MEM_ATTRIB_EXECUTABLE;
				if (program_header->p_flags & PF_W) lSeg->memoryAttributes |= MEM_ATTRIB_WRITABLE;
				
				if (program_header->p_memsz > program_header->p_filesz) {
					// Allocate additional data, and clear it...
					char* tempData = malloc(program_header->p_memsz);
					memcpy(tempData, image + program_header->p_offset, program_header->p_filesz);
					memset(tempData + program_header->p_filesz, 0, program_header->p_memsz - program_header->p_filesz);
					
					// change pointer accordingly
					lSeg->segmentStart = (UINT32*)tempData;
					
					*freeData = (UINT32*)tempData;
					freeData++;
				}
				lSeg++;
			}
			program_header++;
		}
		
		READTSC(processCreationTime);
		BOOL result = ProcessLoad(lProcess);
		free(lProcess->loadableSegments);
		free(lProcess->additionalMemory);
		free(lProcess);
		
		// free unused
		while(freeData > unusedData) {
			freeData--;
			free(*freeData);
		}
		return result == TRUE? processCreationTime :  0;
	} else {
		return FALSE;
	}
	
}
