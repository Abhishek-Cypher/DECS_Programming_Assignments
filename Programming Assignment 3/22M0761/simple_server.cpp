/* run using ./server <port> */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netinet/in.h>

#include <pthread.h>

#include "http_server.hh"
#include <string>
#include <iostream>
#include <queue>

void error(char *msg)
{
  perror(msg);
  exit(1);
}

queue<int> argq; // SHARED QUEUE
pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condi1 = PTHREAD_COND_INITIALIZER; // SIGNAL IF QUEUE EMPTY
pthread_cond_t condi2 = PTHREAD_COND_INITIALIZER; // SIGNAL IF QUEUE FULL
int reqcnt = 0;

// FUNCTION WHICH THE WORKER THREADS RUN
void *clienthandler(void *arguments)
{
  while (1)
  {
    pthread_mutex_lock(&lock1); // LOCK CRITICAL SECTION
    while (argq.empty())
    {
      pthread_cond_wait(&condi1, &lock1);
    }

    int newsockfd = argq.front();
    argq.pop();
    reqcnt--;
    pthread_cond_signal(&condi2);
    pthread_mutex_unlock(&lock1); // UNLOCK CRITICAL SECTION

    /* accept a new request, create a newsockfd */
    char buffer[256];
    int n;

    // int *ptrargument = (int *)arguments;
    // int newsockfd = *ptrargument;

    bzero(buffer, 256);
    n = read(newsockfd, buffer, 255);

    if (n < 0)
      error("ERROR reading from socket");
      
    // printf("\n\n\nHere is the message: %s", buffer);

    // MY CODE -----------------------------------------
    struct HTTP_Response *responseobject;
    responseobject = handle_request(buffer);

    string display = responseobject->get_string();
    int display_len = display.length();
    // MY CODE -----------------------------------------

    /* send reply to client */
    n = write(newsockfd, display.c_str(), display_len);

    if (n < 0)
      error("ERROR writing to socket");
  }
  return 0;
}

int main(int argc, char *argv[])
{
  int sockfd, newsockfd, portno;
  socklen_t clilen;
  char buffer[256];
  struct sockaddr_in serv_addr, cli_addr;
  int n;

  if (argc < 2)
  {
    fprintf(stderr, "ERROR, no port provided\n");
    exit(1);
  }

  /* create socket */

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    error("ERROR opening socket");

  /* fill in port number to listen on. IP address can be anything (INADDR_ANY)
   */

  bzero((char *)&serv_addr, sizeof(serv_addr));
  portno = atoi(argv[1]);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  /* bind socket to this port number on this machine */

  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    error("ERROR on binding");

  /* listen for incoming connection requests */
  listen(sockfd, 5);

  // WEEK 4 CODE STARTS
  pthread_t wrkt[20];

  for (int i = 0; i < 20; i++)
  {
    int ret = pthread_create(&wrkt[i], NULL, &clienthandler, NULL);
    if (ret != 0)
    {
      printf("pthread_create FAILED\n");
    }
  }

  while (1)
  {
    clilen = sizeof(cli_addr);

    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);

    if (newsockfd < 0)
      error("ERROR on accept");
    else
    {
      pthread_mutex_lock(&lock1); // LOCK CRITICAL SECTION
      while (reqcnt > 100) // TO ACCEPT MAXIMUM 100 REQUESTS ONLY
      {
        pthread_cond_wait(&condi2, &lock1);
      }
      argq.push(newsockfd);
      reqcnt++;
      pthread_cond_signal(&condi1);

      pthread_mutex_unlock(&lock1); // UNLOCK CRITICAL SECTION
    }
  }

  // while (1)
  // {
  //   clilen = sizeof(cli_addr);

  //   newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
  //   if (newsockfd < 0)
  //     error("ERROR on accept");
  //   else
  //   {
  //     pthread_t workerthread;

  //     int arguments = newsockfd;

  //     int ret = pthread_create(&workerthread, NULL, &clienthandler, &arguments);
  //     if (ret != 0)
  //     {
  //       printf("pthread_create FAILED\n");
  //     }
  //   }
  // }
  return 0;
}
