 #include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()

/**
* Client code
* 1. Create a socket and connect to the server specified in the command arugments.
* 2. Prompt the user for input and send that input as a message to the server.
* 3. Print the message received from the server and exit the program.
*/

// Error function used for reporting issues
void error(const char *msg) { 
  perror(msg); 
  exit(0); 
} 

// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address, 
                        int portNumber){
 
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address)); 

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);

  // Get the DNS entry for this host name
  struct hostent* hostInfo = gethostbyname("localhost"); 
  if (hostInfo == NULL) { 
    fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
    exit(0); 
  }
  // Copy the first IP address from the DNS entry to sin_addr.s_addr
  memcpy((char*) &address->sin_addr.s_addr, 
        hostInfo->h_addr_list[0],
        hostInfo->h_length);
}

int main(int argc, char *argv[]) {
  int socketFD, portNumber, textOut, keyOut, charsRead;
  struct sockaddr_in serverAddress;
  char buffer[256];
  // Check usage & args
  if (argc < 4) { 
    fprintf(stderr,"USAGE: %s plaintext key port\n", argv[0]); 
    exit(0); 
  }

  //open files from user input and move htem into variables
  FILE *file_key = fopen(argv[2], "r");
  if (file_key == NULL) {
      fprintf(stderr, "Error: Unable to open file %s\n", argv[2]);
  return 1;
  }

  fseek(file_key, 0, SEEK_END);
  long fileSize = ftell(file_key)+1;
  rewind(file_key);

  //read user input parameter as filepath. Grab contents of file and get size
  FILE *file_input = fopen(argv[1], "r");
  if (file_input == NULL) {
      fprintf(stderr, "Error: Unable to open file %s\n", argv[1]);
  return 1;
  }

  fseek(file_input, 0, SEEK_END);
  long fileSize2 = ftell(file_input)+1;
  rewind(file_input);

  if (fileSize2 > fileSize){ //If your key is longer than input, program returns an error
    fprintf(stderr, "Error: text is longer than key by: %lli\n", fileSize2 - fileSize);
    return 1;
  }

  //Populate strings with information from respective files
  char *keygen = (char *)malloc(fileSize * sizeof(char));
  if (fgets(keygen, fileSize, file_key) == NULL) {
      fprintf(stderr, "Error reading from file %s\n", argv[2]);
      fclose(file_key); // Close the file before returning
      return 1;
  }

  char *inputString = (char *)malloc(fileSize2 * sizeof(char));
  if (fgets(inputString, fileSize2, file_input) == NULL) {
      fprintf(stderr, "Error reading from file %s\n", argv[1]);
      fclose(file_input); // Close the file before returning
      return 1;
  }

  // Create a socket
  socketFD = socket(AF_INET, SOCK_STREAM, 0); 
  if (socketFD < 0){
    error("CLIENT: ERROR opening socket");
  }

   // Set up the server address struct
  setupAddressStruct(&serverAddress, atoi(argv[3]));

  // Connect to server
  if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
    error("CLIENT: ERROR connecting");
  }

  //Combines key and user input into 1 string to be packaged and sent to server
  char *outputString = (char *)malloc((fileSize + fileSize2 + 3) * sizeof(char));
  outputString[0] = '\0'; //strcat looks for null terminator when deciding end of string
  strcat(outputString, inputString);
  strcat(outputString, "2"); //for strtoking in the decoder but I've made it a 2 so it's also a flag that this output comes from a decoder. Encoder has a 1
  strcat(outputString, keygen);

  // Write to the server
  textOut = send(socketFD, outputString, strlen(outputString), 0); 

  if (textOut < 0){
    error("CLIENT: ERROR writing to socket");
  }
  if (textOut < strlen(outputString)){
    printf("CLIENT: WARNING: Not all data written to socket!\n");
  }

  // Clear out the buffer again for reuse
  memset(buffer, '\0', sizeof(buffer));
  
  // Read data from the socket, leaving \0 at end
  charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); 
  if (charsRead < 0){
    error("CLIENT: ERROR reading from socket");
  }
  printf("%s", buffer); //Encoded text sent back from server. I tried to make this really plain so it would be taken into an output file easily

  // Close the socket
  close(socketFD); 

  free(keygen);
  free(inputString);
  free(outputString);

  return 0;
}