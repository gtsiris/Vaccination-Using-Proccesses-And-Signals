#include "class_Country.h"

using namespace std;

country::country(const string& name) : name(name) {
}

country::country(const country& cntr) : name(cntr.name) {
}

string country::GetName() const {
	return name;
}
