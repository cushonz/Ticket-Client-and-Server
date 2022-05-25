#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <string>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <arpa/inet.h>
using namespace std;

// Referenced from code demos provided in class
#define BUFFER_SIZE 1024
#define PORT_NUMBER 5437

// Default values if none are given
int xSeats = 5;
int ySeats = 8; 
int s[] = {xSeats,ySeats};
int ** ticketArray;
int connfd,listenfd;
char sendBuff[BUFFER_SIZE];

/* A method responsible for selling the requested ticket provided in the
char array 'coord'*/

bool sellTicket(int* coord){
    int x = coord[0]; // Char to int conversion
    int y = coord[1]; // Char to int conversion
    if (x > (xSeats-1) || y > (ySeats-1)){ // Check if ticket selection is out of bounds
        printf("%s", "Ticket out of range");
        return false; // Indicate that the sale failed
    }
        else {
            if (ticketArray[x][y] == 1) {// If ticket is available
                ticketArray[x][y] = 0; // Sell ticket and set to unavailable
                return true; // Indicate that the ticket was succesfully sold
            } else 
                return false; // Indicate that the sale failed
        }
}


// Helper function referenced from stack overflow to print the matrix in a clean manner 
//https://stackoverflow.com/questions/69962700/print-2d-array-as-a-method-in-c

void printMatrix(int** matrix) {
    cout<<"\n Available Seats : \n";
    for(int i=0 ; i<=xSeats-1 ; i++) {
        for(int j=0 ; j<=ySeats-1 ; j++)
            cout<< *(*(ticketArray+i)+j)<<" ";
        cout<<endl;
    }
    cout<<endl;
}

/* Generates a matrix of Dimension x Dimension
   Which is then populated with 1's*/

int** genMatrix(){
    
    srand(time(0));
    int** ticketArray = 0;
    ticketArray = new int*[xSeats];


    // Populate ticket array with 1s and intialize each row
    for (int i = 0; i < xSeats; i++){
        ticketArray[i] = new int[ySeats];
        for (int j = 0; j < ySeats; j++){
            ticketArray[i][j] = 1;
        }
    }
    return ticketArray; // Return array filled with ones
}

/*A simple method to check if tickets are sold out.
This method will be responsible for stopping the loop on the client-side.*/

bool checkDone(){
    int counter=0;
    for (int i = 0; i < xSeats; i++)
        for (int j = 0; j < ySeats; j++)
            if (ticketArray[i][j] == 0)
                counter++;
    if (counter == (xSeats*ySeats))
        return true;
    else
        return false;
}


int cord[2];
int main(int argc, char *argv[]){

    // If dimensions were passed in
    if (argc >= 3){
        // Get X dimension from sys line
        xSeats = atoi(argv[1]);     
        // Get Y dimension from sys line
        ySeats = atoi(argv[2]);
    }

    // Allocate memory for the ticketArray and populate with 1's
    ticketArray = genMatrix();

    cout << endl; // space
    
    // Random seed
    srand(time(NULL));

    // Setting up socket variables
    listenfd = 0; 
    connfd = 0;
    struct sockaddr_in serv_addr; 

    //buffer to load messages into
    sendBuff[BUFFER_SIZE];

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff)); 

    // Setting Server address parameters
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT_NUMBER); 

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

    // Listen
    listen(listenfd, 10);
    char xdim,ydim;
    xdim = (char)xSeats;
    ydim = (char)ySeats;
    char dims[] = {xdim,',',ydim};

    while(1)
        {   
            checkDone();
            // Display available tickets
            printMatrix(ticketArray);   
            // Accept connection
            connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
            // Read command from receiver
            read(connfd, sendBuff, sizeof(sendBuff));
            // Print buffer
            printf("%s",sendBuff);


            /* This outer for loop will evaluate an inital message
            after determining what action is being requested, await secondary message from the client
            that will indicate what seats the client is interested in*/

            if (strcmp(sendBuff,"PURCHASE ORDER") == 0){
                read(connfd, cord, sizeof(cord));
                cout << endl << "Selling :" << cord[0] << "," << cord[1] <<endl;
                if (sellTicket(cord)){
                    // Send a message to confirm the purchase was succesful 
                    sprintf(sendBuff, "Succesfully purchased tickets");
                    write(connfd,sendBuff,sizeof(sendBuff));
                }else{
                    // Send a message to indicate that the purchase failed
                    if (!checkDone()){ // Is it because we're sold out completely?
                        sprintf(sendBuff, "FAILED! Ticket not available");
                        write(connfd,sendBuff,sizeof(sendBuff));
                    } else {
                        sprintf(sendBuff, "SOLD OUT!"); // indicate sold out
                        write(connfd,sendBuff,sizeof(sendBuff)); // Send flag
                        close(connfd);
                        exit(0);
                    }
                } 
            } else if (strcmp(sendBuff,"DIMX") == 0){
                int toBuy[2];
                memset(toBuy,0,sizeof(toBuy));
                write(connfd,s, sizeof(s)-1);
                read(connfd,sendBuff,sizeof(sendBuff)-1); // Reads PURCHASE ORDER
                read(connfd,toBuy,8); // Im unsure why this was neccesary 
                read(connfd,toBuy,8);
                sellTicket(toBuy);
                write(connfd, sendBuff,sizeof(sendBuff)-1);
            }   
        }
      }
        
    // While connected
     
