#include "class_Travel_Request.h"

using namespace std;

travelRequest::travelRequest(const tm& date, const bool& status) : date({0}), status(status) {
	this->date.tm_mday = date.tm_mday;
	this->date.tm_mon = date.tm_mon;
	this->date.tm_year = date.tm_year;
}

travelRequest::travelRequest(const travelRequest& tRequest) : date({0}), status(tRequest.status) {
	date.tm_mday = tRequest.date.tm_mday;
	date.tm_mon = tRequest.date.tm_mon;
	date.tm_year = tRequest.date.tm_year;
}

tm travelRequest::GetDate() const {
	return date;
}

bool travelRequest::GetStatus() const {
	return status;
}
