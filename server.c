#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>

#define TRUE   1
#define FALSE  0

#define MAX_CLIENTS 100
int CLIENTS[MAX_CLIENTS];
struct sockaddr_in ADDRESS;

void CHECK(int returnVal) {
  if (returnVal < 0) {
    perror("Failed!");
    exit(EXIT_FAILURE);
  }
}


int startServer(int port) {

  int serverSocket;

  ADDRESS.sin_family = AF_INET;
  ADDRESS.sin_addr.s_addr = INADDR_ANY;
  ADDRESS.sin_port = htons(port);

  CHECK((serverSocket = socket(AF_INET , SOCK_STREAM , 0)));

  int opt = 1;
  //set master socket to allow multiple connections.
  if(setsockopt(serverSocket, SOL_SOCKET,
                SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
  {
      perror("setsockopt");
      exit(EXIT_FAILURE);
  }

  CHECK(bind(serverSocket, (struct sockaddr *)&ADDRESS, sizeof(ADDRESS)));
  return serverSocket;
}


int handleNewConnection(int serverSocket) {
  int newSocket, i, addrlen;
  char *message = "Hello From Server.";

  addrlen = sizeof(ADDRESS);
  if ((newSocket = accept(serverSocket,
    (struct sockaddr *)&ADDRESS, (socklen_t*)&addrlen))<0)
  {
      perror("accept");
      exit(EXIT_FAILURE);
  }

  //inform user of socket number - used in send and receive commands
  printf("New connection , socket fd is %d , ip is : %s , port : %d \
        \n" , newSocket , inet_ntoa(ADDRESS.sin_addr) , ntohs(ADDRESS.sin_port));

  //send new connection greeting message
  if(send(newSocket, message, strlen(message), 0) != strlen(message) )
  {
      perror("send");
  }

  puts("Welcome message sent successfully");

  //add new socket to array of sockets
  for (i = 0; i < MAX_CLIENTS; i++)
  {
      //if position is empty
      if(CLIENTS[i] == 0)
      {
          CLIENTS[i] = newSocket;
          printf("Adding to list of sockets as %d\n" , i);
          break;
      }
  }

  return 1;
}


int handleClientActivity(int clientSd, int clientSdIndex) {
  int valread, addrlen;
  char buffer[1025];

  addrlen = sizeof(ADDRESS);

  if ((valread = read(clientSd, buffer, 1024)) == 0)
  {
      printf("Closing client %d\n", clientSd);
      close(clientSd);
      CLIENTS[clientSdIndex] = 0;
  } else {
      printf("sending message to client.....\n");
      buffer[valread] = '\0';
      send(clientSd, buffer, strlen(buffer), 0);
  }

  return 1;
}


int listenForConnection(int serverSocket) {
  CHECK(listen(serverSocket, MAX_CLIENTS));
  fd_set readfds;
  int maxSd, sd, i, activity;

  while (TRUE) {
    FD_ZERO(&readfds);

    //add master socket to set
    FD_SET(serverSocket, &readfds);
    maxSd = serverSocket;

    //add child sockets to set
    for ( i = 0; i < MAX_CLIENTS ; i++) {
        //socket descriptor
        sd = CLIENTS[i];

        //if valid socket descriptor then add to read list
        if(sd > 0)
            FD_SET( sd , &readfds);

        //highest file descriptor number, need it for the select function
        if(sd > maxSd)
            maxSd = sd;
    }

    activity = select(maxSd + 1 , &readfds , NULL , NULL , NULL);
    if ((activity < 0) && (errno!=EINTR)) {
        printf("select error");
    }

    if (FD_ISSET(serverSocket, &readfds)) {
      handleNewConnection(serverSocket);
    }

    //else its some IO operation on some other socket
    for (i = 0; i < MAX_CLIENTS; i++) {
        sd = CLIENTS[i];

        if (FD_ISSET( sd , &readfds)) {
          handleClientActivity(sd, i);
        }
    }
  }

  return 1;
}

int main(int argc , char *argv[])
{
  int masterSocket;
  CHECK((masterSocket = startServer(atoi(argv[1]))));
  CHECK(listenForConnection(masterSocket));
  return 1;
}
