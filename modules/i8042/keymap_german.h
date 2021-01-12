#ifndef _KEYMAP_GERMAN_H
#define _KEYMAP_GERMAN_H

#define EVNT_KEYUP	0x80

#define KEY_CTRL	29
#define KEY_ALT		56
#define KEY_SHIFT	42

#include <drivers/keyboard/raykeybind.h>


unsigned char keyMapUnshifted [128] = {
	   0,  27,  '1', '2',  '3',
	 '4', '5',  '6', '7',  '8',
	 '9', '0',  '·', 'Ô', SP_KEY_BACKSPACE,
    SP_KEY_TAB, 'q',  'w', 'e',  'r',
	 't', 'z',  'u', 'i',  'o',
	 'p', 'Å',  '+', SP_KEY_RETURN,    0,
	 'a', 's',  'd', 'f',  'g',
	 'h', 'j',  'k', 'l',  'î', 
	 'Ñ', '#',   0,   0,  'y',  
	 'x', 'c',  'v', 'b',  'n', 
	 'm', ',',  '.', '-',    '?', 
	  '*',    0,  ' ',    0,   59,
       60,   61,   62,   63,   64,
	   65,   66,   67,   68,    0,
	    0,    SP_KEY_HOME,   72,    SP_KEY_PGUP,    0, 
	   75,    0,   77,    0,    SP_KEY_END,
       80,    SP_KEY_PGDN,    SP_KEY_INS, SP_KEY_DEL,    0, 
	0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 
	0, 0, 0
};

unsigned char keyMapShifted[128] = {
        0,   27,  '!',  '"',  'ı', 
	  '$',  '%',  '&',  '/',  '(', 
	  ')',  '=',  '?',  '`', SP_KEY_BACKSPACE, 
	 SP_KEY_TAB,  'Q',  'W',  'E',  'R',  
	  'T',  'Z',  'U',  'I',  'O', 
	  'P',  'ö',  '*', SP_KEY_RETURN,    0, 
	  'A',  'S',  'D',  'F',  'G', 
	  'H',  'J',  'K',  'L',  'ô', 
	  'é',  '\'',    0,    0,  'Y', 
	  'X',  'C',  'V',  'B',  'N', 
	  'M',  ';',  ':',  '_',    0, 
	  '*',    0,  ' ',    0,   59,
       60,   61,   62,   63,   64,
	   65,   66,   67,   68,    0,
	    0,    SP_KEY_HOME,   72,    SP_KEY_PGUP,    0, 
	   75,    0,   77,    0,    SP_KEY_END,
       80,    SP_KEY_PGDN,    SP_KEY_INS, SP_KEY_DEL,    0, 
	0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 
	0, 0, 0
};

#endif
