objects=class_Citizen.o class_Country.o class_Population_Status.o class_Travel_Request.o class_Vaccine_Status.o class_Virus.o struct_Bloom_Filter.o struct_Hash_Table.o struct_Linked_List.o struct_Linked_List_Node.o struct_List.o struct_List_Node.o struct_Monitor.o struct_Node_Data.o struct_Skip_List.o struct_Skip_List_Node.o
headers=class_Citizen.h class_Country.h class_Population_Status.h class_Travel_Request.h class_Vaccine_Status.h class_Virus.h struct_Bloom_Filter.h struct_Hash_Table.h struct_Linked_List.h struct_Linked_List_Node.h struct_List.h struct_List_Node.h struct_Monitor.h struct_Node_Data.h struct_Skip_List.h struct_Skip_List_Node.h


all: Monitor travelMonitor

Monitor: Monitor.o $(objects)
	g++ Monitor.o $(objects) -o Monitor

Monitor.o: Monitor.cpp $(headers)
	g++ -c Monitor.cpp

travelMonitor: travelMonitor.o $(objects)
	g++ travelMonitor.o $(objects) -o travelMonitor

travelMonitor.o: travelMonitor.cpp $(headers)
	g++ -c travelMonitor.cpp

class_Citizen.o: class_Citizen.cpp $(headers)
	g++ -c class_Citizen.cpp

class_Country.o: class_Country.cpp $(headers)
	g++ -c class_Country.cpp

class_Population_Status.o: class_Population_Status.cpp $(headers)
	g++ -c class_Population_Status.cpp

class_Travel_Request.o: class_Travel_Request.cpp $(headers)
	g++ -c class_Travel_Request.cpp

class_Vaccine_Status.o: class_Vaccine_Status.cpp $(headers)
	g++ -c class_Vaccine_Status.cpp

class_Virus.o: class_Virus.cpp $(headers)
	g++ -c class_Virus.cpp

struct_Bloom_Filter.o: struct_Bloom_Filter.cpp $(headers)
	g++ -c struct_Bloom_Filter.cpp

struct_Hash_Table.o: struct_Hash_Table.cpp $(headers)
	g++ -c struct_Hash_Table.cpp

struct_Linked_List.o: struct_Linked_List.cpp $(headers)
	g++ -c struct_Linked_List.cpp

struct_Linked_List_Node.o: struct_Linked_List_Node.cpp $(headers)
	g++ -c struct_Linked_List_Node.cpp

struct_List.o: struct_List.cpp $(headers)
	g++ -c struct_List.cpp

struct_List_Node.o: struct_List_Node.cpp $(headers)
	g++ -c struct_List_Node.cpp

struct_Monitor.o: struct_Monitor.cpp $(headers)
	g++ -c struct_Monitor.cpp

struct_Node_Data.o: struct_Node_Data.cpp $(headers)
	g++ -c struct_Node_Data.cpp

struct_Skip_List.o: struct_Skip_List.cpp $(headers)
	g++ -c struct_Skip_List.cpp

struct_Skip_List_Node.o: struct_Skip_List_Node.cpp $(headers)
	g++ -c struct_Skip_List_Node.cpp

clean:
	rm travelMonitor travelMonitor.o Monitor Monitor.o $(objects)
