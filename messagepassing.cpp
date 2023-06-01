// INSTRUCTIONS ON HOW TO RUN:
// Compile command: g++ messagepassing.cpp -o messagepassing.exe
// Run command: ./messagepassing instructions.txt
// Program will search for a prose.txt file and run it.
// My project can only transfer one instruction and execute it

#include <iostream>
#include <sys/types.h>     
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <mqueue.h>
#include <time.h>
#include <errno.h>
#include <error.h>
#include <string.h>
#include <cstring>
using namespace std;

// SIZE OF BUFFER USED TO READ INSTRUCTIONS
const int SIZ = 256;

// SIZE OF EACH MESSAGE IN A MESSAGE QUEUE
const int MSG_SIZE = 30;

// AMOUNT OF MESSAGES IN MESSAGE QUEUE
const int MSG_CAPACITY = 10; 

// LINE OF INSTRUCTION THAT THE PROGRAM WILL EXECUTE
// 0 - 19
const int instructToRead = 0;

int main(int argc, const char *argv[]) {

	// TAKE THE CORRECT AMOUNT OF ARGUMENTS
	// AND OPEN FILE DESCRIPTOR
	int f1;	

	if (argc != 3){
		cout << "Wrong number of command line arguments\n";
		return 1;
	}

	if ((f1 = open(argv[1], O_RDONLY, 0)) == -1){
		cout << "Can't open input file \n" ;
		return 2;
	}
	
	// CREATE MESSAGE QUEUE ATTRIBUTE
	cout << ("Parent: Before mq_open...\n");
	
	struct mq_attr attr;
	attr.mq_maxmsg = MSG_CAPACITY;
	attr.mq_msgsize = MSG_SIZE;
	
	// OPEN MESSAGE QUEUE
	mqd_t mqd = mq_open("/myQueueGurdev", O_CREAT | O_RDWR, 0644, &attr);
	if (mqd == (mqd_t) -1) {
      cout << "Parent:  mq_open error\n";
      cout << "errno is " << errno << " and string is " << strerror(errno);
      mq_unlink("/myQueueGurdev");
      return 2;
      }
      
    // GETTING THE MESSAGE QUEUE'S ATTRIBUTE FOR DEBUGGING
	struct mq_attr ret_attr; 	
	if (mq_getattr(mqd, &ret_attr) == -1) {
      printf("Parent: mq_getattr error");
      return 2;
      }

	// READ EVERYTHING FROM f1 AND PUT IT INTO BUF
	char buf[SIZ];
	int n;
	
	while ((n = read(f1, buf, SIZ)) > 0){
	}
	
	// CREATING A STRING ARRAY OF 20 INSTRUCTIONS
	string instructions[20];
	int currInstruction = 0; 
	int newlines = 0;
	
	// STORE THE INSTRUCTIONS IN THE INSTRUCTION ARRAY
	for (int i = 0; i < SIZ; i++) {
		if (buf[i] != '\n') {
			instructions[currInstruction] += buf[i];
		}
		else {
			currInstruction++;
		} 	
	}
	
	// FORK THE PROCESS
	int id = fork();
		
	if (id < 0) { // FORK NOT EXECUTED
		printf("An error occured with fork\n");
		return 4;
	}
	else if (id == 0) { // CHILD PROCESS
		
		printf("Child process ID is:\t%d\n", getpid());
		
		// RECEIVING INSTRUCTIONS FROM MESSAGE QUEUE
		char childBuf[MSG_SIZE];
		unsigned int prio;
		
		ssize_t numRead = mq_timedreceive(mqd, childBuf, MSG_SIZE, &prio, NULL);
		
		// ERROR HANDLING FOR TIMEDRECEIVE
		if (numRead == -1) {
		  	printf("Child: mq_read error\n");
		  	printf("Errno: %d; Error: %s\n", errno, strerror(errno));
		  	return 5;
			}
		
		// PRINT AMOUNT OF MESSAGES RECEIVED
		if (mq_getattr(mqd, &ret_attr) == -1) {
      		printf("Parent: mq_getattr error");
      		return 2;
      		}
		
		// STORING THE ARGUMENTS FOR EACH INSTRUCTION INTO A STRING ARRAY
		string str(childBuf);
		
		int seekArgSize = 3;
		int currArg = 0;
		string seekArgs[seekArgSize];
		for (int i = 0; i < str.length(); i++) {
			if (str[i] != ',') {
				seekArgs[currArg] += str[i];
			}
			else {
				currArg++;
			} 	
		}
		
		// CONVERT EACH STRING IN seekArgs[] INTO AN INT
		int seekArgInt[seekArgSize];
		
		for (int i = 0; i < seekArgSize; i++) {
			string argString = seekArgs[i];
			int argInt;
			argInt = stoi(argString);
			seekArgInt[i] = argInt;
		}
		
		// SET UP FILE DESCRIPTOR AND BEGIN READING THE prose.txt FILE
		int fd;
		
		if ((fd = open(argv[2], O_RDONLY, 0)) == -1){
			cout << "Can't open input file \n" ;
			return 2;
		} 
		
		char outputBuf[SIZ];
		
		// EXECUTE LSEEK BASED ON THE ARGUMENTS
		// seekArgInt[0] = i, [1] = j, [2] = k
		
		int i = seekArgInt[0];
		int j = seekArgInt[1];
		int k = seekArgInt[2];
	
		// EXECUTE DIFFERENT LSEEK CALLS DEPENDING ON THE VALUE OF i
		if (i == 0) {
			if (lseek(fd, j, SEEK_SET) < 0) {
				cout << "Lseek failed";
				cout << "Errno: " << errno << ", Error String: " << strerror(errno) << "\n";
				return 7;
			} 
			if (n = read(fd, outputBuf, k) < 0) {
				cout << "Read failed";
				return 6;
			} 
		} else if (i == 1) {
			if (lseek(fd, j, SEEK_CUR) < 0) {
				cout << "Lseek failed";
				cout << "Errno: " << errno << ", Error String: " << strerror(errno) << "\n";
				return 7;
			} 
			if (n = read(fd, outputBuf, k) < 0) {
				cout << "Read failed";
				return 6;
			}
		} else if (i == 2) {
			if (lseek(fd, j, SEEK_END) < 0) {
				cout << "Lseek failed";
				cout << "Errno: " << errno << ", Error String: " << strerror(errno) << "\n";
				return 7;
			} 
			if (n = read(fd, outputBuf, k) < 0) {
				cout << "Read failed";
				return 6;
			}
		}
		
		// CONVERT OUTPUTBUF TO STRING AND DISPLAY
		string str1(outputBuf);
		
		// CREATING A MESSAGE QUEUE FROM CHILD TO GRANDCHILD
		cout << ("Child: Before mq_open...\n");
	
		struct mq_attr attr1;
		attr1.mq_maxmsg = MSG_CAPACITY;
		attr1.mq_msgsize = MSG_SIZE;
		
		// OPEN ANOTHER MESSAGE QUEUE 
		mqd_t mqd1 = mq_open("/myQueueGurdev1", O_CREAT | O_RDWR, 0644, &attr1);
		if (mqd1 == (mqd_t) -1)
	      {
	      cout << "Parent:  mq_open error\n";
	      cout << "errno is " << errno << " and string is " << strerror(errno);
	      mq_unlink("/myQueueGurdev1");
	      return 2;
	      }
		
		// FORK AGAIN TO CREATE A GRANDCHILD PROCESS
		int id1 = fork();
		
		if (id1 < 0) { //FORK FAILED
			cout << "Fork failed";
			cout << "Errno: " << errno << ", Error String: " << strerror(errno) << "\n";
		} else if (id1 == 0) { //GRANDCHILD PROCESS
			printf("Grandchild process ID is:\t%d\n", getpid());
			
			// RECEIVING THE MESSAGES FROM CHILD AND STORING THEM IN grandChildBuf
			char grandChildBuf[MSG_SIZE];
			unsigned int prio;
		
			ssize_t numRead = mq_timedreceive(mqd1, grandChildBuf, MSG_SIZE, &prio, NULL);
			
			// ERROR HANDLING FOR MQ_TIMEDRECEIVE
			if (numRead == -1) {
			  	printf("Grandchild: mq_read error\n");
			  	printf("Errno: %d; Error: %s\n", errno, strerror(errno));
			  	return 5;
				}
			else {
				string str2(grandChildBuf);
				cout << "Grandchild output: " << str2 << endl;
			}
			
			// END OF GRANDCHILD PROCESS
		} else { //CHILD PROCESS
		
			// SEND MESSAGES TO GRANDCHILD
			printf("Child process ID is:\t%d\n", getpid());
			struct timespec timeout = {0, 0};
			unsigned int prio;
			
			int sendReturnCode = mq_timedsend(mqd1, outputBuf, MSG_SIZE, 1, &timeout);
			
			//ERROR HANDLING FOR MQ SENDING
			if (sendReturnCode == -1) {
				cout << "Parent: mq_send error\n";
				cout << "Errno: " << errno << ", Error String: " << strerror(errno) << "\n";
				return 4;
			} 
			
			// CLOSE THE MESSAGE QUEUE
			if (mq_close(mqd1) == -1) { 
		  		cerr << "Parent: close error" << endl;
		  		cout << "Errno: " << errno << ", Error String: " << strerror(errno) << "\n";
		   		mq_unlink("/myQueueGurdev1");
		  		return 3;
				}
			
	
			// UNLINK THE MESSAGE QUEUE
			if (mq_unlink("/myQueueGurdev1") == -1)
			  	{ 
		  		cerr << "Parent: mq_unlink error" << endl;
		  		cout << "Errno: " << errno << ", Error String: " << strerror(errno) << "\n";
				}
			
			// WAIT FOR GRANDCHILD TO TERMINATE AND THEN RETURN	
			wait(NULL);
			write(STDOUT_FILENO, "Child: Grandchild terminated.. Done waiting...\n", 43);
			return 0;
			}
		
	// END OF CHILD	
	} else { // PARENT PROCESS
		printf("Parent process ID is:\t%d\n", getpid());

		// WRITE INSTRUCTIONS TO MESSAGE QUEUE FROM PARENT TO CHILD
		struct timespec timeout = {0, 0};
		unsigned int prio;

		int i = 0;
	
		// PRINT MESSAGE QUEUE INFO
		if (mq_getattr(mqd, &ret_attr) == -1)
      		{
      		printf("Parent: mq_getattr error");
      		return 2;
      		}
		
		// SEND CHARACTER ARRAY OF INSTRUCTION TO MESSAGE QUEUE
		string currString = instructions[i];
		cout << "Parent output: " << currString << endl;
		char currStringBuf[currString.length()+1];
		strcpy(currStringBuf, currString.c_str());
		int sendReturnCode = mq_timedsend(mqd, currStringBuf, instructions[i].length(), 1, &timeout);
		
		// MQ_TIMEDSEND ERROR HANDLING
		if (sendReturnCode == -1) {
			cout << "Parent: mq_send error\n";
			cout << "Errno: " << errno << ", Error String: " << strerror(errno) << "\n";
			return 4;
		} 
		
		// PRINTING MESSAGE QUEUE INFO
		if (mq_getattr(mqd, &ret_attr) == -1)
      		{
      		printf("Parent: mq_getattr error");
      		return 2;
      		}
		
		// CLOSE THE MESSAGE QUEUE
		if (mq_close(mqd) == -1) 
			{ 
	  		cerr << "Parent: close error" << endl;
	  		cout << "Errno: " << errno << ", Error String: " << strerror(errno) << "\n";
	   		mq_unlink("/myQueueGurdev");
	  		return 3;
			}

		// UNLINK THE MESSAGE QUEUE
		if (mq_unlink("/myQueueGurdev") == -1)
		  	{ 
	  		cerr << "Parent: mq_unlink error" << endl;
	  		cout << "Errno: " << errno << ", Error String: " << strerror(errno) << "\n";
			}
		
		// WAIT FOR CHILD TO TERMINATE AND THEN RETURN	
		wait(NULL);
		write(STDOUT_FILENO, "Parent: Child terminated.. Done waiting...\n", 43);
		return 0;
			
	} //END OF PARENT
	
	// CLOSE FILE DESCRIPTOR
	// END PROGRAM
	
	close(f1);	
	
	return 0;
} 