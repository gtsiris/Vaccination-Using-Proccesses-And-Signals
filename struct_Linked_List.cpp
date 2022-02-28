#include "struct_Linked_List.h"

linkedList::linkedList() : head(NULL), tail(NULL), count(0) {
}

linkedList::~linkedList() {
	linkedListNode *node = head;
	while (node != NULL) {
		linkedListNode *temp = node;
		node = node->GetNext();
		delete temp;
	}
}

bool linkedList::IsEmpty() const {
	return (head == NULL);
}

linkedListNode *linkedList::Search(const vaccineStatus& vacStatus, linkedListNode *node) const {
	if (node == NULL)
		node = head;
	if (IsEmpty() || atoi(vacStatus.GetCitizenID().c_str()) < atoi(node->GetData().GetCitizenID().c_str()))
		return NULL;   /* It isn't in this level */
	if (atoi(node->GetData().GetCitizenID().c_str()) == atoi(vacStatus.GetCitizenID().c_str()))
		return node;  /* It's the head */
	linkedListNode *previous = node;
	linkedListNode *current = node->GetNext();
	while (current != NULL) {
		if (atoi(current->GetData().GetCitizenID().c_str()) < atoi(vacStatus.GetCitizenID().c_str())) { /* Keep going, it might appear */
			previous = current;
			current = current->GetNext();
		}
		else if (atoi(vacStatus.GetCitizenID().c_str()) < atoi(current->GetData().GetCitizenID().c_str()))  /* Surpassed its value, it isn't going to appear */
			return previous->GetDown();  /* Go down and keep searching */
		else
			return current;  /* Found it */
	}
	return previous->GetDown();  /* No more nodes in this level, the lower one may have some more */
}

linkedListNode *linkedList::Insert(const vaccineStatus& vacStatus) {  /* Searches for the right spot to insert in each level */
	if (IsEmpty() || atoi(vacStatus.GetCitizenID().c_str()) < atoi(head->GetData().GetCitizenID().c_str()))
		return NULL;
	linkedListNode *previous = head;
	linkedListNode *current = head->GetNext();
	while (current != NULL) {
		if (atoi(current->GetData().GetCitizenID().c_str()) < atoi(vacStatus.GetCitizenID().c_str())) {
			previous = current;
			current = current->GetNext();
		}	
		else if (atoi(vacStatus.GetCitizenID().c_str()) < atoi(current->GetData().GetCitizenID().c_str()))
			return previous;  /* Returns the previous, so caller can inject the node */
		else
			return NULL;
	}
	return previous;
}

linkedListNode *linkedList::InsertHere(const vaccineStatus& vacStatus, linkedListNode *node, linkedListNode *down) {  /* Actual insertion (given the spot) */
	count++;
	if (node != NULL) {
		linkedListNode *newNode = new linkedListNode(vacStatus, node->GetNext(), down);  /* Inject node */
		node->SetNext(newNode);
		if (newNode->GetNext() == NULL)
			tail = newNode;
		return newNode;
	}
	else {
		head = new linkedListNode(vacStatus, head, down);
		if (count == 1)
			tail = head;
		return head;
	}
}

linkedListNode *linkedList::Remove(const vaccineStatus& vacStatus, linkedListNode *node) {
	if (IsEmpty())
		return NULL;
	if (node == NULL) {
		node = head;
		if (atoi(head->GetData().GetCitizenID().c_str()) == atoi(vacStatus.GetCitizenID().c_str())) {  /* Head is the one to be removed */
			head = node->GetNext();
			delete node;
			count--;
			if (count == 0)
				tail = NULL;
			return NULL;
		}
	}
	if (atoi(vacStatus.GetCitizenID().c_str()) < atoi(node->GetData().GetCitizenID().c_str()))  /* Less than head, so it isn't going to appear */
		return NULL;
	else if (atoi(node->GetData().GetCitizenID().c_str()) == atoi(vacStatus.GetCitizenID().c_str()))
		return Remove(vacStatus);
	linkedListNode *previous = node;
	linkedListNode *current = node->GetNext();
	while (current != NULL) {
		if (atoi(current->GetData().GetCitizenID().c_str()) < atoi(vacStatus.GetCitizenID().c_str())) {  /* Keep going, it might appear */
			previous = current;
			current = current->GetNext();
		}	
		else if (atoi(vacStatus.GetCitizenID().c_str()) < atoi(current->GetData().GetCitizenID().c_str()))  /* Surpassed its value, go to lower level */
			return previous->GetDown();
		else {  /* Found it */
			previous->SetNext(current->GetNext());
			delete current;
			count--;
			if (count == 0)
				tail = NULL;
			return previous->GetDown();
		}
	}
	return previous->GetDown();
}

void linkedList::PrintAll() const {
	linkedListNode *node = head;
	while (node != NULL) {
		node->Print();
		node = node->GetNext();
	}
}
