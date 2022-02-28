#include "struct_Monitor.h"

using namespace std;

monitor::monitor(const int& monitorID, const pid_t& pid) : monitorID(monitorID), pid(pid) {
}

monitor::monitor(const monitor& mntr) : monitorID(mntr.monitorID), pid(mntr.pid) {
}

int monitor::GetMonitorID() const {
	return monitorID;
}

pid_t monitor::GetPID() const {
	return pid;
}

void monitor::SetPID(const pid_t& pid) {
	this->pid = pid;
}

const list& monitor::GetCountries() const {
	return countries;
}

const country *monitor::GetCountry(const std::string& countryName) const {
	country cntrTemp(countryName);
	nodeData *dataPtr = countries.Search(cntrTemp);
	const country *cntrPtr = dynamic_cast<const country *>(dataPtr);
	return cntrPtr;
}

void monitor::AddCountry(const string& countryName) {
	if (GetCountry(countryName) == NULL) {
		country cntrTemp(countryName);
		countries.Insert(cntrTemp);
	}
}
