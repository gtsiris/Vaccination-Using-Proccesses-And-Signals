#include "class_Citizen.h"

using namespace std;

citizen::citizen(const string& citizenID, const string& firstName, const string& lastName, const country& cntr, const unsigned int& age) : citizenID(citizenID), firstName(firstName), lastName(lastName), cntr(cntr), age(age) {
}

citizen::citizen(const citizen& ctzn) : citizenID(ctzn.citizenID), firstName(ctzn.firstName), lastName(ctzn.lastName), cntr(ctzn.cntr), age(ctzn.age) {
}

string citizen::GetCitizenID() const {
	return citizenID;
}

string citizen::GetFirstName() const {
	return firstName;
}

string citizen::GetLastName() const {
	return lastName;
}

const country& citizen::GetCountry() const {
	return cntr;
}

unsigned int citizen::GetAge() const {
	return age;
}

void citizen::Print() const {
	cout << citizenID << " " << firstName << " " << lastName << " " << cntr.GetName() << " " << age << "\n";
}
