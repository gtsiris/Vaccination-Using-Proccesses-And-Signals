#ifndef class_Vaccine_Status_h
#define class_Vaccine_Status_h

#include <iostream>
#include <ctime>
#include "struct_Node_Data.h"
#include "class_Citizen.h"

class vaccineStatus : public nodeData {
		const citizen& ctzn;
		tm *date;  /* NULL: Not vaccinated */
		
	public:
		vaccineStatus(const citizen& ctzn, const tm *const date = NULL);
		
		vaccineStatus(const vaccineStatus& vacStatus);
		
		~vaccineStatus();
		
		std::string GetCitizenID() const;
		
		unsigned int GetCitizenAge() const;
		
		const tm *GetDate() const;
		
		void PrintDate() const;
		
		void Print() const;
		
		virtual vaccineStatus *Clone() const {
			return new vaccineStatus(*this);
		}
		
		virtual vaccineStatus *Search(const nodeData& data) {
			std::string citizenID = data.GetKey();
			if(this->ctzn.GetCitizenID() == citizenID)
				return this;
			return NULL;
		}
		
		virtual std::string GetKey() const {
			return ctzn.GetCitizenID();
		}
};

#endif
