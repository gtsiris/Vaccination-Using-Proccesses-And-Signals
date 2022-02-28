#ifndef struct_List_h
#define struct_List_h

#include <iostream>
#include "struct_List_Node.h"
#include "struct_Node_Data.h"

struct list {
	private:
		listNode *head;
		listNode *tail;
		unsigned int nodeCount;
		
	public:
		list();
		
		~list();
		
		listNode *GetHead() const;
		
		void SetHead(listNode *node);
		
		void SetTail(listNode *node);
		
		void DecreaseCount();
		
		bool IsEmpty() const;
		
		void Insert(const nodeData& data);
		
		nodeData *Search(const nodeData& data) const;
};

#endif
