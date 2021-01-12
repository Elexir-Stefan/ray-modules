#ifndef _VFS_TYPES_H
#define _VFS_TYPES_H

#define VFS_ATTR_DIRECTORY	1
#define VFS_ATTR_READABLE	2
#define VFS_ATTR_WRITABLE	4
#define VFS_ATTR_HIDDEN		8
#define VFS_ATTR_EXECUTABLE	16
#define VFS_ATTR_NOMETA		32
#define VFS_ATTR_DOESNT_APPLY	0x80000000

#define VFS_DONT_USE_ATTRIBUTES(attr) attr.is = VFS_ATTR_DOESNT_APPLY
#define VFS_NO_OWNER(own) own.is = 0

/** @addtogroup COMMON  */
/** @{ */
typedef struct {
	UINT32 is;
} VFS_ATTRIBUTES;

typedef struct {
	UINT32 is;
} OWNER;

/** return values */
typedef enum {
	VFS_SUCCESS		= 0,
	VFS_ERR_OCCUPIED	= 1,
	VFS_ERR_REGISTER	= 2,
	VFS_ERR_ACCESS_DENIED	= 3,
	VFS_ERR_INSUFF_RIGHTS	= 4,
	VFS_ERR_READ		= 5,
	VFS_ERR_WRITE		= 6,
	VFS_ERR_INVALID_HANDLE	= 7,
	VFS_ERR_FILE_NOT_FOUND	= 8,
	VFS_ERR_NOT_A_FILE	= 9,
	VFS_ERR_NOT_A_DIR	= 10,
	VFS_ERR_INVALID_PATH	= 11,
	VFS_ERR_INVALID_MOUNT	= 12,
	VFS_ERR_COMMUNICATION	= 0xfc,
	VFS_ERR_TIMEOUT		= 0xfd,
	VFS_ERR_MEMORY		= 0xfe,
	VFS_ERR_GENERAL		= 0xff
} VFS_STATUS;

/** @} */

/** @addtogroup USER */
/** @{  */

/** Access methods for files */
typedef enum {
	VFS_ACC_READ		= 1,	/**< Used to open a stream that can be read from */
	VFS_ACC_WRITE		= 2,	/**< Used to open a stream that can be written to */
	VFS_ACC_CREATE		= 3,	/**< Used to create a stream and read from */
	VFS_ACC_RAS		= 4,	/**< Used for (R)andom (A)ccess (S)streams - Reading and writing is valid */
	VFS_ACC_CLOSE		= 5,	/**< Stream is no longer valid */
	VFS_ACC_DEL		= 6,	/**< Delete a file */
	VFS_ACC_LIST		= 7
} VFS_ACCESS;


/** Access methods for two files */
typedef enum {
	VFS_PACC_MOVE		= 20,	/**< Move file1 to file2 (normally not the same as copy and delete!) */
	VFS_PACC_COPY		= 21,	/**< Copy file1 to file2 */
	VFS_PACC_HLINK		= 22,	/**< Link file2 to file1 on the same partition - Link will also work of fs is mounted on a different path - filesystem must support linking */
	VFS_PACC_SLINK		= 23	/**< Link file2 statically to string 'file1' whatever might or might not be mounted as file1 */
} VFS_PAIRACCESS;

/** Handle to a file - 0 is invalid */
typedef struct {
	UINT32 handleUID;
} VFS_HANDLE;

/** Information structure of a file or directory */
typedef struct _VFS_INFO {
	TUString _name;			/**< Name of the current file */
	VFS_ATTRIBUTES aOwner;		/**< Rights that the owner of the file has */
	VFS_ATTRIBUTES aGroup;		/**< Rights that members of the owner's group have */
	VFS_ATTRIBUTES aOthers;		/**< Rights that all others (not owner, no members of owner's group) have */
	OWNER owner;			/**< The owner of the file */
	UINT64 fileSize;		/**< Size of the file */
	VFS_STATUS errCode;		/**< Error code */
} *VFS_INFO;

typedef struct {
	TUString _password;
} VFS_TICKET;

typedef struct {
	VFS_STATUS error;
	VFS_INFO nodeInfo;
	TUString _ident;
	RMISERIAL registeredDriver;
} VFS_NODE_INFO;

/** Streams to exchange data */
typedef struct _VFS_BUFFER{
	VFS_STATUS errCode;
	UINT32 bufferLength;
	void* buffer;
} __attribute__((packed)) *VFS_BUFFER;

typedef struct _REGNODE{
	TUString _registerPath;
	TUString _newNodeName;
	TUString _addInfoName;
	UINT32 mountID;
	VFS_STATUS errCode;
} *REGNODE;

typedef struct _NEWDIR{
	TUString _path;
	TUString _nodeName;
	TUString _addInfos;
} *NEWDIR;

typedef struct _ACCOBJ{
	TUString _objPath;
	UINT32 mountID;
	VFS_ACCESS mode;
	VFS_HANDLE handle;
	VFS_STATUS errCode;
	UINT32 extendedErrorInfo;
} *ACCOBJ;

typedef struct _FILESTREAM{
	UINT64 position;
	UINT64 length;
	VFS_HANDLE handle;
} *FILESTREAM;

/** @} */

#endif
