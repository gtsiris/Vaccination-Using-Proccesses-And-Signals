#include "class_Population_Status.h"

using namespace std;

populationStatus::populationStatus(const country& cntr) : cntr(cntr) {
}

populationStatus::populationStatus(const populationStatus& popStatus) : cntr(popStatus.cntr) {
}

std::string populationStatus::GetCountryName() const {
	return cntr.GetName();
}

void populationStatus::AddTravelRequest(const travelRequest& tRequestTemp) {
	travelRequests.Insert(tRequestTemp);  /* Insert to the list of travel requests to this country */
}

void populationStatus::TravelStatsBetween(tm& date1, tm& date2, unsigned int& totalRequests, unsigned int& acceptedRequests) const {
	const listNode *node = travelRequests.GetHead();
	while (node != NULL) {  /* Count total and accepted travel requests between date1 and date2 */
		const nodeData *dataPtr = node->GetData();
		const travelRequest *tRequestPtr = dynamic_cast<const travelRequest *>(dataPtr);
		tm requestDate = tRequestPtr->GetDate();
		if (tRequestPtr != NULL && difftime(mktime(&date1), mktime(&requestDate)) <= 0 && difftime(mktime(&requestDate), mktime(&date2)) <= 0) {
			totalRequests++;  /* If the request date was between date1 and date2, increase total requests */
			if (tRequestPtr->GetStatus() == ACCEPTED)
				acceptedRequests++;  /* If it was accepted, increase accepted requests too */
		}
		node = node->GetNext();
	}
}

vaccineStatus *populationStatus::GetVaccineStatus(const vaccineStatus& vacStatusTemp) {
	nodeData *dataPtr = vaccineStatuses.Search(vacStatusTemp);
	vaccineStatus *vacStatusPtr = dynamic_cast<vaccineStatus *>(dataPtr);
	return vacStatusPtr;  /* Return requested vaccine status */
}

vaccineStatus *populationStatus::Insert(const vaccineStatus& vacStatusTemp) {
	nodeData *dataPtr = vaccineStatuses.Search(vacStatusTemp);
	if (dataPtr != NULL)  /* If corresponding vaccine status already exists, respect it */
		return NULL;
	vaccineStatuses.Insert(vacStatusTemp);
	dataPtr = vaccineStatuses.Search(vacStatusTemp);
	vaccineStatus *vacStatusPtr = dynamic_cast<vaccineStatus *>(dataPtr);
	return vacStatusPtr;
}

void populationStatus::Remove(const vaccineStatus& vacStatusTemp) {
	listNode *previous = vaccineStatuses.GetHead();
	if (previous == NULL)
		return;
	listNode *current = previous->GetNext();
	const nodeData *dataPtr = previous->GetData();
	const vaccineStatus *vacStatusPtr = dynamic_cast<const vaccineStatus *>(dataPtr);
	if(vacStatusPtr->GetCitizenID() == vacStatusTemp.GetCitizenID()) {
		delete previous;
		vaccineStatuses.SetHead(current);
		if (current == NULL)  /* There was just one node */
			vaccineStatuses.SetTail(NULL);  /* So after removal, tail must be updated too */
		vaccineStatuses.DecreaseCount();
		return;
	}
	while (current != NULL) {
		const nodeData *dataPtr = current->GetData();
		const vaccineStatus *vacStatusPtr = dynamic_cast<const vaccineStatus *>(dataPtr);
		if(vacStatusPtr->GetCitizenID() == vacStatusTemp.GetCitizenID() && vacStatusPtr->GetDate() == NULL) {
			listNode *next = current->GetNext();
			previous->SetNext(next);  /* Bypass node */
			if (next == NULL)
				vaccineStatuses.SetTail(previous);
			delete current;
			vaccineStatuses.DecreaseCount();
			return;
		}
		previous = current;
		current = current->GetNext();
	}
}

void populationStatus::PrintPopulationStatusBetween(tm *date1, tm *date2) const {
	unsigned int vaccinatedCount = 0;
	unsigned int allCount = 0;
	const listNode *node = vaccineStatuses.GetHead();
	while (node != NULL) {
		const nodeData *dataPtr = node->GetData();
		const vaccineStatus *vacStatusPtr = dynamic_cast<const vaccineStatus *>(dataPtr);
		if (vacStatusPtr != NULL && vacStatusPtr->GetDate() != NULL) {
			tm *date = new tm(*(vacStatusPtr->GetDate()));
			if (difftime(mktime(date1), mktime(date)) <= 0 && difftime(mktime(date), mktime(date2)) <= 0)
				vaccinatedCount++;
			delete date;
		}
		allCount++;
		node = node->GetNext();
	}
	cout << vaccinatedCount << " "<< (vaccinatedCount * 100) / allCount << "%\n";
}

void populationStatus::PrintPopStatusByAgeBetween(tm *date1, tm *date2) const {
	unsigned int vaccinatedLessThan20 = 0;
	unsigned int vaccinatedFrom20To40 = 0;
	unsigned int vaccinatedFrom40To60 = 0;
	unsigned int vaccinated60Plus = 0;
	unsigned int allLessThan20 = 0;
	unsigned int allFrom20To40 = 0;
	unsigned int allFrom40To60 = 0;
	unsigned int all60Plus = 0;
	const listNode *node = vaccineStatuses.GetHead();
	while (node != NULL) {
		const nodeData *dataPtr = node->GetData();
		const vaccineStatus *vacStatusPtr = dynamic_cast<const vaccineStatus *>(dataPtr);
		unsigned int citizenAge = vacStatusPtr->GetCitizenAge();
		if (vacStatusPtr != NULL && vacStatusPtr->GetDate() != NULL) {
			tm *date = new tm(*(vacStatusPtr->GetDate()));
			if (difftime(mktime(date1), mktime(date)) <= 0 && difftime(mktime(date), mktime(date2)) <= 0) {
				if (citizenAge < 20)
					vaccinatedLessThan20++;
				else if (20 <= citizenAge && citizenAge < 40)
					vaccinatedFrom20To40++;
				else if (40 <= citizenAge && citizenAge < 60)
					vaccinatedFrom40To60++;
				else
					vaccinated60Plus++;
			}
			delete date;
		}
		if (citizenAge < 20)
			allLessThan20++;
		else if (20 <= citizenAge && citizenAge < 40)
			allFrom20To40++;
		else if (40 <= citizenAge && citizenAge < 60)
			allFrom40To60++;
		else
			all60Plus++;
		node = node->GetNext();
	}
	cout << "0-20 " << vaccinatedLessThan20 << " ";
	if (allLessThan20 != 0)
		cout << (vaccinatedLessThan20 * 100) / allLessThan20 << "%\n";
	else
		cout << "0%\n";
	cout << "20-40 " << vaccinatedFrom20To40 << " ";
	if (allFrom20To40 != 0)
		cout << (vaccinatedFrom20To40 * 100) / allFrom20To40 << "%\n";
	else
		cout << "0%\n";
	cout << "40-60 " << vaccinatedFrom40To60 << " ";
	if (allFrom40To60 != 0)
		cout << (vaccinatedFrom40To60 * 100) / allFrom40To60 << "%\n";
	else
		cout << "0%\n";
	cout << "60+ " << vaccinated60Plus << " ";
	if (all60Plus != 0)
		cout << (vaccinated60Plus * 100) / all60Plus << "%\n";
	else
		cout << "0%\n";
}
