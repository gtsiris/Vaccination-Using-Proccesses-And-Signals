#ifndef struct_Skip_List_h
#define struct_Skip_List_h

#include <iostream>
#include <cstdlib>
#include "struct_Skip_List_Node.h"
#include "class_Vaccine_Status.h"

#define MAX 10  /* Upper bound for promotions */
#define TRUE 1
#define FALSE 0
#define HEADS TRUE
#define TAILS FALSE
#define PROBABILITY 0.5
#define N 5000

struct skipList {
	private:
		skipListNode *Lmax;  /* Head */
		skipListNode *L0;  /* Tail */
		unsigned int levels;  /* Current number of levels */
		
		bool FlipCoin();
		
	public:
		skipList();
		
		~skipList();
		
		bool IsEmpty() const;
		
		void AddLevel();
		
		void DeleteLevel();
		
		const vaccineStatus *Search(const vaccineStatus& vacStatus) const;
		
		void Insert(const vaccineStatus& vacStatus);
		
		void Remove(const vaccineStatus& vacStatus);
		
		void PrintAll() const;
};

#endif
