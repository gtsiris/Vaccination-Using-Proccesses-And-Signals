#include "struct_Hash_Table.h"

using namespace std;

unsigned long hashTable::Hash(const string& key) {  /* Implements djb2 hashing */
	unsigned long hash = 5381;
	for (unsigned int i = 0; i < key.size(); i++)
    hash = ((hash << 5) + hash) + key[i];
  return hash;
}

hashTable::hashTable (const unsigned int& bucketCount) : bucketCount(bucketCount) {
	buckets = new list[bucketCount];
}

hashTable::~hashTable() {
	delete[] buckets;
}

void hashTable::Insert(const nodeData& data) {
	string key = data.GetKey();
	buckets[Hash(key) % bucketCount].Insert(data);
}

nodeData *hashTable::Search(const nodeData& data) {
	string key = data.GetKey();
	return buckets[Hash(key) % bucketCount].Search(data);
}
