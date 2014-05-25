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

//若没有title，则返回-1
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

	//生成结构码
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

	//得到内容信息
	std::vector<char *> v_key;
	char * contents = getTreeAttrWList_str(v_key, 0);

	ofile << contents;

	delete [] contents;

	ofile.close();

}

bool theTree::formTreefromTRI(char * filename)
{
	const int n_char = 10000;
	////〇、事先操作
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
	if( strlen(outStr) < 2 )		//针对最后一行无内容的情况
	{
		n_lines --;
	}
	n_lines --;		//减去第一行的结构码
	//得到n_lines即为总长度


	////一、文件操作
	std::ifstream infile;
	infile.open(filename, std::ios::in);

	////二、创建树的结构

	//原始结构码
	std::vector<int> v_rawN;
	//每层包含的节点总数
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

	////验证数据是否足够
	int n_sum_Nodes = 1;			//根节点
	for(int i = 0; i < v_rawN.size(); i++)
	{
		n_sum_Nodes += v_rawN.at(i);
	}
	if( n_lines < n_sum_Nodes )		//若行数少于构成树的节点数，则直接跳出
	{
		v_rawN.clear();
		v_stageN.clear();

		infile.close();
		return false;
	}

	//继续操作结构码
	v_stageN.push_back(1);			//第一层必须是一个单个节点

	int n_bros = 1;					//上一层的原始结构码～

	std::vector<int>::iterator i_rawN;
	i_rawN = v_rawN.begin();


	while(true)					//每轮循环完成一层的统计
	{
		int n_thisStage = 0;
		for(int i = 0; i < n_bros; i++)
		{
			n_thisStage += (*i_rawN);
			i_rawN ++;
		}
		v_stageN.push_back(n_thisStage);
		n_bros = n_thisStage;

		if( i_rawN == v_rawN.end() )			//此循环终止条件规定，必须rawN正确方可跳出循环
		{
			break;
		}
	}

	//建立结点形式
	treeStructure = new theNode* [v_stageN.size()];
	for(int i = 0; i < v_stageN.size(); i++)
	{
		treeStructure[i] = new theNode[v_stageN.at(i)];
	}

	//赋予各个节点的n_Sons的值
	i_rawN = v_rawN.begin();
	for( int stage = 0; stage < v_stageN.size()-1; stage++ )
	{
		for(int i = 0; i < v_stageN.at(stage); i++)
		{
			treeStructure[stage][i].nSon = (*i_rawN);		//可以为0，代表到此终结
			i_rawN++;									//此处也受格式正确与否的影响
		}
	}
	for( int i = 0; i < v_stageN.at(v_stageN.size()-1); i++ )			//最底端的节点一定不存在son
	{
		treeStructure[v_stageN.size()-1][i].nSon = 0;
	}

	//p_pSN关系的确定
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

	//p_pFN关系的确定（由psN的关系逆推）
	treeStructure[0][0].p_FN = NULL;
	for( int stage = 0; stage < v_stageN.size()-1; stage++ )
	{
		for(int i = 0; i < v_stageN.at(stage); i++)
		{
			//这里是父节点的内容
			for( int j = 0; j < treeStructure[stage][i].nSon; j++ )
			{
				((theNode *)(treeStructure[stage][i].p_SN))[j].p_FN = &treeStructure[stage][i];
			}
		}
	}

	//完成树结构的创建


	//三、扫描树中的内容，存入节点的参数表中
	for( int stage = 0; stage < v_stageN.size(); stage++ )
	{
		for(int id = 0; id < v_stageN.at(stage); id++)
		{
			infile.getline(outStr, n_char);

			//开辟新空间～存入theNode类中
			std::vector<int> v_word;
			v_word.push_back(0);
			int len = strlen( outStr );
			for( int i = 0; i < len; i++ )
			{
				if( outStr[i] == '\t' )
				{
					outStr[i] = '\0';
					
					if(i+1 != len)		//避免加入最后一个\t加入
					{
						v_word.push_back(i+1);
					}
				}
			}
			for( int i = 0; i < v_word.size()/2; i++ )		//key-value成对出现
			{
				//新建此行拷贝
				char * key = NULL;
				char * value = NULL;
				int len_k = strlen( &(outStr[v_word.at(i*2)]) );
				int len_v = strlen( &(outStr[v_word.at(i*2+1)]) );
				if(len_k == 0)			//拷贝key
				{
					key = NULL;
				}
				else
				{
					key = new char [len_k+1];
					strcpy( key, &(outStr[v_word.at(i*2)]) );
				}
				if(len_v == 0)			//拷贝value
				{
					value = NULL;
				}
				else
				{
					value = new char [len_v+1];
					strcpy( value, &(outStr[v_word.at(i*2+1)]) );
				}

				//存入相关
				treeStructure[stage][id].attr_table_word.push_back(key);
				treeStructure[stage][id].attr_table_word.push_back(value);
			}
		}
	}


	//四、将扫描的结构信息存入类中
	nStages = v_stageN.size();
	nOfAStage = new int [nStages];
	for(int i = 0; i < nStages; i++)
	{
		nOfAStage[i] = v_stageN.at(i);
	}

	//五、释放内存
	v_stageN.clear();
	v_rawN.clear();

	infile.close();	

	return true;
}

//tree好像只有nSon信息和自身信息是靠谱的。
//其他结构信息，都靠自己补齐。
void theTree::formIETree(std::vector<std::vector<theNode *> *> & tree)
{
	nStages = tree.size();
	nOfAStage = new int [nStages];
	treeStructure = new theNode * [nStages];

	//nOfStage赋值
	for(int stage = 0; stage < nStages; stage ++)
	{
		nOfAStage[stage] = tree.at(stage)->size();
		treeStructure[stage] = new theNode [ nOfAStage[stage] ];
	}

	//Node赋值
	for(int stage = 0; stage < nStages; stage++)
	{
		for(int id = 0; id < nOfAStage[stage]; id++)
		{
			theNode * p_node = tree.at(stage)->at(id);
			treeStructure[stage][id].nSon = p_node->nSon;
			//theTree的赋值
			//int n_attr_i = p_node->attr_table_num.size();
			//for(int i = 0; i < n_attr_i; i++)
			//{
			//	treeStructure[stage][id].attr_table_num.push_back( p_node->attr_table_num.at(i) );
			//}
			//改动：手动将treeStructure中的attr_table_num中插入2个-1
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

	//Node确定父子关系
	treeStructure[0][0].p_FN = NULL;				//补第一行的父节点
	for(int stage = 0; stage < nStages-1; stage++)
	{
		int id_next_byfar = 0;

		for(int id = 0; id < nOfAStage[stage]; id++)
		{
			int n_son = treeStructure[stage][id].nSon;
			treeStructure[stage][id].p_SN = &( treeStructure[stage+1][id_next_byfar] );		//父节点确定子节点
			for(int sid = 0; sid < n_son; sid++)					//子节点确定父节点
			{
				treeStructure[stage+1][sid+id_next_byfar].p_FN = &( treeStructure[stage][id] );
			}
		}
	}
	for(int id = 0; id < nOfAStage[nStages-1]; id++)	//补最后一行的子节点
	{
		treeStructure[nStages-1][id].p_SN = NULL;
	}

	return;
}

char * theTree::getTreeOriginList_str()
{
	std::string str;

	//打印出来
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

//将此函数中value中所有的\t转化为空格
void theTree::resolveChrome()
{
	//按行进行
	for(int stage = 0; stage < nStages; stage++)
	{
		for( int id = 0; id < nOfAStage[stage]; id++ )
		{
			char * pStr = treeStructure[stage][id].pOri_str;
			if(pStr == NULL)			//查错
			{
				break;
			}
						
			std::string line;
			for(int i = 0; true; i++)
			{
				if(pStr[i] == '\n' || pStr[i] == '\0')
				{
					////进行行处理
					//查找冒号和引号
					//冒号代号0,引号代号1
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
					//检测":相接
					int colon_spt = 1;		//将colon看成循环变量j，后面要用
					for(colon_spt = 1; colon_spt < v_simbol.size()/2; colon_spt++)
					{
						if( v_simbol.at(2*colon_spt) - v_simbol.at(2*(colon_spt-1)) == 1 )	//两个符号相连
						{
							if( v_simbol.at( 2*(colon_spt-1)+1 ) == 0 && v_simbol.at( 2*colon_spt+1 ) == 1 )
							{
								colon_pos = v_simbol.at( 2*colon_spt );
								qout1_pos2 = v_simbol.at( 2*(colon_spt-1) );
								break;										//只检测第一个此类型
							}
						}

					}
					//没检测到此行满足要求，则不进行处理，直接等待结束
					if( colon_pos == -1 )		
					{
						;
					}
					else				//检查到此行满足 ":要求，搜寻剩下的"
					{
						for(int j = 0; j < colon_spt; j++)			//正序从头查找第一个"
						{
							if( v_simbol.at(2*j+1) == 0 )		
							{
								qout1_pos1 = v_simbol.at(2*j);
								break;				//找到即结束
							}
						}
						for(int j = colon_spt+1; j < v_simbol.size()/2; j++)		//正序从:开始处查找第一个"
						{
							if( v_simbol.at(2*j+1) == 0 )
							{
								qout2_pos1 = v_simbol.at(2*j);
								break;
							}
						}
						for(int j = v_simbol.size()/2 - 1; j > colon_spt; j--)		//逆序从最后查找第一个遇到的"
						{
							if( v_simbol.at(2*j+1) == 0 )
							{
								qout2_pos2 = v_simbol.at(2*j);
								break;
							}
						}

						if(qout1_pos1 != qout1_pos2)		//首先qout1一定有取值，必定在quet2上。即具有key
						{
							//生成key
							char * p_key = new char [qout1_pos2 - qout1_pos1];
							for(int j = 0; j < qout1_pos2 - qout1_pos1 - 1; j++)
							{
								p_key[j] = line.at(qout1_pos1 + 1 + j);
							}
							p_key[qout1_pos2 - qout1_pos1 - 1] = '\0';

							(treeStructure[stage][id]).attr_table_word.push_back(p_key);

							//生成value
							if(qout2_pos2 == -1 || qout2_pos2-qout2_pos1 <= 1)		//若value没有""或者只有""没有值，则为空
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
						else								//异常情况，只有一个"，则不进行任何操作
						{
							;
						}

						//赋值完毕，检查qout1_pos1第一个引号是否正常
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


//功能，解析firefix
//方法：先通过先后 " 检查key值
//		再通过 : 和之后的符号检查value值，
//		0号value到达 ," 处截止；1号value到达 "," 处截止（希望那些熊网页。。。。。。尼玛）

//附加功能：将参数中的\t转化为空格
void theTree::resolveFirefox()
{
	//按行进行
	for(int stage = 0; stage < nStages; stage++)
	{
		for( int id = 0; id < nOfAStage[stage]; id++ )
		{
			char * pStr = treeStructure[stage][id].pOri_str;
			if(pStr == NULL)			//查错
			{
				break;
			}
			int len = strlen(pStr);

			////获取断句信息
			std::vector<int> v_SentPos;
			v_SentPos.push_back(0);			//先头起点
			//分割标志为 ," 以及最后的 \0
			for(int i = 0; i < len-1; i++)
			{
				if( pStr[i] == ',' && pStr[i+1] == '\"' )	//是一个短句
				{
					//存入‘，’的位置
					v_SentPos.push_back(i);

					//存入下一个 '\"'的位置
					v_SentPos.push_back(i+1);
				}
			}
			v_SentPos.push_back(len);		//结尾终点位置

			////对每个单句进行处理
			for( int i = 0; i < v_SentPos.size()/2; i++ )		//i表示个key-value对
			{
				int startPos = v_SentPos.at(i*2);
				int endPos = v_SentPos.at(i*2 + 1);

				int mode = 0;			//0为value为不加""的值，1为value为加""的值
				int k_s_pos = 0;
				int k_e_pos = 0;
				int v_s_pos = 0;
				int v_e_pos = 0;

				////检查此k-v对的有效性 通过",结构检测
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
				//不符合的数种情况
				if( firstColon == -1 || firstQuata == -1 )		//没找到
				{
					break;
				}
				if( firstColon-firstQuata != 1 )		//第一个"后面不是:
				{
					break;
				}
				if( firstColon-startPos == 1 )		//key为空
				{
					break;
				}
				if( pStr[firstColon+1] == '\"' )
				{
					mode = 1;
					if( pStr[endPos-1] != '\"' || endPos-1 == firstColon+1 )		//value前后"不配对
					{
						break;
					}
				}
				else
				{
					mode = 0;
				}

				////进行取值解析
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
						if( pStr[ v_s_pos+j ] != '\t' )		//转换缩进为空格
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
	//创建查询列表n_key和p_key
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
				//i为key-value的第几对
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
				continue;			//直接去找下一个
			}

			
			for(int j = 0; j < n_key; j++)			//对于所有key进行查找，输出
			{
				char * refKey = p_key[j];
				
				bool FLAG_F = false;
				//i为key-value的第几对
				for(int i = 0; i*2+start_n+1 < node.attr_table_word.size(); i++)
				{
					if( strcmp( node.attr_table_word.at(i*2+start_n), refKey ) == 0 )	//找到
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

						break;			//找到就不必要继续向下找
					}
				}
				if( FLAG_F == false )	//没找到
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


	//释放内存
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

	
	
	favNode ABC_F;			//定义缺失树的实例


	//将文件内容读入string中
	std::stringstream buffer;
	buffer << infile.rdbuf();

	std::string contents(buffer.str());
	int strLen = contents.size();			//后面还会用到的文件长度变量

	/*
#ifdef PRINT_MODE
	for(int i = 0; i < strLen; i++)
	{
		std::cout << contents.at(i);
	}
#endif
	*/

	//获取左右brace位置信息
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

	//收藏夹树的物理存储形式
	favNode ** treeAtLevel;
	int nLevel = 0;
	int * nOfEachLevel = NULL;


	/////////////
	int n_brace = 0;
	n_brace = brace_pos.size()/2;

	/*
#ifdef PRINT_MODE
	//显示部分
	std::cout << "\n***********************************\n";
	for(int i = 0; i < n_brace; i++)
	{
		std::cout << brace_pos.at(2*i) << "------" << brace_pos.at(2*i+1) << "\n";
	}
#endif
	*/

	//将左右brace转化为列表形式
	std::list<bracePosInfo> l_BTerraces;
	for(int i = 0; i < n_brace; i++)
	{
		l_BTerraces.push_back( bracePosInfo( brace_pos.at(2*i), brace_pos.at(2*i+1) ) );
	}


	std::list<bracePosInfo>::iterator i_f;			//先头迭代器
	std::list<bracePosInfo>::iterator i_b;			//尾随迭代器
	std::list<bracePosInfo>::iterator i_bofb;		//尾随迭代器的前一个，用于删除后定位

	std::vector< std::vector<originalNode> * > wholeOriginalTree;		//原始树的所有元素

	int cur_level = 1;

	while( l_BTerraces.begin() != l_BTerraces.end() )					//表示已经释放完毕
	{
		//一轮循环开始

		i_b = l_BTerraces.begin();
		i_f = i_b;
		i_f ++;

		//用于存储此轮循环使用的原始树的层
		std::vector<originalNode> * pThisLevel;			//在释放originalNode 树的时候再一起释放
		pThisLevel = new std::vector<originalNode>;
		int id_thisLevel = 0;

		wholeOriginalTree.push_back(pThisLevel);

		//循环完当前存在数据一遍	
		while( i_f != l_BTerraces.end() )
		{
			//检查是否属于末端结点
			if((*i_f).type == 1 && (*i_b).type == 0)
			{
				//将此点存入原始树中
				(*pThisLevel).push_back(originalNode((*i_b).pos, (*i_f).pos, id_thisLevel));
				id_thisLevel++;

				//进行链表的删减操作
				if( i_b != l_BTerraces.begin() )		//若不在开头
				{
					i_bofb = i_b;						//备份b中前一个位置
					i_bofb --;

					l_BTerraces.erase(i_b);
					l_BTerraces.erase(i_f);

					i_b = i_bofb;						//恢复位置
					i_b ++;
					i_f = i_b;
					i_f ++;
				}
				else									//若恰好在开头					
				{
					l_BTerraces.erase(i_b);
					l_BTerraces.erase(i_f);

					i_b = l_BTerraces.begin();
					if(i_b == l_BTerraces.end())		//若删掉了开头就没有数据了
					{
						break;
					}
					else								//若删掉了开头还有数据
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

	//对于原始树结点的父子关系，进行确认
	//由父结点寻找子结点
	for(int fid = 0; fid < (* (wholeOriginalTree.at( wholeOriginalTree.size()-1 )) ).size(); fid++)
		//对于最高层（根结点……）的父结点编号，设为-1
	{
		(* (wholeOriginalTree.at(wholeOriginalTree.size()-1)) ).at(fid).f_id = -1;		//将父结点编号设为-1
	}

	for(int stage = 0; stage < wholeOriginalTree.size() - 1; stage++)
		//对于除去根结点的所有父结点，都赋值为-2（用于非底层结点上升用）
	{
		for(int id = 0; id < (*wholeOriginalTree.at(stage)).size(); id++ )
		{
			(*wholeOriginalTree.at(stage)).at(id).f_id = -2;
		}
	}

	for(int fid = 0; fid < (* (wholeOriginalTree.at( 0 )) ).size(); fid++)
		//对于最底层结点，子结点起始位置，个数设为0
	{
		(* (wholeOriginalTree.at(0)) ).at(fid).n_son = 0;
		(* (wholeOriginalTree.at(0)) ).at(fid).s_id = 0;
	}


	for(int stage = wholeOriginalTree.size()-1; stage >= 1; stage --)		//由上向下逐层扫描
	{
		for(int fid = 0; fid < (* (wholeOriginalTree.at(stage)) ).size(); fid++  )		//层中逐个扫描
		{
			int lowerThresh = (* (wholeOriginalTree.at(stage)) ).at(fid).startPos;
			int upperThresh = (* (wholeOriginalTree.at(stage)) ).at(fid).endPos;
			int nSons = 0;
			int lastSonID = 0;
			//进行统计子结点的数量
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
			(* (wholeOriginalTree.at(stage)) ).at(fid).s_id = lastSonID - nSons + 1;		//从这个开始的nSons个
		}
		//
	}

	/*
#ifdef PRINT_MODE
	//显示原始收藏夹树
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



	//物理上，跟originalTree是一样的
	//只不过是正序的
	//相应的参数设置
	treeAtLevel = new favNode* [wholeOriginalTree.size()];
	nOfEachLevel = new int [wholeOriginalTree.size()];
	nLevel = wholeOriginalTree.size();

	//先分配地址、复制基本数据
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

	//再复制结点之间的相互关系
	//根结点的父结点，设置为NULL
	for(int id = 0; id < (*(wholeOriginalTree.at( (wholeOriginalTree.size() - 1) ))).size(); id++)
	{
		treeAtLevel[wholeOriginalTree.size() - 1][id].p_fN = NULL;
	}
	//叶子结点，子结点设置为NULL，数量设置为0
	for(int id = 0; id < (*(wholeOriginalTree.at( (0) ))).size(); id++)
	{
		treeAtLevel[0][id].p_psN = NULL;
		treeAtLevel[0][id].nSon = 0;
	}

	//对于其他结点 子结点 关系进行赋值
	for(int stage = (wholeOriginalTree.size()-1); stage >=1; stage --)
	{
		//按照每层的情况分配空间
		for(int id = 0; id < (*(wholeOriginalTree.at(stage))).size(); id++ )
		{
			if( (*(wholeOriginalTree.at(stage))).at(id).n_son > 0 )	//若有儿子
			{
				treeAtLevel[stage][id].nSon = (*(wholeOriginalTree.at(stage))).at(id).n_son;
				treeAtLevel[stage][id].p_psN = new favNode * [ treeAtLevel[stage][id].nSon ];
				for(int sonid = (*(wholeOriginalTree.at(stage))).at(id).s_id, i = 0; 
					i < (*(wholeOriginalTree.at(stage))).at(id).n_son; i++)
				{
					//指针 = & 地址
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

	//对于其他结点 父结点 关系进行赋值
	for(int stage = 0; stage < (wholeOriginalTree.size()-1); stage ++)
	{
		for(int id = 0; id < (*(wholeOriginalTree.at(stage))).size(); id++ )
		{
			if( (*(wholeOriginalTree.at(stage))).at(id).f_id >= 0 )
			{
				//指针 = & 地址
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
	//将原始收藏夹树转换为favNode物理结点形式，输出
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


	//扫描缺少父结点的
	std::vector<favNode *> v_abcf_node;
	std::vector<int> v_stage;

	for(int stage = 0; stage < nLevel; stage++)		//从底层向上找起
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


	//~~找爹行动~~
	//找到了肯定会改动上层～～～因此～只需更新上层数据即可
	//上层数据具体来说是： nOfEachLevel  treeAtLevel[该层]
	for(int i = 0; i < v_abcf_node.size(); i++)
	{
		int fstage = 0;				//找到的父亲结点
		bool foundDIE = false;

		for(fstage = v_stage.at(i)+2; fstage < nLevel; fstage ++)	//对主线向上
		{
			for(int fid = 0; fid < nOfEachLevel[fstage]; fid++)
			{
				if( (treeAtLevel[fstage][fid]).startPos < (*v_abcf_node.at(i)).startPos
					&& (treeAtLevel[fstage][fid]).endPos > (*v_abcf_node.at(i)).endPos )
					//这说明，爹已找到- -
				{
					//更新数据：本身指向爹；爹儿子加1，爹的儿子指针表更新
					//因为同层没有同层关系，因此（爹-1）层不必考虑
					//更新儿子
					(*v_abcf_node.at(i)).p_fN = &(treeAtLevel[fstage][fid]);

					int n_fson = treeAtLevel[fstage][fid].nSon + 1;
					favNode ** p_pson;
					p_pson = new favNode * [n_fson]; 
					for(int j = 0; j < n_fson-1; j++)
					{
						p_pson[j] = treeAtLevel[fstage][fid].p_psN[j];
					}
					p_pson[n_fson - 1] = v_abcf_node.at(i);

					//原来爹的指针释放
					delete [] treeAtLevel[fstage][fid].p_psN;

					//更新爹
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

		//补正还没找到爹的情况
		//意思是，不存在爹，即独立一棵树的根结点~~待补充
	}

	//从文件中扫描树已经完成
/*
#ifdef PRINT_MODE
	//按照物理层访问
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
	//按父子关系进行访问输出
	std::cout << "\n\nACCESS BY THE LOGIC OF MUTUAL RELATION:\n";

	//用链表指针而不直接用链表，是为了前后传递参数方便
	std::vector<favNode *> * p_vLogicAc;			//接下来将要访问的结点
	p_vLogicAc = new std::vector<favNode *>;
	for(int i = 0; i < nOfEachLevel[nLevel - 1]; i++)	//将根结点存入其中
	{
		(*p_vLogicAc).push_back( &(treeAtLevel[nLevel-1][i]) );
	}

	int stage_n = 0;

	while( (* p_vLogicAc).size() != 0 )				//进行逐层显示
	{
		std::cout << "\nlevel:" << stage_n << " *********************\n";
		std::vector<favNode *> * p_next;
		p_next = new std::vector<favNode *>;

		for(int i = 0; i < (*p_vLogicAc).size(); i++)	//每层中的逐个元素
		{
			//显示
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

		//更新下一轮迭代条件
		for(int i = 0; i < (*p_vLogicAc).size(); i++)	//每层中的逐个元素
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

	//做函数对接

	//三个数据成员赋值
	nStages = nLevel;
	nOfAStage = new int [nLevel];
	treeStructure = new theNode * [nLevel];

	//按父子关系进行访问输出
	//std::cout << "\n\nACCESS BY THE LOGIC OF MUTUAL RELATION:\n";

	//用链表指针而不直接用链表，是为了前后传递参数方便
	std::vector<favNode *> * p_vLogicAc;			//接下来将要访问的结点
	p_vLogicAc = new std::vector<favNode *>;
	for(int i = 0; i < nOfEachLevel[nLevel - 1]; i++)	//将根结点存入其中
	{
		(*p_vLogicAc).push_back( &(treeAtLevel[nLevel-1][i]) );
	}

	int stage_n = 0;

	while( (* p_vLogicAc).size() != 0 )				//进行逐层显示
	{
		//std::cout << "\nlevel:" << stage_n << " *********************\n";
		std::vector<favNode *> * p_next;
		p_next = new std::vector<favNode *>;

		//建立此层的treeStructure的结点结构
		treeStructure[stage_n] = new theNode [(*p_vLogicAc).size()];		//结点空间
		nOfAStage[stage_n] = (*p_vLogicAc).size();							//此层结点数量

		for(int i = 0; i < (*p_vLogicAc).size(); i++)	//每层中的逐个元素,对应为treeStructure[stage_n][i]
		{

			//赋值传递
			(*(*p_vLogicAc).at(i)).p_theNode = &(treeStructure[stage_n][i]);
			(treeStructure[stage_n][i]).nSon = (*(*p_vLogicAc).at(i)).nSon;			//nSons
			//对于父结点进行扫描
			if(stage_n != 0)			//非源结点，则有父节点
			{
				(treeStructure[stage_n][i]).p_FN = ((*(*p_vLogicAc).at(i)).p_fN)->p_theNode;
			}
			else						//源结点，则置空
			{
				(treeStructure[stage_n][i]).p_FN = NULL;
			}
			(treeStructure[stage_n][i]).attr_table_num.push_back( (*(*p_vLogicAc).at(i)).startPos );	//只传递起止位置参数
			(treeStructure[stage_n][i]).attr_table_num.push_back( (*(*p_vLogicAc).at(i)).endPos );
			//赋值传递完毕

			//对于子结点进行扫描
		}

		//更新下一轮迭代条件
		for(int i = 0; i < (*p_vLogicAc).size(); i++)	//每层中的逐个元素
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

	(*p_vLogicAc).clear();			//次模块内部释放
	//对于p_tree的子节点进行设置	
	for(int stage = 0; stage < nStages; stage++)			//赋初始值
	{
		for(int id = 0; id < nOfAStage[stage]; id++)
		{
			treeStructure[stage][id].p_SN = NULL;
		}
	}
	for(int stage = 1; stage < nStages; stage++)			//由儿子的父亲决定父亲的儿子
	{
		for(int id = nOfAStage[stage]-1; id >= 0; id--)
		{
			(*(treeStructure[stage][id]).p_FN).p_SN = &(treeStructure[stage][id]);
		}
	}



	////根据文本内容在树中读入内容////
	for(int stage = 0; stage < nStages; stage++)			//赋初始值
	{
		for(int id = 0; id < nOfAStage[stage]; id++)
		{
			//上下两部分
			int upp_start = treeStructure[stage][id].attr_table_num.at(0);
			int upp_end = 0;
			int low_start = 0;
			int low_end = treeStructure[stage][id].attr_table_num.at(1);
			//确定 上部分的下界 下部分上界
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


			char * pStr = NULL;				//接受好的数组

			//若上下界限为相同部分
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


	////完成 根据文本内容在树中读入内容////

	/*
	////进行 unicode 显示转换
	for(int stage = 0; stage < nStages; stage++)			
	{
		for(int id = 0; id < nOfAStage[stage]; id++)
		{
			//确定name的位置
			int i_item = -1;
			for(int i = 1; i < treeStructure[stage][id].attr_table_word.size(); i = i+=2)		//对于key位置
			{
				char * p_attri = treeStructure[stage][id].attr_table_word.at(i);
				if( strcmp(p_attri, "name") == 0 )		//相同时
				{
					i_item = i;
					break;
				}
			}

			if(i_item == -1)		//没找到
			{
				treeStructure[stage][id].attr_table_num.push_back(-1.0);
			}
			else					//找到name
			{
				if( treeStructure[stage][id].attr_table_word.size() - 1 == i_item )			
					//这种情况不大可能出现，因为都是成对存在的，因此不可能name是最后一个
				{
					treeStructure[stage][id].attr_table_num.push_back(-1.0);
				}
				else if( treeStructure[stage][id].attr_table_word.at(i_item+1) == NULL )
					//这种情况对应name属性下一个属性为空情况
				{
					treeStructure[stage][id].attr_table_num.push_back(-1.0);
				}
				else
					//这种情况是货真价实具有name的键值对应的value
				{
					treeStructure[stage][id].attr_table_num.push_back( float(i_item+1) );	//存入value对应的角标
					//进行unicode替换
					char * p_oriValue = treeStructure[stage][id].attr_table_word.at(i_item+1);
					int oriLength = strlen(p_oriValue);
					char * p_chValue = new char [oriLength];

				}
			}
		}
	}
	*/


	////释放资源////
	infile.close();
	buffer.str("");			//据说这样是清空stringstream。流对象？
	//hb_pos.clear();
	//rb_pos.clear();
	brace_pos.clear();

	l_BTerraces.clear();

	//wholeOriginalTree  本身是一个容器，里面还有stage个容器指针，所以
	for(int stage = 0; stage < wholeOriginalTree.size(); stage++)
	{
		( *wholeOriginalTree.at(stage) ).clear();
	}
	wholeOriginalTree.clear();

	//treeAtLevel 本身是数组指针的数组
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

			//遍历所有的key值，寻找合适的选择
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

						//更新内容
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

							//清除原有的attri_word
							for( int j = 0; j < node.attr_table_word.size(); j++ )
							{
								if( node.attr_table_word.at(j) != NULL )
								{
									delete [] node.attr_table_word.at(j);
								}
							}
							node.attr_table_word.clear();

							//插入新的属性值
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

							//清除原有的attri_word
							for( int j = 0; j < node.attr_table_word.size(); j++ )
							{
								if( node.attr_table_word.at(j) != NULL )
								{
									delete [] node.attr_table_word.at(j);
								}
							}
							node.attr_table_word.clear();

							//插入新的属性值
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



						break;			//找到合适类别，跳出循环
					}
					else if( strcmp(value, "folder") == 0 )
					{
						//this is a E_CLASS 默认为空分类，最后还需要进行扫描确认
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
							strcpy(v1, "E_CLASS");			//默认先为E_CLASS

							//清除原有的attri_word
							for( int j = 0; j < node.attr_table_word.size(); j++ )
							{
								if( node.attr_table_word.at(j) != NULL )
								{
									delete [] node.attr_table_word.at(j);
								}
							}
							node.attr_table_word.clear();

							//插入新的属性值
							node.attr_table_word.push_back(k1);
							node.attr_table_word.push_back(v1);
							node.attr_table_word.push_back(k2);
							node.attr_table_word.push_back(v2);
						}
						else
						{
							//定义为INV_CLASS 3
							log_type = 3;
							k1 = new char [5];
							strcpy(k1, "type");
							v1 = new char [10];
							strcpy(v1, "INV_CLASS");

							//清除原有的attri_word
							for( int j = 0; j < node.attr_table_word.size(); j++ )
							{
								if( node.attr_table_word.at(j) != NULL )
								{
									delete [] node.attr_table_word.at(j);
								}
							}
							node.attr_table_word.clear();

							//插入新的属性值
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



						break;		//找到合适类别，跳出循环
					}

					
					
				}
				else
				{
					continue;
				}
			}

			//for循环过后，没有找到合适类别
			if(log_type == -1)
			{
				log_type = 6;		//INV_SON

				char * k1 = NULL;
				char * v1 = NULL;

				k1 = new char [5];
				strcpy(k1, "type");
				v1 = new char [8];
				strcpy(v1, "INV_SON");

				//清除原有的attri_word
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

			//扫描下一个结点的取值
		}
	}

	//这轮循环，是检查N_CLASS和E_CLASS
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
					pFolder[0] = 'N';		//为N_CLASS
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
							//输出到str2中
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
	//清理工作
	labelSum.clear();

	//类标数量统计
	nLabels = 5;
	for(int i = 0; i < nLabels; i++)
	{
		labelSum.push_back(0);
	}
	for(int i = 0; i < g_nTitles; i++)
	{
		labelSum.at( g_labelList[i]-1 ) ++;
	}

	//结构种类标赋值
	int i_cur = 0;
	for( int stage = 0; stage < nStages; stage++ )
	{
		for( int id = 0; id < nOfAStage[stage]; id++ )
		{
			if( (strcmp( treeStructure[stage][id].attr_table_word.at(0), "type")==0) && (treeStructure[stage][id].attr_table_word.size()==6) )
			{
				if( i_cur >= g_nTitles )	//出现故障
				{
					break;
				}
				treeStructure[stage][id].attr_table_num.push_back( g_labelList[i_cur] );
				char * k4 = new char [6];
				strcpy(k4, "label");
				char * v4 = new char [3];	//100以内
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

	int sum_bkmks = 0;					//书签总量
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

	//创建结构信息
	pTree->nStages = 3;

	pTree->nOfAStage = new int [3];
	pTree->nOfAStage[0] = 1;		//树根
	pTree->nOfAStage[1] = 5;		//目录数
	pTree->nOfAStage[2] = sum_bkmks;		//总标题数量

	pTree->treeStructure = new theNode * [3];

	pTree->treeStructure[0] = new theNode [1];	
	//结构信息
	pTree->treeStructure[0][0].nSon = pTree->nOfAStage[1];		//树根结点
	pTree->treeStructure[0][0].p_FN = NULL;

	k1 = new char [5];
	strcpy(k1, "type");
	v1 = new char [10];
	strcpy(v1, "N_CLASS");
	pTree->treeStructure[0][0].attr_table_word.push_back(k1);
	pTree->treeStructure[0][0].attr_table_word.push_back(v1);

	pTree->treeStructure[1] = new theNode [ pTree->nOfAStage[1] ];	//目录节点
	for(int id = 0; id < pTree->nOfAStage[1]; id++)
	{
		theNode * pND = &(pTree->treeStructure[1][id]);
		//结构信息
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

	pTree->treeStructure[2] = new theNode [ pTree->nOfAStage[2] ];	//页面节点
	int i_title = 0;
	for( int fid = 1; fid < pTree->nOfAStage[1]+1; fid++ )		//按照类标号索引，类标索引从1开始
	{
		for( int stage = 0; stage < nStages; stage++ )
		{
			//for( int id = 0; id < nOfAStage[stage]; id++ )
			for( int id = nOfAStage[stage]-1; id >=0; id-- )	//按时间顺序排列
			{
				if( (strcmp( treeStructure[stage][id].attr_table_word.at(1), "BOOKMARK")==0) 
					&& (treeStructure[stage][id].attr_table_word.size()==8) 
					&& (treeStructure[stage][id].attr_table_num.at(2) == fid) )
					//属于
				{
					//vTitle.push_back( treeStructure[stage][id].attr_table_word.at(3) );
					//找到了
					theNode * pOri = &(treeStructure[stage][id]);			//原始结点
					
					theNode * pN = &(pTree->treeStructure[2][i_title]);		//新增结点

					//结构信息
					pN->nSon = 0;
					pN->p_FN = &(pTree->treeStructure[1][fid-1]);

					k1 = new char [5];
					strcpy(k1, "type");
					v1 = new char [9];
					strcpy(v1, "BOOKMARK");
					k2 = new char [5];
					strcpy(k2, "name");
					v2 = new char [ strlen( pOri->attr_table_word.at(3) )+1 ];	//名称
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

	//补全整棵树剩下的p_SN结构信息
	//第1层
	pTree->treeStructure[0][0].p_SN = &( pTree->treeStructure[1][0] );
	//第2层
	int i_temp = 0;			//第3层类标对应数量
	int i_temp_sub = 0;		//脚标
	for(int i = 0; i < nLabels; i++)
	{
		pTree->treeStructure[1][i].p_SN = &( pTree->treeStructure[2][i_temp] );
		i_temp += labelSum.at(i_temp_sub);
		i_temp_sub++;
	}
	//第3层
	for(int id = 0; id < pTree->nOfAStage[2]; id++)
	{
		pTree->treeStructure[2][id].p_SN = NULL;
	}


	return pTree;
}

#endif