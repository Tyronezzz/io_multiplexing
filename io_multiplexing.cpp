//
//  io_multiplexing
//
//  Created by Tyrone Hou on 11/8/20.
//  Copyright © 2020 Tyrone Hou. All rights reserved.
//  Reference by https://devarea.com/linux-io-multiplexing-select-vs-poll-vs-epoll/#.X6ffzNMzaAy
//               https://www.bilibili.com/video/BV1qJ411w7du?from=search&seid=13845780920790423780
//  Run under Ubuntu 14

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <wait.h>
#include <signal.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/poll.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#define MAXBUF 256


void child_process(void)
{
    sleep(2);
    char msg[MAXBUF];
    struct sockaddr_in addr = {0};
    int n, sockfd, num=1;
    srandom(getpid());
    
    /* Create socket and connect to server */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(2000);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    connect(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    
    printf("child {%d} connected \n", getpid());
    while(1){
        int sl = (random() % 10 ) +  1;
        num++;
        sleep(sl);
        sprintf (msg, "Test message %d from client %d", num, getpid()); // msg = ""
        n = write(sockfd, msg, strlen(msg));    /* Send message */
    }
    
}

int main()
{
    char buffer[MAXBUF];
    int fds[5];
    struct sockaddr_in addr;
    struct sockaddr_in client;
    int addrlen, n,i,max=0;;
    int sockfd, commfd;
    fd_set rset;
    pollfd pollfds[5];
    
    
    for(i=0;i<5;i++)
    {
        if(fork() == 0)          //create a new child process
        {
            child_process();
            exit(0);
        }
    }
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&addr, 0, sizeof (addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(2000);
    addr.sin_addr.s_addr = INADDR_ANY;
    bind(sockfd,(struct sockaddr*)&addr ,sizeof(addr));
    listen (sockfd, 5);
    
    
    
    // select
    for (i=0;i<5;i++)
    {
        memset(&client, 0, sizeof (client));
        addrlen = sizeof(client);
        fds[i] = accept(sockfd,(struct sockaddr*)&client, (socklen_t*) &addrlen);
        if(fds[i] > max)
            max = fds[i];
    }
    
    
    
    while(1){
        FD_ZERO(&rset);
        for (i = 0; i< 5; i++ ) {
            FD_SET(fds[i],&rset);            //reset bitmap
        }
        
        puts("round again");
        select(max+1, &rset, NULL, NULL, NULL);         //select here
        
        for(i=0;i<5;i++) {
            if (FD_ISSET(fds[i], &rset)){
                memset(buffer,0,MAXBUF);
                read(fds[i], buffer, MAXBUF);
                puts(buffer);       //printf
                
            }
        }
    }
    
    
    
    
    //poll
//    for (i=0;i<5;i++)
//    {
//        memset(&client, 0, sizeof (client));
//        addrlen = sizeof(client);
//        pollfds[i].fd = accept(sockfd,(struct sockaddr*)&client, (socklen_t*)&addrlen);
//        pollfds[i].events = POLLIN;
//    }
//    sleep(1);
//
//    while(1){
//        puts("round again");
//        poll(pollfds, 5, 50000);
//
//        for(i=0;i<5;i++) {
//            if (pollfds[i].revents & POLLIN){
//                pollfds[i].revents = 0;
//                memset(buffer,0,MAXBUF);
//                read(pollfds[i].fd, buffer, MAXBUF);
//                puts(buffer);
//            }
//        }
//    }

    
    // epoll
//    struct epoll_event events[5];
//    int epfd = epoll_create(10);
//
//    for (i=0;i<5;i++)
//    {
//        static struct epoll_event ev;
//        memset(&client, 0, sizeof (client));
//        addrlen = sizeof(client);
//        ev.data.fd = accept(sockfd,(struct sockaddr*)&client, (socklen_t*) &addrlen);
//        ev.events = EPOLLIN;
//        epoll_ctl(epfd, EPOLL_CTL_ADD, ev.data.fd, &ev);
//    }
//
//    while(1){
//        puts("round again");
//        int nfds = epoll_wait(epfd, events, 5, 10000);
//
//        for(i=0;i<nfds;i++) {
//            memset(buffer,0,MAXBUF);
//            read(events[i].data.fd, buffer, MAXBUF);
//            puts(buffer);
//        }
//    }
    
    return 0;
}
