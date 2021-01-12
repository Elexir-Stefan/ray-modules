#include <raykernel.h>
#include <syscall.h>
#include <kdisplay/kprintf.h>
#include <sct/console.h>

#include <c/stdarg.h>

static void inline itoa (char *buf, int base, int d);
static void SYSTEM KPrintfInternal(const volatile char *format);

void SYSTEM VideoWriteAttribute(UINT32 col, UINT32 row, UINT32 attribute) {
	SysCallN(SYS_VIDEO_ATTR, &col);
}

void KPrintf (const char *format, ...) {
	va_list arg_ptr;
	int c;
	char buf[20];
	char *p;
	const char *initFormat = format;
	
	char outputBuffer[256];
	char *outputString = outputBuffer;
	char *bufferLimit = outputBuffer + 254;
	
	

	va_start(arg_ptr, format);

	while ((c = *format++) != 0) {
		if (c == '%') {
			c = *format++;
			switch (c) {
				case 'd':
				case 'u':
				case 'x':
					itoa(buf, c, va_arg(arg_ptr, int));
					p = buf;
					goto string;
					break;

				case 's':
					p = va_arg(arg_ptr, char*);
				string:
					while (*p) {
						*outputString++ = *p++;
						if (outputString == bufferLimit) {
							KPrintfInternal(initFormat);
							return;
						}
					}
					break;

				case '%':
					*outputString++ = '%';
					break;
				case 'c':
				default:
					*outputString++ = (va_arg(arg_ptr, int));
					break;
			}
		} else {
			*outputString++ = c;
		}
		if (outputString == bufferLimit) {
			KPrintfInternal(initFormat);
			return;
		}

	}
	va_end(arg_ptr);
	
	*outputString++ = 0;
	
	KPrintfInternal(outputBuffer);
}


/**
 * implements the itoa function wich converts an integer/short/... to a string
 */
static void inline itoa (char* buf, int base, int d) {
	char *p = buf;
	char *p1, *p2;
	unsigned long ud = d;
	int divisor = 10;
	UINT8 counter = 0;

	/* If %d is specified and D is minus, put `-' in the head. */
	if ((base == 'd') && (d < 0)) {
		*p++ = '-';
		buf++;
		ud = -d;
	} else {
		if (base == 'x') divisor = 16;
	}
	/* Divide UD by DIVISOR until UD == 0. */
	do {
		int remainder = ud % divisor;

		*p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
		counter++;
	} while ( (ud /= divisor) || ((base == 'x') && (counter != 8)) );

	/* Terminate BUF. */
	*p = 0;

	/* Reverse BUF. */
	p1 = buf;
	p2 = p - 1;
	while (p1 < p2) {
		char tmp = *p1;
		*p1 = *p2;
		*p2 = tmp;
		p1++;
		p2--;
	}
}


static void SYSTEM KPrintfInternal(const volatile char *format) {
	volatile UINT32 argument = (UINT32)format;
	SysCallN(SYS_PRINTF, &argument);
}
