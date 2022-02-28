#ifndef class_Citizen_h
#define class_Citizen_h

#include <iostream>
#include <cstdlib>
#include "struct_Node_Data.h"
#include "class_Country.h"

class citizen :public nodeData {
	std::string citizenID;
	std::string firstName;
	std::string lastName;
	const country& cntr;
	unsigned int age;
	
	public:
		citizen(const std::string& citizenID, const std::string& firstName, const std::string& lastName, const country& cntr, const unsigned int& age);
		
		citizen(const citizen& ctzn);
		
		std::string GetCitizenID() const;
		
		std::string GetFirstName() const;
		
		std::string GetLastName() const;
		
		const country& GetCountry() const;
		
		unsigned int GetAge() const;
		
		void Print() const;
		
		virtual citizen *Clone() const {
			return new citizen(*this);
		}
		
		virtual citizen *Search(const nodeData& data) {
			citizen ctzn = dynamic_cast<const citizen&>(data);
			if(ctzn.citizenID == citizenID) {
				return this;
			}
			return NULL;
		}
		
		virtual std::string GetKey() const {
			return citizenID;
		}
};

#endif
