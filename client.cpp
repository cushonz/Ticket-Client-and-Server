#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fstream>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <iostream>
using namespace std;

/* Projessor Vajda! I just wanted to leave this comment to say thank you for your help with this
lab as well as your general willingness to help and answer students questions. */


// The only thing I believe I left out from the instructions was the timeout.

// Function prototypes
void listenReply(int seconds);
void menu();
void purchaseOrder(int* coord);


// One gigabyte buffer size
#define BUFFER_SIZE 1024
// Define port number

char messBuff[BUFFER_SIZE];

// Variables to be read from setting.ini
char* IP; 
int port;
int timeout;


int sockfd,n;
struct sockaddr_in serv_addr;
bool incomplete;
int xSeat = -1;
int ySeat = -1;


// Command line argument assigned at run time
char * mode;

//Connection file for reading IP, Port, and timeout.
string settingFile;


// Helper function used in retriveSettings(), respomsible for validating the length of the connection file.
bool validateFile(){
    int length = 0;
    string line;
    ifstream myfile (settingFile);
    if (myfile.is_open())
        while ( getline (myfile,line) )
        length++;
    myfile.close();
    if (length != 4)
        return false;
    else 
        return true;
    
}



/* A method responsible for retriving connection information from the supplied file
 if the file is not the correct length the system will display a warning message and exit*/

void retriveSettings(string settingFile){
    if (validateFile){
             int length = 0;
            string line;
            string settingArr[3]; //Setting string arr
            ifstream myfile ("connection.ini");
            if (myfile.is_open())
            {
                getline(myfile,line); // skip first line[connection]
                getline(myfile,line); // Grab IP line
                settingArr[0] = line.substr(5,13); // Strip IP = 
                getline(myfile,line); // Grab Port
                settingArr[1] = line.substr(7,11); // Strip port = 
                getline(myfile,line); // Grab timeout
                settingArr[2] = line.substr(10,12);
                
                IP = (char*)settingArr[0].c_str();
                port = stoi(settingArr[1]);
                timeout = stoi(settingArr[2]);
            }
                myfile.close();
    } else
    cout << "UNABLE TO READ CONNECTION FILE"<< endl;
}
    
int cord[2];
//Request dimensions from the ticket server and save in order to avoid 
//
void askDimension(){
    // Request X Dimension
    sprintf(messBuff,"DIMX");
    write(sockfd,messBuff,sizeof(messBuff)-1);
    read(sockfd, cord, 8);
    xSeat = cord[0];
    ySeat = cord[1];
}

/* General purpose menu function responsible for handling
user input and calling important methods*/

void menu(char* mode){


    if (strcmp(mode, "automatic") == 0){
        int toBuy[2];
        if (ySeat < 1 || xSeat < 1){
            askDimension(); // sets xSeat and ySeat
            srand(time(NULL)); // Set random seed

            toBuy[0] = rand() % (xSeat) + 1;
            toBuy[1] = rand() % (ySeat) + 1;

            purchaseOrder(toBuy);
        }
        else{
            toBuy[0] = rand() % (xSeat);
            toBuy[1] = rand() % (ySeat);
            purchaseOrder(toBuy);
        }
            
    } else if (strcmp(mode, "manual")==0){ // Fully functional DONT TOUCH
        cout << "Ticket X Position: ";
        cin  >> cord[0]; // input for x
        cout << "Ticket Y position: ";
        cin >> cord[1]; // input for y
        if (cord[0] > -1 && cord[1] > -1)
            purchaseOrder(cord); // Attempt to purchase the designated ticket
        else 
            cout << "Bad input! Try"<endl;
        
    }
}

/* 
A method responsible for communicating the desire to 
purchase as well as the desire ticket location
*/

void purchaseOrder(int* coord){
    // Load inital command into buffer
    sprintf(messBuff,"PURCHASE ORDER");
    // Write intial command across socket
    write(sockfd,messBuff, sizeof(messBuff));
    // Load desired ticket position
    // Send delisten(listenfd, 10);sired position to server
    write(sockfd,coord, 8);
    //cout << "output: "<< listenReply(10) <<endl;
    n = read(sockfd, messBuff, sizeof(messBuff)-1);
    cout << messBuff<<endl;
    if (strcmp(messBuff,"SOLD OUT!") == 0){
        incomplete = false;
    } else{
        incomplete = true;
        //n = read(sockfd, messBuff, sizeof(messBuff)); // Read for
    }
                 
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

    incomplete = true;

    // Make sure the user is following the correct format
    if (argc < 2){
        cout << "Usage: " << argv[0] << " ";
        cout << "<mode> <settingsFile.ini>"<<endl; 
        exit(0);
    }

    mode = argv[1];
    settingFile = argv[1];
    
    // Initialize variables related to connection settings
    retriveSettings(settingFile);  

    while (incomplete)
    {    
        messBuff[BUFFER_SIZE];
    
        // initalize all values in messBuff to 0
        memset(messBuff, '0',sizeof(messBuff)); 

        if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0){ // Create the socket if anything less than 0; error
            printf("\n Error : Could not create socket \n");
            return 1;
        }

        memset(&serv_addr, '0', sizeof(serv_addr));  // fill server address with 0s 

        serv_addr.sin_family = AF_INET;  //set server domain
        serv_addr.sin_port = htons(port); // set server port
        
        serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

        
        if(inet_pton(AF_INET, (const char*)IP, &serv_addr.sin_addr)<0){ // run inet pton and error catch
            printf("\n inet_pton error occured\n");
            cout << IP<< endl;
            return 1;
        } 

        
        if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) // lastly connect and error catch 
        {
        printf("\n Error : Connect Failed \n");
        return 1;
        } 
            int sleepTime = rand() % 7+1;
            menu(mode);
            sleep(sleepTime); // Random sleep time as per instructions
        }
        
    cout << "Out of tickets."<<endl;
}