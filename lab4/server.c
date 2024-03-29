/*
** selectserver.c -- a cheezy multiperson chat server
*/

#include "includes.h"

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
    fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number

    int listener;     // listening socket descriptor
    int newfd;        // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;

    char buf[256];    // buffer for client data
    int nbytes;

    char remoteIP[INET6_ADDRSTRLEN];

    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    int i, j, rv;

    struct addrinfo hints, *ai, *p;

    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);

    // get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }
    
    for(p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) { 
            continue;
        }
        
        // lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    // if we got here, it means we didn't get bound
    if (p == NULL) {
        fprintf(stderr, "selectserver: failed to bind\n");
        exit(2);
    }

    freeaddrinfo(ai); // all done with this

    // listen
    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(3);
    }

    // add the listener to the master set
    FD_SET(listener, &master);

    // keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one


    char all_user[MAXDATASIZE][MAXDATASIZE];
    int all_user_fd[MAXDATASIZE];
    memset(&all_user, (unsigned long)'\0', MAXDATASIZE);
    memset(&all_user_fd, (unsigned long)0, MAXDATASIZE);
    for(int i = 0; i < MAXDATASIZE; ++i){
        memset(&all_user[i], (unsigned long)'\0', MAXDATASIZE);
    }

    // main loop
    while(true) {
        read_fds = master; // copy it
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }

        // run through the existing connections looking for data to read
        for(i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) { // we got one!!
                if (i == listener) {
                    // handle new connections
                    addrlen = sizeof remoteaddr;
                    newfd = accept(listener,
                        (struct sockaddr *)&remoteaddr,
                        &addrlen);

                    if (newfd == -1) {
                        perror("accept");
                    } else {
                        FD_SET(newfd, &master); // add to master set
                        if (newfd > fdmax) {    // keep track of the max
                            fdmax = newfd;
                        }
                        printf("selectserver: new connection from %s on "
                            "socket %d\n",
                            inet_ntop(remoteaddr.ss_family,
                                get_in_addr((struct sockaddr*)&remoteaddr),
                                remoteIP, INET6_ADDRSTRLEN),
                            newfd);
                    }
                } else {
                    // handle data from a client
                    if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0) {
                        // got error or connection closed by client
                        if (nbytes == 0) {
                            // connection closed
                            printf("selectserver: socket %d hung up\n", i);
                        } else {
                            perror("recv");
                        }
                        close(i); // bye!
                        FD_CLR(i, &master); // remove from master set
                    } 
                    else {
                        printf("Message received: %s\n", buf);
                        
                        char command;
                        char username[MAXDATASIZE];
                        char message[MAXDATASIZE];

                        memset(&username, (unsigned long)'\0', MAXDATASIZE);
                        memset(&message, (unsigned long)'\0', MAXDATASIZE);
                        command = buf[0]; // a,bbbb,ccccc

                        int d;
                        for(d = 2; d < MAXDATASIZE; ++d){
                            if(!ispunct(buf[d])){
                                username[d-2] = buf[d];
                                //printf("here is " + i);
                            }
                            else {
                                break;
                            }
                        }
                        int k;
                        for(k = d; k < MAXDATASIZE; ++k){
                            if(!ispunct(buf[k])){
                                message[k] = buf[k];
                            }
                            else {
                                break;
                            }
                        }
                      


                        if (command == '1'){
                            for(int e = 0; e<MAXDATASIZE; ++e){
                                if (all_user[e][0] == '\0'){
                                    strcpy(all_user[e], username);
                                    all_user_fd[e] = i;
                                    break;
                                }
                            }
                            for(j = 0; j <= fdmax; j++) {
                                // send to everyone!
                                if (FD_ISSET(j, &master)) {
                                    // except the listener and ourselves
                                    if (j != listener && j != i) {
                                        if (send(j, buf, nbytes, 0) == -1) {
                                            perror("send");
                                        }
                                    }
                                }
                            }
                        }
                        else if (command == '2'){
                            char messageToClient[MAXDATASIZE];
                            memset(messageToClient, '\0', MAXDATASIZE);
                            strcat(messageToClient, "2,");
                            for( int e = 0; e < MAXDATASIZE; e ++){
                                if (all_user[e][0] != '\0'){
                                    strcat(messageToClient, all_user[e]);
                                    strcat(messageToClient,"\n");
                                }
                            }
                            printf("Message send: %s\n", messageToClient);
                            send(i, messageToClient, MAXDATASIZE, 0);
                        }
                        else if (command == '3'){
                            int targetSockFd = 0;
                            for( int e = 0; e < MAXDATASIZE; e ++){
                                if (strcmp(all_user[e], username) == 0){
                                    targetSockFd = all_user_fd[e];
                                }
                            }
                            if (send(targetSockFd, buf, nbytes, 0) == -1) {
                                perror("send");
                            }
                        }
                        else if (command == '4') {
                            // we got some data from a client
                            for(j = 0; j <= fdmax; j++) {
                                // send to everyone!
                                if (FD_ISSET(j, &master)) {
                                    // except the listener and ourselves
                                    if (j != listener && j != i) {
                                        if (send(j, buf, nbytes, 0) == -1) {
                                            perror("send");
                                        }
                                    }
                                }
                            }
                        }
                        else if (command == '5'){
                            for( int e = 0; e < MAXDATASIZE; e ++){
                                if (strcmp(all_user[e], username) == 0){
                                    memset(all_user[e], (unsigned long)'\0', MAXDATASIZE);
                                }
                            }
                            for(j = 0; j <= fdmax; j++) {
                                // send to everyone!
                                if (FD_ISSET(j, &master)) {
                                    // except the listener and ourselves
                                    if (j != listener && j != i) {
                                        if (send(j, buf, nbytes, 0) == -1) {
                                            perror("send");
                                        }
                                    }
                                }
                            }
                        }
                        else;
                    }
                } // END handle data from client
            } // END got new incoming connection
        } // END looping through file descriptors
    } // END for(;;)--and you thought it would never end!
    
    return 0;
}