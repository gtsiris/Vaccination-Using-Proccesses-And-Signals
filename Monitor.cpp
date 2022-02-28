#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#include <signal.h>
#include "class_Citizen.h"
#include "class_Country.h"
#include "class_Virus.h"
#include "struct_List.h"
#include "struct_Hash_Table.h"

#define END_OF_MESSAGE "#"  /* Let receiver know that message has been completed */
#define ACK "@"  /* Follow up each message with acknowledgement */
#define STOP "$"  /* Doesn't require ACK. Signifies change of direction in communication */
#define MINIMUM_BUFFER_SIZE 1  /* Atleast 1 byte is required */
#define OK 0
#define ERROR !OK
#define NOT_AVAILABLE -1
#define DEFAULT_BUCKET_COUNT 1000  /* Rule of thumb: logN, where N is the expected number of records */

using namespace std;

void SetSignalHandlers();

bool Configure(const int& argc, char** argv, string& fifoRead, string& fifoWrite);

void PrintProperExec();

bool InitializeMonitor(const string& fifoRead, const string& fifoWrite, unsigned int& bufferSize, unsigned int& sizeOfBloom, string& inputDir, hashTable& citizens, list& countries, list& viruses, list& oldCountryFiles);

bool SendBloomFilters(const string& fifoRead, const string& fifoWrite, const unsigned int& bufferSize, const unsigned int& sizeOfBloom, list& viruses);

bool SendMessageReceiveACK(const string& fifoRead, const string& fifoWrite, const string& message, const unsigned int& bufferSize, const int& messageSize = NOT_AVAILABLE);

bool ReceiveMessageSendACK(const string& fifoRead, const string& fifoWrite, string& message, const unsigned int& bufferSize);

bool ReadFIFO(const string& fifoRead, string& message, const unsigned int& bufferSize);

bool WriteFIFO(const string& fifoWrite, const string& message, const unsigned int& bufferSize);

int GetSizeFIFO(const string& fifo);

string BitArrayToString(const char *bitArray, const unsigned int& size);

bool IsRegularFile(const string& path);

bool InputFromDirectory(const string& path, const unsigned int& sizeOfBloom, hashTable& citizens, list& countries, list& viruses, list& oldCountryFiles);

bool InputFromFile(const string& path, hashTable& citizens, list& countries, list& viruses, const unsigned int& bloomSize);

bool InputFromLine(string line, hashTable& citizens, list& countries, list& viruses, const unsigned int& bloomSize);

country *RegisterCountry(const string& countryName, list& countries);

citizen *RegisterCitizen(const string& citizenID, const string& firstName, const string& lastName, const country& cntr, const unsigned int& age, hashTable& citizens);

virus *RegisterVirus(const string& virusName, const unsigned int& bloomSize, list& viruses);

string CreateAttribute(string& line, const char& delimiter = ' ');

tm CreateDate(string& line);

string DateToString(const tm& date);

void PrintDate(const tm& date);

void ReceiveCommands(const string& fifoRead, const string& fifoWrite, const unsigned int& bufferSize, const unsigned int& sizeOfBloom, const string& inputDir, hashTable& citizens, list& countries, list& viruses, list& oldCountryFiles);

void PrintProperUse();

unsigned int CountArguments(const string& command);

void TravelRequest(const string& fifoRead, const string& fifoWrite, const unsigned int& bufferSize, hashTable& citizens, list& viruses, unsigned int& totalRequests, unsigned int& acceptedRequests, const string& citizenID, const string& dateStr, const string& countryFrom, const string& virusName);

void AddVaccinationRecords(const string& fifoRead, const string& fifoWrite, const unsigned int& bufferSize, const unsigned int& sizeOfBloom, const string& inputDir, hashTable& citizens, list& countries, list& viruses, list& oldCountryFiles);

void SearchVaccinationStatus(const string& fifoRead, const string& fifoWrite, const unsigned int& bufferSize, hashTable& citizens, list& viruses, const string& citizenID);

void CreateLogFile(list& countries, const unsigned int& totalRequests, const unsigned int& acceptedRequests);

bool newVaccinationRecords = 0;
bool interupt = 0;

void CatchNewRecords(int signo) {
	newVaccinationRecords = 1;
}

void CatchInterupt(int signo) {
	interupt = 1;
}

int main (int argc, char** argv) {
	SetSignalHandlers();
	
	string fifoRead;
	string fifoWrite;
	if (Configure(argc, argv, fifoRead, fifoWrite) == ERROR) {
		PrintProperExec();
		return ERROR;
	}
	unsigned int bufferSize = MINIMUM_BUFFER_SIZE;
	unsigned int sizeOfBloom;
	string inputDir;
	hashTable citizens(DEFAULT_BUCKET_COUNT);
	list countries;
	list viruses;
	list oldCountryFiles;  /* List of the already processed country files */
	if (InitializeMonitor(fifoRead, fifoWrite, bufferSize, sizeOfBloom, inputDir, citizens, countries, viruses, oldCountryFiles) == ERROR) {
		cout << "Monitor failed to initialize\n";
		return ERROR;
	}
	if (SendBloomFilters(fifoRead, fifoWrite, bufferSize, sizeOfBloom, viruses) == ERROR) {
		cout << "Monitor failed to send bloom filters\n";
		return ERROR;
	}
	
	ReceiveCommands(fifoRead, fifoWrite, bufferSize, sizeOfBloom, inputDir, citizens, countries, viruses, oldCountryFiles);
	
	return OK;
}

void SetSignalHandlers() {
	static struct sigaction act;
	act.sa_handler = CatchNewRecords;
	act.sa_flags = SA_RESTART;
	sigfillset(&(act.sa_mask));
	sigaction(SIGUSR1, &act, NULL);  /* Set the handler for SIGUSR1 */
	
	static struct sigaction act2;
	act2.sa_handler = CatchInterupt;
	act2.sa_flags = SA_RESTART;
	sigfillset(&(act2.sa_mask));
	sigaction(SIGINT, &act2, NULL);  /* Set the handler for SIGINT */
	sigaction(SIGQUIT, &act2, NULL);  /* The handler for SIGQUIT is the same */
}

bool Configure(const int& argc, char** argv, string& fifoRead, string& fifoWrite) {
	if (argc == 3) {
		fifoRead = string(argv[1]);
		fifoWrite = string(argv[2]);
		return OK;
	}
	return ERROR;
}

void PrintProperExec() {
	cout << "To exec monitor please try: ./Monitor fifoRead fifoWrite\n";
}

bool InitializeMonitor(const string& fifoRead, const string& fifoWrite, unsigned int& bufferSize, unsigned int& sizeOfBloom, string& inputDir, hashTable& citizens, list& countries, list& viruses, list& oldCountryFiles) {
	string message;
	
	if (ReceiveMessageSendACK(fifoRead, fifoWrite, message, bufferSize) == ERROR)  /* Receive bufferSize */
		return ERROR;
	bufferSize = atoi(message.c_str());
	
	if (ReceiveMessageSendACK(fifoRead, fifoWrite, message, bufferSize) == ERROR)  /* Receive sizeOfBloom */
		return ERROR;
	sizeOfBloom = atoi(message.c_str());
	
	if (ReceiveMessageSendACK(fifoRead, fifoWrite, message, bufferSize) == ERROR)  /* Receive inputDir */
		return ERROR;
	inputDir = message;
	
	while (1) {
		if (ReceiveMessageSendACK(fifoRead, fifoWrite, message, bufferSize) == ERROR)  /* Receive country */
			return ERROR;
		if (message == STOP) {
			break;
		}
		string countryName = message;
		country *cntrPtr = RegisterCountry(countryName, countries);
		string path = "./" + inputDir + "/" + countryName;
		if (InputFromDirectory(path, sizeOfBloom, citizens, countries, viruses, oldCountryFiles) == ERROR)  /* Input from country's directory */
			return ERROR;
	}
	
	return OK;
}

bool SendBloomFilters(const string& fifoRead, const string& fifoWrite, const unsigned int& bufferSize, const unsigned int& sizeOfBloom, list& viruses) {
	string message;
	const listNode *node = viruses.GetHead();
	while (node != NULL) {
		const nodeData *dataPtr = node->GetData();
		const virus *vrsPtr = dynamic_cast<const virus *>(dataPtr);
		string virusName = vrsPtr->GetName();
		message = virusName;
		if (SendMessageReceiveACK(fifoRead, fifoWrite, message, bufferSize) == ERROR)  /* Send virusName */
			return ERROR;
		
		string bitArray = BitArrayToString(vrsPtr->GetBloom().GetBitArray(), sizeOfBloom);
		message = bitArray;
		if (SendMessageReceiveACK(fifoRead, fifoWrite, message, bufferSize, sizeOfBloom) == ERROR)  /* Send bitArray of virus' bloom */
			return ERROR;
		
		node = node->GetNext();
	}
	message = STOP;
	if (SendMessageReceiveACK(fifoRead, fifoWrite, message, bufferSize) == ERROR)
		return ERROR;
	return OK;
}

bool SendMessageReceiveACK(const string& fifoRead, const string& fifoWrite, const string& message, const unsigned int& bufferSize, const int& messageSize) {
	int fifoSize = GetSizeFIFO(fifoWrite);
	if (fifoSize < 0) {
		cout << "Unable to specify FIFO's size\n";
		return ERROR;
	}
	
	string messageRemain = message;
	int remainSize = messageSize;
	string response;
	while (remainSize > fifoSize) {  /* If necessary, break down message into parts that fit in fifo */
		string messagePart = messageRemain.substr(0, fifoSize);
		if (WriteFIFO(fifoWrite, messagePart, bufferSize) == ERROR) {  /* Write messagePart */
			cout << "Cannot write to " << fifoWrite << "\n";
			return ERROR;
		}
		if (ReadFIFO(fifoRead, response, bufferSize) == ERROR) {  /* Read response */
			cout << "Cannot read from " << fifoRead << "\n";
			return ERROR;
		}
		if (response != ACK) {  /* Response must be an acknowledgement */
			cout << "Didn't receive acknowledgement\n";
			return ERROR;
		}
		messageRemain = messageRemain.substr(fifoSize);
		remainSize -= fifoSize;  /* The size of the remaining message */
	}
	
	string messagePart = messageRemain;  /* This is the final messagePart */
	if (WriteFIFO(fifoWrite, messagePart, bufferSize) == ERROR) {
		cout << "Cannot write to " << fifoWrite << "\n";
		return ERROR;
	}
	if (messagePart == STOP)
		return OK;
	if (ReadFIFO(fifoRead, response, bufferSize) == ERROR) {
		cout << "Cannot read from " << fifoRead << "\n";
		return ERROR;
	}
	if (response != ACK) {
		cout << "Didn't receive acknowledgment\n";
		return ERROR;
	}
	
	messagePart = END_OF_MESSAGE;  /* End of message follows the actual content */
	if (WriteFIFO(fifoWrite, messagePart, bufferSize) == ERROR) {
		cout << "Cannot write to " << fifoWrite << "\n";
		return ERROR;
	}
	if (ReadFIFO(fifoRead, response, bufferSize) == ERROR) {
		cout << "Cannot read from " << fifoRead << "\n";
		return ERROR;
	}
	if (response != ACK) {
		cout << "Didn't receive acknowledgment\n";
		return ERROR;
	}
	return OK;
}

bool ReceiveMessageSendACK(const string& fifoRead, const string& fifoWrite, string& message, const unsigned int& bufferSize) {
	string messageAccumulator = "";
	string response = ACK;
	while (message != END_OF_MESSAGE) {  /* Accumulate all parts to compose the overall message */
		if (ReadFIFO(fifoRead, message, bufferSize) == ERROR) {  /* Read message part */
			cout << "Cannot read from " << fifoRead << "\n";
			return ERROR;
		}
		if (message == STOP)  /* In case of STOP, there's nothing to do */
			return OK;
		if (message != END_OF_MESSAGE)  /* End of message should not be included in the overall message */
			messageAccumulator += message;  /* Append message part */
		if (WriteFIFO(fifoWrite, response, bufferSize) == ERROR) {  /* Write ACK as a response to each part */
			cout << "Cannot write to " << fifoWrite << "\n";
			return ERROR;
		}
	}
	message = messageAccumulator;
	return OK;
}

bool ReadFIFO(const string& fifoRead, string& message, const unsigned int& bufferSize) {
	message = "";
	char *buffer = new char[bufferSize];
	
	int readfd = open(fifoRead.c_str(), O_RDONLY);  /* Rendez-vous with the write end */
	if (readfd < 0) {
		perror("open");
		delete[] buffer;
		return ERROR;
	}
	int bytes;
	while (bytes = read(readfd, buffer, bufferSize)) {  /* Read at most bufferSize bytes */
		if (bytes <= 0) {
			perror("read\n");
			close(readfd);
			delete[] buffer;
			return ERROR;
		}
		message += string(buffer, bytes);  /* Append buffer to message */
	}
	close(readfd);
	delete[] buffer;
	return OK;
}

bool WriteFIFO(const string& fifoWrite, const string& message, const unsigned int& bufferSize) {
	char *buffer = new char[bufferSize];
	int writefd = open(fifoWrite.c_str(), O_WRONLY);  /* Rendez-vous with the read end */
	if (writefd < 0) {
		perror("open");
		delete[] buffer;
		return ERROR;
	}
	for (unsigned int send = 0; send < message.length();) {  /* Send message through buffer */
		unsigned int size = bufferSize;
		if (send + bufferSize > message.length())
			size = message.length() - send;
		memcpy(buffer, message.substr(send, size).c_str(), size);
		if (write(writefd, buffer, size) != size) {  /* Write size bytes at each time */
			perror("write\n");
			close(writefd);
			delete[] buffer;
			return ERROR;
		}
		send += size;  /* Sent bytes so far */
	}
	
	close(writefd);
	delete[] buffer;
	return OK;
}

int GetSizeFIFO(const string& fifo) {
	int fd = open(fifo.c_str(), O_RDONLY|O_NONBLOCK);
	if (fd < 0) {
		perror("open");
		return fd;
	}
	int fifoSize = fcntl(fd, F_GETPIPE_SZ);  /* Get size of fifo in bytes */
	close(fd);
	return fifoSize;
}

string BitArrayToString(const char *bitArray, const unsigned int& size) {
	string bitString = "";
	for (unsigned int i = 0; i < size; i++)
		bitString += bitArray[i];
	return bitString;
}

bool IsRegularFile(const string& path) {
	struct stat statBuf;
	if (stat(path.c_str(), &statBuf))
		return 0;
	return S_ISREG(statBuf.st_mode);
}

bool InputFromDirectory(const string& path, const unsigned int& sizeOfBloom, hashTable& citizens, list& countries, list& viruses, list& oldCountryFiles) {
	DIR *dir;
  struct dirent *dirEntry;
	dir = opendir(path.c_str());
	if (dir) {
		while (dirEntry = readdir(dir)) {
			string entryName = dirEntry->d_name;
			string newPath = path + "/" + entryName;
			if (IsRegularFile(newPath)) {
				country countryFileTemp(entryName);
				if (oldCountryFiles.Search(countryFileTemp) == NULL) {  /* If this country file isn't already processed, use it as input */
					if (InputFromFile(newPath, citizens, countries, viruses, sizeOfBloom) == ERROR) {
						closedir(dir);
						return ERROR;
					}
					oldCountryFiles.Insert(countryFileTemp);  /* Add it to processed country files */
				}
			}
		}
		closedir(dir);
		return OK;
	}
	return ERROR;
}

bool InputFromFile(const string& path, hashTable& citizens, list& countries, list& viruses, const unsigned int& bloomSize) {
	ifstream file(path.c_str());
	if (file.is_open()) {
		string line;
		while(getline(file, line)) {
			if (InputFromLine(line, citizens, countries, viruses, bloomSize) == ERROR)
				cout << "ERROR IN RECORD " << line << "\n";
		}
		file.close();
		return OK;
	}
	return ERROR;
}

bool InputFromLine(string line, hashTable& citizens, list& countries, list& viruses, const unsigned int& bloomSize) {
	string citizenID = CreateAttribute(line);
	string firstName = CreateAttribute(line);
	string lastName = CreateAttribute(line);
	string countryName = CreateAttribute(line);
	unsigned int age = atoi(CreateAttribute(line).c_str());
	string virusName = CreateAttribute(line);
	string vaccinated = CreateAttribute(line);
	
	country *cntrPtr = RegisterCountry(countryName, countries);
	citizen *ctznPtr = RegisterCitizen(citizenID, firstName, lastName, *cntrPtr, age, citizens);
	virus *vrsPtr = RegisterVirus(virusName, bloomSize, viruses);
	
	if (vaccinated == "YES") {
		tm date = CreateDate(line);
		vrsPtr->Vaccinated(*ctznPtr, date);
	}
	else if (vaccinated == "NO" || vaccinated == "NO\r") {  /* Works for both LF and CRLF file formats */
		if (line != "")
			return ERROR;
		vrsPtr->NotVaccinated(*ctznPtr);
	}
	else
		return ERROR;
	return OK;
}

country *RegisterCountry(const string& countryName, list& countries) {
	country cntrTemp(countryName);
	nodeData *dataPtr = countries.Search(cntrTemp);
	if (dataPtr == NULL) {
		countries.Insert(cntrTemp);
		dataPtr = countries.Search(cntrTemp);
	}
	country *cntrPtr = dynamic_cast<country *>(dataPtr);
	return cntrPtr;
}

citizen *RegisterCitizen(const string& citizenID, const string& firstName, const string& lastName, const country& cntr, const unsigned int& age, hashTable& citizens) {
	citizen ctznTemp(citizenID, firstName, lastName, cntr, age);
	nodeData *dataPtr = citizens.Search(ctznTemp);
	if (dataPtr == NULL) {
		citizens.Insert(ctznTemp);
		dataPtr = citizens.Search(ctznTemp);
	}
	citizen *ctznPtr = dynamic_cast<citizen *>(dataPtr);
	return ctznPtr;
}

virus *RegisterVirus(const string& virusName, const unsigned int& bloomSize, list& viruses) {
	virus vrsTemp(virusName, bloomSize);
	nodeData *dataPtr = viruses.Search(vrsTemp);
	if (dataPtr == NULL) {
		viruses.Insert(vrsTemp);
		dataPtr = viruses.Search(vrsTemp);
	}
	virus *vrsPtr = dynamic_cast<virus *>(dataPtr);
	return vrsPtr;
}

string CreateAttribute(string& line, const char& delimiter) {  /* Extract substring before delimiter */
	unsigned int attrLength = line.find_first_of(delimiter);
	string attribute = line.substr(0, attrLength);
	if (line.length() > attrLength)
		line = line.substr(attrLength + 1);
	else
		line = "";
	return attribute;
}

tm CreateDate(string& line) {
	tm date = {0};
	date.tm_mday = atoi(CreateAttribute(line, '-').c_str());
	date.tm_mon = atoi(CreateAttribute(line, '-').c_str()) - 1;
	date.tm_year = atoi(CreateAttribute(line, '-').c_str()) - 1900;
	return date;
}

string DateToString(const tm& date) {
	string dateStr = to_string(date.tm_mday) + "-" + to_string(1 + date.tm_mon) + "-" + to_string(1900 + date.tm_year);
	return dateStr;
}

void PrintDate(const tm& date) {
	cout << date.tm_mday << "-" << 1 + date.tm_mon << "-" << 1900 + date.tm_year;
}

void ReceiveCommands(const string& fifoRead, const string& fifoWrite, const unsigned int& bufferSize, const unsigned int& sizeOfBloom, const string& inputDir, hashTable& citizens, list& countries, list& viruses, list& oldCountryFiles) {
	unsigned int totalRequests = 0;
	unsigned int acceptedRequests = 0;
	string command;
	while (1) {
		if (ReceiveMessageSendACK(fifoRead, fifoWrite, command, bufferSize) == ERROR)
			return;
		if (interupt) {  /* SIGINT/SIGQUIT has been caught, so create log file */
			CreateLogFile(countries, totalRequests, acceptedRequests);
			interupt = 0;  /* Reset */
			continue;
		}
		if (newVaccinationRecords) {  /* SIGUSR1 has been caught, so new country files are available */
			AddVaccinationRecords(fifoRead, fifoWrite, bufferSize, sizeOfBloom, inputDir, citizens, countries, viruses, oldCountryFiles);
			newVaccinationRecords = 0;  /* Reset */
			continue;
		}
		unsigned int argCount = CountArguments(command);
		istringstream sstream(command);
		string function, argument1, argument2, argument3, argument4;
		sstream >> function >> argument1 >> argument2 >> argument3 >> argument4;
		if (function == "/travelRequest") {
			if (argCount == 4)
				TravelRequest(fifoRead, fifoWrite, bufferSize, citizens, viruses, totalRequests, acceptedRequests, argument1, argument2, argument3, argument4);
			else
				PrintProperUse();
		}
		else if (function == "/searchVaccinationStatus") {
			if (argCount == 1)
				SearchVaccinationStatus(fifoRead, fifoWrite, bufferSize, citizens, viruses, argument1);
			else
				PrintProperUse();
		}
		else
			PrintProperUse();
	}
}

void PrintProperUse() {
	cout << "Please try one of the following:\n"
				<< "/travelRequest citizenID date countryFrom virusName\n"
				<< "/searchVaccinationStatus citizenID\n";
}

unsigned int CountArguments(const string& command) {
	istringstream sstream(command);
	string function, argument;
	sstream >> function;
	unsigned int count = 0;
	do {
		argument = "";
		sstream >> argument;
		if (!argument.empty())
			count++;
	} while(!argument.empty());
	return count;
}

void TravelRequest(const string& fifoRead, const string& fifoWrite, const unsigned int& bufferSize, hashTable& citizens, list& viruses, unsigned int& totalRequests, unsigned int& acceptedRequests, const string& citizenID, const string& dateStr, const string& countryFrom, const string& virusName) {
	string message;
	if (ReceiveMessageSendACK(fifoRead, fifoWrite, message, bufferSize) == ERROR)
		return;
	if (message != STOP)
		return;	
	totalRequests++;  /* Increase total requests */
	country cntrTemp("");
	citizen ctznTemp(citizenID, "", "", cntrTemp, 0);
	nodeData* dataPtr = citizens.Search(ctznTemp);
	citizen *ctznPtr = dynamic_cast<citizen *>(dataPtr);
	if (ctznPtr != NULL && ctznPtr->GetCountry().GetName() != countryFrom) {  /* Check that countryFrom is valid for this citizen */
		message = "NO";
	}
	else {
		virus vrsTemp(virusName, 0);
		dataPtr = viruses.Search(vrsTemp);
		if (dataPtr == NULL)  /* Not registered virus */
			message = "NO";
		else {
			virus *vrsPtr = dynamic_cast<virus *>(dataPtr);
			const tm *date = vrsPtr->SearchVaccinatedPersons(citizenID);
			if (date == NULL)  /* Not vaccinated citizen */
				message = "NO";
			else {  /* Vaccinated citizen */
				string vaccineDateStr = DateToString(*date);
				message = "YES " + vaccineDateStr;
				tm vaccineDate = CreateDate(vaccineDateStr);
				string travelDateStr = dateStr;
				tm travelDate = CreateDate(travelDateStr);
				double difSeconds = difftime(mktime(&travelDate), mktime(&vaccineDate));  /* Compare travel date to vaccine date */
				if (difSeconds >= 0 && difSeconds / 60 / 60 / 24 / 30 < 6)  /* Difference in seconds -> minutes -> hours -> days -> months */
					acceptedRequests++;  /* Accept request if difference is less than 6 months (180 days) */
			}
		}
	}
	if (SendMessageReceiveACK(fifoRead, fifoWrite, message, bufferSize) == ERROR)
		return;
	message = STOP;
	if (SendMessageReceiveACK(fifoRead, fifoWrite, message, bufferSize) == ERROR)
		return;
}

void AddVaccinationRecords(const string& fifoRead, const string& fifoWrite, const unsigned int& bufferSize, const unsigned int& sizeOfBloom, const string& inputDir, hashTable& citizens, list& countries, list& viruses, list& oldCountryFiles) {
	const listNode *node = countries.GetHead();
	while (node != NULL) {  /* Check every country for new files */
		const nodeData *dataPtr = node->GetData();
		const country *cntrPtr = dynamic_cast<const country *>(dataPtr);
		if (cntrPtr != NULL) {
			string path = "./" + inputDir + "/" + cntrPtr->GetName();
			if (InputFromDirectory(path, sizeOfBloom, citizens, countries, viruses, oldCountryFiles) == ERROR)
				return;
		}
		node = node->GetNext();
	}
	
	if (SendBloomFilters(fifoRead, fifoWrite, bufferSize, sizeOfBloom, viruses) == ERROR)
		return;
}

void SearchVaccinationStatus(const string& fifoRead, const string& fifoWrite, const unsigned int& bufferSize, hashTable& citizens, list& viruses, const string& citizenID) {
	string message;
	if (ReceiveMessageSendACK(fifoRead, fifoWrite, message, bufferSize) == ERROR)
		return;
	if (message != STOP)
		return;
	country cntrTemp("");
	citizen ctznTemp(citizenID, "", "", cntrTemp, 0);
	nodeData* dataPtr = citizens.Search(ctznTemp);
	if (dataPtr != NULL) {  /* Found citizen with given citizenID */
		citizen *ctznPtr = dynamic_cast<citizen *>(dataPtr);
		message = citizenID + " " + ctznPtr->GetFirstName() + " " + ctznPtr->GetLastName() + " " + ctznPtr->GetCountry().GetName();
		if (SendMessageReceiveACK(fifoRead, fifoWrite, message, bufferSize) == ERROR)  /* Send citizenID, firstName, lastName, country */
			return;
		message = "AGE " + to_string(ctznPtr->GetAge());
		if (SendMessageReceiveACK(fifoRead, fifoWrite, message, bufferSize) == ERROR)  /* Also send age*/
			return;
		
		const listNode *node = viruses.GetHead();
		unsigned int count = 0;
		while (node != NULL) { /* Check his vaccine status for each virus */
			const nodeData *dataPtr = node->GetData();
			const virus *vrsPtr = dynamic_cast<const virus *>(dataPtr);
			const tm *date = vrsPtr->SearchVaccinatedPersons(citizenID);
			if (date != NULL) {  /* Vaccinated against this virus */
				message = vrsPtr->GetName() + " VACCINATED ON " + DateToString(*date);
				if (SendMessageReceiveACK(fifoRead, fifoWrite, message, bufferSize) == ERROR)
					return;
				count++;
			}
			else {  /* Not vaccinated against this virus */
				message = vrsPtr->GetName() + " NOT YET VACCINATED";
				if (SendMessageReceiveACK(fifoRead, fifoWrite, message, bufferSize) == ERROR)
					return;
				count++;
			}
			node = node->GetNext();
		}
		if (count == 0) {
			message = "NOT VACCINATED AGAINST ANY VIRUS";
			if (SendMessageReceiveACK(fifoRead, fifoWrite, message, bufferSize) == ERROR)
					return;
		}
	}
	message = STOP;
	if (SendMessageReceiveACK(fifoRead, fifoWrite, message, bufferSize) == ERROR)
		return;
}

void CreateLogFile(list& countries, const unsigned int& totalRequests, const unsigned int& acceptedRequests) {
	string filename = "log_file." + to_string(getpid());
	ofstream file(filename);
	const listNode *node = countries.GetHead();
	while (node != NULL) {  /* Write all countries that monitor is aware of */
		const nodeData *dataPtr = node->GetData();
		const country *cntrPtr = dynamic_cast<const country *>(dataPtr);
		if (cntrPtr != NULL)
			file << cntrPtr->GetName() << "\n";
		node = node->GetNext();
	}
	file << "TOTAL TRAVEL REQUESTS " << totalRequests << "\n";
	file << "ACCEPTED " << acceptedRequests << "\n";
	file << "REJECTED " << (totalRequests - acceptedRequests) << "\n";
	file.close();
	kill(getppid(), SIGUSR1);  /* Let parent know that log file has been created */
}
