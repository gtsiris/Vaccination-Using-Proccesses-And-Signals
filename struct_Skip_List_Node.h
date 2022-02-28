#ifndef struct_Skip_List_Node_h
#define struct_Skip_List_Node_h

#include <iostream>
#include <cstdlib>
#include "struct_Linked_List.h"
#include "struct_Linked_List_Node.h"
#include "class_Vaccine_Status.h"

#define TRUE 1
#define FALSE 0

struct skipListNode {
	private:
		linkedList list;
		skipListNode *previous;
		
	public:
		skipListNode(skipListNode *previous = NULL);
		
		skipListNode *GetPrevious() const;
		
		bool IsEmpty() const;
		
		const vaccineStatus *Search(const vaccineStatus& vacStatus) const;
		
		linkedListNode *Insert(const vaccineStatus& vacStatus, int& promotion);
		
		void Remove(const vaccineStatus& vacStatus);
		
		void PrintAll() const;
};

#endif
