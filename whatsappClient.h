//
// Created by yadids1 on 6/17/18.
//

#ifndef EX4_WHATSAPPCLIENT_H
#define EX4_WHATSAPPCLIENT_H

#include "whatsappio.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <regex>

using namespace std;

struct sockaddr_in sa;
struct hostent* hp;
fd_set readfds;
fd_set tempReadfds;
int clientSocket;
string clientName;
string serverAddress;
int numPort;
regex reg("[A-Za-z0-9]{1,30}");
struct msgHolder{
    string msg;
};
struct WAmsg{
    char content[WA_MAX_MESSAGE + WA_MAX_MESSAGE];
};


int main(int argc, char* argv[]);
bool isValidCName(string name);
void processMessage(int fd, msgHolder& buf);
void waitForMessage(int clientSocket);
void readMsg(msgHolder buf);
bool communicateToServer(msgHolder& buf);
bool createGroup(vector<string> groupClients);


#endif //EX4_WHATSAPPCLIENT_H
