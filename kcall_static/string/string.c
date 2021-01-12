#include <raykernel.h>
#include <string.h>


POINTER memcpy(POINTER dest, const POINTER src, UINT32 count){
	UINT8 *c_src, *c_dst;
	c_src = (UINT8*)src;
	c_dst = (UINT8*)dest;
	while(count){
		*c_dst++ = *c_src++;
		count--;
	}
	return(dest);
}

POINTER memset(POINTER const dest, UINT8 c, UINT32 count){
	volatile UINT8 *p = dest;
	while(count--) {
		*p++ = c;
	}
	return(dest);
}

POINTER memsetw(POINTER dest, UINT16 c, UINT32 count){
	volatile UINT16 *p = dest;
	while(count--) {
		*p++ = c;
	}
	return(dest);
}


POINTER memmove(POINTER dst, const  POINTER src, UINT32 count){
	UINT8 *c_src, *c_dst;
	if(src != dst)
	{
		c_src = (UINT8 *)src;
		c_dst = (UINT8 *)dst;
		if(dst > src){
			c_src += (count - 1);
			c_dst += (count - 1);
			while(count){
				*c_dst = *c_src;
				c_src--;
				c_dst--;
				count--;
			}
		} else {
			while(count){
				*c_dst = *c_src;
				c_src++;
				c_dst++;
				count--;
			}
		}
	}
	return(dst);
}

char *strcpy(volatile char *dest,volatile const char *src) {
	while((*dest++ = *src++));
	return (char*)dest;
}


char *strncpy(volatile char *dest, volatile const char *src, UINT32 num) {
	while(num--) *dest++ = *src++;
	return (char*)dest;
}

int strncmp(volatile const char *string1, volatile const char *string2, UINT32 n) {
	while((*string1==*string2) && (*string1) && (*string2) && (--n)) {
		string1++;
		string2++;
	}
	return *string1 - *string2;
}


int strcmp(volatile const char *string1, volatile const char *string2) {
	while((*string1==*string2) && (*string1)) {
		string1++;
		string2++;
	}
	return *string1 - *string2;
}


int strlen(volatile const char *string) {
	volatile const char *counter = string;
	while (*counter) {
		counter++;
	}
	return (int)(counter - string);
}

char *strchr(volatile char *string, char search) {
	while (*string) {
		if (*string == search) {
			return (char*)string;
		}
		string++;
	}
	return NULL;
}

char *strnchr(volatile char *string, UINT32 length, char search) {
	while (*string && length--) {
		if (*string == search) {
			return (char*)string;
		}
		string++;
	}
	return NULL;
}

char *strcat(volatile char *string1,volatile  char *string2) {
	string1 += strlen(string1);
	return strcpy(string1, string2);
}

SINT32 strpos(volatile char *string, char search) {
	SINT32 distance = 0;
	while (*string) {
		if (*string == search) {
			return distance;
		}
		string++;
		distance++;
	}
	return -1;
}

SINT32 strnpos(volatile char *string, UINT32 length, char search) {
	SINT32 distance = 0;
	while (*string && length--) {
		if (*string == search) {
			return distance;
		}
		string++;
		distance++;
	}
	return -1;
}

UINT32 strnlen(volatile char *string, UINT32 maxLength) {
	UINT32 length = maxLength;
	while (*string++ && maxLength--);
	return length - maxLength;
	
}
