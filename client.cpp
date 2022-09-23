#include <stdio.h>
#include <stdlib.h>
/* You will to add includes here */
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <iostream>

using namespace std;

void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET)
  {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    printf("Invalid intput.\n");
    exit(1);
  }
  char delim[] = ":";
  char *Desthost = strtok(argv[1], delim);
  char *Destport = strtok(NULL, delim);
  if (Desthost == NULL || Destport == NULL)
  {
    printf("Invalid intput.\n");
    exit(1);
  }

  int rv;
  int sock;
  char s[INET6_ADDRSTRLEN];
  char buf[10000];
  int numbytes;
  string userInput;

  struct addrinfo hint, *servinfo, *p;
  memset(&hint, 0, sizeof(hint));
  hint.ai_family = AF_UNSPEC;
  hint.ai_socktype = SOCK_STREAM;

  rv = getaddrinfo(Desthost, Destport, &hint, &servinfo);
  if (rv != 0)
  {
    //Getaddrinfo faild
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    exit(1);
  }

  //Loop trough all the information and try to make a sockt
  for (p = servinfo; p != NULL; p->ai_next)
  {
    sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (sock < 0)
    {
      //Could not make socket try again
      continue;
    }
    //If we made it here we succesfully created a socket. Exit loop
    break;
  }
  freeaddrinfo(servinfo);
  //Check if the for loop did not make a socket
  if (p == NULL)
  {
    printf("Faild to create socket.\n");
    exit(1);
  }
  //We now have a socket so we try to connect to the server
  if (connect(sock, p->ai_addr, p->ai_addrlen) < 0)
  {
    //Faild to connect
    printf("Could not connect.\n");
    close(sock);
    exit(1);
  }

  inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof(s));
  printf("client: connecting to %s\n", s);
  //We are now connected to the server
  //First we need to recive a message from the server with ist protocol. Ex "Hello 1.0\n"
  memset(buf, 0, sizeof(buf));
  numbytes = recv(sock, buf, sizeof(buf), 0);
  if (numbytes < 0)
  {
    //Did not recive
    printf("Could not recive.\n");
    close(sock);
    exit(1);
  }
  printf("Server: %s\n", buf);
  if (strcmp("1.0\n", buf) == 0)
  {
    //We support this protocol
    printf("Protocol supported.\n");
    numbytes = send(sock, "OK\n", strlen("OK\n"), 0);
  }
  else
  {
    //we dont support this protocol
    printf("Protocol NOT supported.\n");
    close(sock);
    exit(1);
  }

  fd_set master;
  fd_set read_fds;
  FD_ZERO(&master);
  FD_ZERO(&read_fds);
  FD_SET(0, &master);
  FD_SET(sock, &master);
  int nfds = sock;
  int r;

  //In this stage we can send and recive messages to/from the server
  while (true)
  {
    read_fds = master;

    if (select(nfds + 1, &read_fds, NULL, NULL, NULL) == -1)
    {
      printf("select.\n");
      close(sock);
      exit(1);
    }
    if (FD_ISSET(nfds, &read_fds))
    {
      //Got something from server
      memset(buf, 0, sizeof(buf));
      numbytes = recv(sock, buf, sizeof(buf), 0);
      if (numbytes <= 0)
      {
        printf("Could not recive.\n");
        close(sock);
        exit(1);
      }
      printf("%s", buf);
    }
    if (FD_ISSET(0, &read_fds))
    {
      //Client is writing something
      getline(cin, userInput);
      if (strlen(userInput.c_str()) > 2255)
      {
        printf("Message is too long.\n");
      }
      else
      {
        //printf("Sending: %s\n", userInput.c_str());
        string temp = userInput+"\n";
        numbytes = send(sock, temp.c_str(), temp.length(), 0);
        if (numbytes < 0)
        {
          printf("Could not send.\n");
          close(sock);
          exit(1);
        }
      }
    }
  }

  return 0;
}