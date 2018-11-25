//
// Created by yadids1 on 6/17/18.
//

#include <unordered_set>
#include <iostream>
#include "whatsappClient.h"
#define SOCKET "socket"
#define CONNECT "connect"
#define SENDMACRO "Send"
#define SELECT "select"
#define RECV "recv"
#define USEDMSG "name already in use"
#define SUCCESS "Success"
#define FAIL "Failed"
#define EXITMACRO "EXIT"


int main(int argc, char* argv[]) {
    if(argc != 4){
        print_fail_connection();
        exit(1);
    }
    try {
        clientName = argv[1];
        serverAddress = argv[2];
        numPort = stoi(argv[3]);
    } catch(exception & e) {
        print_fail_connection();
        exit(1);
    }
    if (!isValidCName(clientName)){
        print_fail_connection();
        exit(1);
    }
    inet_pton(AF_INET, serverAddress.c_str(), &(sa.sin_addr));
    if ((hp = gethostbyname(serverAddress.c_str())) == NULL){
        print_fail_connection();
        exit(1);
    }
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        print_error(SOCKET, errno);
        exit(1);
    }
    memset(&sa, 0, sizeof(sa));
    memcpy((char*)&sa.sin_addr, hp->h_addr_list[0], hp->h_length);
    sa.sin_family = hp->h_addrtype;
    sa.sin_port = htons((u_short)numPort);
    if (connect(clientSocket, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
        close(clientSocket);
        print_error(CONNECT, errno);
        exit(1);
    }
    FD_ZERO(&readfds);
    FD_SET(clientSocket, &readfds);
    FD_SET(STDIN_FILENO, &readfds);
    FD_ZERO(&tempReadfds);
    FD_SET(clientSocket, &tempReadfds);
    FD_SET(STDIN_FILENO, &tempReadfds);
    WAmsg nameToCreate;
    memset(&nameToCreate, 0, sizeof(WAmsg));
    strcpy(nameToCreate.content ,clientName.c_str());
    ssize_t result = send(clientSocket, &nameToCreate, sizeof(nameToCreate), 0);
    if (result < 0){
        print_error(SENDMACRO, errno);
        exit(1);
    }
    tempReadfds = readfds;
    if(select(clientSocket + 1, &tempReadfds, NULL, NULL, NULL) < 0){
        print_error(SELECT, errno);
        exit(1);
    }
    if(FD_ISSET(clientSocket, &readfds)){
        msgHolder buf;
        processMessage(clientSocket, buf);
        if(buf.msg == "Error"){
            print_fail_connection();
            exit(1);
        }
        if(buf.msg == USEDMSG){
            print_dup_connection();
            exit(1);
        } else if(buf.msg == SUCCESS) {
            print_connection();
        } else {
            printf("error join massage\n");
        }
    }
    waitForMessage(clientSocket);
}


void waitForMessage(int clientSocket){
    while(1){
        tempReadfds = readfds;
        if(select(clientSocket + 1, &tempReadfds, NULL, NULL, NULL) < 0){
            print_error(SELECT, errno);
            exit(1);
        }
        if(FD_ISSET(STDIN_FILENO, &tempReadfds)){
            msgHolder buf;
            getline(cin, buf.msg);
            if(buf.msg == "Error") {
                print_fail_connection();
                exit(1);
            }
            readMsg(buf);
        }
        if(FD_ISSET(clientSocket, &tempReadfds)){
            msgHolder buf2;
            processMessage(clientSocket, buf2);
            if(buf2.msg == "Error"){
                print_fail_connection();
                exit(1);
            } else if(buf2.msg == EXITMACRO){
                close(clientSocket);
                exit(0);
            } else{
                cout << buf2.msg << endl;
            }
        }
    }
}


bool createGroup(vector<string> groupClients){
    unordered_set<string> memberSet(groupClients.begin(), groupClients.end());
    memberSet.emplace(clientName);
    if(memberSet.size() < 2){
        return false;
    }
    return true;
}


void readMsg(msgHolder buf){
    command_type commandType;
    string name = "";
    string message = "";
    vector<string> groupClients;
    parse_command(buf.msg, commandType, name, message, groupClients);
    if (commandType == CREATE_GROUP){
        if(!isValidCName(name) || !createGroup(groupClients) || !communicateToServer(buf)){
            print_create_group(false, false, clientName, name);
            return;
        }
        print_create_group(false, true, clientName, name);
    } else if (commandType == SEND){
        if(strlen(message.c_str()) > WA_MAX_MESSAGE)
        {
            print_send(false, false, clientName, name, message);
            return;
        }
        if(!isValidCName(name) || name == clientName || !communicateToServer(buf)){
            print_send(false, false, clientName, name, message);
            return;
        }
        print_send(false, true, clientName, name, message);
    } else if (commandType == WHO){
        if(!communicateToServer(buf)){
            cout << buf.msg << endl;
        } else {
            printf("ERROR: failed to receive list of connected clients.\n");
        }
    } else if (commandType == EXIT){
        communicateToServer(buf);
        close(clientSocket);
        exit(0);
    } else {
        print_invalid_input();
    }
}


bool communicateToServer(msgHolder& buf){
    WAmsg toSend;
    memset(&toSend, 0, sizeof(WAmsg));
    strcpy(toSend.content ,buf.msg.c_str());
    if(send(clientSocket, &toSend, sizeof(toSend), 0) < 0)
    {
        print_error(SENDMACRO, errno);
        exit(1);
    }
    ssize_t br = 0;
    WAmsg incoming;
    memset(&incoming, 0, sizeof(WAmsg));
    br = recv(clientSocket, &incoming, sizeof(incoming),0);
    if(br < 1)
    {
        print_error(RECV, errno);
        strcpy(incoming.content, "Error");
    }
    if(strcmp(incoming.content, SUCCESS) == 0){
        return true;
    }
    buf.msg = incoming.content;
    return false;
}


bool isValidCName(string name){
    if(!regex_match(name, reg)) {
        return false;
    }
    return true;
}


void processMessage(int fd, msgHolder& buf){
    ssize_t br = 0;
    WAmsg msgToPRC;
    memset(&msgToPRC, 0, sizeof(WAmsg));
    br = recv(fd, &msgToPRC,sizeof(msgToPRC), 0);
    buf.msg = msgToPRC.content;
    if ( br < 1) {
        print_error(RECV, errno);
        buf.msg = "Error";
    }
}