//
// Created by yadids1 on 6/17/18.
//

#ifndef EX4_WHATSAPPSERVER_H
#define EX4_WHATSAPPSERVER_H

#include "whatsappio.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <algorithm>
#include <unordered_set>
#define MAX_WAIT_QUEUE 10
using namespace std;


struct WAclient{
    string name;
    int fd;
};

struct msgHolder{
    string msg;
    string senderName;
};
struct WAgroup{
    string name;
    vector<string> groupMembers;
};

struct WAmsg{
    char content[WA_MAX_MESSAGE + WA_MAX_MESSAGE];
};


int portNum;
vector<WAclient> clients;
vector<WAgroup> groups;
struct sockaddr_in sa;
struct hostent* hp;
char myName[WA_MAX_NAME + 1];
int serverSocket;
fd_set clientsfds;
fd_set readfds;
int maxClients;
vector<int> fds;



void checkInputFds(fd_set readfds);
int main(int argc, char* argv[]);
int receive(int socketNum);
void whoCommand(string senderName);
string getNameByFd(int fd);
void readMsg(msgHolder buf);
void processMessage(int fd, msgHolder& buf);
bool isValidName(string name);
int getFdByName(string name);
WAgroup* getGroupByName(string name);
bool isInGroup(WAgroup group, string name);
bool isExistClient(string name);
void terminateAll();
void exitClient(string senderName);
void sendMSG(string name, string message, string senderName);
bool checkGroupMembers(unordered_set<string> memberList);
void createGroup(string name, vector<string>& clientsList, string senderName);

#endif //EX4_WHATSAPPSERVER_H
