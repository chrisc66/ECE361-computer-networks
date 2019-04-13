/*
** client.c -- a stream socket client demo
*/

#include "includes.h"
int fakeSockFd;
char username[MAXDATASIZE];


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}



void sendToServer(){
    char command[MAXDATASIZE];
    char clientInput[MAXDATASIZE];
    char messageToServer[MAXDATASIZE];
    int numbytes = 0;
    while (true) {
        // get message from user input
        // format --- command,username,message
        memset(&command, (unsigned long)'\0', MAXDATASIZE);
        memset(&clientInput, (unsigned long)'\0', MAXDATASIZE);
        memset(&messageToServer, (unsigned long)'\0', MAXDATASIZE);
        // printf("Enter text: ");
        scanf("%s", command);
        if(strcmp(command, "list") == 0){
            // 2,,
            strcat(messageToServer, "2,");
            strcat(messageToServer, ",");
        }
        else if(strcmp(command, "boardcast") == 0){
            // 4,,message
            scanf("%[^\n]%*c", clientInput);
            strcat(messageToServer, "4,");
            strcat(messageToServer, ",");
            strcat(messageToServer, clientInput);
        }
        else if(strcmp(command, "exit") == 0){
            // 5,self-username,
            strcat(messageToServer, "5,");
            strcat(messageToServer, username);
            strcat(messageToServer, ",");
        }
        else{
            // 3,username,message
            scanf("%[^\n]%*c", clientInput);
            strcat(messageToServer, "3,");
            strcat(messageToServer, command);
            strcat(messageToServer, ",");
            strcat(messageToServer, clientInput);
        }
        // send message to server
        numbytes = send(fakeSockFd, messageToServer, MAXDATASIZE-1, 0);
        if (numbytes == -1) {
            perror("send()\n");
            assert(0);
        }
        if (strcmp(command, "exit") == 0){
            printf("Leaving chat room ...\n");
            close(fakeSockFd);
            exit(0);
        }
    }
    return;
}



void reveiveFromServer(){
    char receiverMessage[MAXDATASIZE];
    char command;
    char message[MAXDATASIZE];
    char username[MAXDATASIZE];
    int numbytes = 0;
    while (true) {
        memset(&receiverMessage, (unsigned long)'\0', MAXDATASIZE);
        memset(&message, (unsigned long)'\0', MAXDATASIZE);
        memset(&username, (unsigned long)'\0', MAXDATASIZE);
        // receive message from server
        numbytes = recv(fakeSockFd, receiverMessage, MAXDATASIZE-1, 0);
        if (numbytes == -1) {
            perror("recv()\n");
            assert(0);
        }
        command = receiverMessage[0];
        printf("\n");
        // printf("\nmessage received: %s\n", receiverMessage);
        receiverMessage[numbytes] = '\0';
        if (command == '1'){    // 1 - new user
            strcpy(username, &receiverMessage[2]);
            printf("-New user: %s\n", username);
        }
        else if (command == '2'){   // 2 - list
            strcpy(message, &receiverMessage[2]);
            printf("-List of users:\n%s", message);
        }
        else if (command == '3'){   // 3 - message to one
            int offset = 0;
            for (int i = 0; i < MAXDATASIZE; i ++){
                if(ispunct(receiverMessage[i]))
                    offset = i;
            }
            for (int i = offset + 1; i < MAXDATASIZE; i ++){
                if(ispunct(receiverMessage[i]))
                    offset = offset + i;
            }
            strcpy(message, &receiverMessage[offset + 1]);
            printf("-%s\n", message);
        }
        else if (command == '4'){   // 4 - message to all
            int offset = 0;
            for (int i = 0; i < MAXDATASIZE; i ++){
                if(ispunct(receiverMessage[i]))
                    offset = i;
            }
            for (int i = offset + 1; i < MAXDATASIZE; i ++){
                if(ispunct(receiverMessage[i]))
                    offset = offset + i;
            }
            strcpy(message, &receiverMessage[offset + 1]);
            printf("-%s\n", message);
        }
        else if (command == '5'){   // 5 - exit
            strcpy(username, &receiverMessage[2]);
            printf("-User leaves: %s\n", username);
        }
        else;
    }
    return;
}



void getUserName(){
    char messageToServer[MAXDATASIZE];
    memset(&username, (unsigned long)'\0', MAXDATASIZE);
    printf("Please Enter Username: ");
    scanf("%[^\n]%*c", username);
    strcat(messageToServer, "1,");
    strcat(messageToServer, username);
    int numbytes = send(fakeSockFd, messageToServer, MAXDATASIZE-1, 0);
    if (numbytes == -1) {
        perror("send()\n");
        assert(0);
    }
    printf("Welcome, %s, you are logged in!\n", username);
    return;
}



int main(int argc, char *argv[])
{
    int sockfd;
    struct addrinfo hints;
    struct addrinfo *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    
    if (argc != 2) {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    // get a hint address information
    // if ((rv = getaddrinfo(SERVER_IP, PORT, &hints, &servinfo)) != 0) {
    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    
    // Setting up connection to server, loop through all the results and connect to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        fakeSockFd = sockfd;
        if (sockfd == -1) {
            perror("client: socket");
            continue;
        }
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }
        break;
    }
    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof(s));
    printf("client: connecting to %s\n", s);
    freeaddrinfo(servinfo); 
    // all done with this structure, connection established

    getUserName();

    // sending / receving message from server
    pthread_t thread1, thread2;
    pthread_create( &thread1, NULL, (void*) &sendToServer, NULL);
    pthread_create( &thread2, NULL, (void*) &reveiveFromServer, NULL);
    
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    close(sockfd);

    return 0;
}