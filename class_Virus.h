#ifndef class_Virus_h
#define class_Virus_h

#include <iostream>
#include "struct_Node_Data.h"
#include "struct_List.h"
#include "class_Population_Status.h"
#include "class_Country.h"
#include "class_Vaccine_Status.h"
#include "class_Citizen.h"
#include "struct_Bloom_Filter.h"
#include "struct_Skip_List.h"

class virus :public nodeData {
	std::string name;
	list populationStatuses;
	bloomFilter bloom;
	skipList vaccinatedPersons;
	skipList notVaccinatedPersons;
		
	public:
		virus(const std::string& name, const unsigned int& bloomSize);
		
		virus(const virus& vrs);
		
		std::string GetName() const;
		
		const bloomFilter& GetBloom() const;
		
		void UpdateBloom(const bloomFilter& bloom);
		
		bool SearchBloom(const std::string& citizenID) const;
		
		void AddTravelRequest(const travelRequest& tRequestTemp, const country& cntr);
		
		void PrintTravelStats(tm& date1, tm& date2, const std::string& countryTo = "NOT A COUNTRY");
		
		const tm *SearchVaccinatedPersons(const std::string& citizenID) const;
		
		bool SearchNotVaccinatedPersons(const std::string& citizenID) const;
		
		void Vaccinated(const citizen& ctzn, const tm& date);
		
		void NotVaccinated(const citizen& ctzn);
		
		void InsertCitizenRecord(const citizen& ctzn, const tm *date = NULL);
		
		void PrintPopulationStatus(const std::string& countryName, tm *date1, tm *date2) const;
		
		void PrintPopulationStatuses(tm *date1, tm *date2) const;
		
		void PrintPopStatusByAge(const std::string& countryName, tm *date1, tm *date2) const;
		
		void PrintPopStatusesByAge(tm *date1, tm *date2) const;
		
		void PrintNotVaccinatedList() const;
		
		virtual virus *Clone() const {
			return new virus(*this);
		}
		
		virtual virus *Search(const nodeData& data) {
			std::string name = data.GetKey();
			if(this->name == name)
				return this;
			return NULL;
		}
		
		virtual std::string GetKey() const {
			return name;
		}
};

#endif
