#include "struct_List.h"

using namespace std;

list::list() : head(NULL), tail(NULL), nodeCount(0) {
}

list::~list() {
	listNode *node = head;
	while (node != NULL) {
		listNode *temp = node;
		node = node->GetNext();
		delete temp;
	}
}

listNode *list::GetHead() const {
	return head;
}

void list::SetHead(listNode *node) {
	head = node;
}

void list::SetTail(listNode *node) {
	tail = node;
}

void list::DecreaseCount() {
	nodeCount--;
}

bool list::IsEmpty() const {
	return (head == NULL);
}

void list::Insert(const nodeData& data) {
	listNode *node = new listNode(data);
	if (IsEmpty()) {
		head = node;
		tail = head;
	}
	else {
		tail->SetNext(node);
		tail = node;
	}
	nodeCount++;
}

nodeData *list::Search(const nodeData& data) const {
	listNode *node = head;
	while (node != NULL) {
		nodeData* result = node->Search(data);
		if (result != NULL)
			return result;
		node = node->GetNext();
	}
	return NULL;
}
