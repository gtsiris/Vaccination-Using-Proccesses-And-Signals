#include "struct_List_Node.h"

using namespace std;

listNode::listNode(const nodeData& dataTemp) : next(NULL) {
	data = dataTemp.Clone();
}

listNode::~listNode() {
	delete data;
}

const nodeData *listNode::GetData() const {
	return data;
}

listNode *listNode::GetNext() const {
	return next;
}

void listNode::SetNext(listNode *next) {
	this->next = next;
}

nodeData *listNode::Search(const nodeData& dataTemp) {
	if (data != NULL)
		return data->Search(dataTemp);
	return NULL;
}
