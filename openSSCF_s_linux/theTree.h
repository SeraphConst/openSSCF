#ifndef THETREE_H
#define THETREE_H

#include <iostream>
#include <fstream>

#include <vector>
#include <list>

#include <string.h>

#include <strstream>
#include <sstream>

#include <stdio.h>
#include <stdlib.h>

#pragma once

class theNode
{
public:
	theNode * p_SN;
	int nSon;
	theNode * p_FN;

	char * pOri_str;
	std::vector<char *> attr_table_word;
	std::vector<int> attr_table_num;

public:
	theNode();
	~theNode();

	bool isBOOKMARK();
	int indexOfTitle();


	theNode(char * g_path, int g_attr);		//此函数仅用于读ie文件
};


class theTree
{
public:
	theNode ** treeStructure;
	int nStages;
	int * nOfAStage;

	int nLabels;
	std::vector<int> labelSum;

public:
	theTree();
	~theTree();


	void formTreefromJSON(char * filename);

	void resolveChrome();
	void resolveFirefox();

	void saveTreeIntoTRI(char * filename);	
	bool formTreefromTRI(char * filename);

	char * getTreeOriginList_str();
	char * getTreeAttrWList_str(std::vector<char *> & v_key, int mode);


	void formIETree(std::vector<std::vector<theNode *> *> & tree);

	void completeChromeTree();

	void simpleOutput( char * filename1, char * filename2 );

	void outputTitles(std::vector<char*> & vTitle);
	void updateLabels(int * labelList, int nLabels);

	void fixLabels();

	theTree * getMenuTree();
};



class favNode
{
public:
	favNode ** p_psN;
	int nSon;
	favNode * p_fN;
	theNode * p_theNode;

	int startPos;
	int endPos;

	std::vector<char *> attr_table;

public:
	favNode(int givenSP, int givenEP);
	favNode();
	~favNode();
};

class bracePosInfo
{
public:
	int pos;
	int type;

public:
	bracePosInfo(int given_pos, int given_type);
	~bracePosInfo();
};


class originalNode
{
public:
	int startPos;
	int endPos;
	int id;

	int f_id;
	int s_id;
	int n_son;

public:
	originalNode(int givenS, int givenE, int givenI);

	~originalNode();
};

//释放tree中内容
void releaseTempTree(std::vector<std::vector<theNode *> *> & tree);

#endif
