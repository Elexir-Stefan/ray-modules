#ifndef _RAYKEYBIND_H
#define _RAYKEYBIND_H


/**
  * Additional keys that are normally pressed simultaniously (Ctrl, Alt, Shift)
  */
#define ADDKEYS_CTRL	0x10000
#define ADDKEYS_ALT	0x20000
#define ADDKEYS_SHIFT	0x40000

/**
  * Special keys, no characters
  */
/** Ctrl, Alt, Shift, but not other keys pressed */
#define SP_KEY_NONE		0

#define SP_KEY_TAB		9
#define SP_KEY_BACKSPACE	10
#define SP_KEY_RETURN		13

#define SP_KEY_INS		14
#define SP_KEY_DEL		15
#define SP_KEY_HOME		16
#define SP_KEY_END		17
#define SP_KEY_PGUP		18
#define SP_KEY_PGDN		19

#endif
