#ifndef struct_List_Node_h
#define struct_List_Node_h

#include <iostream>
#include "struct_Node_Data.h"

struct listNode {
	private:
		nodeData *data;
		listNode *next;
	
	public:
		listNode(const nodeData& dataTemp);
		
		~listNode();
		
		const nodeData *GetData() const;
		
		listNode *GetNext() const;
		
		void SetNext(listNode *next);
		
		nodeData *Search(const nodeData& dataTemp);
};

#endif
