// ClientWin.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"

#include "openSSCF.h"


int main(int argc, char * argv[])
{
    Client client("182.92.80.204");
 
    if (!client.Start())
	{
        return 1;
	}
	client.Recv();

	/*
	//�ļ�����
	sendFStream(client, "d:\\file.txt", "mingran4");
	*/

	/*
	//�ļ�����
	std::string buffer;
	reqFStream( client, "mingran", "getSomething", buffer );
	*/

	/*
	//�����Զ�����
	req_autoCate(client, "mingran");
	*/

    client.Stop();


	return 0;
}

