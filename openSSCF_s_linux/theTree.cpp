#include "theTree.h"

#ifndef THETREE_CPP

#define THETREE_CPP


//// theNode

theNode::theNode()
{
	p_FN = NULL;
	p_SN = NULL;
	nSon = 0;
	pOri_str = NULL;
}

theNode::theNode(char * g_path, int g_attr)
{
	p_FN = NULL;
	p_SN = NULL;
	nSon = 0;
	pOri_str = NULL;

	if(g_path == NULL)
	{
		attr_table_word.push_back(NULL);
	}
	else
	{
		char * path = new char [strlen(g_path)+1];
		strcpy(path, g_path);
		attr_table_word.push_back(path);
	}

	attr_table_num.push_back(g_attr);
}

theNode::~theNode()
{
	if(pOri_str != NULL)
	{
		delete [] pOri_str;
	}
	attr_table_num.clear();

	for(int i = 0; i < attr_table_word.size(); i++)
	{
		if( attr_table_word.at(i) != NULL )
		{
			delete [] attr_table_word.at(i);
		}
	}
	attr_table_word.clear();
}

bool theNode::isBOOKMARK()
{
	for(int i = 0; i < attr_table_word.size()/2; i++)
	{
		char * key = attr_table_word.at(2*i);
		char * value = attr_table_word.at(2*i+1);

		if(key == NULL)
		{
			continue;
		}
		if( strcmp(key, "type")==0 )
		{
			if(value == NULL)
			{
				continue;
			}
			if( strcmp(value, "BOOKMARK")==0 )
			{
				//this is a bookmark
				return true;
			}
			else
			{
				return false;
			}
		}
	}
	return false;
}

//��û��title���򷵻�-1
int theNode::indexOfTitle()
{
	int index = -1;
	for(int i = 0; i < attr_table_word.size()/2; i++)
	{
		char * key = attr_table_word.at(2*i);
		char * value = attr_table_word.at(2*i+1);

		if(key == NULL)
		{
			continue;
		}
		if( strcmp(key, "name")==0 )
		{
			if(value == NULL)
			{
				break;
			}
			else
			{
				index = 2*i + 1;
				break;
			}
		}
	}

	return index;
}

//// theTree

theTree::theTree()
{
	nOfAStage = NULL;
	treeStructure = NULL;
	nStages = 0;

	nLabels = 0;
}

theTree::~theTree()
{
	if(treeStructure != NULL)
	{
		for(int stage = 0; stage < nStages; stage++)
		{
			delete [] ((theNode * ) (treeStructure[stage]) );
		}
	}
	if(nOfAStage != NULL)
	{
		delete [] nOfAStage;
	}
	labelSum.clear();
}

void theTree::saveTreeIntoTRI(char * filename)
{
	std::ofstream ofile;
	ofile.open(filename, std::ios::out);

	if(nStages == 0)
	{
		ofile << "THIS TREE IS EMPTY";
		ofile.close();
		return;
	}

	//���ɽṹ��
	std::vector<int> v_structCode;
	for(int stage = 0; stage < nStages-1; stage++)
	{
		for(int id = 0; id < nOfAStage[stage]; id++)
		{
			v_structCode.push_back( treeStructure[stage][id].nSon );
		}
	}
	std::string str_structCode;
	for(int i = 0; i < v_structCode.size(); i++)
	{
		char numStr[10];
		sprintf( numStr, "%d ", v_structCode.at(i) );
		str_structCode.append(numStr);
	}
	ofile << str_structCode;

	//�õ�������Ϣ
	std::vector<char *> v_key;
	char * contents = getTreeAttrWList_str(v_key, 0);

	ofile << contents;

	delete [] contents;

	ofile.close();

}

bool theTree::formTreefromTRI(char * filename)
{
	const int n_char = 10000;
	////�������Ȳ���
	char outStr[n_char];

	std::ifstream ifile;
	ifile.open(filename, std::ios::in);
	int n_lines = 0;
	while( !ifile.eof() )
	{
		ifile.getline(outStr, n_char);
		n_lines ++;
	}
	ifile.close();
	if( strlen(outStr) < 2 )		//������һ�������ݵ����
	{
		n_lines --;
	}
	n_lines --;		//��ȥ��һ�еĽṹ��
	//�õ�n_lines��Ϊ�ܳ���


	////һ���ļ�����
	std::ifstream infile;
	infile.open(filename, std::ios::in);

	////�����������Ľṹ

	//ԭʼ�ṹ��
	std::vector<int> v_rawN;
	//ÿ������Ľڵ�����
	std::vector<int> v_stageN;		

	
	infile.getline(outStr, n_char);

	char * label;
	label = strtok(outStr, " ");

	while(label != NULL)
	{
		//std::cout << atoi(label);
		//std::cout << "~~~~~~";
		v_rawN.push_back( atoi(label) );
		label = strtok(NULL, " ");
	}

	////��֤�����Ƿ��㹻
	int n_sum_Nodes = 1;			//���ڵ�
	for(int i = 0; i < v_rawN.size(); i++)
	{
		n_sum_Nodes += v_rawN.at(i);
	}
	if( n_lines < n_sum_Nodes )		//���������ڹ������Ľڵ�������ֱ������
	{
		v_rawN.clear();
		v_stageN.clear();

		infile.close();
		return false;
	}

	//���������ṹ��
	v_stageN.push_back(1);			//��һ�������һ�������ڵ�

	int n_bros = 1;					//��һ���ԭʼ�ṹ�롫

	std::vector<int>::iterator i_rawN;
	i_rawN = v_rawN.begin();


	while(true)					//ÿ��ѭ�����һ���ͳ��
	{
		int n_thisStage = 0;
		for(int i = 0; i < n_bros; i++)
		{
			n_thisStage += (*i_rawN);
			i_rawN ++;
		}
		v_stageN.push_back(n_thisStage);
		n_bros = n_thisStage;

		if( i_rawN == v_rawN.end() )			//��ѭ����ֹ�����涨������rawN��ȷ��������ѭ��
		{
			break;
		}
	}

	//���������ʽ
	treeStructure = new theNode* [v_stageN.size()];
	for(int i = 0; i < v_stageN.size(); i++)
	{
		treeStructure[i] = new theNode[v_stageN.at(i)];
	}

	//��������ڵ��n_Sons��ֵ
	i_rawN = v_rawN.begin();
	for( int stage = 0; stage < v_stageN.size()-1; stage++ )
	{
		for(int i = 0; i < v_stageN.at(stage); i++)
		{
			treeStructure[stage][i].nSon = (*i_rawN);		//����Ϊ0���������ս�
			i_rawN++;									//�˴�Ҳ�ܸ�ʽ��ȷ����Ӱ��
		}
	}
	for( int i = 0; i < v_stageN.at(v_stageN.size()-1); i++ )			//��׶˵Ľڵ�һ��������son
	{
		treeStructure[v_stageN.size()-1][i].nSon = 0;
	}

	//p_pSN��ϵ��ȷ��
	i_rawN = v_rawN.begin();
	for( int stage = 0; stage < v_stageN.size()-1; stage++ )
	{
		int n_nextStage = 0;
		for(int i = 0; i < v_stageN.at(stage); i++)
		{
			treeStructure[stage][i].p_SN = &(treeStructure[stage+1][n_nextStage]);
			n_nextStage += treeStructure[stage][i].nSon;
		}
	}
	for( int i = 0; i < v_stageN.at(v_stageN.size()-1); i++ )
	{
		treeStructure[v_stageN.size()-1][i].p_SN = NULL;
	}

	//p_pFN��ϵ��ȷ������psN�Ĺ�ϵ���ƣ�
	treeStructure[0][0].p_FN = NULL;
	for( int stage = 0; stage < v_stageN.size()-1; stage++ )
	{
		for(int i = 0; i < v_stageN.at(stage); i++)
		{
			//�����Ǹ��ڵ������
			for( int j = 0; j < treeStructure[stage][i].nSon; j++ )
			{
				((theNode *)(treeStructure[stage][i].p_SN))[j].p_FN = &treeStructure[stage][i];
			}
		}
	}

	//������ṹ�Ĵ���


	//����ɨ�����е����ݣ�����ڵ�Ĳ�������
	for( int stage = 0; stage < v_stageN.size(); stage++ )
	{
		for(int id = 0; id < v_stageN.at(stage); id++)
		{
			infile.getline(outStr, n_char);

			//�����¿ռ䡫����theNode����
			std::vector<int> v_word;
			v_word.push_back(0);
			int len = strlen( outStr );
			for( int i = 0; i < len; i++ )
			{
				if( outStr[i] == '\t' )
				{
					outStr[i] = '\0';
					
					if(i+1 != len)		//����������һ��\t����
					{
						v_word.push_back(i+1);
					}
				}
			}
			for( int i = 0; i < v_word.size()/2; i++ )		//key-value�ɶԳ���
			{
				//�½����п���
				char * key = NULL;
				char * value = NULL;
				int len_k = strlen( &(outStr[v_word.at(i*2)]) );
				int len_v = strlen( &(outStr[v_word.at(i*2+1)]) );
				if(len_k == 0)			//����key
				{
					key = NULL;
				}
				else
				{
					key = new char [len_k+1];
					strcpy( key, &(outStr[v_word.at(i*2)]) );
				}
				if(len_v == 0)			//����value
				{
					value = NULL;
				}
				else
				{
					value = new char [len_v+1];
					strcpy( value, &(outStr[v_word.at(i*2+1)]) );
				}

				//�������
				treeStructure[stage][id].attr_table_word.push_back(key);
				treeStructure[stage][id].attr_table_word.push_back(value);
			}
		}
	}


	//�ġ���ɨ��Ľṹ��Ϣ��������
	nStages = v_stageN.size();
	nOfAStage = new int [nStages];
	for(int i = 0; i < nStages; i++)
	{
		nOfAStage[i] = v_stageN.at(i);
	}

	//�塢�ͷ��ڴ�
	v_stageN.clear();
	v_rawN.clear();

	infile.close();	

	return true;
}

//tree����ֻ��nSon��Ϣ��������Ϣ�ǿ��׵ġ�
//�����ṹ��Ϣ�������Լ����롣
void theTree::formIETree(std::vector<std::vector<theNode *> *> & tree)
{
	nStages = tree.size();
	nOfAStage = new int [nStages];
	treeStructure = new theNode * [nStages];

	//nOfStage��ֵ
	for(int stage = 0; stage < nStages; stage ++)
	{
		nOfAStage[stage] = tree.at(stage)->size();
		treeStructure[stage] = new theNode [ nOfAStage[stage] ];
	}

	//Node��ֵ
	for(int stage = 0; stage < nStages; stage++)
	{
		for(int id = 0; id < nOfAStage[stage]; id++)
		{
			theNode * p_node = tree.at(stage)->at(id);
			treeStructure[stage][id].nSon = p_node->nSon;
			//theTree�ĸ�ֵ
			//int n_attr_i = p_node->attr_table_num.size();
			//for(int i = 0; i < n_attr_i; i++)
			//{
			//	treeStructure[stage][id].attr_table_num.push_back( p_node->attr_table_num.at(i) );
			//}
			//�Ķ����ֶ���treeStructure�е�attr_table_num�в���2��-1
			treeStructure[stage][id].attr_table_num.push_back(-1);
			treeStructure[stage][id].attr_table_num.push_back(-1);

			int n_attr_a = p_node->attr_table_word.size();
			for(int i = 0; i < n_attr_a; i++)
			{
				char * oldAttrW = p_node->attr_table_word.at(i);
				if(oldAttrW == NULL)
				{
					treeStructure[stage][id].attr_table_word.push_back(NULL);
				}
				else
				{
					char * newAttrW = new char [ strlen(oldAttrW)+1 ];
					strcpy(newAttrW, oldAttrW);
					treeStructure[stage][id].attr_table_word.push_back(newAttrW);
				}
			}
		}
	}

	//Nodeȷ�����ӹ�ϵ
	treeStructure[0][0].p_FN = NULL;				//����һ�еĸ��ڵ�
	for(int stage = 0; stage < nStages-1; stage++)
	{
		int id_next_byfar = 0;

		for(int id = 0; id < nOfAStage[stage]; id++)
		{
			int n_son = treeStructure[stage][id].nSon;
			treeStructure[stage][id].p_SN = &( treeStructure[stage+1][id_next_byfar] );		//���ڵ�ȷ���ӽڵ�
			for(int sid = 0; sid < n_son; sid++)					//�ӽڵ�ȷ�����ڵ�
			{
				treeStructure[stage+1][sid+id_next_byfar].p_FN = &( treeStructure[stage][id] );
			}
		}
	}
	for(int id = 0; id < nOfAStage[nStages-1]; id++)	//�����һ�е��ӽڵ�
	{
		treeStructure[nStages-1][id].p_SN = NULL;
	}

	return;
}

char * theTree::getTreeOriginList_str()
{
	std::string str;

	//��ӡ����
	for( int stage = 0; stage < nStages; stage++ )
	{
		for(int id = 0; id < nOfAStage[stage]; id++)
		{
			//std::cout << treeStructure[stage][id].pOri_str;
			//std::cout << "\n";
			char preInfo[100];
			sprintf(preInfo, "\n\n[stage:%d id:%d] \n", stage, id);
			str.append( preInfo );
			str.append( treeStructure[stage][id].pOri_str );
		}
	}

	const char * p_cstr = str.c_str();
	char * res = new char [ strlen(p_cstr)+1 ];
	strcpy(res, p_cstr);

	return res;
}

//���˺�����value�����е�\tת��Ϊ�ո�
void theTree::resolveChrome()
{
	//���н���
	for(int stage = 0; stage < nStages; stage++)
	{
		for( int id = 0; id < nOfAStage[stage]; id++ )
		{
			char * pStr = treeStructure[stage][id].pOri_str;
			if(pStr == NULL)			//���
			{
				break;
			}
						
			std::string line;
			for(int i = 0; true; i++)
			{
				if(pStr[i] == '\n' || pStr[i] == '\0')
				{
					////�����д���
					//����ð�ź�����
					//ð�Ŵ���0,���Ŵ���1
					std::vector<int> v_simbol;

					int colon_pos = -1;
					int qout1_pos1 = -1;
					int qout1_pos2 = -1;
					int qout2_pos1 = -1;
					int qout2_pos2 = -1;

					for(int j = 0; j < line.size(); j++)
					{
						if( line.at(j) == '\"' )
						{
							v_simbol.push_back(j);
							v_simbol.push_back(0);
						}
						if(line.at(j) == ':' )
						{
							v_simbol.push_back(j);
							v_simbol.push_back(1);
						}
					}
					//���":���
					int colon_spt = 1;		//��colon����ѭ������j������Ҫ��
					for(colon_spt = 1; colon_spt < v_simbol.size()/2; colon_spt++)
					{
						if( v_simbol.at(2*colon_spt) - v_simbol.at(2*(colon_spt-1)) == 1 )	//������������
						{
							if( v_simbol.at( 2*(colon_spt-1)+1 ) == 0 && v_simbol.at( 2*colon_spt+1 ) == 1 )
							{
								colon_pos = v_simbol.at( 2*colon_spt );
								qout1_pos2 = v_simbol.at( 2*(colon_spt-1) );
								break;										//ֻ����һ��������
							}
						}

					}
					//û��⵽��������Ҫ���򲻽��д���ֱ�ӵȴ�����
					if( colon_pos == -1 )		
					{
						;
					}
					else				//��鵽�������� ":Ҫ����Ѱʣ�µ�"
					{
						for(int j = 0; j < colon_spt; j++)			//�����ͷ���ҵ�һ��"
						{
							if( v_simbol.at(2*j+1) == 0 )		
							{
								qout1_pos1 = v_simbol.at(2*j);
								break;				//�ҵ�������
							}
						}
						for(int j = colon_spt+1; j < v_simbol.size()/2; j++)		//�����:��ʼ�����ҵ�һ��"
						{
							if( v_simbol.at(2*j+1) == 0 )
							{
								qout2_pos1 = v_simbol.at(2*j);
								break;
							}
						}
						for(int j = v_simbol.size()/2 - 1; j > colon_spt; j--)		//����������ҵ�һ��������"
						{
							if( v_simbol.at(2*j+1) == 0 )
							{
								qout2_pos2 = v_simbol.at(2*j);
								break;
							}
						}

						if(qout1_pos1 != qout1_pos2)		//����qout1һ����ȡֵ���ض���quet2�ϡ�������key
						{
							//����key
							char * p_key = new char [qout1_pos2 - qout1_pos1];
							for(int j = 0; j < qout1_pos2 - qout1_pos1 - 1; j++)
							{
								p_key[j] = line.at(qout1_pos1 + 1 + j);
							}
							p_key[qout1_pos2 - qout1_pos1 - 1] = '\0';

							(treeStructure[stage][id]).attr_table_word.push_back(p_key);

							//����value
							if(qout2_pos2 == -1 || qout2_pos2-qout2_pos1 <= 1)		//��valueû��""����ֻ��""û��ֵ����Ϊ��
							{
								(treeStructure[stage][id]).attr_table_word.push_back(NULL);
							}
							else
							{
								char * p_value = new char [qout2_pos2 - qout2_pos1];
								for(int j = 0; j < qout2_pos2 - qout2_pos1 - 1; j++)
								{
									if( line.at(qout2_pos1 + 1 + j) != '\t' )
									{
										p_value[j] = line.at(qout2_pos1 + 1 + j);
									}
									else
									{
										p_value[j] = ' ';
									}
								}
								p_value[qout2_pos2 - qout2_pos1 - 1] = '\0';
								(treeStructure[stage][id]).attr_table_word.push_back(p_value);
							}
						}
						else								//�쳣�����ֻ��һ��"���򲻽����κβ���
						{
							;
						}

						//��ֵ��ϣ����qout1_pos1��һ�������Ƿ�����
					}


					line.clear();
					v_simbol.clear();
					if(pStr[i] == '\0')
					{
						break;
					}
				}
				else
				{
					line.push_back( pStr[i]  );
				}
			}
			
		}
	}
}


//���ܣ�����firefix
//��������ͨ���Ⱥ� " ���keyֵ
//		��ͨ�� : ��֮��ķ��ż��valueֵ��
//		0��value���� ," ����ֹ��1��value���� "," ����ֹ��ϣ����Щ����ҳ���������������꣩

//���ӹ��ܣ��������е�\tת��Ϊ�ո�
void theTree::resolveFirefox()
{
	//���н���
	for(int stage = 0; stage < nStages; stage++)
	{
		for( int id = 0; id < nOfAStage[stage]; id++ )
		{
			char * pStr = treeStructure[stage][id].pOri_str;
			if(pStr == NULL)			//���
			{
				break;
			}
			int len = strlen(pStr);

			////��ȡ�Ͼ���Ϣ
			std::vector<int> v_SentPos;
			v_SentPos.push_back(0);			//��ͷ���
			//�ָ��־Ϊ ," �Լ����� \0
			for(int i = 0; i < len-1; i++)
			{
				if( pStr[i] == ',' && pStr[i+1] == '\"' )	//��һ���̾�
				{
					//���롮������λ��
					v_SentPos.push_back(i);

					//������һ�� '\"'��λ��
					v_SentPos.push_back(i+1);
				}
			}
			v_SentPos.push_back(len);		//��β�յ�λ��

			////��ÿ��������д���
			for( int i = 0; i < v_SentPos.size()/2; i++ )		//i��ʾ��key-value��
			{
				int startPos = v_SentPos.at(i*2);
				int endPos = v_SentPos.at(i*2 + 1);

				int mode = 0;			//0ΪvalueΪ����""��ֵ��1ΪvalueΪ��""��ֵ
				int k_s_pos = 0;
				int k_e_pos = 0;
				int v_s_pos = 0;
				int v_e_pos = 0;

				////����k-v�Ե���Ч�� ͨ��",�ṹ���
				int firstQuata = -1;
				int firstColon = -1;
				for(int j = startPos+1; j < endPos; j++)
				{
					if( pStr[j] == '\"' )
					{
						firstQuata = j;
						break;
					}
				}
				for(int j = startPos+1; j < endPos; j++)
				{
					if( pStr[j] == ':' )
					{
						firstColon = j;
						break;
					}
				}
				//�����ϵ��������
				if( firstColon == -1 || firstQuata == -1 )		//û�ҵ�
				{
					break;
				}
				if( firstColon-firstQuata != 1 )		//��һ��"���治��:
				{
					break;
				}
				if( firstColon-startPos == 1 )		//keyΪ��
				{
					break;
				}
				if( pStr[firstColon+1] == '\"' )
				{
					mode = 1;
					if( pStr[endPos-1] != '\"' || endPos-1 == firstColon+1 )		//valueǰ��"�����
					{
						break;
					}
				}
				else
				{
					mode = 0;
				}

				////����ȡֵ����
				k_s_pos = startPos+1;
				k_e_pos = firstQuata;
				if( mode == 1 )
				{
					v_s_pos = firstColon+2;
					v_e_pos = endPos-1;
				}
				else
				{
					v_s_pos = firstColon+1;
					v_e_pos = endPos;
				}
				char * key  = new char [k_e_pos - k_s_pos + 1];
				for(int i = 0; i < k_e_pos - k_s_pos; i++)
				{
					key[i] = pStr[k_s_pos+i];
				}
				key[k_e_pos-k_s_pos] = '\0';
				char * value = NULL;
				if(v_e_pos - v_s_pos > 0)
				{
					value = new char [v_e_pos - v_s_pos + 1];
					for( int j = 0; j < v_e_pos - v_s_pos; j++ )
					{
						if( pStr[ v_s_pos+j ] != '\t' )		//ת������Ϊ�ո�
						{
							value[j] = pStr[ v_s_pos+j ];
						}
						else
						{
							value[j] = ' ';
						}
					}
					value[v_e_pos - v_s_pos] = '\0';
				}

				treeStructure[stage][id].attr_table_word.push_back(key);
				treeStructure[stage][id].attr_table_word.push_back(value);
			}

		}
	}
}

char * theTree::getTreeAttrWList_str(std::vector<char *> & v_key, int start_n)
{
	//������ѯ�б�n_key��p_key
	int n_key = v_key.size();
	char * * p_key = NULL;
	if(n_key != 0)
	{
		p_key = new char * [n_key];
		for(int i = 0; i < n_key; i++)
		{
			p_key[i] = v_key.at(i);
		}
	}

	std::string outStr;

	for(int stage = 0; stage < nStages; stage++)
	{
		for(int id = 0; id < nOfAStage[stage]; id++)
		{
			//char preInfo[100];
			//sprintf(preInfo, "\n\n[stage: %d  id: %d] \n", stage, id);
			//outStr.append(preInfo);
			outStr.append("\n");

			theNode & node = treeStructure[stage][id];

			if(n_key == 0)
			{
				//iΪkey-value�ĵڼ���
				for(int i = 0; i*2+start_n+1 < node.attr_table_word.size(); i++)
				{
					if( node.attr_table_word.at(i*2+start_n) != NULL )
					{
						outStr.append( node.attr_table_word.at(i*2+start_n) );	//key
					}
					else
					{
						outStr.append("NULL");
					}
					outStr.append( "\t" );

					if( node.attr_table_word.at(i*2+start_n+1) != NULL )
					{
						outStr.append( node.attr_table_word.at(i*2+start_n+1) );	//value
					}
					else
					{
						outStr.append( "NULL" );
					}
					outStr.append( "\t" );
				}
				continue;			//ֱ��ȥ����һ��
			}

			
			for(int j = 0; j < n_key; j++)			//��������key���в��ң����
			{
				char * refKey = p_key[j];
				
				bool FLAG_F = false;
				//iΪkey-value�ĵڼ���
				for(int i = 0; i*2+start_n+1 < node.attr_table_word.size(); i++)
				{
					if( strcmp( node.attr_table_word.at(i*2+start_n), refKey ) == 0 )	//�ҵ�
					{
						if( node.attr_table_word.at(i*2+start_n) != NULL )
						{
							outStr.append( node.attr_table_word.at(i*2+start_n) );	//key
						}
						else
						{
							outStr.append("NULL");
						}
						outStr.append( "\t" );
						if( node.attr_table_word.at(i*2+start_n+1) != NULL )
						{
							outStr.append( node.attr_table_word.at(i*2+start_n+1) );	//value
						}
						else
						{
							outStr.append( "NULL" );
						}
						outStr.append( "\t" );
						FLAG_F = true;

						break;			//�ҵ��Ͳ���Ҫ����������
					}
				}
				if( FLAG_F == false )	//û�ҵ�
				{
					outStr.append( refKey );	//key
					outStr.append( "\t" );
					outStr.append( "NOT EXIST" );	//value
					outStr.append( "\t" );
				}
			}
		}
	}

	const char * c_str = outStr.c_str();
	char * res = new char [strlen(c_str)+1];
	strcpy(res, c_str);


	//�ͷ��ڴ�
	if(n_key != 0)
	{
		delete [] p_key;
	}

	return res;
}




//// favNode

favNode::favNode()
{
	;
}

favNode::favNode(int givenSP, int givenEP)
{
	startPos = givenSP;
	endPos = givenEP;
}

favNode::~favNode()
{
	delete [] p_psN;

	for(int i = 0; i < attr_table.size(); i++)
	{
		if( attr_table.at(i) != NULL )
		{
			delete [] attr_table.at(i);
		}
	}

	attr_table.clear();

}


////

bracePosInfo::bracePosInfo(int given_pos, int given_type)
{
	pos = given_pos;
	type = given_type;
}

bracePosInfo::~bracePosInfo()
{
	;
}



////

originalNode::originalNode(int givenS, int givenE, int givenI)
{
	startPos = givenS;
	endPos = givenE;
	id = givenI;
}

originalNode::~originalNode()
{
	;
}


void theTree::formTreefromJSON(char * filename)
{
	std::ifstream infile;
	infile.open(filename, std::ios::in);

	
	
	favNode ABC_F;			//����ȱʧ����ʵ��


	//���ļ����ݶ���string��
	std::stringstream buffer;
	buffer << infile.rdbuf();

	std::string contents(buffer.str());
	int strLen = contents.size();			//���滹���õ����ļ����ȱ���

	/*
#ifdef PRINT_MODE
	for(int i = 0; i < strLen; i++)
	{
		std::cout << contents.at(i);
	}
#endif
	*/

	//��ȡ����braceλ����Ϣ
	int n_headBrace = 0;
	int n_rearBrace = 0;
	//	std::vector<int> hb_pos;
	//	std::vector<int> rb_pos;

	std::vector<int> brace_pos;

	for(int i = 0; i < strLen; i++)
	{
		if(contents.at(i) == '{')
		{
			n_headBrace++;
			//			hb_pos.push_back(i);
			brace_pos.push_back(i);
			brace_pos.push_back(0);
		}
		else if(contents.at(i) == '}')
		{
			n_rearBrace++;
			//			rb_pos.push_back(i);
			brace_pos.push_back(i);
			brace_pos.push_back(1);
		}
	}

	//�ղؼ���������洢��ʽ
	favNode ** treeAtLevel;
	int nLevel = 0;
	int * nOfEachLevel = NULL;


	/////////////
	int n_brace = 0;
	n_brace = brace_pos.size()/2;

	/*
#ifdef PRINT_MODE
	//��ʾ����
	std::cout << "\n***********************************\n";
	for(int i = 0; i < n_brace; i++)
	{
		std::cout << brace_pos.at(2*i) << "------" << brace_pos.at(2*i+1) << "\n";
	}
#endif
	*/

	//������braceת��Ϊ�б���ʽ
	std::list<bracePosInfo> l_BTerraces;
	for(int i = 0; i < n_brace; i++)
	{
		l_BTerraces.push_back( bracePosInfo( brace_pos.at(2*i), brace_pos.at(2*i+1) ) );
	}


	std::list<bracePosInfo>::iterator i_f;			//��ͷ������
	std::list<bracePosInfo>::iterator i_b;			//β�������
	std::list<bracePosInfo>::iterator i_bofb;		//β���������ǰһ��������ɾ����λ

	std::vector< std::vector<originalNode> * > wholeOriginalTree;		//ԭʼ��������Ԫ��

	int cur_level = 1;

	while( l_BTerraces.begin() != l_BTerraces.end() )					//��ʾ�Ѿ��ͷ����
	{
		//һ��ѭ����ʼ

		i_b = l_BTerraces.begin();
		i_f = i_b;
		i_f ++;

		//���ڴ洢����ѭ��ʹ�õ�ԭʼ���Ĳ�
		std::vector<originalNode> * pThisLevel;			//���ͷ�originalNode ����ʱ����һ���ͷ�
		pThisLevel = new std::vector<originalNode>;
		int id_thisLevel = 0;

		wholeOriginalTree.push_back(pThisLevel);

		//ѭ���굱ǰ��������һ��	
		while( i_f != l_BTerraces.end() )
		{
			//����Ƿ�����ĩ�˽��
			if((*i_f).type == 1 && (*i_b).type == 0)
			{
				//���˵����ԭʼ����
				(*pThisLevel).push_back(originalNode((*i_b).pos, (*i_f).pos, id_thisLevel));
				id_thisLevel++;

				//���������ɾ������
				if( i_b != l_BTerraces.begin() )		//�����ڿ�ͷ
				{
					i_bofb = i_b;						//����b��ǰһ��λ��
					i_bofb --;

					l_BTerraces.erase(i_b);
					l_BTerraces.erase(i_f);

					i_b = i_bofb;						//�ָ�λ��
					i_b ++;
					i_f = i_b;
					i_f ++;
				}
				else									//��ǡ���ڿ�ͷ					
				{
					l_BTerraces.erase(i_b);
					l_BTerraces.erase(i_f);

					i_b = l_BTerraces.begin();
					if(i_b == l_BTerraces.end())		//��ɾ���˿�ͷ��û��������
					{
						break;
					}
					else								//��ɾ���˿�ͷ��������
					{
						i_f = i_b;
						i_f ++;
					}
				}

			}
			else
			{
				i_f ++;
				i_b ++;
			}


		}

		cur_level ++;
	}

	//����ԭʼ�����ĸ��ӹ�ϵ������ȷ��
	//�ɸ����Ѱ���ӽ��
	for(int fid = 0; fid < (* (wholeOriginalTree.at( wholeOriginalTree.size()-1 )) ).size(); fid++)
		//������߲㣨����㡭�����ĸ�����ţ���Ϊ-1
	{
		(* (wholeOriginalTree.at(wholeOriginalTree.size()-1)) ).at(fid).f_id = -1;		//�����������Ϊ-1
	}

	for(int stage = 0; stage < wholeOriginalTree.size() - 1; stage++)
		//���ڳ�ȥ���������и���㣬����ֵΪ-2�����ڷǵײ��������ã�
	{
		for(int id = 0; id < (*wholeOriginalTree.at(stage)).size(); id++ )
		{
			(*wholeOriginalTree.at(stage)).at(id).f_id = -2;
		}
	}

	for(int fid = 0; fid < (* (wholeOriginalTree.at( 0 )) ).size(); fid++)
		//������ײ��㣬�ӽ����ʼλ�ã�������Ϊ0
	{
		(* (wholeOriginalTree.at(0)) ).at(fid).n_son = 0;
		(* (wholeOriginalTree.at(0)) ).at(fid).s_id = 0;
	}


	for(int stage = wholeOriginalTree.size()-1; stage >= 1; stage --)		//�����������ɨ��
	{
		for(int fid = 0; fid < (* (wholeOriginalTree.at(stage)) ).size(); fid++  )		//�������ɨ��
		{
			int lowerThresh = (* (wholeOriginalTree.at(stage)) ).at(fid).startPos;
			int upperThresh = (* (wholeOriginalTree.at(stage)) ).at(fid).endPos;
			int nSons = 0;
			int lastSonID = 0;
			//����ͳ���ӽ�������
			for(int son_id = 0; son_id < (* (wholeOriginalTree.at(stage-1)) ).size(); son_id++)
			{
				if( (* (wholeOriginalTree.at(stage-1)) ).at(son_id).startPos > lowerThresh
					&& (* (wholeOriginalTree.at(stage-1)) ).at(son_id).endPos < upperThresh )
				{
					nSons ++;

					(* (wholeOriginalTree.at(stage-1)) ).at(son_id).f_id = fid;
					lastSonID = son_id;
				}
			}
			(* (wholeOriginalTree.at(stage)) ).at(fid).n_son = nSons;
			(* (wholeOriginalTree.at(stage)) ).at(fid).s_id = lastSonID - nSons + 1;		//�������ʼ��nSons��
		}
		//
	}

	/*
#ifdef PRINT_MODE
	//��ʾԭʼ�ղؼ���
	for(int stage = (wholeOriginalTree.size()-1); stage >=0; stage --)
	{
		std::cout << "\n*********************\n";
		for(int id = 0; id < (*(wholeOriginalTree.at(stage))).size(); id++ )
		{
			std::cout << "( node:" << id;
			std::cout << " f_id:" << (*(wholeOriginalTree.at(stage))).at(id).f_id;
			std::cout << " s_id:" << (*(wholeOriginalTree.at(stage))).at(id).s_id;
			std::cout << " n_son:" << (*(wholeOriginalTree.at(stage))).at(id).n_son;
			std::cout << ")\t";
		}
	}
#endif
	*/



	//�����ϣ���originalTree��һ����
	//ֻ�����������
	//��Ӧ�Ĳ�������
	treeAtLevel = new favNode* [wholeOriginalTree.size()];
	nOfEachLevel = new int [wholeOriginalTree.size()];
	nLevel = wholeOriginalTree.size();

	//�ȷ����ַ�����ƻ�������
	for(int stage = (wholeOriginalTree.size()-1); stage >=0; stage --)
	{
		nOfEachLevel[stage] = (*(wholeOriginalTree.at(stage))).size();
		treeAtLevel[stage] = new favNode [(*(wholeOriginalTree.at(stage))).size()];
		for(int id = 0; id < (*(wholeOriginalTree.at(stage))).size(); id++ )
		{
			treeAtLevel[stage][id].startPos = (*(wholeOriginalTree.at(stage))).at(id).startPos;
			treeAtLevel[stage][id].endPos = (*(wholeOriginalTree.at(stage))).at(id).endPos;
		}
	}

	//�ٸ��ƽ��֮����໥��ϵ
	//�����ĸ���㣬����ΪNULL
	for(int id = 0; id < (*(wholeOriginalTree.at( (wholeOriginalTree.size() - 1) ))).size(); id++)
	{
		treeAtLevel[wholeOriginalTree.size() - 1][id].p_fN = NULL;
	}
	//Ҷ�ӽ�㣬�ӽ������ΪNULL����������Ϊ0
	for(int id = 0; id < (*(wholeOriginalTree.at( (0) ))).size(); id++)
	{
		treeAtLevel[0][id].p_psN = NULL;
		treeAtLevel[0][id].nSon = 0;
	}

	//����������� �ӽ�� ��ϵ���и�ֵ
	for(int stage = (wholeOriginalTree.size()-1); stage >=1; stage --)
	{
		//����ÿ����������ռ�
		for(int id = 0; id < (*(wholeOriginalTree.at(stage))).size(); id++ )
		{
			if( (*(wholeOriginalTree.at(stage))).at(id).n_son > 0 )	//���ж���
			{
				treeAtLevel[stage][id].nSon = (*(wholeOriginalTree.at(stage))).at(id).n_son;
				treeAtLevel[stage][id].p_psN = new favNode * [ treeAtLevel[stage][id].nSon ];
				for(int sonid = (*(wholeOriginalTree.at(stage))).at(id).s_id, i = 0; 
					i < (*(wholeOriginalTree.at(stage))).at(id).n_son; i++)
				{
					//ָ�� = & ��ַ
					treeAtLevel[stage][id].p_psN[i]
					= &(treeAtLevel[stage-1][sonid+i]);
				}
			}
			else
			{
				treeAtLevel[stage][id].nSon = 0;
				treeAtLevel[stage][id].p_psN = NULL;
			}
		}
	}

	//����������� ����� ��ϵ���и�ֵ
	for(int stage = 0; stage < (wholeOriginalTree.size()-1); stage ++)
	{
		for(int id = 0; id < (*(wholeOriginalTree.at(stage))).size(); id++ )
		{
			if( (*(wholeOriginalTree.at(stage))).at(id).f_id >= 0 )
			{
				//ָ�� = & ��ַ
				treeAtLevel[stage][id].p_fN = &(treeAtLevel[stage+1]
				[ (*(wholeOriginalTree.at(stage))).at(id).f_id ]);
			}
			else
			{
				treeAtLevel[stage][id].p_fN	= &ABC_F;
			}
		}
	}

	/*
#ifdef PRINT_MODE
	//��ԭʼ�ղؼ���ת��ΪfavNode��������ʽ�����
	for(int stage = 0; stage < nLevel; stage++)
	{
		std::cout << "\nlevel: " << stage << "******************************\n";
		for(int i = 0; i < nOfEachLevel[stage]; i++)
		{
			std::cout << "(@:" << &(treeAtLevel[stage][i]);
			if(treeAtLevel[stage][i].p_fN == NULL)
			{
				std::cout << " Source Node ";
			}
			else if( treeAtLevel[stage][i].p_fN == &ABC_F )
			{
				std::cout << " Absent father ";
			}
			else		
			{
				std::cout << "  FNode:" << treeAtLevel[stage][i].p_fN;
			}


			if(treeAtLevel[stage][i].nSon == 0)
			{
				std::cout << " No Son ";
			}
			else
			{
				for(int j = 0; j < treeAtLevel[stage][i].nSon; j++)
				{
					std::cout << " pSon" << j << ":" << treeAtLevel[stage][i].p_psN[j];
				}
			}

			std::cout << ")-------";
			std::cout << "(" << treeAtLevel[stage][i].startPos;
			std::cout << " , " << treeAtLevel[stage][i].endPos <<")\n";
		}
	}
#endif
	*/


	//ɨ��ȱ�ٸ�����
	std::vector<favNode *> v_abcf_node;
	std::vector<int> v_stage;

	for(int stage = 0; stage < nLevel; stage++)		//�ӵײ���������
	{
		for(int i = 0; i < nOfEachLevel[stage]; i++)
		{
			if( treeAtLevel[stage][i].p_fN == &ABC_F )
			{
				v_abcf_node.push_back( &treeAtLevel[stage][i] );
				v_stage.push_back(stage);
			}
		}
	}


	//~~�ҵ��ж�~~
	//�ҵ��˿϶���Ķ��ϲ㡫������ˡ�ֻ������ϲ����ݼ���
	//�ϲ����ݾ�����˵�ǣ� nOfEachLevel  treeAtLevel[�ò�]
	for(int i = 0; i < v_abcf_node.size(); i++)
	{
		int fstage = 0;				//�ҵ��ĸ��׽��
		bool foundDIE = false;

		for(fstage = v_stage.at(i)+2; fstage < nLevel; fstage ++)	//����������
		{
			for(int fid = 0; fid < nOfEachLevel[fstage]; fid++)
			{
				if( (treeAtLevel[fstage][fid]).startPos < (*v_abcf_node.at(i)).startPos
					&& (treeAtLevel[fstage][fid]).endPos > (*v_abcf_node.at(i)).endPos )
					//��˵���������ҵ�- -
				{
					//�������ݣ�����ָ����������Ӽ�1�����Ķ���ָ������
					//��Ϊͬ��û��ͬ���ϵ����ˣ���-1���㲻�ؿ���
					//���¶���
					(*v_abcf_node.at(i)).p_fN = &(treeAtLevel[fstage][fid]);

					int n_fson = treeAtLevel[fstage][fid].nSon + 1;
					favNode ** p_pson;
					p_pson = new favNode * [n_fson]; 
					for(int j = 0; j < n_fson-1; j++)
					{
						p_pson[j] = treeAtLevel[fstage][fid].p_psN[j];
					}
					p_pson[n_fson - 1] = v_abcf_node.at(i);

					//ԭ������ָ���ͷ�
					delete [] treeAtLevel[fstage][fid].p_psN;

					//���µ�
					treeAtLevel[fstage][fid].p_psN = p_pson;
					treeAtLevel[fstage][fid].nSon ++;

					foundDIE = true;
					break;
				}
			}

			if(foundDIE == true)
			{
				break;
			}
		}

		//������û�ҵ��������
		//��˼�ǣ������ڵ���������һ�����ĸ����~~������
	}

	//���ļ���ɨ�����Ѿ����
/*
#ifdef PRINT_MODE
	//������������
	std::cout << "\nACCESS BY THE MEM STORAGE:\n";
	for(int stage = 0; stage < nLevel; stage++)
	{
		std::cout << "\nlevel: " << stage << "==================================\n";
		for(int i = 0; i < nOfEachLevel[stage]; i++)
		{
			std::cout << "(@:" << &(treeAtLevel[stage][i]);
			if(treeAtLevel[stage][i].p_fN == NULL)
			{
				std::cout << " Source Node ";
			}
			else if( treeAtLevel[stage][i].p_fN == &ABC_F )
			{
				std::cout << " Absent father ";
			}
			else		
			{
				std::cout << "  FNode:" << treeAtLevel[stage][i].p_fN;
			}


			if(treeAtLevel[stage][i].nSon == 0)
			{
				std::cout << " No Son ";
			}
			else
			{
				for(int j = 0; j < treeAtLevel[stage][i].nSon; j++)
				{
					std::cout << " pSon" << j << ":" << treeAtLevel[stage][i].p_psN[j];
				}
			}

			std::cout << ")-------";
			std::cout << "(" << treeAtLevel[stage][i].startPos;
			std::cout << " , " << treeAtLevel[stage][i].endPos <<")\n";
		}
	}
#endif
	*/


	/*
#ifdef PRINT_MODE
	//�����ӹ�ϵ���з������
	std::cout << "\n\nACCESS BY THE LOGIC OF MUTUAL RELATION:\n";

	//������ָ�����ֱ����������Ϊ��ǰ�󴫵ݲ�������
	std::vector<favNode *> * p_vLogicAc;			//��������Ҫ���ʵĽ��
	p_vLogicAc = new std::vector<favNode *>;
	for(int i = 0; i < nOfEachLevel[nLevel - 1]; i++)	//��������������
	{
		(*p_vLogicAc).push_back( &(treeAtLevel[nLevel-1][i]) );
	}

	int stage_n = 0;

	while( (* p_vLogicAc).size() != 0 )				//���������ʾ
	{
		std::cout << "\nlevel:" << stage_n << " *********************\n";
		std::vector<favNode *> * p_next;
		p_next = new std::vector<favNode *>;

		for(int i = 0; i < (*p_vLogicAc).size(); i++)	//ÿ���е����Ԫ��
		{
			//��ʾ
			std::cout << "\t( @" << (*p_vLogicAc).at(i);
			if( (*(*p_vLogicAc).at(i)).p_fN == NULL )
			{
				std::cout << " source node ";
			}
			else
			{
				std::cout << " FNode:" << (*(*p_vLogicAc).at(i)).p_fN;
			}
			if( (*(*p_vLogicAc).at(i)).nSon == 0 )
			{
				std::cout << " do not have son";
			}
			else
			{
				for(int j = 0; j < (*(*p_vLogicAc).at(i)).nSon; j++)
				{
					std::cout << " son" << j << ": " << (*(*p_vLogicAc).at(i)).p_psN[j];
				}
			}
			std::cout << ")-------(" << (*(*p_vLogicAc).at(i)).startPos << ",";
			std::cout << (*(*p_vLogicAc).at(i)).endPos << ")\n";
		}

		//������һ�ֵ�������
		for(int i = 0; i < (*p_vLogicAc).size(); i++)	//ÿ���е����Ԫ��
		{
			for(int j = 0; j < (*(*p_vLogicAc).at(i)).nSon; j++)
			{
				(*p_next).push_back( (*(*p_vLogicAc).at(i)).p_psN[j] );
			}
		}

		(*p_vLogicAc).clear();
		p_vLogicAc = p_next;

		stage_n ++;
	}

	(*p_vLogicAc).clear();
#endif
	*/

	//�������Խ�

	//�������ݳ�Ա��ֵ
	nStages = nLevel;
	nOfAStage = new int [nLevel];
	treeStructure = new theNode * [nLevel];

	//�����ӹ�ϵ���з������
	//std::cout << "\n\nACCESS BY THE LOGIC OF MUTUAL RELATION:\n";

	//������ָ�����ֱ����������Ϊ��ǰ�󴫵ݲ�������
	std::vector<favNode *> * p_vLogicAc;			//��������Ҫ���ʵĽ��
	p_vLogicAc = new std::vector<favNode *>;
	for(int i = 0; i < nOfEachLevel[nLevel - 1]; i++)	//��������������
	{
		(*p_vLogicAc).push_back( &(treeAtLevel[nLevel-1][i]) );
	}

	int stage_n = 0;

	while( (* p_vLogicAc).size() != 0 )				//���������ʾ
	{
		//std::cout << "\nlevel:" << stage_n << " *********************\n";
		std::vector<favNode *> * p_next;
		p_next = new std::vector<favNode *>;

		//�����˲��treeStructure�Ľ��ṹ
		treeStructure[stage_n] = new theNode [(*p_vLogicAc).size()];		//���ռ�
		nOfAStage[stage_n] = (*p_vLogicAc).size();							//�˲�������

		for(int i = 0; i < (*p_vLogicAc).size(); i++)	//ÿ���е����Ԫ��,��ӦΪtreeStructure[stage_n][i]
		{

			//��ֵ����
			(*(*p_vLogicAc).at(i)).p_theNode = &(treeStructure[stage_n][i]);
			(treeStructure[stage_n][i]).nSon = (*(*p_vLogicAc).at(i)).nSon;			//nSons
			//���ڸ�������ɨ��
			if(stage_n != 0)			//��Դ��㣬���и��ڵ�
			{
				(treeStructure[stage_n][i]).p_FN = ((*(*p_vLogicAc).at(i)).p_fN)->p_theNode;
			}
			else						//Դ��㣬���ÿ�
			{
				(treeStructure[stage_n][i]).p_FN = NULL;
			}
			(treeStructure[stage_n][i]).attr_table_num.push_back( (*(*p_vLogicAc).at(i)).startPos );	//ֻ������ֹλ�ò���
			(treeStructure[stage_n][i]).attr_table_num.push_back( (*(*p_vLogicAc).at(i)).endPos );
			//��ֵ�������

			//�����ӽ�����ɨ��
		}

		//������һ�ֵ�������
		for(int i = 0; i < (*p_vLogicAc).size(); i++)	//ÿ���е����Ԫ��
		{
			for(int j = 0; j < (*(*p_vLogicAc).at(i)).nSon; j++)
			{
				(*p_next).push_back( (*(*p_vLogicAc).at(i)).p_psN[j] );
			}
		}

		(*p_vLogicAc).clear();
		p_vLogicAc = p_next;

		stage_n ++;
	}

	(*p_vLogicAc).clear();			//��ģ���ڲ��ͷ�
	//����p_tree���ӽڵ��������	
	for(int stage = 0; stage < nStages; stage++)			//����ʼֵ
	{
		for(int id = 0; id < nOfAStage[stage]; id++)
		{
			treeStructure[stage][id].p_SN = NULL;
		}
	}
	for(int stage = 1; stage < nStages; stage++)			//�ɶ��ӵĸ��׾������׵Ķ���
	{
		for(int id = nOfAStage[stage]-1; id >= 0; id--)
		{
			(*(treeStructure[stage][id]).p_FN).p_SN = &(treeStructure[stage][id]);
		}
	}



	////�����ı����������ж�������////
	for(int stage = 0; stage < nStages; stage++)			//����ʼֵ
	{
		for(int id = 0; id < nOfAStage[stage]; id++)
		{
			//����������
			int upp_start = treeStructure[stage][id].attr_table_num.at(0);
			int upp_end = 0;
			int low_start = 0;
			int low_end = treeStructure[stage][id].attr_table_num.at(1);
			//ȷ�� �ϲ��ֵ��½� �²����Ͻ�
			for(int i = 0; i < brace_pos.size()/2; i++)
			{
				if( brace_pos.at(2*i) > upp_start )
				{
					upp_end = brace_pos.at(2*i);
					break;
				}
			}
			for(int i = brace_pos.size()/2 - 1; i >= 0; i--)
			{
				if( brace_pos.at(2*i) < low_end )
				{
					low_start = brace_pos.at(2*i);
					break;
				}
			}


			char * pStr = NULL;				//���ܺõ�����

			//�����½���Ϊ��ͬ����
			if(upp_start == low_start && upp_end == low_end)
			{
				if(upp_end - upp_start > 0)
				{		
					pStr = new char [upp_end - upp_start];
					for(int i = 0; i < upp_end - upp_start-1; i++)
					{
						pStr[i] = contents.at(upp_start + 1 + i);
					}
					pStr[upp_end - upp_start-1] = '\0';
				}
				else
				{
					pStr = NULL;
				}

				//treeStructure[stage][id].attr_table_word.push_back(pStr);
				treeStructure[stage][id].pOri_str = pStr;
			}
			else
			{
				if( upp_end - upp_start + low_end - low_start - 1 > 0 )
				{
					pStr = new char [upp_end - upp_start + low_end - low_start - 1];
					for(int i = 0; i < upp_end - upp_start -1; i++)
					{
						pStr[i] = contents.at(upp_start + 1 + i);
					}
					int bias = upp_end - upp_start - 1;
					for(int i = 0; i < low_end - low_start - 1; i++)
					{
						pStr[bias + i] = contents.at(low_start + 1 + i);
					}
					pStr[upp_end - upp_start + low_end - low_start - 2] = '\0';
				}
				else
				{
					pStr = NULL;
				}

				//treeStructure[stage][id].attr_table_word.push_back(pStr);
				treeStructure[stage][id].pOri_str = pStr;
			}



		}
	}


	////��� �����ı����������ж�������////

	/*
	////���� unicode ��ʾת��
	for(int stage = 0; stage < nStages; stage++)			
	{
		for(int id = 0; id < nOfAStage[stage]; id++)
		{
			//ȷ��name��λ��
			int i_item = -1;
			for(int i = 1; i < treeStructure[stage][id].attr_table_word.size(); i = i+=2)		//����keyλ��
			{
				char * p_attri = treeStructure[stage][id].attr_table_word.at(i);
				if( strcmp(p_attri, "name") == 0 )		//��ͬʱ
				{
					i_item = i;
					break;
				}
			}

			if(i_item == -1)		//û�ҵ�
			{
				treeStructure[stage][id].attr_table_num.push_back(-1.0);
			}
			else					//�ҵ�name
			{
				if( treeStructure[stage][id].attr_table_word.size() - 1 == i_item )			
					//�������������ܳ��֣���Ϊ���ǳɶԴ��ڵģ���˲�����name�����һ��
				{
					treeStructure[stage][id].attr_table_num.push_back(-1.0);
				}
				else if( treeStructure[stage][id].attr_table_word.at(i_item+1) == NULL )
					//���������Ӧname������һ������Ϊ�����
				{
					treeStructure[stage][id].attr_table_num.push_back(-1.0);
				}
				else
					//��������ǻ����ʵ����name�ļ�ֵ��Ӧ��value
				{
					treeStructure[stage][id].attr_table_num.push_back( float(i_item+1) );	//����value��Ӧ�ĽǱ�
					//����unicode�滻
					char * p_oriValue = treeStructure[stage][id].attr_table_word.at(i_item+1);
					int oriLength = strlen(p_oriValue);
					char * p_chValue = new char [oriLength];

				}
			}
		}
	}
	*/


	////�ͷ���Դ////
	infile.close();
	buffer.str("");			//��˵���������stringstream��������
	//hb_pos.clear();
	//rb_pos.clear();
	brace_pos.clear();

	l_BTerraces.clear();

	//wholeOriginalTree  ������һ�����������滹��stage������ָ�룬����
	for(int stage = 0; stage < wholeOriginalTree.size(); stage++)
	{
		( *wholeOriginalTree.at(stage) ).clear();
	}
	wholeOriginalTree.clear();

	//treeAtLevel ����������ָ�������
	for(int stage = 0; stage < nLevel; stage ++)
	{
		delete [] (favNode *)(treeAtLevel[stage]);
	}
	delete [] (favNode* *)(treeAtLevel);

	delete [] nOfEachLevel;

	v_abcf_node.clear();

	v_stage.clear();
}


void releaseTempTree(std::vector<std::vector<theNode *> *> & tree)
{
	for( int stage = 0; stage < tree.size(); stage++ )
	{
		for(int id = 0; id < tree.at(stage)->size(); id++ )
		{
			theNode * pNode = tree.at(stage)->at(id);
			delete (theNode *)pNode;
		}
	}
	for(int stage = 0; stage < tree.size(); stage ++)
	{
		tree.at(stage)->clear();
	}
	tree.clear();
}


//1---N_CLASS 2---E_CLASS 3---INV_CLASS 
//4---BOOKMARK 
//not exists ... 5---OTHERS 6---INV_SON
void theTree::completeChromeTree()
{
	for(int stage = 0; stage < nStages; stage++)
	{
		for(int id = 0; id < nOfAStage[stage]; id++)
		{
			theNode & node = treeStructure[stage][id];
			int log_type = -1;

			//�������е�keyֵ��Ѱ�Һ��ʵ�ѡ��
			for(int i = 0; i < node.attr_table_word.size()/2; i++)
			{
				char * key = node.attr_table_word.at( 2*i );
				if( strcmp(key, "type") == 0 )	//the key is type
				{
					char * value = node.attr_table_word.at( 2*i+1 );
					if( strcmp(value, "url")==0 )
					{
						//this is a BOOKMARK
						log_type = 4;

						//��������
						std::vector<char *> new_attri;

						char * k1 = NULL;
						char * k2 = NULL;
						char * k3 = NULL;
						char * v1 = NULL;
						char * v2 = NULL;
						char * v3 = NULL;
						bool FOUND_NAME = false;
						bool FOUND_URL = false;
						for(int j = 0; j < node.attr_table_word.size()/2; j++)
						{
							if( strcmp( node.attr_table_word.at( 2*j ), "name" ) == 0 && FOUND_NAME == false)
							{
								FOUND_NAME = true;
								k2 = new char [5];
								strcpy(k2, "name");
								char * bm_name = node.attr_table_word.at( 2*j+1 );
								if(bm_name != NULL)
								{
									v2 = new char [strlen(bm_name)+1];
									strcpy(v2, bm_name);
								}
								else
								{
									v2 = new char [5];
									strcpy(v2, "NULL");
								}
							}
							if( strcmp( node.attr_table_word.at( 2*j ), "url" ) == 0 && FOUND_URL == false )
							{
								FOUND_URL = true;
								k3 = new char [4];
								strcpy(k3, "url");
								char * bm_url = node.attr_table_word.at( 2*j+1 );
								if(bm_url != NULL)
								{
									v3 = new char [strlen(bm_url)+1];
									strcpy(v3, bm_url);
								}
								else
								{
									v3 = new char [5];
									strcpy(v3, "NULL");
								}
							}

							if(FOUND_NAME == true && FOUND_URL == true)
							{
								break;
							}
						}
						if(FOUND_NAME == true && FOUND_URL == true)
						{
							k1 = new char [5];
							strcpy(k1, "type");
							v1 = new char [9];
							strcpy(v1, "BOOKMARK");

							//���ԭ�е�attri_word
							for( int j = 0; j < node.attr_table_word.size(); j++ )
							{
								if( node.attr_table_word.at(j) != NULL )
								{
									delete [] node.attr_table_word.at(j);
								}
							}
							node.attr_table_word.clear();

							//�����µ�����ֵ
							node.attr_table_word.push_back(k1);
							node.attr_table_word.push_back(v1);
							node.attr_table_word.push_back(k2);
							node.attr_table_word.push_back(v2);
							node.attr_table_word.push_back(k3);
							node.attr_table_word.push_back(v3);
						}
						else
						{
							log_type = 5;			//OTHERS
							k1 = new char [5];
							strcpy(k1, "type");
							v1 = new char [7];
							strcpy(v1, "OTHERS");

							//���ԭ�е�attri_word
							for( int j = 0; j < node.attr_table_word.size(); j++ )
							{
								if( node.attr_table_word.at(j) != NULL )
								{
									delete [] node.attr_table_word.at(j);
								}
							}
							node.attr_table_word.clear();

							//�����µ�����ֵ
							node.attr_table_word.push_back(k1);
							node.attr_table_word.push_back(v1);

							if(k2 != NULL)
							{
								delete [] k2;
							}
							if(v2 != NULL)
							{
								delete [] v2;
							}
							if(k3 != NULL)
							{
								delete [] k3;
							}
							if(v3 != NULL)
							{
								delete [] v3;
							}
						}



						break;			//�ҵ������������ѭ��
					}
					else if( strcmp(value, "folder") == 0 )
					{
						//this is a E_CLASS Ĭ��Ϊ�շ��࣬�����Ҫ����ɨ��ȷ��
						log_type = 2;

						char * k1 = NULL;
						char * v1 = NULL;
						char * k2 = NULL;
						char * v2 = NULL;
						bool FOUND_NAME = false;
						for(int j = 0; j < node.attr_table_word.size()/2; j++)
						{
							if( strcmp( node.attr_table_word.at( 2*j ), "name" ) == 0 && FOUND_NAME == false)
							{
								FOUND_NAME = true;
								k2 = new char [5];
								strcpy(k2, "name");
								char * fd_name = node.attr_table_word.at( 2*j+1 );
								if(fd_name != NULL)
								{
									v2 = new char [strlen(fd_name)+1];
									strcpy(v2, fd_name);
								}
								else
								{
									v2 = new char [5];
									strcpy(v2, "NULL");
								}
							}
							if(FOUND_NAME == true)
							{
								break;
							}
						}

						if( FOUND_NAME == true )
						{
							k1 = new char [5];
							strcpy(k1, "type");
							v1 = new char [8];
							strcpy(v1, "E_CLASS");			//Ĭ����ΪE_CLASS

							//���ԭ�е�attri_word
							for( int j = 0; j < node.attr_table_word.size(); j++ )
							{
								if( node.attr_table_word.at(j) != NULL )
								{
									delete [] node.attr_table_word.at(j);
								}
							}
							node.attr_table_word.clear();

							//�����µ�����ֵ
							node.attr_table_word.push_back(k1);
							node.attr_table_word.push_back(v1);
							node.attr_table_word.push_back(k2);
							node.attr_table_word.push_back(v2);
						}
						else
						{
							//����ΪINV_CLASS 3
							log_type = 3;
							k1 = new char [5];
							strcpy(k1, "type");
							v1 = new char [10];
							strcpy(v1, "INV_CLASS");

							//���ԭ�е�attri_word
							for( int j = 0; j < node.attr_table_word.size(); j++ )
							{
								if( node.attr_table_word.at(j) != NULL )
								{
									delete [] node.attr_table_word.at(j);
								}
							}
							node.attr_table_word.clear();

							//�����µ�����ֵ
							node.attr_table_word.push_back(k1);
							node.attr_table_word.push_back(v1);

							if(k2 != NULL)
							{
								delete [] k2;
							}
							if(v2 != NULL)
							{
								delete [] v2;
							}
						}



						break;		//�ҵ������������ѭ��
					}

					
					
				}
				else
				{
					continue;
				}
			}

			//forѭ������û���ҵ��������
			if(log_type == -1)
			{
				log_type = 6;		//INV_SON

				char * k1 = NULL;
				char * v1 = NULL;

				k1 = new char [5];
				strcpy(k1, "type");
				v1 = new char [8];
				strcpy(v1, "INV_SON");

				//���ԭ�е�attri_word
				for( int j = 0; j < node.attr_table_word.size(); j++ )
				{
					if( node.attr_table_word.at(j) != NULL )
					{
						delete [] node.attr_table_word.at(j);
					}
				}
				node.attr_table_word.clear();

				node.attr_table_word.push_back(k1);
				node.attr_table_word.push_back(v1);
			}

			//ɨ����һ������ȡֵ
		}
	}

	//����ѭ�����Ǽ��N_CLASS��E_CLASS
	for(int stage = 0; stage < nStages; stage++)
	{
		for(int id = 0; id < nOfAStage[stage]; id++)
		{
			theNode & node = treeStructure[stage][id];
			char * pFolder = node.attr_table_word.at(1);
			if( strcmp(pFolder, "E_CLASS" )==0 )
			{
				if(node.nSon > 0)
				{
					pFolder[0] = 'N';		//ΪN_CLASS
				}
			}
		}
	}
}


void theTree::simpleOutput( char * filename1, char * filename2 )
{
	std::ofstream ofile1;
	std::ofstream ofile2;
	ofile1.open(filename1, std::ios::out);
	ofile2.open(filename2, std::ios::out);

	for( int stage = 0; stage < nStages; stage++ )
	{
		for( int id = 0; id < nOfAStage[stage]; id++ )
		{
			theNode & node = treeStructure[stage][id];
			bool IS_BOOKMARK = false;
			for(int i = 0; i < node.attr_table_word.size()/2; i++)
			{
				if( strcmp( node.attr_table_word.at(i*2), "type" ) == 0 )
				{
					if( strcmp( node.attr_table_word.at(i*2+1), "BOOKMARK" ) == 0 )
					{
						IS_BOOKMARK = true;
					}
					break;
				}
			}

			if( IS_BOOKMARK )
			{
				std::string str1;
				std::string str2;
				for( int i = 0; i < node.attr_table_word.size()/2; i++ )
				{
					char * key = node.attr_table_word.at( i*2 );
					char * value =node.attr_table_word.at( i*2+1 );
					if( key != NULL )
					{
						if( strcmp(key, "name") == 0 )
						{
							//�����str2��
							if( value != NULL )
							{
								str2.append( value );
								str2.append( "\t" );
							}
							else
							{
								str2.append( "NULL\t" );
							}
						}
						str1.append(key);
						str1.append("\t");
					}
					else
					{
						str1.append("NULL\t");
					}
					if(value != NULL)
					{
						str1.append(value);
						str1.append("\t");
					}
					else
					{
						str1.append("NULL\t");
					}
				}


				str1.append("\n");
				str2.append("\n");

				ofile1 << str1.c_str();
				ofile2 << str2.c_str();

				str1.clear();
				str2.clear();
			}



		}
	}

	ofile1.close();
	ofile2.close();

}

void theTree::outputTitles(std::vector<char*> & vTitle)
{
	for( int stage = 0; stage < nStages; stage++ )
	{
		for( int id = 0; id < nOfAStage[stage]; id++ )
		{
			if( (strcmp( treeStructure[stage][id].attr_table_word.at(0), "type")==0) && (treeStructure[stage][id].attr_table_word.size()==6) )
			{
				vTitle.push_back( treeStructure[stage][id].attr_table_word.at(3) );
			}
		}
	}
}


void theTree::updateLabels(int * g_labelList, int g_nTitles)
{
	//������
	labelSum.clear();

	//�������ͳ��
	nLabels = 5;
	for(int i = 0; i < nLabels; i++)
	{
		labelSum.push_back(0);
	}
	for(int i = 0; i < g_nTitles; i++)
	{
		labelSum.at( g_labelList[i]-1 ) ++;
	}

	//�ṹ����긳ֵ
	int i_cur = 0;
	for( int stage = 0; stage < nStages; stage++ )
	{
		for( int id = 0; id < nOfAStage[stage]; id++ )
		{
			if( (strcmp( treeStructure[stage][id].attr_table_word.at(0), "type")==0) && (treeStructure[stage][id].attr_table_word.size()==6) )
			{
				if( i_cur >= g_nTitles )	//���ֹ���
				{
					break;
				}
				treeStructure[stage][id].attr_table_num.push_back( g_labelList[i_cur] );
				char * k4 = new char [6];
				strcpy(k4, "label");
				char * v4 = new char [3];	//100����
				sprintf( v4, "%d", (g_labelList[i_cur])%100 );
				treeStructure[stage][id].attr_table_word.push_back( k4 );
				treeStructure[stage][id].attr_table_word.push_back( v4 );

				i_cur++;
			}
		}
	}
	

}

void theTree::fixLabels()
{
	nLabels = 5;
	for( int i = 0; i < nLabels; i++ )
	{
		labelSum.push_back(0);
	}

	for( int stage = 0; stage < nStages; stage++ )
	{
		for( int id = 0; id < nOfAStage[stage]; id++ )
		{
			treeStructure[stage][id].attr_table_num.clear();

			treeStructure[stage][id].attr_table_num.push_back(0);
			treeStructure[stage][id].attr_table_num.push_back(0);
			treeStructure[stage][id].attr_table_num.push_back(0);

			if( treeStructure[stage][id].attr_table_word.size()==8 )
			{
				if( strcmp(treeStructure[stage][id].attr_table_word.at(6), "label")==0 )
				{
					int label = atoi( treeStructure[stage][id].attr_table_word.at(7) );
					labelSum.at(label-1) ++;
					treeStructure[stage][id].attr_table_num.at(2) = label;
				}
			}
		}
	}
}

theTree * theTree::getMenuTree()
{

	if( nLabels==0 || labelSum.size()!=nLabels )
	{
		return NULL;
	}
	theTree * pTree = new theTree;

	int sum_bkmks = 0;					//��ǩ����
	for(int i = 0; i < nLabels; i++)
	{
		sum_bkmks += labelSum.at(i);
	}

	char pClassName[5][20] = {"economy", "education", "entertainment", "sport", "technology"};
	char * k1;
	char * k2;
	char * k3;
	char * v1;
	char * v2;
	char * v3;

	//�����ṹ��Ϣ
	pTree->nStages = 3;

	pTree->nOfAStage = new int [3];
	pTree->nOfAStage[0] = 1;		//����
	pTree->nOfAStage[1] = 5;		//Ŀ¼��
	pTree->nOfAStage[2] = sum_bkmks;		//�ܱ�������

	pTree->treeStructure = new theNode * [3];

	pTree->treeStructure[0] = new theNode [1];	
	//�ṹ��Ϣ
	pTree->treeStructure[0][0].nSon = pTree->nOfAStage[1];		//�������
	pTree->treeStructure[0][0].p_FN = NULL;

	k1 = new char [5];
	strcpy(k1, "type");
	v1 = new char [10];
	strcpy(v1, "N_CLASS");
	pTree->treeStructure[0][0].attr_table_word.push_back(k1);
	pTree->treeStructure[0][0].attr_table_word.push_back(v1);

	pTree->treeStructure[1] = new theNode [ pTree->nOfAStage[1] ];	//Ŀ¼�ڵ�
	for(int id = 0; id < pTree->nOfAStage[1]; id++)
	{
		theNode * pND = &(pTree->treeStructure[1][id]);
		//�ṹ��Ϣ
		pND->nSon = labelSum.at(id);
		pND->p_FN = &(pTree->treeStructure[0][0]);

		k1 = new char [5];
		strcpy(k1, "type");
		v1 = new char [10];
		strcpy(v1, "N_CLASS");
		k2 = new char [5];
		strcpy(k2, "name");
		v2 = new char [20];
		strcpy(v2, pClassName[id]);
		pND->attr_table_word.push_back( k1 );
		pND->attr_table_word.push_back( v1 );
		pND->attr_table_word.push_back( k2 );
		pND->attr_table_word.push_back( v2 );
	}

	pTree->treeStructure[2] = new theNode [ pTree->nOfAStage[2] ];	//ҳ��ڵ�
	int i_title = 0;
	for( int fid = 1; fid < pTree->nOfAStage[1]+1; fid++ )		//�����������������������1��ʼ
	{
		for( int stage = 0; stage < nStages; stage++ )
		{
			//for( int id = 0; id < nOfAStage[stage]; id++ )
			for( int id = nOfAStage[stage]-1; id >=0; id-- )	//��ʱ��˳������
			{
				if( (strcmp( treeStructure[stage][id].attr_table_word.at(1), "BOOKMARK")==0) 
					&& (treeStructure[stage][id].attr_table_word.size()==8) 
					&& (treeStructure[stage][id].attr_table_num.at(2) == fid) )
					//����
				{
					//vTitle.push_back( treeStructure[stage][id].attr_table_word.at(3) );
					//�ҵ���
					theNode * pOri = &(treeStructure[stage][id]);			//ԭʼ���
					
					theNode * pN = &(pTree->treeStructure[2][i_title]);		//�������

					//�ṹ��Ϣ
					pN->nSon = 0;
					pN->p_FN = &(pTree->treeStructure[1][fid-1]);

					k1 = new char [5];
					strcpy(k1, "type");
					v1 = new char [9];
					strcpy(v1, "BOOKMARK");
					k2 = new char [5];
					strcpy(k2, "name");
					v2 = new char [ strlen( pOri->attr_table_word.at(3) )+1 ];	//����
					strcpy( v2, pOri->attr_table_word.at(3) );
					k3 = new char [4];
					strcpy(k3, "url");
					v3 = new char [ strlen( pOri->attr_table_word.at(5) )+1 ];	//url
					strcpy( v3, pOri->attr_table_word.at(5) );

					pN->attr_table_word.push_back(k1);
					pN->attr_table_word.push_back(v1);
					pN->attr_table_word.push_back(k2);
					pN->attr_table_word.push_back(v2);
					pN->attr_table_word.push_back(k3);
					pN->attr_table_word.push_back(v3);

					i_title ++;
				}
			}
		}
	}

	//��ȫ������ʣ�µ�p_SN�ṹ��Ϣ
	//��1��
	pTree->treeStructure[0][0].p_SN = &( pTree->treeStructure[1][0] );
	//��2��
	int i_temp = 0;			//��3������Ӧ����
	int i_temp_sub = 0;		//�ű�
	for(int i = 0; i < nLabels; i++)
	{
		pTree->treeStructure[1][i].p_SN = &( pTree->treeStructure[2][i_temp] );
		i_temp += labelSum.at(i_temp_sub);
		i_temp_sub++;
	}
	//��3��
	for(int id = 0; id < pTree->nOfAStage[2]; id++)
	{
		pTree->treeStructure[2][id].p_SN = NULL;
	}


	return pTree;
}

#endif