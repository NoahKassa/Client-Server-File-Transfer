// Noah Kassa
// COSC 3360-10751 Spring semester 2024
// —Operating System Fundamentals
// Second Spring 2024 Assignment: A Simple File Repository

//compiled and ran in VScode using its terminal
//compile and start running AFTER server program

//how to run:
//this code takes 2 arguments, the host name and the port number
//host should be localhost and port number must match server 

//ex: ./client localhost 4658


#include <iostream> 
#include <string.h>
#include <stdio.h>
#include <fstream>
#include <stdlib.h>
#include <sys/socket.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <netdb.h>
#include <unistd.h> 
#include <sys/stat.h>
#include <cstring>
#include <string>
#include <bits/stdc++.h>
#include <fcntl.h>
#include <filesystem>

using namespace std;
using namespace std::filesystem;

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

//just a menu function to make the while loop in main more readable
void Menu() {
    cout << endl;
    cout << "******************************************************" << endl;
    cout << "WELCOME TO THE FILE REPOSITORY. PLEASE PICK AN OPTION" << endl;
    cout << "******************************************************" << endl;
    cout << "1: Get a file" << endl;
    cout << "2: Exit client program" << endl;
    cout << "3: Terminate server" << endl;
    cout << endl;
}

//sends the fileName to the server to check if it exists in the repository
void GetFileName(int& clientSocket, string fileName)
{
    const char* message = fileName.c_str(); 
    send(clientSocket, message, strlen(message), 0); 
}

//sends a terminate message to the server so both programs will end
void Terminate(int& clientsock)
{
    const char* term = "TERMINATE";
    send(clientsock, term, strlen(term),0);
}

//sends the server an exit message so it can disconnect the client and keep running
void Exit(int &clientsock)
{
    const char* exit = "EXIT";
    send(clientsock, exit, strlen(exit),0);
}

int main(int argc,char  *argv[]) {
   
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    //gets the file path of the program so that it can transfer files easier
    path path = std::filesystem::current_path();
    string clientPath = path.string();
 
    //sets up the buffer to have 1024 bytes
    char buffer[1024];

    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    //takes the 2nd argument and assigns it to the port number variable
    portno = atoi (argv[2]);    

    //gets the hosts name from the arguments and will exit if host cannot be found
    server = gethostbyname(argv[1]);
    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
    serv_addr.sin_port = htons(portno);

    

    //this while loop will keep the client connected and running until either exit or terminate is chosen
   
    char choice;//tracks the menu inputs of the use 
    int fd; //used to transfer the requested file to the client
    string fileName; // used to hold and store the fileName input by user
    do
    {
        //checks if the socket succesfully oppened and connected
        sockfd = socket (AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
            error("ERROR opening socket");

        if (connect (sockfd, (struct sockaddr *) &serv_addr, sizeof (serv_addr)) < 0)
            error("ERROR connecting");
        
        //displays choice menu
        Menu();

        cin>>choice;
        //a switch case for each option available
        switch (choice)
        {
        case '1':
        {
            // prompts the user to enter the name of the file
            cout << "Enter the file name:" << endl;
            cin >> fileName;
            
            //requests the file from the server it to the server 
            //the server sees if file is in the repository and will send the data if it is
            GetFileName(sockfd,fileName);
            cout << endl;

            

            //client then waits for the server to respond
            n = read(sockfd, buffer, 1024) ;

            //catches read errors and ends the client 
            if (n < 0) error ("ERROR reading from socket");

            //will print out the file data if the file exists
            string check = buffer;
            if (check  != "ERROR File Not Found")
            {
                //open a new file from the path to copy the contents of the server file. 
                //overwrites and previously existing files of the same path
                string filePath = clientPath + "/" + fileName;
                fd = open(filePath.c_str(), O_WRONLY|O_CREAT,0600);

                //outputs the file name and the amount of bytes read to terminal
                cout << "Got file " <<fileName << "(" << n << " bytes)";

                //write contents to the new file
                write(fd,buffer,n);
            }
            else
            {
                cout << "File Does Not Exist! Please try again"<<endl;
            }
            n = -1;
        }    
            break;
        case '2':
            Exit(sockfd);
            break;
 
        case '3':
            //sends the server the terminate signal
            Terminate(sockfd);
            cout << "Goodbye!"<<endl;
            break;
        
        default:
            cout << "command not recognized please enter 1, 2, or 3."<<endl;
            break;
        }
        
    } while (choice != '2' && choice != '3');
    
    //erases the buffer and closes the socket to end the program
    bzero(buffer,1024);

    close(sockfd);
    return 0;
    
}