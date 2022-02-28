#ifndef struct_Hash_Table_h
#define struct_Hash_Table_h

#include <iostream>
#include "struct_List.h"
#include "struct_Node_Data.h"

struct hashTable {
	private:
		list *buckets;
		const unsigned int bucketCount;
		
		unsigned long Hash(const std::string& key);
		
	public:
		hashTable(const unsigned int& bucketCount);
		
		~hashTable();
		
		void Insert(const nodeData& data);
		
		nodeData *Search(const nodeData& data);
};

#endif
