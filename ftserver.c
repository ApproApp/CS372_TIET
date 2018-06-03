#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <stdint.h>
#include <dirent.h>
#include <signal.h>
#include <netdb.h>

#define BUFFER 1024 // msg buffer
#define MAXBUFFER 8192 // file read buffer (larger than BUFFER)
#define HANDLE "ftserver" //define handle

// Function Declarations

int checkExit(char *buffer, char *handle);
int readFile(char *fileName, char *string);
void sigintHandle(int sigNum);
int sendMsg(int sockfd, char *msg);
int getLine(char *buffer, char *handle);
int recvMsg(int, char*, unsigned);
unsigned recvNum(int sockfd);
void removeNewline(char *string);
int sendResponse(char *outgoing, char *message, int client_sock);
int startUp(int port);
unsigned sendNum(int sockfd, unsigned num);
int verifyPort(char *input);
int max(int a, int b);
int sendFile(char* file, int client_data_sock, int client_sock);
int handleRequest(int client_sock, char *buffer);

// FUNCTION DEFINITIONS

// input a client request, parse it, and
// gives an integer code representing the request type
int handleRequest(int client_sock, char *buffer){
    int size = recvNum(client_sock);    // get the size of the upcoming message
    int r = recvMsg(client_sock, buffer, size);        // get the command
    buffer[size] = '\0';

    // return a code for the request type
    if(strncmp("-g", buffer, strlen(buffer)) == 0){
        return 2;
    } else if(strncmp("-l", buffer, strlen(buffer)) == 0) {
        return 1;
    } else {
        return -1;
    }
}

// receives input two integers, gives the larger of them
int max(int a, int b){
    if(a >= b){
        return a;
    }
    else return b;
}

// receives a filename and a string pointer
// opens the file, reads in the contents, and writes them to a string
// gives 0 if successful, -1 if unsuccessful
int readFile(char *fileName, char *string){

        // open the file
        FILE *fileptr;
        fileptr = fopen(fileName, "rb");
        fseek(fileptr, 0, SEEK_END);
        long fsize = ftell(fileptr);
        fseek(fileptr, 0, SEEK_SET);

        char *buffer = malloc(fsize + 1);
        fread(buffer, fsize, 1, fileptr);
        buffer[fsize] = '\0';	// make sure it's terminated
        
        fclose(fileptr);
        strncpy(string, buffer, MAXBUFFER);
        return 0;
    }

// receives a file buffer, data socketfd, control sockfd
// sends the file to the client, then closes the data connection
int sendFile(char* file, int client_data_sock, int client_sock){
    int r = sendNum(client_data_sock, (strlen(file)));      // send the size
    r = sendMsg(client_data_sock, file);            // send the reponse
    printf("File Sent: Closing Data Connection to Client\n");
    close(client_data_sock);            // close the data line
}


// receives an outgoing message buffer, a message, data sockfd, control sockfd
// copies the message into the buffer and sends it to the client
// then closes the data connection
int sendResponse(char *outgoing, char* message, int client_sock){
    strncpy(outgoing, message, BUFFER);			// copy the message
    int r = sendNum(client_sock, (strlen(outgoing)));	// send the size
    r = sendMsg(client_sock, outgoing);		// send the reponse
}

// catch exiting children
void sigintHandle(int sigNum){
    pid_t pid;
    int status;
    pid = waitpid(-1, &status, WNOHANG);
}

// gets a line of user input from stdin, removes the trailing newline,
// and puts it in a buffer
int getLine(char *buffer, char *handle){
    printf("%s$ ", handle);
    fgets(buffer, (BUFFER -1), stdin);
    removeNewline(buffer);
    return 0;
}

// gets the user's input and checks if they want to quit
int checkExit(char *buffer, char *handle){
    getLine(buffer, handle);
    if(strncmp(buffer, "\\quit", 5) == 0){
        printf("Exiting chat application\n");
        return 1;
    }
    else return 0;
}

// receives a socket file descriptor, output string, and
// size of message
// reads until entire message has been received, then
// copies data to output string
// gives -1 if failure, 0 if success
int recvMsg(int sockfd, char *output, unsigned size){
	char buffer[BUFFER];
	unsigned r;
	unsigned total = 0;

	while(total < size){
		r = read(sockfd, buffer+total, size-total);
		total += r;

		if(r < 0){
			return -1;
		}

		else if(r == 0){
			total = size - total;
		}
	}

	strncpy(output, buffer, size);

	return 0;
}

// receives a socket file descriptor and a message
// to be sent. sends until entire message has
// been transferred. gives -1 on failure, or
// 0 on success
int sendMsg(int sockfd, char *msg){
	// status variable
	unsigned r;

	// size of message
	unsigned size = strlen(msg)+1;

	// total bytes sent
	unsigned total = 0;

	// while not done sending
	while(total < size) {
		// send message starting where it left off
		r = write(sockfd, msg+total, size-total);

		// add bytes sent to total
		total += r;

		// error out
		if(r < 0){
			return -1;
		}

		// completed
		else if(r == 0){

			total = size - total;
		}
	}

	return 0;
}

// receives a socket file descriptor and
// an integer, sends the integer over
// the socket. gives -1 on failure,
// 0 on success
unsigned sendNum(int sockfd, unsigned num){
	unsigned realnum = num;
	int r = write(sockfd, &realnum, sizeof(unsigned));
	if(r < 0){
		return -1;
	}
	else return 0;
}

// receives a socket file descriptor and receives
// an integer over the socket.
// gives -1 on failure, the integer on success
unsigned recvNum(int sockfd){
	unsigned num;
	int r = read(sockfd, &num, sizeof(unsigned));
	if(r < 0){
		return -1;
	}
	else return num;
}

// receives a port number
// opens and binds a socket and sets the server to listen
// gives -1 if any action fails or the socket file
// descriptor otherwise
int startUp(int port){
	int sockfd;

	// failure
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		return -1;
	}
	
	// some necessary structs and info
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = INADDR_ANY;


        int optval = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

	// failure
	if(bind(sockfd, (struct sockaddr *) &server, sizeof(server)) < 0){
		return -1;
	}

	// failure
	if(listen(sockfd, 10) < 0){
		return -1;
	}

	// success
	return sockfd;
} 

// receives a string and removes
// the trailing newline character
// if present
void removeNewline(char *string){
        // strip newlines
        int last = strlen(string) - 1;
        if(string[last] == '\n') {
                string[last] = '\0';
        }
}

// receives an input string
// converts it to an integer and
// verifies it's > 1024 and < 65535
// gives -1 if error, else port number
int verifyPort(char *input){
	int port = atoi(input);
	if(port < 1024 || port > 65535){
		return -1;
	}
	return port;
}

// MAIN

int main(int argc, char *argv[]){
    signal(SIGCHLD, sigintHandle); // start the signal handler

    char buffer[BUFFER];        // buffer for messages
    char message[BUFFER];       // string for recv'd messages
    char outgoing[BUFFER];	// string for outgoing messages
    int size;                   // size of message expected/sent
    int r;                      // status of message send/receive
    int dataport;		// port number for data connections

    // check the cmd line args
    if(argc != 2){
        fprintf(stderr, "\nusage:\n $ ftserver [port number]\n\n");
        exit(0);
    }

    // verify the port # supplied
    int port = verifyPort(argv[1]);
    if(port < 0){
        fprintf(stderr, "Port Invalid; Acceptable Range: 1024 - 65535\n");
        exit(1);
    }

    // open/bind/listen on the socket
    int sockfd = startUp(port);
    if(sockfd < 0){
        fprintf(stderr, "Error: Failed Listening on Port %d", port);
        exit(1);
    }
    printf("Starting Server; Listening on Port %d\n", port);

    int pid, status;		// used by children

    // always listen for connections
    while(1){
        int client_sock = accept(sockfd, NULL, NULL);

        // failed connection; close it
        if(client_sock < 0){
            fprintf(stderr, "Error: Failed Accepting Connection\n");
            close(client_sock);

        // good connection; use it
        } else {
            // fork a process
            pid = fork();

            // failed fork
            if(pid < 0){
                printf("Error Forking Process; Terminating");
                exit(1);
            }

            // child process
            else if(pid == 0){
                printf("Client Has Connected; Starting Session\n");

                int requestNum = handleRequest(client_sock, message);
                printf("Request # is: %d\n", requestNum);

                dataport = recvNum(client_sock);		// get the port number to use for data connection

                // got a complete message
                if(r == 0){
                    // open the new port for data
                    int client_data_fd = startUp(dataport);
                    printf("Opening Data Connection on Port %d\n", dataport);

                    // listen on it for the connection
                    int client_data_sock = accept(client_data_fd, NULL, NULL);

                    // (-l) list command
                    if(requestNum == 1){
                        char directory[BUFFER];

                        // get the full path for the current working directory
                        getcwd(buffer, BUFFER);


                        // http://www.thegeekstuff.com/2012/06/c-directory/
                        /*************************************************************************************************************************************************/
                        // get the directory contents
                        DIR *dirpointer = NULL;
                        dirpointer = opendir(buffer);
                        struct dirent *dirstruct = NULL;
                        while((dirstruct = readdir(dirpointer)) != NULL){
                            // filter the "." and ".." listings and build the file list
                            if((strncmp(dirstruct->d_name, ".", strlen(dirstruct->d_name)) != 0) && (strncmp(dirstruct->d_name, "..", strlen(dirstruct->d_name)) != 0)){
                                strncat(directory, "[", 1);
                                strncat(directory, dirstruct->d_name, strlen(dirstruct->d_name));
                                strncat(directory, "] ", 2);
                            }
                        }
                        /*************************************************************************************************************************************************/

                        // send response on control connection to rec'v on data connection
                        sendResponse(outgoing, "DATA", client_sock);
                        // send the file list
                        sendResponse(outgoing, directory, client_data_sock);
                        close(client_sock); // just to be sure
                        _exit(0);

                    // (-g) get command
                    } else if (requestNum == 2){
                        // get the filename into the 'message' string
                        size = recvNum(client_sock);
                        r = recvMsg(client_sock, message, size);
                        message[size] = '\0';               // add a NULL (python doesn't do this?)
                        printf("Client Has Requested [%s]\n", message);

                        // validate the file exists
                        // get the full path for the current working directory
                        getcwd(buffer, BUFFER);

                        // get the directory contents
                        DIR *dirpointer = NULL;
                        dirpointer = opendir(buffer);
                        struct dirent *dirstruct = NULL;
                        int validFile = 0;
                        while((dirstruct = readdir(dirpointer)) != NULL){
                            // compare the filename supplied to all filenames present to ensure it exists
                            if(strncmp(message, dirstruct->d_name, max(strlen(dirstruct->d_name), strlen(message))) == 0){
                                validFile = 1;	// flag it found
                            }
                        }

                        // file exists
                        if(validFile == 1){
                            printf("[%s] is a Valid Filename\n", message);
                            // read the file into a buffer
                            char filebuffer[MAXBUFFER];
                            readFile(message, filebuffer);

                            // send the response on control connection to rec'v on data connection
                            sendResponse(outgoing, "DATA", client_sock);

                            // send the file over the data port
                            sendFile(filebuffer, client_data_sock, client_sock);
                            printf("Transfer Complete\n");
                            close(client_sock); // just to be sure
                            _exit(0);

                        // file does not exist
                        } else {
                            printf("[%s] is an Invalid Filename\n", message);
                            // send error message on control connection
                            sendResponse(outgoing, "Requested File Does Not Exist; Please Try Again", client_sock);
                            close(client_sock); // just to be sure
                            _exit(0);
                        }
                    
                    // erroneous command; send error message on control connection, close socket, exit child
                    } else {
                        sendResponse(outgoing, "Invalid Command; Please Try Again", client_sock);
                        close(client_sock); // just to be sure
                        _exit(0);
                    }


                // didn't get a complete message (unkown error); error out
                } else {
                    fprintf(stderr, "There Was an Error Receiving the Message\n");
                    close(client_sock); // just to be sure
                    _exit(0);
                }

            // parent process
            } else {
                // nothing
            }
        }
    }
    
    return 0;
}