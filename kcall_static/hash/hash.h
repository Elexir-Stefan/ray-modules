#define H_OCCUPIED 0x00000001

#define HASHFUNCTION(key, table) (key % table->primeSize)
#define COLLISION(var, table) ((var + 1) % table->primeSize)
#define STEPWIDTH(table, index) (table[index].chunkInfo >> 1)
#define DISTANCE(from, to) (((to - from) << 1) | H_OCCUPIED)
#define OCCUPIED(table, index) (table[index].chunkInfo & H_OCCUPIED)
#define OCCUPIED_E(entry) (entry->chunkInfo & H_OCCUPIED)
