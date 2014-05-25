//#include "fileProc_win.h"

#include "theTree.h"

//#include "findBookmarkPath_win.h"

#include "TC_process.h"

//#include "visualizationTree.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>

#include <memory.h>

#include <vector>

#include <iostream>

#include <fstream>

#include <sstream>
#include <strstream>

#define SERVPORT 23051	//
#define BACKLOG 10	//the most num of request at the same time

#define DEFAULT_BUFFER_LENGTH 512	//the length of a sentence


//only this function call the "TC_process.h" "theTree.h"
//this function is a example of server response to the client's request~
void proc()
{
	//ieOperation();
	
	//firefoxOperation();

	theTree t;
	t.formTreefromTRI( "./data/chromeBookmark2.tri" );			//打开原浏览器精简后的tri文件
	std::cout << "\nreading data successfully\n";

	//建立标题表
	std::vector<char *> titleList;
	t.outputTitles(titleList);
	int nTitles = titleList.size();
	char ** titles = new char* [ nTitles ];
	for(int i = 0; i < nTitles; i++)
	{
		titles[i] = titleList.at(i);
	}
	int * labelList = new int [ nTitles ];

	//NLP分类确定
	textCategorization_new(titles, nTitles, labelList);

	//在树上更新上类标
	t.updateLabels(labelList, nTitles);
	t.saveTreeIntoTRI("./data/chromeBookmark3.tri");

	//load the labeled data
	theTree t_lbl;
	t_lbl.formTreefromTRI("./data/chromeBookmark3.tri");

	t_lbl.fixLabels();

	//changed into menued tri
	theTree * t1 = t_lbl.getMenuTree();
	t1->saveTreeIntoTRI("newTree.tri");

	std::cout << "\nsaved\n";


	delete t1;
	
	delete [] titles;
	titleList.clear();
	delete [] labelList;
}



void file2request(const char * filepath, std::vector<char *> & reqList)
{
	std::ifstream infile;
	infile.open(filepath, std::ios::in);

	std::stringstream buffer;
	buffer << infile.rdbuf();

	std::string contents(buffer.str());
	int strLen = contents.size();			//后面还会用到的文件长度变量

	infile.close();
	buffer.str("");			//据说这样是清空stringstream。流对象？



	//std::cout << contents;
	std::string msgLine;				//报文内容
	int msgLineLen = 0;

	int msgLen = DEFAULT_BUFFER_LENGTH - 50;		//每次传输msgLen个
	int nMsg = strLen/msgLen+1;

	////对于前nMsg-1个报文
	for(int i = 0; i < nMsg-1; i++)		
	{
		//报文头生成
		msgLine.clear();
		char msgHeader_str[50];											
		//sprintf(msgHeader_str, "[REQ_TRI %d %d %s]", i+1, nMsg, usrName);
		//msgLine.append( msgHeader_str );

		//相应报文内容复制
		for( int j = 0; j < msgLen; j++ )
		{
			msgLine.push_back( contents.at( j+i*msgLen ) );
		}

		//导入字符数组
		msgLineLen = msgLine.size();
		char * pMsg = new char [msgLineLen+1];
		pMsg[msgLineLen] = '\0';
		strcpy( pMsg, msgLine.c_str() );

		reqList.push_back( pMsg );
	}
	////对于最后一条报文
	msgLine.clear();
	char msgHeader_str[50];											
	//sprintf(msgHeader_str, "[REQ_TRI %d %d %s]", nMsg, nMsg, usrName);
	//msgLine.append( msgHeader_str );
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



//proc char * buf
//if this buf ends a command~~ SEN_FLAG = TRUE
//string sentence
//string cmd_header
//iOfSen nOfSen just for examine
bool proc_buf(const char * buf, bool & SEN_FLAG, std::string & sentence, std::string & cmd_header, std::string & usr_name, int & nOfSen, int & iOfSen, bool & isINV)
{
	int i_f_brac = 0;
	int i_l_brac = -1;

	std::vector<int> v_space;

	//1st assumption: buf's content doesnt cross the border
	//2nd assumption: the sentences in a command has the same cmd header 
	//3rd assumption: there're only 2 types of REQ ~~ single REQ | compound REQ
	//4th assumption: compound REQ has a stereotype( has 2 digital page indexs ... username's upper limit is 20 )
	//5th assumption: compound REQ must have a sentence behind it

	//get i_f_brac and i_l_brac
	for( int i = 0; i < DEFAULT_BUFFER_LENGTH; i++ )
	{
		if( buf[i] == ']' )
		{
			i_l_brac = i;
			break;
		}
	}
	if( (buf[0] != '[') || (i_l_brac == DEFAULT_BUFFER_LENGTH-1) )
	{
		return false;
	}

	//judge whether this cammand is a single REQ~
	for( int i = i_f_brac; i < i_l_brac; i++ )
	{
		if( buf[i] == ' ' )
		{
			v_space.push_back(i);
		}
	}

	if( v_space.size()==0 )		//is a single REQ
	{
		SEN_FLAG = true;

		//make a replica of header~~
		char * header = new char [ i_l_brac - i_f_brac ];
		for(int i = 0; i < i_l_brac - i_f_brac - 1; i++)
		{
			header[i] = buf[ i_f_brac+1 + i ];
		}
		header[ i_l_brac-i_f_brac-1 ] = '\0';

		cmd_header.append(header);

		printf( "\n%s\n", cmd_header.c_str() );

		delete [] header;

		return true;
	}
	else if( v_space.size()==3 )		//is a compound REQ
	{
		//****seperate the buff****

		int i_sep[6] = { i_f_brac, v_space.at(0), v_space.at(1), v_space.at(2), i_l_brac, strlen(buf) };

		std::string g_cmd;
		std::string g_iPage;
		std::string g_nPage;
		std::string g_user;
		std::string g_sen;
		
		//char * cmd_str = new char [ i_sep[1]-i_sep[0] ];
		//char * iCUR_str = new char [ i_sep[2]-i_sep[1] ];
		//char * iALL_str = new char [ i_sep[3]-i_sep[2] ];
		//char * username_str = new char [ i_sep[4]-i_sep[3] ];
		//char * sen_str = new char [ i_sep[5]-i_sep[4] ];
		int iCUR = 0;
		int iALL = 0;

		//cmd_str
		for( int i = 0; i < i_sep[1]-i_sep[0] - 1; i++ )
		{
			//cmd_str[i] = buf[ i_sep[0]+1 + i ];
			g_cmd.push_back( buf[ i_sep[0]+1 + i ] );
		}
		//cmd_str[i_sep[1]-i_sep[0] - 1] = '\0';

		//iCUR_str
		for( int i = 0; i < i_sep[2]-i_sep[1] - 1; i++ )
		{
			//iCUR_str[i] = buf[ i_sep[1]+1 + i ];
			g_iPage.push_back( buf[ i_sep[1]+1 + i ] );
		}
		//iCUR_str[i_sep[2]-i_sep[1] - 1] = '\0';
		//iALL_str
		for( int i = 0; i < i_sep[3]-i_sep[2] - 1; i++ )
		{
			//iALL_str[i] = buf[ i_sep[2]+1 + i ];
			g_nPage.push_back( buf[ i_sep[2]+1 + i ] );
		}
		//iALL_str[i_sep[3]-i_sep[2] - 1] = '\0';
		//username_str
		for( int i = 0; i < i_sep[4]-i_sep[3] - 1; i++ )
		{
			//username_str[i] = buf[ i_sep[3]+1 + i ];
			g_user.push_back( buf[ i_sep[3]+1 + i ] );
		}
		//username_str[i_sep[4]-i_sep[3] - 1] = '\0';	
		//sen_str
		for( int i = 0; i < i_sep[5]-i_sep[4] - 1; i++ )
		{
			//sen_str[i] = buf[ i_sep[4]+1 + i ];
			g_sen.push_back( buf[ i_sep[4]+1 + i ] );
		}
		//sen_str[i_sep[5]-i_sep[4] - 1] = '\0';

		iCUR = atoi( g_iPage.c_str() );
		iALL = atoi( g_nPage.c_str() );
		

		//****VALIDATION****
		//whether this is an initialization?
		if( nOfSen == -1 && iOfSen == -1 )
		{
			iOfSen = iCUR;
			nOfSen = iALL;

			usr_name.append( g_user );
			cmd_header.append( g_cmd );
		}

		//clauses
		if( usr_name != g_user )
		{
			isINV = true;
		}
		if( nOfSen != iALL )
		{
			isINV = true;
		}
		if( (iOfSen != 1) && (iOfSen+1 != iCUR) )
		{
			isINV = true;
		}
		if( nOfSen != iALL )
		{
			isINV = true;
		}
		if( g_cmd != cmd_header )
		{
			isINV = true;
		}
		
		//**** return parameters ****
		iOfSen = iCUR;
		sentence.append( g_sen );

		printf( "\n%s (%d, %d) %d\n", g_cmd.c_str(), iOfSen, nOfSen, (int)isINV );
		//printf( "%s\n\n", sentence.c_str() );
		
		if( iOfSen == nOfSen )
		{
			SEN_FLAG = true;
			return true;
		}
		else
		{
			SEN_FLAG = false;
			return false;
		}
				

		//return true;
	}
	else
	{
		//this case would never happen
		return false;
	}

	return true;
}


int main()
{
	int sockfd;			//sock_fd: monitoring socket
	int client_fd;			//dota transporting socket

	struct sockaddr_in my_addr;	//ip&tcp info of this pc
	struct sockaddr_in remote_addr;	//client's info


	if( ( sockfd = socket(AF_INET, SOCK_STREAM, 0) ) == -1 )
	{
		perror("error creating socket");
		exit(1);
	}

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(SERVPORT);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	bzero( &(my_addr.sin_zero), 8 );

	if( ( bind( sockfd, (struct sockaddr*)&my_addr, sizeof(struct sockaddr) ) ) ==-1 )
	{
		perror("error binding socket");
		exit(1);
	}

	if( listen(sockfd, BACKLOG)==-1 )
	{
		perror("error listening");
		exit(1);
	}


	//is a flag of the running of the server program
	bool SERV_FLAG = true;	

	while(SERV_FLAG)
	{
		socklen_t sin_size = sizeof( struct sockaddr_in );
		if( ( client_fd = accept(sockfd, (struct sockaddr *)&remote_addr, &sin_size) )==-1 )
		{
			perror("error accepting");
			continue;
		}


		
		if( !fork() )		//sub process part
		{
			////response with connection infomation
			printf( "received a connection from %s\n", inet_ntoa(remote_addr.sin_addr) );
			char rcv_msg[DEFAULT_BUFFER_LENGTH];
			sprintf( rcv_msg, "received your connection from %s\n", inet_ntoa(remote_addr.sin_addr) );
			if( (send(client_fd, rcv_msg, DEFAULT_BUFFER_LENGTH, 0))==-1 )
			{
				perror("error sending");
				close(client_fd);
				exit(0);
			}
			////response with connection infomation


			bool EXE_FLAG = true;		// a flag of the execution to this client

			while( EXE_FLAG )
			{
				std::string sentence;		//the body of sentence 
				std::string cmd_header;		//the header of cmd
				std::string usr_name;		//name of this user

				int nOfSentence = -1;		//total n
				int iOfSentence = -1;		//current n
				bool isINV = false;		//judgement of validition of command

				bool SEN_FLAG = false;	// a flag of the judgement of integrity of a sentence

				//****REQUEST PEROID****
				while( !SEN_FLAG )
				{
					char buf[DEFAULT_BUFFER_LENGTH];	//a buf get once
					memset((char *)buf, '\0', sizeof(char)*DEFAULT_BUFFER_LENGTH);

					//receive buf from client
					int recv_n = 0;
					if( ( recv_n=recv( client_fd, buf, DEFAULT_BUFFER_LENGTH, 0 ) )==-1 )
					{
						perror("error receiving");
						close(client_fd);
						exit(0);
					}

					//proc this sentence
					proc_buf(buf, SEN_FLAG, sentence, cmd_header, usr_name, nOfSentence, iOfSentence, isINV);
					
					//for test~~~ send anything 
					char testMSG1[DEFAULT_BUFFER_LENGTH] = "received\n";
					if( (send(client_fd, testMSG1, DEFAULT_BUFFER_LENGTH, 0))==-1 )
					{
						perror("error sending");
						close(client_fd);
						exit(0);
					}

				}

				//****EXECUTE PEROID****
				//receive anything to execute
				char buf[DEFAULT_BUFFER_LENGTH];	//a buf get once
				memset((char *)buf, '\0', sizeof(char)*DEFAULT_BUFFER_LENGTH);
				int recv_n = 0;
				if( ( recv_n=recv( client_fd, buf, DEFAULT_BUFFER_LENGTH, 0 ) )==-1 )
				{
					perror("error receiving");
					close(client_fd);
					exit(0);
				}	
			
				//execute
				printf( "\n%s (%d) %d %s\n", cmd_header.c_str(), nOfSentence, (int)isINV, usr_name.c_str() );

				if( strcmp("[EXE]", buf)==0 )
				{
					if( strcmp("REQ_TRI", cmd_header.c_str())==0 )
					{
						if( isINV == false )
						{
							printf( "%s", "\nREQ_TRI\n" );

							char triFileName[50];
							sprintf( triFileName, "%s.tri", usr_name.c_str() );
							std::ofstream ofile;
							ofile.open(triFileName, std::ios::out);
							ofile << sentence.c_str();
							ofile.close();

							//send the result to client
							char testMSG2[DEFAULT_BUFFER_LENGTH] = "PROCESSED YOUR REQUEST [REQ_TRI].\n";
							if( (send(client_fd, testMSG2, DEFAULT_BUFFER_LENGTH, 0))==-1 )
							{
								perror("error sending");
								close(client_fd);
								exit(0);
							}
						}
						else
						{
							printf( "%s", "\nINV REQ_TRI\n" );

							//send the result to client
							char testMSG2[DEFAULT_BUFFER_LENGTH] = "INVALID REQUEST.\n";
							if( (send(client_fd, testMSG2, DEFAULT_BUFFER_LENGTH, 0))==-1 )
							{
								perror("error sending");
								close(client_fd);
								exit(0);
							}
						}
					}
					else if( strcmp("REQ_GET", cmd_header.c_str())==0 )
					{
						if( isINV == false )
						{
							std::vector<char *> backList;
							file2request( "./data/mingran_menu.tri", backList );
							
							//send a number to the result to client
							char testMSG2[DEFAULT_BUFFER_LENGTH];
							sprintf( testMSG2, "%d", backList.size() );
							if( (send(client_fd, testMSG2, DEFAULT_BUFFER_LENGTH, 0))==-1 )
							{
								perror("error sending");
								close(client_fd);
								exit(0);
							}
							//send the file by number's messages
							for( int i = 0; i < backList.size(); i++ )
							{
								char testMSG3[DEFAULT_BUFFER_LENGTH];
								sprintf( testMSG3, "%s", backList.at(i) );
								if( (send(client_fd, testMSG3, DEFAULT_BUFFER_LENGTH, 0))==-1 )
								{
									perror("error sending");
									close(client_fd);
									exit(0);
								}
							}

							printf( "\nREQ_TRI mingran %d\n", backList.size() );

							//clear memory
							for( int i = 0; i < backList.size(); i++ )
							{
								char * pLine = backList.at(i);
								backList.at(i) = NULL;
								delete [] pLine;
							}
							backList.clear();
						}
						else
						{
							printf( "%s", "\nINV REQ_GET\n" );

							//send the result to client
							char testMSG2[DEFAULT_BUFFER_LENGTH] = "INVALID REQUEST.\n";
							if( (send(client_fd, testMSG2, DEFAULT_BUFFER_LENGTH, 0))==-1 )
							{
								perror("error sending");
								close(client_fd);
								exit(0);
							}
						}
					}
					else if( strcmp("REQ_AUTOC", cmd_header.c_str())==0  )
					{
						printf("\nauto Cate\n");
						proc();

						char testMSG2[DEFAULT_BUFFER_LENGTH] = "Finished process.\n";
						if( (send(client_fd, testMSG2, DEFAULT_BUFFER_LENGTH, 0))==-1 )
						{
							perror("error sending");
							close(client_fd);
							exit(0);
						}

					}
					else
					{
						//printf( "%s", "\nCANNOT RECOGNISE REQUEST\n" );
						printf( "%s", "\nmingran LOGIN\n" );
						//send the result to client
						//char testMSG2[DEFAULT_BUFFER_LENGTH] = "CANNOT RECOGNISE YOUR REQUEST.\n";
						char testMSG2[DEFAULT_BUFFER_LENGTH] = "WELCOME mingran.\n";
						if( (send(client_fd, testMSG2, DEFAULT_BUFFER_LENGTH, 0))==-1 )
						{
							perror("error sending");
							close(client_fd);
							exit(0);
						}
					}
				}
				else
				{
					printf( "%s", "\nWILL NOT PROCESSE\n" );

					//send the result to client
					char testMSG2[DEFAULT_BUFFER_LENGTH] = "WILL NOT PROCESSE YOUR REQUEST.\n";
					if( (send(client_fd, testMSG2, DEFAULT_BUFFER_LENGTH, 0))==-1 )
					{
						perror("error sending");
						close(client_fd);
						exit(0);
					}
				}

				//EXE_FLAG = false;
			}

		}
		
	}

	//shutdown(sockfd, 2);
	close(sockfd);
	printf("the server program is ending now...\n");

	//cannot figure out the reason for the ablity to avoid being killed of this program...

	//exit(0);

	return -1;
}
