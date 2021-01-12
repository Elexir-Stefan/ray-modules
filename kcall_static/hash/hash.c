#include <raykernel.h>
#include <miscellaneous/hash.h>
#include <miscellaneous/math.h>
#include <memory/memory.h>
#include <string.h>

#include "hash.h"

PHASH_ENTRY HashInit(PHASH hashTable) {
	hashTable->hashTabPtr = (PHASH_ENTRY)malloc(hashTable->primeSize * sizeof(HASH_ENTRY));
	if (!hashTable->hashTabPtr) {
		return NULL;
	} else {
		memset(hashTable->hashTabPtr, 0, hashTable->primeSize * sizeof(HASH_ENTRY));
		return hashTable->hashTabPtr;
	}
}

void HashCreate(HASH *hashTable, UINT32 entries) {
	UINT8 isprime;
	UINT32 check_prime, max_check;
	UINT32 hashPrime;


	hashPrime = entries;
	/* change to odd number if it isn't already */
	if ((hashPrime % 2) == 0) hashPrime--;
	max_check = MathSquareRoot(hashPrime);
	/* check only odd numbers */
	do {
		isprime = 1;
		for (check_prime = 3; check_prime < max_check; check_prime++) {
			if ((hashPrime % check_prime) == 0) {
				isprime = 0;
				break;
			}
		}
		if (isprime) break;
	}while (hashPrime -= 2);

	hashTable->primeSize = hashPrime;
	hashTable->numEntries = 0;
}

UINT32 HashRetrieve(HASH *hashTable, UINT32 hashKey) {
	UINT32 mainHash;
	HASH_ENTRY *entries = hashTable->hashTabPtr;

	mainHash = HASHFUNCTION(hashKey, hashTable);

	/* search as long it's occupied */
	while (OCCUPIED(entries, mainHash)) {
		/* but stop if we found the item we are looking for */
		if (entries[mainHash].hashKey == hashKey) {
			return entries[mainHash].hashValue;
		}

		// go to next
		if (STEPWIDTH(entries, mainHash) == 0) {
			/* last one found, end of row */
			return 0;
		}
		mainHash += STEPWIDTH(entries, mainHash);
	}
	return 0;
}

void HashDelete(HASH *hashTable, UINT32 hashKey) {
	UINT32 currHash, mainHash, oldPos;
	HASH_ENTRY *entries = hashTable->hashTabPtr;

	mainHash = HASHFUNCTION(hashKey, hashTable);
	oldPos = mainHash;

	while (OCCUPIED(entries, mainHash)) {
		if (entries[mainHash].hashKey == hashKey) {
			// Entry found
			oldPos = mainHash;
			currHash = mainHash;

			currHash += STEPWIDTH(entries, currHash);
			while (OCCUPIED(entries, currHash)) {		// if collided when inserted
				/* copy (move) the rest one element backward */
				entries[oldPos].hashKey = entries[currHash].hashKey;
				entries[oldPos].hashValue = entries[currHash].hashValue;
				//entries[oldPos].chunkInfo = DISTANCE(oldPos, currHash);	// correct pointer - should already be correct

				oldPos = currHash;

				// set currHash to next element
				currHash += STEPWIDTH(entries, currHash);
				if (currHash == oldPos) {	/* stepwidth was 0 */
					/* last one found */
					break;
				}
			}

			// delete oldPos
			entries[oldPos].hashKey = 0;
			entries[oldPos].chunkInfo = 0;
			hashTable->numEntries--;
			return;
		}

		/**
		 * @note Assumes that element was found via HashRetrieve, otherwise this routine may end in an infinite loop, if
		 * an entry with another key (but same hashvalue) is the only one and therefore has stepwidth 0 (end-mark)
		 */
		/* get next element */
		mainHash += STEPWIDTH(entries, mainHash);

	}

	//TODO: when ported to C++ use throw instead
	return;

}

void HashInsert(HASH *hashTable, UINT32 key, UINT32 value) {
	UINT32 mainHash;
	UINT32 lastPos;
	HASH_ENTRY *entries = hashTable->hashTabPtr;

	mainHash = HASHFUNCTION(key, hashTable);

	/* while stepsize > 0 */
	while (STEPWIDTH(entries, mainHash)) {
		/* follow row */
		mainHash += STEPWIDTH(entries, mainHash);
	}
	lastPos = mainHash;

	/* linear probing (clusters but good cache performance) */
	while (OCCUPIED(entries, mainHash)) {
		mainHash = COLLISION(mainHash, hashTable);
	}

	// set stepwitdh to next entry
	entries[lastPos].chunkInfo = DISTANCE(lastPos, mainHash);

	entries[mainHash].hashKey = key;
	entries[mainHash].hashValue = value;

	entries[mainHash].chunkInfo = H_OCCUPIED;

	hashTable->numEntries++;

}
