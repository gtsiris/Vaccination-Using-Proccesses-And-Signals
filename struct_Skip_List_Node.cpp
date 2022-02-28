#include "struct_Skip_List_Node.h"

skipListNode::skipListNode(skipListNode *previous) : previous(previous) {
}

skipListNode *skipListNode::GetPrevious() const {
	return previous;
}

bool skipListNode::IsEmpty() const {
	return (list.IsEmpty());
}

const vaccineStatus *skipListNode::Search(const vaccineStatus& vacStatus) const {
	const skipListNode *L = this;
	linkedListNode *node = NULL;
	do {
		node = L->list.Search(vacStatus, node);
		if (node != NULL && atoi(node->GetData().GetCitizenID().c_str()) == atoi(vacStatus.GetCitizenID().c_str()))  /* Found it */
			return &(node->GetData());
		L = L->previous;
	} while (L != NULL);
	return NULL;
}

linkedListNode *skipListNode::Insert(const vaccineStatus& vacStatus, int& promotion) {
	linkedListNode *down = NULL;
	if (previous != NULL)
		down = previous->Insert(vacStatus, promotion);
	if (promotion > 0) {
		linkedListNode *node = list.Insert(vacStatus);
		linkedListNode *newNode = list.InsertHere(vacStatus, node, down);
		promotion--;
		return newNode;
	}
}

void skipListNode::Remove(const vaccineStatus& vacStatus) {
	skipListNode *L = this;
	linkedListNode *node = NULL;
	do {
		node = L->list.Remove(vacStatus, node);
		L = L->previous;
	} while (L != NULL);
}

void skipListNode::PrintAll() const {
	list.PrintAll();
}
