typedef struct _ELEMENT{
	UINT32 request;
	PCHAR_DISK disk;
	UINT16 *buffer;
	struct _ELEMENT *next;
} ELEMENT;

typedef struct {
	ELEMENT *root;
	ELEMENT *tail;
} LIST;

BOOL Enqueue(LIST *list, UINT16 *buffer, PCHAR_DISK disk, UINT32 msgID);
void Done(LIST *list);
