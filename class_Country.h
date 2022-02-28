#ifndef class_Country_h
#define class_Country_h

#include <iostream>
#include "struct_Node_Data.h"

class country : public nodeData {
	std::string name;
		
	public:
		country(const std::string& name);
		
		country(const country& cntr);
		
		std::string GetName() const;
		
		virtual country *Clone() const {
			return new country(*this);
		}
		
		virtual country *Search(const nodeData& data) {
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
