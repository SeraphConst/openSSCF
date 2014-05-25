#include "stdafx.h"

#include "openSSCF.h"

Client::Client(char* servername)
{
    szServerName = servername;
    ConnectSocket = INVALID_SOCKET;
}

bool Client::Start() 
{
    WSADATA wsaData;

    // Initialize Winsock
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if(iResult != 0)
    {
        printf("WSAStartup failed: %d\n", iResult);
        return false;
    }

    struct addrinfo    *result = NULL,
                    *ptr = NULL,
                    hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;        
    hints.ai_socktype = SOCK_STREAM;    
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(szServerName, DEFAULT_PORT, &hints, &result);
    if (iResult != 0)
    {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return false;
    }

    ptr = result;

    // Create a SOCKET for connecting to server
    ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

    if (ConnectSocket == INVALID_SOCKET)
    {
        printf("Error at socket(): %d\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return false;
    }

    // Connect to server
    iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);

    if (iResult == SOCKET_ERROR)
    {
        closesocket(ConnectSocket);
        ConnectSocket = INVALID_SOCKET;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET)
    {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return false;
    }

    return true;
};

void Client::Stop() 
{
    int iResult = shutdown(ConnectSocket, SD_SEND);

    if (iResult == SOCKET_ERROR)
    {
        printf("shutdown failed: %d\n", WSAGetLastError());
    }

    closesocket(ConnectSocket);
    WSACleanup();
};

bool Client::Send(char* szMsg)
{
    
    int iResult = send(ConnectSocket, szMsg, strlen(szMsg), 0);

    if (iResult == SOCKET_ERROR)
    {
        printf("send failed: %d\n", WSAGetLastError());
        Stop();
        return false;
    }

    return true;
};

bool Client::Recv()
{
    char recvbuf[DEFAULT_BUFFER_LENGTH];
    int iResult = recv(ConnectSocket, recvbuf, DEFAULT_BUFFER_LENGTH, 0);

    if (iResult > 0)
    {
        char msg[DEFAULT_BUFFER_LENGTH];
        memset(&msg, 0, sizeof(msg));
        strncpy(msg, recvbuf, iResult);

        printf("Received: %s\n", msg);

        return true;
    }


    return false;
}

bool Client::Recv(std::string &ori_str)
{
    char recvbuf[DEFAULT_BUFFER_LENGTH];
    int iResult = recv(ConnectSocket, recvbuf, DEFAULT_BUFFER_LENGTH, 0);

    if (iResult > 0)
    {
        char msg[DEFAULT_BUFFER_LENGTH];
        memset(&msg, 0, sizeof(msg));
        strncpy(msg, recvbuf, iResult);

		ori_str.append(msg);

        printf("Received: %s\n", msg);

        return true;
    }


    return false;
}

void file2request(const char * filepath, std::vector<char *> & reqList, const char * usrName)
{
	std::ifstream infile;
	infile.open(filepath, std::ios::in);

	std::stringstream buffer;
	buffer << infile.rdbuf();

	std::string contents(buffer.str());
	int strLen = contents.size();			//���滹���õ����ļ����ȱ���

	infile.close();
	buffer.str("");			//��˵���������stringstream��������



	//std::cout << contents;
	std::string msgLine;				//��������
	int msgLineLen = 0;

	int msgLen = DEFAULT_BUFFER_LENGTH - 50;		//ÿ�δ���msgLen��
	int nMsg = strLen/msgLen+1;

	////����ǰnMsg-1������
	for(int i = 0; i < nMsg-1; i++)		
	{
		//����ͷ����
		msgLine.clear();
		char msgHeader_str[50];											
		sprintf(msgHeader_str, "[REQ_TRI %d %d %s]", i+1, nMsg, usrName);
		msgLine.append( msgHeader_str );

		//��Ӧ�������ݸ���
		for( int j = 0; j < msgLen; j++ )
		{
			msgLine.push_back( contents.at( j+i*msgLen ) );
		}

		//�����ַ�����
		msgLineLen = msgLine.size();
		char * pMsg = new char [msgLineLen+1];
		pMsg[msgLineLen] = '\0';
		strcpy( pMsg, msgLine.c_str() );

		reqList.push_back( pMsg );
	}
	////�������һ������
	msgLine.clear();
	char msgHeader_str[50];											
	sprintf(msgHeader_str, "[REQ_TRI %d %d %s]", nMsg, nMsg, usrName);
	msgLine.append( msgHeader_str );
	for( int i = (nMsg-1)*msgLen; i < strLen; i++ )
	{
		msgLine.push_back( contents.at(i) );
	}
	msgLineLen = msgLine.size();
	char * pMsg = new char [msgLineLen+1];
	pMsg[msgLineLen] = '\0';
	strcpy( pMsg, msgLine.c_str() );
	reqList.push_back( pMsg );


}

void requestFile(Client& client, char * userName)
{
	char req_MSG[DEFAULT_BUFFER_LENGTH];
	memset((char *)req_MSG, '\0', sizeof(char)*DEFAULT_BUFFER_LENGTH);

	sprintf( req_MSG, "[REQ_GET 1 1 %s]g", userName );
	
	client.Send( req_MSG );

	client.Recv(  );
}

void requestFile(Client& client, char * userName, char * reqMSG)
{
	char req_MSG[DEFAULT_BUFFER_LENGTH];
	memset((char *)req_MSG, '\0', sizeof(char)*DEFAULT_BUFFER_LENGTH);

	sprintf( req_MSG, "[REQ_GET 1 1 %s]%s", userName, reqMSG );
	
	client.Send( req_MSG );

	client.Recv(  );

}

//ԭ�ͣ�
//bool reqFStream( Client& client, char * userName, char * reqMSG, std::string& buffer )
//�ͻ���TCP/IP�� client
//�û����� userName
//������Ϣ reqMSG
//�����ַ��� buffer
bool reqFStream( Client& client, char * userName, char * reqMSG, std::string& buffer )
{
	////�����ļ�ģʽ

	requestFile(client, userName);

	client.Send( "[EXE]" );
	std::string getNStr;
	client.Recv(getNStr);

	//Լ��1������ֵisDigit
	int nReturns = atoi(getNStr.c_str()) ;

	//���յ�buffer��
	buffer.clear();
	for(int i = 0; i < nReturns; i++)
	{
		client.Recv(buffer);
	}

	return true;
}

//ԭ��
//bool req_autoCate(Client & client)
//���� client TCP/IP�ͻ���
//���� userName
bool req_autoCate(Client & client, char * userName)
{
	char autocMSG[100];
	sprintf(autocMSG, "[REQ_AUTOC 1 1 %s]cont", userName);

	client.Send( autocMSG );

	client.Recv(  );

	client.Send( "[EXE]" );

	client.Recv( );

	return true;
}

//ԭ�ͣ�
//bool sendFStream( Client& client, char * filePath, char * userName )
//�ͻ���TCP/IP�� client
//�����ļ�·�� filePath
//�û����� userName
//���أ�true
bool sendFStream( Client& client, char * filePath, char * userName )
{
	std::vector<char *> reqList;
	//file2request("d:\\data\\hello.txt", reqList, "lichen");
	file2request(filePath, reqList, userName);	

//	for(int i = 0; i < reqList.size(); i++)
//	{
//		std::cout << reqList.at(i);
//		std::cout << "\n\n";
//	}


	for( int i = 0; i < reqList.size(); i++)
	{

		client.Send( reqList.at(i) );

		client.Recv(  );
	}

	//�ͷ��ڴ�
	for(int i = 0; i < reqList.size(); i++)
	{
		char * tempBuf = reqList.at(i);
		reqList.at(i) = NULL;
		delete [] tempBuf;
	}
	reqList.clear();

	client.Send( "[EXE]" ); 
	client.Recv(); 

	return true;
}
