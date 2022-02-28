#ifndef struct_Linked_List_Node_h
#define struct_Linked_List_Node_h

#include <iostream>
#include "class_Vaccine_Status.h"

#define TRUE 1
#define FALSE 0

struct linkedListNode {
	private:
		const vaccineStatus& data;
		linkedListNode *next;
		linkedListNode *down;
		
	public:
		linkedListNode(const vaccineStatus& vacStatus, linkedListNode *next = NULL, linkedListNode *down = NULL);
		
		const vaccineStatus& GetData() const;
		
		linkedListNode *GetNext() const;
		
		linkedListNode *GetDown() const;
		
		void SetNext(linkedListNode *node);
		
		void Print() const;
};

#endif
