#ifndef struct_Monitor_h
#define struct_Monitor_h

#include <iostream>
#include <cstdlib>
#include <sys/types.h>
#include "struct_Node_Data.h"
#include "struct_List.h"
#include "struct_List_Node.h"
#include "class_Country.h"

#define NOT_AVAILABLE -1

struct monitor : public nodeData {
	private:
		int monitorID;
		pid_t pid;
		list countries;
	
	public:
		monitor(const int& monitorID = NOT_AVAILABLE, const pid_t& pid = NOT_AVAILABLE);
		
		monitor(const monitor& mntr);
		
		int GetMonitorID() const;
		
		pid_t GetPID() const;
		
		void SetPID(const pid_t& pid);
		
		const list& GetCountries() const;
		
		const country *GetCountry(const std::string& countryName) const;
		
		void AddCountry(const std::string& countryName);
		
		virtual monitor *Clone() const {
			return new monitor(*this);
		}
		
		virtual monitor *Search(const nodeData& data) {
			int monitorID = atoi(data.GetKey().c_str());
			if (monitorID == NOT_AVAILABLE) {
				monitor mntr = dynamic_cast<const monitor&>(data);
				if (mntr.pid == pid)
					return this;
				return NULL;
			}
			if(this->monitorID == monitorID)
				return this;
			return NULL;
		}
		
		virtual std::string GetKey() const {
			return std::to_string(monitorID);
		}
};

#endif
