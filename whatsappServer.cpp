//
// Created by yadids1 on 6/17/18.
//

#include <iostream>
#include "whatsappServer.h"
#define SOCKET "socket"
#define BIND "bind"
#define ACCEPT "accept"
#define SELECT "select"
#define EXITMACRO "EXIT"
#define RECV "recv"
#define SENDMACRO "Send"
#define USEDMSG "name already in use"
#define SUCCESS "Success"
#define FAIL "Failed"


int main(int argc, char* argv[]){
    if(argc != 2){
        print_server_usage();
        exit(1);
    }
    try {
        portNum = stoi(argv[1]);
    } catch(exception & e) {
        print_server_usage();
        exit(1);
    }
    FD_ZERO(&clientsfds);
    FD_SET(STDIN_FILENO, &clientsfds);
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        print_error(SOCKET, errno);
        return -1;
    }
    FD_SET(serverSocket, &clientsfds);
    sa.sin_addr.s_addr = INADDR_ANY;
    memset(&sa, 0, sizeof(struct sockaddr_in));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(portNum);
    maxClients = (serverSocket > STDIN_FILENO) ? serverSocket : STDIN_FILENO;
    if(bind(serverSocket, (struct sockaddr*) &sa, sizeof(struct sockaddr_in)) < 0){
        close(serverSocket);
        print_error(BIND, errno);
        return -1;
    }
    listen(serverSocket, MAX_WAIT_QUEUE + 2);
    while(1){
        readfds = clientsfds;
        if(select(maxClients + 2, &readfds, NULL, NULL, NULL) < 0){
            print_error(SELECT, errno);
            return -1;
        }
        if(FD_ISSET(serverSocket, &readfds)){
            msgHolder newInput;
            int newSocket = receive(serverSocket);
            if (newSocket == -1) {
                return -1;
            }
            processMessage(newSocket, newInput);
            if(newInput.msg == "Error")
                continue;
            if(!isValidName(newInput.msg))
            {
                WAmsg usedMsg;
                memset(&usedMsg, 0, sizeof(WAmsg));
                strcpy(usedMsg.content ,USEDMSG);
                if (send(newSocket, &usedMsg, sizeof(usedMsg), 0) < 0){
                    print_error(SENDMACRO, errno);
                }
            }
            else {
                WAclient newClient;
                newClient.name = newInput.msg;
                fds.push_back(newSocket);
                newClient.fd = newSocket;
                clients.push_back(newClient);
                FD_SET(newSocket, &clientsfds);
                maxClients = (maxClients > newSocket) ? maxClients : newSocket;
                print_connection_server(newClient.name);
                WAmsg created;
                memset(&created, 0, sizeof(WAmsg));
                strcpy(created.content ,SUCCESS);
                if (send(newSocket, &created, sizeof(created), 0) < 0){
                    print_error(SENDMACRO, errno);
                }
            }
        }
        string input;
        if(FD_ISSET(STDIN_FILENO, &readfds)){
            cin >> input;
            if(input == EXITMACRO){
                print_exit();
                terminateAll();
                exit(0);
            }
        } else {
            checkInputFds(readfds);
        }
    }
}


void terminateAll(){
    WAmsg exitAll;
    memset(&exitAll, 0, sizeof(WAmsg));
    strcpy(exitAll.content ,EXITMACRO);
    for (auto it = clients.begin(); it != clients.end(); it++){
        if (send((*it).fd,&exitAll , sizeof(exitAll), 0) < 0){
            print_error(SENDMACRO, errno);
        }
    }
    clients.clear();
    groups.clear();
}

int receive(int socketNum){
    int t;
    if ((t = accept(socketNum, NULL, NULL)) < 0)
    {
        print_error(ACCEPT, errno);
        return -1;
    }
    return t;
}


void checkInputFds(fd_set readfds){
    for (int fd: fds){
        if(FD_ISSET(fd, &readfds))
        {
            msgHolder temp;
            temp.senderName = getNameByFd(fd);
            processMessage(fd, temp);
            readMsg(temp);
        }
    }
}


void processMessage(int fd, msgHolder& buf){
    ssize_t br = 0;
    WAmsg inputMsg;
    memset(&inputMsg, 0, sizeof(WAmsg));
    br = recv(fd, &inputMsg ,sizeof(inputMsg), 0);
    if ( br < 1)
    {
        print_error(RECV, errno);
        strcpy(inputMsg.content, "Error");
    }
    buf.msg = inputMsg.content;
}


string getNameByFd(int fd){
    for(auto it = clients.begin(); it != clients.end(); it++) {
        if ((*it).fd == fd) {
            return (*it).name;
        }
    }
    return "Error";
}


void readMsg(msgHolder buf){
    command_type commandType;
    string name;
    string message;
    vector<string> groupClients;
    parse_command(buf.msg, commandType, name, message, groupClients);
    if (commandType == CREATE_GROUP){
        createGroup(name, groupClients, buf.senderName);
    } else if (commandType == SEND){
        sendMSG(name, message, buf.senderName);
    } else if (commandType == WHO){
        whoCommand(buf.senderName);
    } else if (commandType == EXIT){
        exitClient(buf.senderName);
    } else {
        print_invalid_input();
    }
}


void createGroup(string name, vector<string>& clientsList, string senderName) {
    bool isValid = isValidName(name);
    if (isValid) {
        unordered_set<string> memberSet(clientsList.begin(), clientsList.end());
        memberSet.emplace(senderName);
        if (checkGroupMembers(memberSet)) {
            WAgroup newGroup;
            newGroup.name = name;
            vector<string> newClientsList(memberSet.begin(), memberSet.end());
            newGroup.groupMembers = newClientsList;
            groups.push_back(newGroup);
            WAmsg groupToCre;
            memset(&groupToCre, 0, sizeof(WAmsg));
            strcpy(groupToCre.content ,SUCCESS);
            if (send(getFdByName(senderName), &groupToCre, sizeof(groupToCre), 0) < 0){
                print_error(SENDMACRO, errno);
            }
            print_create_group(true, true, senderName, name);
            return;
        }
    }
    print_create_group(true, false, senderName, name);
    int fd = getFdByName(senderName);
    WAmsg groupFail;
    memset(&groupFail, 0, sizeof(WAmsg));
    strcpy(groupFail.content ,FAIL);
    if (send(fd, &groupFail, sizeof(groupFail), 0) < 0){
        print_error(SENDMACRO, errno);
    }
}


bool isValidName(string name) {
    for(auto it = clients.begin(); it != clients.end(); it++){
        if ((*it).name == name){
            return false;
        }
    }
    for(auto it = groups.begin(); it != groups.end(); it++){
        if ((*it).name == name){
            return false;
        }
    }
    return true;
}


bool checkGroupMembers(unordered_set<string> memberList)
{
    if(memberList.size() < 2){
        return false;
    }
    for (auto it = memberList.begin(); it != memberList.end(); it++){
        if(!isExistClient(*it)){
            return false;
        }
    }
    return true;
}


int getFdByName(string name){
    for(auto it = clients.begin(); it != clients.end(); it++) {
        if ((*it).name == name) {
            return (*it).fd;
        }
    }
    return -1;
}


bool isExistClient(string name){
    for (auto it = clients.begin(); it != clients.end(); it++){
        if ((*it).name == name){
            return true;
        }
    }
    return false;
}


WAgroup* getGroupByName(string name){
    for (auto it = groups.begin(); it != groups.end(); it++){
        if ((*it).name == name){
            return &(*it);
        }
    }
    return nullptr;
}


bool isInGroup(WAgroup group, string name){
    for (auto it = group.groupMembers.begin(); it != group.groupMembers.end(); it++){
        if ((*it) == name){
            return true;
        }
    }
    return false;
}


void sendMSG(string name, string message, string senderName){
    int fds = getFdByName(senderName);
    int fd = getFdByName(name);
    string msgToSend = senderName + ": " + message;
    if(fd != -1){
        WAmsg temp;
        memset(&temp, 0, sizeof(WAmsg));

        strcpy(temp.content, msgToSend.c_str());
        if (send(fd, &temp, sizeof(temp), 0) < 0){
            print_error(SENDMACRO, errno);
            return;
        }
        print_send(true, true, senderName, name, message);
        string msg = SUCCESS;
        strcpy(temp.content, msg.c_str());
        if (send(fds, &temp, sizeof(temp), 0) < 0){
            print_error(SENDMACRO, errno);
        }
        return;
    }
    WAgroup* myGroup = getGroupByName(name);
    if (myGroup != nullptr){
        if(isInGroup(*myGroup, senderName)){
            for (auto it = myGroup->groupMembers.begin(); it != myGroup->groupMembers.end(); it++){
                if ((*it) != senderName){
                    fd = getFdByName(*it);
                    WAmsg sendGroup;
                    memset(&sendGroup, 0, sizeof(WAmsg));
                    strcpy(sendGroup.content , msgToSend.c_str());
                    if (send(fd, &sendGroup, sizeof(sendGroup), 0) < 0){
                        print_error(SENDMACRO, errno);
                        return;
                    }
                }
            }
            print_send(true, true, senderName, name, message);
            WAmsg sendGroupS;
            memset(&sendGroupS, 0, sizeof(WAmsg));
            strcpy(sendGroupS.content , SUCCESS);
            if (send(fds, &sendGroupS, sizeof(sendGroupS), 0) < 0){
                print_error(SENDMACRO, errno);
            }
            return;
        }
    }
    print_send(true, false, senderName, name, message);
    WAmsg failSend;
    memset(&failSend, 0, sizeof(WAmsg));
    strcpy(failSend.content , FAIL);
    if (send(fds, &failSend, sizeof(failSend), 0) < 0){
        print_error(SENDMACRO, errno);
    }
    return;
}


void whoCommand(string senderName){
    print_who_server(senderName);
    int fd = getFdByName(senderName);
    vector<string> curClient;
    for(auto it = clients.begin(); it != clients.end(); it++){
        curClient.push_back((*it).name);
    }
    sort(curClient.begin(), curClient.end());
    string allClients = "";
    for(auto it = curClient.begin(); it != curClient.end(); it++){
        allClients += (*it) + ',';
    }
    WAmsg whoMsg;
    memset(&whoMsg, 0, sizeof(WAmsg));
    string tempStr =allClients.substr(0, allClients.size() -1);
    strcpy(whoMsg.content ,tempStr.c_str());
    if (send(fd, &whoMsg, sizeof(whoMsg), 0) < 0){
        print_error(SENDMACRO, errno);
    }
}


void exitClient(string senderName){
    int fd = getFdByName(senderName);
    for (auto it = clients.begin(); it != clients.end(); it++) {
        if ((*it).name == senderName) {
            clients.erase((it));
            break;
        }
    }
    for (auto it = groups.begin(); it != groups.end(); it++){
        for(auto iter = (*it).groupMembers.begin(); iter != (*it).groupMembers.end(); iter++){
            if((*iter) == senderName){
                (*it).groupMembers.erase(iter);
                break;
            }
        }
    }
    fds.erase(remove(fds.begin(), fds.end(), fd), fds.end());
    print_exit(true, senderName);
    WAmsg exitMsg;
    memset(&exitMsg, 0, sizeof(WAmsg));
    strcpy(exitMsg.content , SUCCESS);
    if (send(fd, &exitMsg, sizeof(exitMsg), 0) < 0){
        print_error(SENDMACRO, errno);
    }
}