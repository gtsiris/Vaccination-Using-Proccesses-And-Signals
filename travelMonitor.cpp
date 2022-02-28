#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <unistd.h>
#include <dirent.h>
#include <signal.h>
#include "struct_Monitor.h"
#include "class_Travel_Request.h"
#include "class_Country.h"
#include "class_Virus.h"
#include "struct_List.h"

#define END_OF_MESSAGE "#"  /* Let receiver know that message has been completed */
#define ACK "@"  /* Follow up each message with acknowledgement */
#define STOP "$"  /* Doesn't require ACK. Signifies change of direction in communication */
#define MINIMUM_BUFFER_SIZE 1  /* Atleast 1 byte is required */
#define PERMITIONS 0666
#define OK 0
#define ERROR !OK
#define NOT_AVAILABLE -1

using namespace std;

void SetSignalHandlers();

bool Configure(const int& argc, char** argv, unsigned int& numMonitors, unsigned int& bufferSize, unsigned int& sizeOfBloom, string& inputDir);

void PrintProperExec();

bool CreateFIFOs(const unsigned int& numMonitors);

void DeleteFIFOs(const unsigned int& numMonitors);

bool CreateMonitors(const unsigned int& numMonitors, const unsigned int& sizeOfBloom, list& monitors);

bool InitializeMonitors(const unsigned int& numMonitors, const unsigned int& bufferSize, const unsigned int& sizeOfBloom, const string& inputDir, list& monitors);

bool ReceiveBloomFilters(const unsigned int& numMonitors, const unsigned int& bufferSize, const unsigned int& sizeOfBloom, list& monitors, list& viruses);

bool RoundRobin(const unsigned int& numMonitors, const unsigned int& bufferSize, const string& inputDir, list& monitors);

bool SendMessageReceiveACK(const string& fifoRead, const string& fifoWrite, const string& message, const unsigned int& bufferSize, const int& messageSize = NOT_AVAILABLE);

bool ReceiveMessageSendACK(const string& fifoRead, const string& fifoWrite, string& message, const unsigned int& bufferSize);

bool ReadFIFO(const string& fifoRead, string& message, const unsigned int& bufferSize);

bool WriteFIFO(const string& fifoWrite, const string& message, const unsigned int& bufferSize);

int GetSizeFIFO(const string& fifo);

bool IsFIFO(const string& path);

bool IsSubdirectory(const string& path, const string& name);

bool IsDirectory(const string& path);

virus *RegisterVirus(const string& virusName, const unsigned int& bloomSize, list& viruses);

string CreateAttribute(string& line, const char& delimiter = ' ');

tm CreateDate(string& line);

void PrintDate(const tm& date);

void ReceiveCommands(const unsigned int& numMonitors, const unsigned int& bufferSize, const unsigned int& sizeOfBloom, const string& inputDir, list& monitors, list& viruses);

void PrintProperUse();

unsigned int CountArguments(const string& command);

void TravelRequest(const unsigned int& bufferSize, list& monitors, list& viruses, unsigned int& totalRequests, unsigned int& acceptedRequests, const string& citizenID, const string& dateStr, const string& countryFrom, const string& countryTo, const string& virusName);

void TravelStats(const unsigned int& bufferSize, list& monitors, list& viruses, const string& virusName, string& dateStr1, string& dateStr2, const string& countryTo = "NOT A COUNTRY");

void AddVaccinationRecords(const unsigned int& bufferSize, const unsigned int& sizeOfBloom, list& monitors, list& viruses, const string& countryFrom);

void SearchVaccinationStatus(const unsigned int& numMonitors, const unsigned int& bufferSize, list& monitors, list& viruses, const string& citizenID);

void Exit(list& monitors, const unsigned int& bufferSize = NOT_AVAILABLE, const unsigned int& totalRequests = 0, const unsigned int& acceptedRequests = 0);

void RegenerateMonitors(const unsigned int& bufferSize, const unsigned int& sizeOfBloom, const string& inputDir, list& monitors, list& viruses);

bool RegenerateMonitor(const unsigned int& monitorID, const list& countries, const unsigned int& bufferSize, const unsigned int& sizeOfBloom, const string& inputDir, list& monitors, list& viruses);

void KillMonitors(list& monitors);

bool regeneration = 1;  /* By default regenerate monitors. Unset this flag to be able to kill monitors without regeneration. */
bool interupt = 0;
bool monitorTerminated = 0;

void CatchChild(int signo) {
	monitorTerminated = 1;
}

void CatchInterupt(int signo) {
	interupt = 1;
}

void CatchResponse(int signo) {
}

int main (int argc, char** argv) {
	SetSignalHandlers();
	
	unsigned int numMonitors;
	unsigned int bufferSize;
	unsigned int sizeOfBloom;
	string inputDir;
	if (Configure(argc, argv, numMonitors, bufferSize, sizeOfBloom, inputDir) == ERROR) {
		PrintProperExec();
		return ERROR;
	}
	if (bufferSize < MINIMUM_BUFFER_SIZE) {
		cout << "Buffer size is too small\n";
		return ERROR;
	}
	if (CreateFIFOs(numMonitors) == ERROR) {
		cout << "Error occured during the creation of named pipes\n";
		DeleteFIFOs(numMonitors);
		return ERROR;
	}
	list monitors;
	if (CreateMonitors(numMonitors, sizeOfBloom, monitors) == ERROR) {
		cout << "Error occured during the creation of monitors\n";
		Exit(monitors);
		DeleteFIFOs(numMonitors);
		return ERROR;
	}
	if (InitializeMonitors(numMonitors, bufferSize, sizeOfBloom, inputDir, monitors) == ERROR) {
		cout << "Error occured during the initialization of monitors\n";
		Exit(monitors);
		DeleteFIFOs(numMonitors);
		return ERROR;
	}
	list viruses;
	if (ReceiveBloomFilters(numMonitors, bufferSize, sizeOfBloom, monitors, viruses) == ERROR) {
		cout << "Error occured during the reception of bloom filters\n";
		Exit(monitors);
		DeleteFIFOs(numMonitors);
		return ERROR;
	}
	
	ReceiveCommands(numMonitors, bufferSize, sizeOfBloom, inputDir, monitors, viruses);
	
	DeleteFIFOs(numMonitors);
	return OK;
}

void SetSignalHandlers() {
	static struct sigaction act;
	act.sa_handler = CatchChild;
	act.sa_flags = SA_RESTART;
	sigfillset(&(act.sa_mask));
	sigaction(SIGCHLD, &act, NULL);  /* Set the handler for SIGCHLD */
	
	static struct sigaction act2;
	act2.sa_handler = CatchInterupt;
	act2.sa_flags = SA_RESTART;
	sigfillset(&(act2.sa_mask));
	sigaction(SIGINT, &act2, NULL);  /* Set the handler for SIGINT */
	sigaction(SIGQUIT, &act2, NULL);  /* The handler for SIGQUIT is the same */
	
	static struct sigaction act3;
	act3.sa_handler = CatchResponse;
	sigfillset(&(act3.sa_mask));
	sigaction(SIGUSR1, &act3, NULL);  /* Set the handler for SIGINT */
}

bool Configure(const int& argc, char** argv, unsigned int& numMonitors, unsigned int& bufferSize, unsigned int& sizeOfBloom, string& inputDir) {
	if (argc == 9) {
		srand(time(NULL));
		for (unsigned i = 1; i < argc; i += 2) {
			if (string(argv[i]) == "-m")
				numMonitors = atoi(argv[i + 1]);
			else if (string(argv[i]) == "-b")
				bufferSize = atoi(argv[i + 1]);
			else if (string(argv[i]) == "-s")
				sizeOfBloom = atoi(argv[i + 1]);
			else if (string(argv[i]) == "-i")
				inputDir = string(argv[i + 1]);
		}
		return OK;
	}
	return ERROR;
}

void PrintProperExec() {
	cout << "Please try: ./travelMonitor -m numMonitors -b bufferSize -s sizeOfBloom -i input_dir\n";
}

bool CreateFIFOs(const unsigned int& numMonitors) {
	DeleteFIFOs(numMonitors);  /* Make sure those fifos don't already exist */
	for (unsigned int i = 1; i <= numMonitors; i++) {
		string fifoRead = "/tmp/fifo" + to_string(i) + "R";
		if (mkfifo(fifoRead.c_str(), PERMITIONS) < 0) {  /* Create fifo for reading */
			perror("read");
			return ERROR;
		}
		string fifoWrite = "/tmp/fifo" + to_string(i) + "W";
		if (mkfifo(fifoWrite.c_str(), PERMITIONS) < 0) {  /* Create fifo for writing */
			perror("write");
			return ERROR;
		}
	}
	return OK;
}

void DeleteFIFOs(const unsigned int& numMonitors) {
	for (unsigned int i = 1; i <= numMonitors; i++) {
		string fifoRead = "/tmp/fifo" + to_string(i) + "R";
		if (IsFIFO(fifoRead) && unlink(fifoRead.c_str()) < 0) {  /* Delete existing fifo */
			perror("unlink");
			return;
		}
		string fifoWrite = "/tmp/fifo" + to_string(i) + "W";
		if (IsFIFO(fifoWrite) && unlink(fifoWrite.c_str()) < 0) {  /* Delete existing fifo */
			perror("unlink");
			return;
		}
	}
}

bool CreateMonitors(const unsigned int& numMonitors, const unsigned int& sizeOfBloom, list& monitors) {
	pid_t childpid;
	for (unsigned int i = 1; i <= numMonitors; i++) {
		childpid = fork();
		if (childpid < 0) {  /* Error */
			perror("fork");
			return ERROR;
		}
		else if (childpid == 0) {  /* Child process */
			string fifoRead = "/tmp/fifo" + to_string(i) + "R";
			string fifoWrite = "/tmp/fifo" + to_string(i) + "W";
			execl("./Monitor", "./Monitor", fifoWrite.c_str(), fifoRead.c_str(), NULL);
			perror("execl");  /* This point shouldn't be reached */
			return ERROR;
		}
		else {  /* Parent process */
			monitor mntrTemp(i, childpid);  /* Store info for monitor */
			monitors.Insert(mntrTemp);  /* Add to the list */
		}
	}
	return OK;
}

bool InitializeMonitors(const unsigned int& numMonitors, const unsigned int& bufferSize, const unsigned int& sizeOfBloom, const string& inputDir, list& monitors) {
	string message;
	
	for (unsigned int i = 1; i <= numMonitors; i++) {  /* For each monitor */
		string fifoRead = "/tmp/fifo" + to_string(i) + "R";
		string fifoWrite = "/tmp/fifo" + to_string(i) + "W";
		
		message = to_string(bufferSize);
		if (SendMessageReceiveACK(fifoRead, fifoWrite, message, bufferSize) == ERROR)  /* Send bufferSize */
			return ERROR;
		
		message = to_string(sizeOfBloom);
		if (SendMessageReceiveACK(fifoRead, fifoWrite, message, bufferSize) == ERROR)  /* Send sizeOfBloom */
			return ERROR;
		
		message = inputDir;
		if (SendMessageReceiveACK(fifoRead, fifoWrite, message, bufferSize) == ERROR)  /* Send inputDir */
			return ERROR;
	}
	
	if (RoundRobin(numMonitors, bufferSize, inputDir, monitors) == ERROR)  /* Perform round robin to distribute countries */
		return ERROR;
	
	for (unsigned int i = 1; i <= numMonitors; i++) {
		string fifoRead = "/tmp/fifo" + to_string(i) + "R";
		string fifoWrite = "/tmp/fifo" + to_string(i) + "W";
		
		message = STOP;
		if (SendMessageReceiveACK(fifoRead, fifoWrite, message, bufferSize) == ERROR)
			return ERROR;
	}
	
	return OK;
}

bool ReceiveBloomFilters(const unsigned int& numMonitors, const unsigned int& bufferSize, const unsigned int& sizeOfBloom, list& monitors, list& viruses) {
	string message;
	
	for (unsigned int i = 1; i <= numMonitors; i++) {  /* For each monitor */
		string fifoRead = "/tmp/fifo" + to_string(i) + "R";
		string fifoWrite = "/tmp/fifo" + to_string(i) + "W";
		
		while (1) {  /* Receive bloom filters of every virus this monitor has available */
			if (ReceiveMessageSendACK(fifoRead, fifoWrite, message, bufferSize) == ERROR)  /* Receive virusName */
				return ERROR;
			if (message == STOP) {
				break;
			}
			string virusName = message;
			virus *vrsPtr = RegisterVirus(virusName, sizeOfBloom, viruses);
			
			if (ReceiveMessageSendACK(fifoRead, fifoWrite, message, bufferSize) == ERROR)  /* Receive bitArray of its bloom */
				return ERROR;
			string bitArray = message;
			bloomFilter bloom(sizeOfBloom);  /* Create a temporary bloom with same size */
			bloom.SetBitArray(bitArray.c_str());  /* Set bitArray as the received one */
			vrsPtr->UpdateBloom(bloom);  /* Update the existing bloom of this virus */
		}
	}
	
	return OK;
}

bool RoundRobin(const unsigned int& numMonitors, const unsigned int& bufferSize, const string& inputDir, list& monitors) {
	cout << "Loading records...\n";
	string path = "./" + inputDir;
	struct dirent **entries;
	int numOfEntries = scandir(path.c_str(), &entries, NULL, alphasort);  /* Get the entries of inputDir in alphabetical order */
	if (numOfEntries < 0)
		return ERROR;
	unsigned int i = 1;
	for(unsigned int j = 0; j < numOfEntries; j++ ) {
		string countryName = entries[j]->d_name;
		string newPath = path + "/" + countryName;
		if (IsSubdirectory(newPath, countryName)) {  /* If it is actually a subdirectory */
			string fifoRead = "/tmp/fifo" + to_string(i) + "R";
			string fifoWrite = "/tmp/fifo" + to_string(i) + "W";
			string message = countryName;
			if (SendMessageReceiveACK(fifoRead, fifoWrite, message, bufferSize) == ERROR)  /* Send countryName to the appropriate monitor */
				return ERROR;
			
			monitor mntrTemp(i);
			nodeData *dataPtr = monitors.Search(mntrTemp);
			if (dataPtr == NULL)
				return ERROR;
			monitor *mntrPtr = dynamic_cast<monitor *>(dataPtr);
			mntrPtr->AddCountry(countryName);  /* Update the info about this monitor */
			if (++i > numMonitors)  /* Find next monitor */
				i = 1;  /* If it exceeds numMonitors, start all over again (Round Robin) */
		}
		free(entries[j]);  /* Free memory allocated by scandir */
	}
	free(entries);  /* Free memory allocated by scandir */
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

bool IsFIFO(const string& path) {
	struct stat statBuf;
	if (stat(path.c_str(), &statBuf))
		return 0;
	return S_ISFIFO(statBuf.st_mode);
}

bool IsSubdirectory(const string& path, const string& name) {
	if (name != "." && name != "..")
		return IsDirectory(path);
	return 0;
}

bool IsDirectory(const string& path) {
	struct stat statBuf;
	if (stat(path.c_str(), &statBuf))
		return 0;
	return S_ISDIR(statBuf.st_mode);
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

void PrintDate(const tm& date) {
	cout << date.tm_mday << "-" << 1 + date.tm_mon << "-" << 1900 + date.tm_year;
}

void ReceiveCommands(const unsigned int& numMonitors, const unsigned int& bufferSize, const unsigned int& sizeOfBloom, const string& inputDir, list& monitors, list& viruses) {
	unsigned int totalRequests = 0;
	unsigned int acceptedRequests = 0;
	cout << "Give any command:\n";
	string command;
	while (getline(cin, command)) {
		if (interupt) {  /* SIGINT/SIGQUIT has been caught, so exit */
			Exit(monitors, bufferSize, totalRequests, acceptedRequests);
			return;
		}
		if (monitorTerminated && regeneration) {  /* SIGCHLD has been caught. If regeneration flag is set, regenerate monitors */
			RegenerateMonitors(bufferSize, sizeOfBloom, inputDir, monitors, viruses);
			monitorTerminated = 0;  /* All monitors are alive and running */
		}
		unsigned int argCount = CountArguments(command);
		istringstream sstream(command);
		string function, argument1, argument2, argument3, argument4, argument5;
		sstream >> function >> argument1 >> argument2 >> argument3 >> argument4 >> argument5;
		if (function == "/travelRequest") {
			if (argCount == 5) {
				TravelRequest(bufferSize, monitors, viruses, totalRequests, acceptedRequests, argument1, argument2, argument3, argument4, argument5);
			}
			else
				PrintProperUse();
		}
		else if (function == "/travelStats") {
			if (argCount == 3)
				TravelStats(bufferSize, monitors, viruses, argument1, argument2, argument3);
			else if (argCount == 4)
				TravelStats(bufferSize, monitors, viruses, argument1, argument2, argument3, argument4);
			else
				PrintProperUse();
		}
		else if (function == "/addVaccinationRecords") {
			if (argCount == 1)
				AddVaccinationRecords(bufferSize, sizeOfBloom, monitors, viruses, argument1);
			else
				PrintProperUse();
		}
		else if (function == "/searchVaccinationStatus") {
			if (argCount == 1)
				SearchVaccinationStatus(numMonitors, bufferSize, monitors, viruses, argument1);
			else
				PrintProperUse();
		}
		else if (function == "/exit") {
			if (argCount == 0) {
				Exit(monitors, bufferSize, totalRequests, acceptedRequests);
				return;
			}
			else
				PrintProperUse();
		}
		else if (function == "/killMonitors") {  /* Bonus function to test regeneration */
			if (argCount == 0) {
				KillMonitors(monitors);
			}
			else
				PrintProperUse();
		}
		else
			PrintProperUse();
		if (interupt) {  /* SIGINT/SIGQUIT has been caught, so exit */
			Exit(monitors, bufferSize, totalRequests, acceptedRequests);
			return;
		}
		if (monitorTerminated && regeneration) {  /* SIGCHLD has been caught. If regeneration flag is set, regenerate monitors */
			RegenerateMonitors(bufferSize, sizeOfBloom, inputDir, monitors, viruses);
			monitorTerminated = 0;  /* All monitors are alive and running */
		}
	}
}

void PrintProperUse() {
	cout << "Please try one of the following:\n"
				<< "/travelRequest citizenID date countryFrom countryTo virusName\n"
				<< "/travelStats virusName date1 date2 [country]\n"
				<< "/addVaccinationRecords country\n"
				<< "/searchVaccinationStatus citizenID\n"
				<< "/exit\n"
				<< "/killMonitors        /* Showcase regeneration feature */\n";
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

void TravelRequest(const unsigned int& bufferSize, list& monitors, list& viruses, unsigned int& totalRequests, unsigned int& acceptedRequests, const string& citizenID, const string& dateStr, const string& countryFrom, const string& countryTo, const string& virusName) {
	int monitorID;
	const country *cntrPtr;
	const listNode *node = monitors.GetHead();
	unsigned int count = 0;
	while (node != NULL) {  /* Find which monitor is responsible for countryFrom */
		const nodeData *dataPtr = node->GetData();
		const monitor *mntrPtr = dynamic_cast<const monitor *>(dataPtr);
		cntrPtr = mntrPtr->GetCountry(countryFrom);
		if (cntrPtr != NULL) {
			monitorID = mntrPtr->GetMonitorID();
			count++;
			break;
		}
		node = node->GetNext();
	}
	if (count == 0) {  /* Not registered countryFrom */
		cout << "ERROR: " << countryFrom << " IS NOT A VALID COUNTRY\n";
		return;
	}
	node = monitors.GetHead();
	count = 0;
	while (node != NULL) {  /* Check if there is a monitor responsible for countryTo */
		const nodeData *dataPtr = node->GetData();
		const monitor *mntrPtr = dynamic_cast<const monitor *>(dataPtr);
		cntrPtr = mntrPtr->GetCountry(countryTo);
		if (cntrPtr != NULL) {
			count++;
			break;
		}
		node = node->GetNext();
	}
	if (count == 0) {  /* Not registered countryTo */
		cout << "ERROR: " << countryTo << " IS NOT A VALID COUNTRY\n";
		return;
	}
	virus vrsTemp(virusName, 0);
	nodeData *dataPtr = viruses.Search(vrsTemp);
	if (dataPtr == NULL) {  /* Not registered virus */
		cout << "ERROR: " << virusName << " IS NOT A VALID VIRUS\n";
		return;
	}
	totalRequests++;  /* Increase total requests */
	string travelDateStr = dateStr;
	tm travelDate = CreateDate(travelDateStr);
	virus *vrsPtr = dynamic_cast<virus *>(dataPtr);
	if (vrsPtr->SearchBloom(citizenID)) {  /* If virus' bloom says that it's possible for citizen to be vaccinated */
		string fifoRead = "/tmp/fifo" + to_string(monitorID) + "R";
		string fifoWrite = "/tmp/fifo" + to_string(monitorID) + "W";
		string message = "/travelRequest " + citizenID + " " + dateStr + " " + countryFrom + " " + virusName;
		if (SendMessageReceiveACK(fifoRead, fifoWrite, message, bufferSize) == ERROR)  /* Send citizenID, date, countryFrom, virus */
			return;
		message = STOP;
		if (SendMessageReceiveACK(fifoRead, fifoWrite, message, bufferSize) == ERROR)
			return;
		if (ReceiveMessageSendACK(fifoRead, fifoWrite, message, bufferSize) == ERROR)  /* Receive vaccinated status */
			return;
		string vaccinated = CreateAttribute(message);
		if (vaccinated == "YES") {  /* If citizen has been vaccinated, check the vaccine date to determine if he's eligible to travel */
			tm vaccineDate = CreateDate(message);
			double difSeconds = difftime(mktime(&travelDate), mktime(&vaccineDate));  /* Compare travel date to vaccine date */
			if (difSeconds >= 0 && difSeconds / 60 / 60 / 24 / 30 < 6) {  /* Difference in seconds -> minutes -> hours -> days -> months */
				cout << "REQUEST ACCEPTED - HAPPY TRAVELS\n";  /* Accept request if difference is less than 6 months (180 days) */
				travelRequest tRequestTemp(travelDate, ACCEPTED);
				vrsPtr->AddTravelRequest(tRequestTemp, *cntrPtr);  /* Store travel request */
				acceptedRequests++;  /* Increase accepted requests */
				if (ReceiveMessageSendACK(fifoRead, fifoWrite, message, bufferSize) == ERROR)
					return;
				if (message != STOP)
					return;
				return;
			}
			else {  /* If the difference is greater than 6 months or the travel date is before the vaccine date */
				cout << "REQUEST REJECTED - YOU WILL NEED ANOTHER VACCINATION BEFORE TRAVEL DATE\n";  /* He has to get vaccinated before travel date */
				travelRequest tRequestTemp(travelDate, REJECTED);
				vrsPtr->AddTravelRequest(tRequestTemp, *cntrPtr);  /* Store travel request */
			}
		}
		else if (vaccinated == "NO") {  /* If citizen hasn't been vaccinated, reject his request */
			cout << "REQUEST REJECTED - YOU ARE NOT VACCINATED\n";
			travelRequest tRequestTemp(travelDate, REJECTED);
			vrsPtr->AddTravelRequest(tRequestTemp, *cntrPtr);  /* Store travel request */
		}
		if (ReceiveMessageSendACK(fifoRead, fifoWrite, message, bufferSize) == ERROR)
			return;
		if (message != STOP)
			return;
	}
	else {  /* If virus' bloom has negative answer, citizen is definitely not vaccinated and his request is rejected */
		cout << "REQUEST REJECTED - YOU ARE NOT VACCINATED\n";
		travelRequest tRequestTemp(travelDate, REJECTED);
		vrsPtr->AddTravelRequest(tRequestTemp, *cntrPtr);  /* Store travel request */
	}
}

void TravelStats(const unsigned int& bufferSize, list& monitors, list& viruses, const string& virusName, string& dateStr1, string& dateStr2, const string& countryTo) {
	virus vrsTemp(virusName, 0);
	nodeData *dataPtr = viruses.Search(vrsTemp);
	if (dataPtr == NULL) {  /* Not registered virus */
		cout << "ERROR: " << virusName << " IS NOT A VALID VIRUS\n";
		return;
	}
	if (countryTo != "NOT A COUNTRY") {
		const listNode *node = monitors.GetHead();
		unsigned int count = 0;
		while (node != NULL) {  /* Check if there is a monitor responsible for countryTo */
			const nodeData *dataPointer = node->GetData();
			const monitor *mntrPtr = dynamic_cast<const monitor *>(dataPointer);
			const country *cntrPtr = mntrPtr->GetCountry(countryTo);
			if (cntrPtr != NULL) {
				count++;
				break;
			}
			node = node->GetNext();
		}
		if (count == 0) {  /* Not registered countryTo */
			cout << "ERROR: " << countryTo << " IS NOT A VALID COUNTRY\n";
			return;
		}
	}
	virus *vrsPtr = dynamic_cast<virus *>(dataPtr);
	tm date1 = CreateDate(dateStr1);
	tm date2 = CreateDate(dateStr2);
	vrsPtr->PrintTravelStats(date1, date2, countryTo);  /* Print virus' stats based on travel requests between date1 and date2 */
}

void AddVaccinationRecords(const unsigned int& bufferSize, const unsigned int& sizeOfBloom, list& monitors, list& viruses, const string& countryFrom) {
	int monitorID;
	pid_t pid;
	const country *cntrPtr;
	const listNode *node = monitors.GetHead();
	unsigned int count = 0;
	while (node != NULL) {  /* Find which monitor is responsible for countryFrom */
		const nodeData *dataPtr = node->GetData();
		const monitor *mntrPtr = dynamic_cast<const monitor *>(dataPtr);
		cntrPtr = mntrPtr->GetCountry(countryFrom);
		if (cntrPtr != NULL) {
			monitorID = mntrPtr->GetMonitorID();
			pid = mntrPtr->GetPID();
			count++;
			break;
		}
		node = node->GetNext();
	}
	if (count == 0) {  /* Not registered countryFrom */
		cout << "ERROR: " << countryFrom << " IS NOT A VALID COUNTRY\n";
		return;
	}
	
	kill(pid, SIGUSR1);  /* Send SIGUSR1 to that monitor process */
	
	string fifoRead = "/tmp/fifo" + to_string(monitorID) + "R";
	string fifoWrite = "/tmp/fifo" + to_string(monitorID) + "W";
	
	string message = STOP;
	if (SendMessageReceiveACK(fifoRead, fifoWrite, message, bufferSize) == ERROR)
		return;
	
	while (1) {  /* Receive new bloom filters of every virus this monitor has available */
		if (ReceiveMessageSendACK(fifoRead, fifoWrite, message, bufferSize) == ERROR)  /* Receive virusName */
			return;
		if (message == STOP) {
			break;
		}
		string virusName = message;
		virus *vrsPtr = RegisterVirus(virusName, sizeOfBloom, viruses);
		
		if (ReceiveMessageSendACK(fifoRead, fifoWrite, message, bufferSize) == ERROR)  /* Receive bitArray of its bloom */
			return;
		string bitArray = message;
		bloomFilter bloom(sizeOfBloom);  /* Create a temporary bloom with same size */
		bloom.SetBitArray(bitArray.c_str());  /* Set bitArray as the received one */
		vrsPtr->UpdateBloom(bloom);  /* Update the existing bloom of this virus */
	}
	cout << "DONE\n";
}

void SearchVaccinationStatus(const unsigned int& numMonitors, const unsigned int& bufferSize, list& monitors, list& viruses, const string& citizenID) {
	int count = 0;
	for (unsigned int i = 1; i <= numMonitors; i++) { /* To every monitor */
		string fifoRead = "/tmp/fifo" + to_string(i) + "R";
		string fifoWrite = "/tmp/fifo" + to_string(i) + "W";
		string message = "/searchVaccinationStatus " + citizenID;
		if (SendMessageReceiveACK(fifoRead, fifoWrite, message, bufferSize) == ERROR)  /* Send citizenID */
			return;
		message = STOP;
		if (SendMessageReceiveACK(fifoRead, fifoWrite, message, bufferSize) == ERROR)
			return;
		while (1) {
			if (ReceiveMessageSendACK(fifoRead, fifoWrite, message, bufferSize) == ERROR)  /* Receive info about this citizen */
				return;
			if (message == STOP)  /* If message was just a STOP, the current monitor doesn't know anything about him */
				break;
			count++;
			cout << message << "\n";  /* Print info */
		}
	}
	if (count == 0) {  /* Not registered countryFrom */
		cout << "ERROR: CITIZEN " << citizenID << " NOT FOUND AT ANY COUNTRY\n";
		return;
	}
}

void Exit(list& monitors, const unsigned int& bufferSize, const unsigned int& totalRequests, const unsigned int& acceptedRequests) {
	regeneration = 0;  /* Unset flag to deactivate regeneration */
	string filename = "log_file." + to_string(getpid());
	ofstream file(filename);
	const listNode *node = monitors.GetHead();
	while (node != NULL) {  /* For each monitor */
		const nodeData *dataPtr = node->GetData();
		const monitor *mntrPtr = dynamic_cast<const monitor *>(dataPtr);
		if (mntrPtr != NULL) {
			pid_t pid = mntrPtr->GetPID();
			
			kill(pid, SIGINT);  /* Send SIGINT */
			if (bufferSize != NOT_AVAILABLE) {
				int monitoID = mntrPtr->GetMonitorID();
				string fifoRead = "/tmp/fifo" + to_string(monitoID) + "R";
				string fifoWrite = "/tmp/fifo" + to_string(monitoID) + "W";
				string message = "";
				if (SendMessageReceiveACK(fifoRead, fifoWrite, message, bufferSize) == ERROR)  /* Send empty message to awaiting read end */
					return;
			}
			pause();
			kill(pid, SIGKILL);  /* Send SIGKILL */
			waitpid(pid, NULL, 0);  /* Wait monitor process to terminate */
			const listNode *cntrNode = mntrPtr->GetCountries().GetHead();
			while (cntrNode != NULL) {  /* Write all countries that monitor is aware of */
				dataPtr = cntrNode->GetData();
				const country *cntrPtr = dynamic_cast<const country *>(dataPtr);
				file << cntrPtr->GetName() << "\n";
				cntrNode = cntrNode->GetNext();
			}
		}
		node = node->GetNext();
	}
	file << "TOTAL TRAVEL REQUESTS " << totalRequests << "\n";
	file << "ACCEPTED " << acceptedRequests << "\n";
	file << "REJECTED " << (totalRequests - acceptedRequests) << "\n";
	file.close();
}

void RegenerateMonitors(const unsigned int& bufferSize, const unsigned int& sizeOfBloom, const string& inputDir, list& monitors, list& viruses) {
	pid_t pid;
	while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {  /* For every process that has been terminated */
		monitor mntrTemp(NOT_AVAILABLE, pid);
		nodeData *dataPtr = monitors.Search(mntrTemp);  /* Find the monitor corresponding to this pid */
		const monitor *mntrPtr = dynamic_cast<const monitor *>(dataPtr);
		cout << "Regenerating monitor "<< mntrPtr->GetMonitorID() << "\n";
		if (RegenerateMonitor(mntrPtr->GetMonitorID(), mntrPtr->GetCountries(), bufferSize, sizeOfBloom, inputDir, monitors, viruses) == ERROR)  /* Regenerate this monitor */
			continue;
	}
}

bool RegenerateMonitor(const unsigned int& monitorID, const list& countries, const unsigned int& bufferSize, const unsigned int& sizeOfBloom, const string& inputDir, list& monitors, list& viruses) {
	string fifoRead = "/tmp/fifo" + to_string(monitorID) + "R";
	string fifoWrite = "/tmp/fifo" + to_string(monitorID) + "W";
	pid_t childpid = fork();
	if (childpid < 0) {  /* Error */
		perror("fork");
		return ERROR;
	}
	else if (childpid == 0) {  /* Child */
		execl("./Monitor", "./Monitor", fifoWrite.c_str(), fifoRead.c_str(), NULL);
		perror("execl");
		return ERROR;
	}
	/* Due to exec only parent process will reach this point */
	monitor mntrTemp(monitorID);
	nodeData *dataPtr = monitors.Search(mntrTemp);  /* Find the monitor corresponding to this monitorID */
	monitor *mntrPtr = dynamic_cast<monitor *>(dataPtr);
	mntrPtr->SetPID(childpid);  /* Update pid */
	
	string message;
	/* Initialization is required again */
	message = to_string(bufferSize);
	if (SendMessageReceiveACK(fifoRead, fifoWrite, message, bufferSize) == ERROR)  /* Send bufferSize */
		return ERROR;
	
	message = to_string(sizeOfBloom);
	if (SendMessageReceiveACK(fifoRead, fifoWrite, message, bufferSize) == ERROR)  /* Send sizeOfBloom */
		return ERROR;
	
	message = inputDir;
	if (SendMessageReceiveACK(fifoRead, fifoWrite, message, bufferSize) == ERROR)  /* Send inputDir */
		return ERROR;
	
	const listNode *node = countries.GetHead();
	while (node != NULL) {  /* Make the monitor responsible for the same countries that it previously was */
		const nodeData *dataPtr = node->GetData();
		const country *cntrPtr = dynamic_cast<const country *>(dataPtr);
		if (cntrPtr != NULL) {
			string countryName = cntrPtr->GetName();
			message = countryName;
			if (SendMessageReceiveACK(fifoRead, fifoWrite, message, bufferSize) == ERROR)
				return ERROR;
		}
		node = node->GetNext();
	}
	
	message = STOP;
	if (SendMessageReceiveACK(fifoRead, fifoWrite, message, bufferSize) == ERROR)
		return ERROR;
	
	while (1) {  /* Receive bloom filters of every virus this monitor has available */
		if (ReceiveMessageSendACK(fifoRead, fifoWrite, message, bufferSize) == ERROR)  /* Receive virusName */
			return ERROR;
		if (message == STOP) {
			break;
		}
		string virusName = message;
		virus *vrsPtr = RegisterVirus(virusName, sizeOfBloom, viruses);
		
		if (ReceiveMessageSendACK(fifoRead, fifoWrite, message, bufferSize) == ERROR)  /* Receive bitArray of its bloom */
			return ERROR;
		string bitArray = message;
		bloomFilter bloom(sizeOfBloom);  /* Create a temporary bloom with same size */
		bloom.SetBitArray(bitArray.c_str());  /* Set bitArray as the received one */
		vrsPtr->UpdateBloom(bloom);  /* Update the existing bloom of this virus */
	}
	
	return OK;
}

void KillMonitors(list& monitors) {
	const listNode *node = monitors.GetHead();
	while (node != NULL) {  /* For each monitor */
		const nodeData *dataPtr = node->GetData();
		const monitor *mntrPtr = dynamic_cast<const monitor *>(dataPtr);
		if (mntrPtr != NULL) {
			pid_t pid = mntrPtr->GetPID();
			kill(pid, SIGKILL);  /* Send SIGKILL */
		}
		node = node->GetNext();
	}
	pause();
}
