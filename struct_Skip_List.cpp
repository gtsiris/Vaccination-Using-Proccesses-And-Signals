#include "struct_Skip_List.h"

using namespace std;

skipList::skipList() : L0(NULL), Lmax(NULL), levels(0) {
}

skipList::~skipList() {
	skipListNode *L = Lmax;
	while (L != NULL) {
		skipListNode *temp = L;
		L = L->GetPrevious();
		delete temp;
	}
}

bool skipList::FlipCoin() {
	unsigned int n = rand() % N;
	if (n <= N * PROBABILITY)
		return HEADS;
	return TAILS;
}

bool skipList::IsEmpty() const {
	return (L0 == NULL);
}

void skipList::AddLevel() {
	if (IsEmpty()) {
		L0 = new skipListNode;
		Lmax = L0;
	}
	else {
		skipListNode *Lnew;
		Lnew = new skipListNode(Lmax);
		Lmax = Lnew;
	}
	levels++;
}

void skipList::DeleteLevel() {
	if (!IsEmpty()) {
		skipListNode *temp = Lmax->GetPrevious();
		delete Lmax;
		Lmax = temp;
		if (Lmax == NULL)
			L0 = NULL;
		levels--;
	}
}

const vaccineStatus *skipList::Search(const vaccineStatus& vacStatus) const {
	if (IsEmpty())
		return NULL;
	return Lmax->Search(vacStatus);
}

void skipList::Insert(const vaccineStatus& vacStatus) {
	int promotion;  /* Determines how many levels it will reach */
	for (promotion = 1; promotion <= MAX; promotion++)
		if (FlipCoin() == TAILS)
			break;
	while (levels < promotion)  /* If promotion needs more levels to happen */
		AddLevel();  /* Keep adding levels */
	Lmax->Insert(vacStatus, promotion);  /* Then insert it to the top and it will propagate */
}

void skipList::Remove(const vaccineStatus& vacStatus) {
	if (!IsEmpty()) {
		Lmax->Remove(vacStatus);
		while (!IsEmpty() && Lmax->IsEmpty())
			DeleteLevel();  /* If there are empty levels after removal, delete them */
	}
}

void skipList::PrintAll() const {
	if (!IsEmpty())
		L0->PrintAll();  /* The lower level has every value available */
	else
		cout << "Empty list\n";
}
