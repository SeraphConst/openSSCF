#include "stdafx.h"

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <string>

#include <vector>
#include <fstream>
#include <strstream>
#include <sstream>

#include <string.h>



// link with Ws2_32.lib
#pragma comment(lib, "C:\\lib\\Ws2_32.lib")

#define DEFAULT_PORT "23051" 
#define DEFAULT_BUFFER_LENGTH    512


class Client {
public:
    Client(char* servername);
 
    bool Start();
 
    // Free the resouces
    void Stop();
 
    // Send message to server
    bool Send(char* szMsg);
 
    // Receive message from server
    bool Recv();

	bool Recv(std::string & ori_str);
 
private:
    char* szServerName;
    SOCKET ConnectSocket;
};

//request a file from server
bool reqFStream( Client& client, char * userName, char * reqMSG, std::string& buffer );

//request server to make some computation~ NLP for example
bool req_autoCate(Client & client, char * userName);

//send some local stream file to server
bool sendFStream( Client& client, char * filePath, char * userName );

//functional function
void file2request(const char * filepath, std::vector<char *> & reqList, const char * usrName);
void requestFile(Client& client, char * userName);
void requestFile(Client& client, char * userName, char * reqMSG);


