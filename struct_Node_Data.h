#ifndef struct_Node_Data_h
#define struct_Node_Data_h

#include <iostream>

struct nodeData {  /* Interface of a node's data */
		nodeData();
		
		virtual ~nodeData();
		
		virtual nodeData *Clone() const = 0;
		
		virtual nodeData *Search(const nodeData& data) = 0;
		
		virtual std::string GetKey() const = 0;
};

#endif
