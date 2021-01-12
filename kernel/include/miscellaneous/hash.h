
typedef struct {
	UINT32 hashKey;
	UINT32 hashValue;
	UINT8 chunkInfo;
} __attribute__((packed)) HASH_ENTRY, *PHASH_ENTRY;

typedef struct {
	HASH_ENTRY *hashTabPtr;
	UINT32 numEntries;
	UINT32 primeSize;
} HASH, *PHASH;

/**
 * initialize hash size (prime!)
 * @param hashTable to use
 * @param entries Number of hash-table entries to be "rounded" to next prime
 */
void HashCreate(PHASH hashTable, UINT32 entries);

/**
 * Allocates kernel(!) space to the created hash talbe and initializes it to zero
 * @param hashTable hash table to initialize
 */
PHASH_ENTRY HashInit(PHASH hashTable);

/**
 * inserts a value into the hash-table
 * @param hashTable to use
 * @param key to retrieve/delete later
 * @param value the assiciated value
 */
void HashInsert(PHASH hashTable, UINT32 key, UINT32 value);


/**
 * retrieves a value assiciated with a key
 * @param hashTable tu use
 * @param hashKey to find
 * @return the value found
 */
UINT32 HashRetrieve(PHASH hashTable, UINT32 hashKey);

/**
 * deletes a value in the hash-table according to it's key
 * @param hashTable to use
 * @param hashKey to delete
 */
void HashDelete(PHASH hashTable, UINT32 hashKey);

