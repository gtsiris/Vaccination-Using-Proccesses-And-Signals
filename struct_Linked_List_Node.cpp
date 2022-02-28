#include "struct_Linked_List_Node.h"

linkedListNode::linkedListNode(const vaccineStatus& vacStatus, linkedListNode *next, linkedListNode *down) : data(vacStatus),  next(next),  down(down) {
}

const vaccineStatus& linkedListNode::GetData() const {
	return data;
}

linkedListNode *linkedListNode::GetNext() const {
	return next;
}

linkedListNode *linkedListNode::GetDown() const {
	return down;
}

void linkedListNode::SetNext(linkedListNode *node) {
	next = node;
}

void linkedListNode::Print() const {
	data.Print();
}
