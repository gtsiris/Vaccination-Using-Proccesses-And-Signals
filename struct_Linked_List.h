#ifndef struct_Linked_List_h
#define struct_Linked_List_h

#include <iostream>
#include <cstdlib>
#include "struct_Linked_List_Node.h"
#include "class_Vaccine_Status.h"

#define FALSE 0
#define TRUE 1

struct linkedList {
	private:
		linkedListNode *head;
		linkedListNode *tail;
		unsigned int count;
		
	public:
		linkedList();
		
		~linkedList();
		
		bool IsEmpty() const;
		
		linkedListNode *Search(const vaccineStatus& vacStatus, linkedListNode *node = NULL) const;
		
		linkedListNode *Insert(const vaccineStatus& vacStatus);
		
		linkedListNode *InsertHere(const vaccineStatus& vacStatus, linkedListNode *node, linkedListNode *down);
		
		linkedListNode *Remove(const vaccineStatus& vacStatus, linkedListNode *node = NULL);
		
		void PrintAll() const;
};

#endif
