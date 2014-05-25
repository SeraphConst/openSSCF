// ClientWin.cpp : 定义控制台应用程序的入口点。
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
	//文件传送
	sendFStream(client, "d:\\file.txt", "mingran4");
	*/

	/*
	//文件接收
	std::string buffer;
	reqFStream( client, "mingran", "getSomething", buffer );
	*/

	/*
	//请求自动分类
	req_autoCate(client, "mingran");
	*/

    client.Stop();


	return 0;
}

