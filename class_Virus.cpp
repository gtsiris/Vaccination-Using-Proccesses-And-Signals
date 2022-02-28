#include "class_Virus.h"

using namespace std;

virus::virus(const string& name, const unsigned int& bloomSize) : name(name), bloom(bloomSize) {
}

virus::virus(const virus& vrs) : name(vrs.name), bloom(vrs.bloom.GetSize()) {
}

string virus::GetName() const {
	return name;
}

const bloomFilter& virus::GetBloom() const {
	return bloom;
}

void virus::UpdateBloom(const bloomFilter& bloom) {
	this->bloom.Update(bloom);
}

bool virus::SearchBloom(const string& citizenID) const {
	return bloom.Search(citizenID);
}

void virus::AddTravelRequest(const travelRequest& tRequestTemp, const country& cntr) {
	populationStatus popStatusTemp(cntr);
	nodeData *dataPtr = populationStatuses.Search(popStatusTemp);
	if (dataPtr == NULL) {
		populationStatuses.Insert(popStatusTemp);  /* Insert population status for cntr if doesn't already exists */
		dataPtr = populationStatuses.Search(popStatusTemp);
	}
	populationStatus *popStatusPtr = dynamic_cast<populationStatus *>(dataPtr);
	popStatusPtr->AddTravelRequest(tRequestTemp);  /* Take travel request into account */
}

void virus::PrintTravelStats(tm& date1, tm& date2, const std::string& countryTo) {
	unsigned int totalRequests = 0;
	unsigned int acceptedRequests = 0;
	if (countryTo != "NOT A COUNTRY") {  /* Country has been provided as argument */
		country countryTemp(countryTo);
		populationStatus popStatusTemp(countryTemp);
		nodeData *dataPtr = populationStatuses.Search(popStatusTemp);
		if (dataPtr != NULL) {  /* There is population status for this country */
			populationStatus *popStatusPtr = dynamic_cast<populationStatus *>(dataPtr);
			popStatusPtr->TravelStatsBetween(date1, date2, totalRequests, acceptedRequests);  /* Count total and accepted requests to this country */
			cout << "TOTAL REQUESTS " << totalRequests << "\n";
			cout << "ACCEPTED " << acceptedRequests << "\n";
			cout << "REJECTED " << (totalRequests - acceptedRequests) << "\n";
		}
		else {
			cout << "TOTAL REQUESTS 0\n";
			cout << "ACCEPTED 0\n";
			cout << "REJECTED 0\n";
		}
		return;
	}
	/* Country hasn't been provided as argument */
	const listNode *node = populationStatuses.GetHead();
	while (node != NULL) {  /* For each country that has been requested (regarding this virus) */
		const nodeData *dataPtr = node->GetData();
		const populationStatus *popStatusPtr = dynamic_cast<const populationStatus *>(dataPtr);
		if (popStatusPtr != NULL)
			popStatusPtr->TravelStatsBetween(date1, date2, totalRequests, acceptedRequests);  /* Accumulate total and accepted requests */
		node = node->GetNext();
	}
	cout << "TOTAL REQUESTS " << totalRequests << "\n";
	cout << "ACCEPTED " << acceptedRequests << "\n";
	cout << "REJECTED " << (totalRequests - acceptedRequests) << "\n";
}

const tm *virus::SearchVaccinatedPersons(const std::string& citizenID) const {
	country cntrTemp("");
	citizen ctznTemp(citizenID, "", "", cntrTemp, 0);
	vaccineStatus vacStatusTemp(ctznTemp);
	const vaccineStatus *vacStatusPtr = vaccinatedPersons.Search(vacStatusTemp);  /* Search in vaccinated skip list */
	if (vacStatusPtr != NULL)
		return vacStatusPtr->GetDate();
	return NULL;
}

bool virus::SearchNotVaccinatedPersons(const std::string& citizenID) const {
	country cntrTemp("");
	citizen ctznTemp(citizenID, "", "", cntrTemp, 0);
	vaccineStatus vacStatusTemp(ctznTemp);
	const vaccineStatus *vacStatusPtr = notVaccinatedPersons.Search(vacStatusTemp);  /* Search in not vaccinated skip list */
	return (vacStatusPtr != NULL);
}

void virus::Vaccinated(const citizen& ctzn, const tm& date) {  /* In case of contradiction with existing information, respect the older */
	populationStatus popStatusTemp(ctzn.GetCountry());
	nodeData *dataPtr = populationStatuses.Search(popStatusTemp);
	if (dataPtr == NULL) {
		populationStatuses.Insert(popStatusTemp);
		dataPtr = populationStatuses.Search(popStatusTemp);
	}
	populationStatus *popStatusPtr = dynamic_cast<populationStatus *>(dataPtr);
	vaccineStatus vacStatusTemp(ctzn, &date);
	vaccineStatus *vacStatusPtr = popStatusPtr->Insert(vacStatusTemp);  /* If there was a contradictory record, returns NULL */
	if (vacStatusPtr != NULL) {
		bloom.Insert(ctzn.GetCitizenID());
		vaccinatedPersons.Insert(*vacStatusPtr);
	}
}

void virus::NotVaccinated(const citizen& ctzn) {  /* In case of contradiction with existing information, respect the older */
	populationStatus popStatusTemp(ctzn.GetCountry());
	nodeData *dataPtr = populationStatuses.Search(popStatusTemp);
	if (dataPtr == NULL) {
		populationStatuses.Insert(popStatusTemp);
		dataPtr = populationStatuses.Search(popStatusTemp);
	}
	populationStatus *popStatusPtr = dynamic_cast<populationStatus *>(dataPtr);
	vaccineStatus vacStatusTemp(ctzn);
	vaccineStatus *vacStatusPtr = popStatusPtr->Insert(vacStatusTemp);  /* If there was a contradictory record, returns NULL */
	if (vacStatusPtr != NULL)
		notVaccinatedPersons.Insert(*vacStatusPtr);
}

void virus::InsertCitizenRecord(const citizen& ctzn, const tm *date) {  /* In case of vaccination, remove contradictory information */
	populationStatus popStatusTemp(ctzn.GetCountry());
	nodeData *dataPtr = populationStatuses.Search(popStatusTemp);
	if (dataPtr == NULL) {
		populationStatuses.Insert(popStatusTemp);
		dataPtr = populationStatuses.Search(popStatusTemp);
	}
	populationStatus *popStatusPtr = dynamic_cast<populationStatus *>(dataPtr);
	vaccineStatus vacStatusTemp(ctzn, date);
	vaccineStatus *vacStatusPtr = popStatusPtr->GetVaccineStatus(vacStatusTemp);  /* Get previous vacccine status */
	if (vacStatusPtr == NULL) {  /* NULL means there wasn't anything regarding ctzn and this virus */
		bloom.Insert(ctzn.GetCitizenID());
		vacStatusPtr = popStatusPtr->Insert(vacStatusTemp);  /* So casually insert it */
		if (date != NULL)  /* Depending on date */
			vaccinatedPersons.Insert(*vacStatusPtr);  /* Add to vaccinated skip list */
		else
			notVaccinatedPersons.Insert(*vacStatusPtr);  /* Or add it to not vaccinated */
	}
	else if (vacStatusPtr->GetDate() != NULL) {  /* If ctzn has been previously vaccinated against this virus, report it */
		cout << "ERROR: CITIZEN " << vacStatusPtr->GetCitizenID() << " ALREADY VACCINATED ON ";
		vacStatusPtr->PrintDate();
		cout << "\n";
		return;
	}
	if (vacStatusTemp.GetDate() != NULL) {  /* Finally if he has been not vaccinated so far and now he is getting vaccinated */
		bloom.Insert(ctzn.GetCitizenID());
		notVaccinatedPersons.Remove(vacStatusTemp);  /* Remove from not vaccinated skip list */
		popStatusPtr->Remove(vacStatusTemp);  /* Remove the previous vaccine status */
		vacStatusPtr = popStatusPtr->Insert(vacStatusTemp);  /* Insert the new vaccine status */
		if (vacStatusPtr != NULL)
			vaccinatedPersons.Insert(*vacStatusPtr);  /* Add it to vacinated skip list */
	}
	cout << "Done\n";
}

void virus::PrintPopulationStatus(const std::string& countryName, tm *date1, tm *date2) const {
	cout << countryName << " ";
	const listNode *node = populationStatuses.GetHead();
	while (node != NULL) {
		const nodeData *dataPtr = node->GetData();
		const populationStatus *popStatusPtr = dynamic_cast<const populationStatus *>(dataPtr);
		if (popStatusPtr != NULL && popStatusPtr->GetCountryName() == countryName) {  /* Found population status regarding given country */
			popStatusPtr->PrintPopulationStatusBetween(date1, date2);
			return;
		}
		node = node->GetNext();
	}
	cout << "0 0%\n";
}

void virus::PrintPopulationStatuses(tm *date1, tm *date2) const {
	const listNode *node = populationStatuses.GetHead();
	while (node != NULL) {
		const nodeData *dataPtr = node->GetData();
		const populationStatus *popStatusPtr = dynamic_cast<const populationStatus *>(dataPtr);
		if (popStatusPtr != NULL) {
			cout << popStatusPtr->GetCountryName() << " ";
			popStatusPtr->PrintPopulationStatusBetween(date1, date2);
		}
		node = node->GetNext();
	}
}

void virus::PrintPopStatusByAge(const std::string& countryName, tm *date1, tm *date2) const {
	cout << countryName << "\n";
	const listNode *node = populationStatuses.GetHead();
	while (node != NULL) {
		const nodeData *dataPtr = node->GetData();
		const populationStatus *popStatusPtr = dynamic_cast<const populationStatus *>(dataPtr);
		if (popStatusPtr != NULL && popStatusPtr->GetCountryName() == countryName) {  /* Found population status regarding given country */
			popStatusPtr->PrintPopStatusByAgeBetween(date1, date2);
			return;
		}
		node = node->GetNext();
	}
	cout << "0-20 0 0%\n" << "20-40 0 0%\n" << "40-60 0 0%\n" << "60+ 0 0%\n";
}

void virus::PrintPopStatusesByAge(tm *date1, tm *date2) const {
	const listNode *node = populationStatuses.GetHead();
	while (node != NULL) {
		const nodeData *dataPtr = node->GetData();
		const populationStatus *popStatusPtr = dynamic_cast<const populationStatus *>(dataPtr);
		cout << popStatusPtr->GetCountryName() << "\n";
		if (popStatusPtr != NULL) {
			popStatusPtr->PrintPopStatusByAgeBetween(date1, date2);
			cout << "\n";
		}
		node = node->GetNext();
	}
}

void virus::PrintNotVaccinatedList() const {
	notVaccinatedPersons.PrintAll();
}
