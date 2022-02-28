#ifndef class_Travel_Request_h
#define class_Travel_Request_h

#include <iostream>
#include <ctime>
#include "struct_Node_Data.h"

#define REJECTED 0
#define ACCEPTED !REJECTED

class travelRequest : public nodeData {
		tm date;  /* Requested date to travel */
		bool status;  /* ACCEPTED or REJECTED */
		
	public:
		travelRequest(const tm& date, const bool& status);
		
		travelRequest(const travelRequest& tRequest);
		
		tm GetDate() const;
		
		bool GetStatus() const;
		
		virtual travelRequest *Clone() const {
			return new travelRequest(*this);
		}
		
		virtual travelRequest *Search(const nodeData& data) {
			return NULL;
		}
		
		virtual std::string GetKey() const {
			return "";
		}
};

#endif
