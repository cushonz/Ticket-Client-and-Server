#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <iostream>
using namespace std;

// Function prototypes
void listenReply(int seconds);
void menu();
void purchaseOrder(char* coord);


// One gigabyte buffer size
#define BUFFER_SIZE 1024
// Define port number
#define PORT_NUMBER 5437

char messBuff[BUFFER_SIZE];
int sockfd,n;
struct sockaddr_in serv_addr;
bool incomplete;
int xSeat = 5;
int ySeat = 5;
char xC,yC;


void askDimension(){
    //Request X Dimension
    sprintf(messBuff,"DIM");
    write(sockfd,messBuff,sizeof(messBuff));
    listenReply(10);
    xC = messBuff[0];
    yC = messBuff[2];
    cout << "XC"<<xC<<endl;
    

    
}

/* General purpose menu function responsible for handling
user input and calling important methods*/

void menu(){
    int reply; // User input goes here
    cout << "1.) Automatic mode" <<endl; // Attempt to purchases random tickets until they are sold out
    cout << "2.) User Select" << endl; //  Allows users to select specific tickets for sale
    cout << "3.) Exit" << endl; // Exit prior to being sold out
    cout << "Select : ";
    cin >> reply;

    if (reply == 1){
        askDimension(); // sets xSeat and ySeat
        int xCoord = rand() % (xSeat-1);
        int yCoord = rand() % (ySeat-1);
        char xC = xCoord;
        char yC = yCoord;
        char coord[] = {xC,',',yC};
        cout << coord;
        purchaseOrder(coord);
    } else if (reply == 2){
        char x;
        char y;
        cout << "Ticket X Position: ";
        cin  >> x; // input for x
        cout << "Ticket Y position: ";
        cin >> y; // input for y
        char coord[] = {x,',',y}; // Combine into expected string
        purchaseOrder(coord); // Attempt to purchase the designated ticket
        
    }
}

/* 
A method responsible for communicating the desire to 
purchase as well as the desire ticket location
*/
void purchaseOrder(char* coord){
    // Load inital command into buffer
    sprintf(messBuff,"PURCHASE ORDER");
    // Write intial command across socket
    write(sockfd,messBuff, sizeof(messBuff));
    // Load desired ticket position
    sprintf(messBuff,"%s",coord);
    // Send delisten(listenfd, 10);sired position to server
    write(sockfd,messBuff, sizeof(messBuff));
    //cout << "output: "<< listenReply(10) <<endl;
    n = read(sockfd, messBuff, sizeof(messBuff)-1);
    if (strcmp(messBuff,"SOLD OUT!") == 0){
        incomplete = false;
    }
    else
        incomplete = true;
        
}

/* A simple function to listen replies from the server
for a specifice amount of time*/

void listenReply(int seconds){

    
    while ( (n = read(sockfd, messBuff, sizeof(messBuff)-1)) > 0 && seconds > 0) // read from socket for the duration of buffer
    {
        messBuff[n] = 0; // assure that the position is intialized to 0
        if(fputs(messBuff, stdout) == EOF)
        {
            printf("\n Error : Fputs error\n");
        }
        sleep(1);
        seconds--;
    }
}

int main(int argc, char* argv[]){

    srand(time(NULL)); // Set random seed
    incomplete = true;
    
    while (incomplete)
    {
        // Needed variables
        sockfd = 0; 
        n = 0;
        messBuff[BUFFER_SIZE];
    

        if (argc != 2){ // Check an ip address is passed in to attempt to connect to

            printf("\n Usage: %s <ip of server> \n",argv[0]);
            return 1;
        }

        // initalize all values in messBuff to 0
        memset(messBuff, '0',sizeof(messBuff)); 

        if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0){ // Create the socket if anything less than 0; error
            printf("\n Error : Could not create socket \n");
            return 1;
        }

        memset(&serv_addr, '0', sizeof(serv_addr));  // fill server address with 0s 

        serv_addr.sin_family = AF_INET;  //set server domain
        serv_addr.sin_port = htons(PORT_NUMBER); // set server port
        
        serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

        
        if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0){ // run inet pton and error catch
            printf("\n inet_pton error occured\n");
            return 1;
        } 

        if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) // lastly connect and error catch 
        {
        printf("\n Error : Connect Failed \n");
        return 1;
        } 
            menu();
    }
        
    cout << "Out of tickets."<<endl;
}