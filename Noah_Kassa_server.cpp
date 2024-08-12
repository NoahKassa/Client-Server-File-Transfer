// Noah Kassa
// COSC 3360-10751 Spring semester 2024
// —Operating System Fundamentals
// Second Spring 2024 Assignment: A Simple File Repository

//compiled and ran in VScode using its terminal
//compile the server cpp before the client
//make sure the Repository folder has a capital R

//how to run:
// the only argument is the port number the user wants
//ex: ./server 4658

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string.h>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <filesystem>

using namespace std;
using namespace std::filesystem;

//gives easy to read error messages 
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    
    //the current file path to server so it will be easier to get the filepath to the repository and requested file
    path serverPath = std::filesystem::current_path();
    string path = serverPath.string();
   

    int sockfd, newsockfd, portno;
    //stores the size of the address of the client. this is needed for the accept system call
    socklen_t clilen;

    //a buffer to send and recieve messages
    char buffer[1024];

    //catches incorrect argument inputs
    int n;
    if (argc<2)
    {
        fprintf(stderr,"ERROR, NO PORT GIVEN\n");
        exit(1);
    }
    
    
    //create the socket
    sockfd = socket(AF_INET,SOCK_STREAM,0);
    if (sockfd < 0)
    {
        error("ERROR opening socket");
    }

    //a sockaddr_in is a structure containing an internet address. this structure is defined in netinet/in.h
    struct sockaddr_in serv_addr, cli_addr;
    
    //1 - pointer to the buffer
    //sets it to zero 
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    //build off each other
    serv_addr.sin_family = AF_INET; // code for the address family
    serv_addr.sin_addr.s_addr = INADDR_ANY; //the IP address of the host


    serv_addr.sin_port = htons (portno); //converts the port number in host byte order to a port number in network byte order.
    if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    listen (sockfd,5);
    clilen = sizeof(cli_addr);


    //the server while loop that will keep waiting for the client to send a file name to respond until it gets the message to exit or terminate
    bool terminate =false;
    while (!terminate)
    {
        cout <<endl << "WAITING FOR REQUEST"<<endl;

        newsockfd = accept (sockfd,(struct sockaddr *) &cli_addr,&clilen);
        if (newsockfd < 0)
            error("ERROR On Accepting");
        
        //reads the given file name and stores it in the buffer
        bzero (buffer, 1024);
        n = read (newsockfd, buffer, 1023) ;
        if (n < 0) error ("ERROR Reading From Socket");


        //if it is a terminate command the server will output something to terminal and end 
        string check = buffer;
        if (check == "TERMINATE")
        {
            cout << "Terminating server. Goodbye!"<<endl;
            terminate = true;
            break;
        }
        //if the command is exit then the server will print a message to the terminal and close the socket 
        else if(check == "EXIT")
        {
            cout << "a client has left the sever"<<endl;
            close(newsockfd);
            break;
        }
        
        //adding the repository and file name to the server path for the files path
        string filePath = path + "/Repository/" + buffer;
        int fd = open(filePath.c_str(),O_RDONLY);
        cout << "Client requesting the file "<<buffer<< " ";


        //checks if the file given is in the repository if not the server notifys the client and loops 
        if (fd < 0) {
            n = write(newsockfd,"ERROR File Not Found", 21);
        }
        else
        {
            //gets the amount of bytes in the file and outputs it to terminal
            int bytesRead = read(fd,buffer,1024);
            printf("Sent back %u bytes\n",bytesRead);

            //Send the file contents to the client
            n = write (newsockfd, buffer, bytesRead);
            if (n < 0) error ("ERROR writing to socket");
        }
        close (newsockfd);
    }

    //close the socket and end the program
    close (sockfd);
    return 0;
}