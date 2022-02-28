#include "class_Vaccine_Status.h"

using namespace std;

vaccineStatus::vaccineStatus(const citizen& ctzn, const tm *const date) : ctzn(ctzn), date(NULL) {
	if (date != NULL) {
		this->date = new tm({0});
		this->date->tm_mday = date->tm_mday;
		this->date->tm_mon = date->tm_mon;
		this->date->tm_year = date->tm_year;
	}
}

vaccineStatus::vaccineStatus(const vaccineStatus& vacStatus) : ctzn(vacStatus.ctzn), date(NULL) {
	if (vacStatus.date != NULL) {
		date = new tm({0});
		date->tm_mday = vacStatus.date->tm_mday;
		date->tm_mon = vacStatus.date->tm_mon;
		date->tm_year = vacStatus.date->tm_year;
	}
}

vaccineStatus::~vaccineStatus() {
	delete date;
}

string vaccineStatus::GetCitizenID() const {
	return ctzn.GetCitizenID();
}

unsigned int vaccineStatus::GetCitizenAge() const {
	return ctzn.GetAge();
}

const tm *vaccineStatus::GetDate() const {
	return date;
}

void vaccineStatus::PrintDate() const {
	cout << date->tm_mday << "-" << 1 + date->tm_mon << "-" << 1900 + date->tm_year;
}

void vaccineStatus::Print() const {
	ctzn.Print();
}
