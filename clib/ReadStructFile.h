#ifndef READSTRUCTFILE_H
#define READSTRUCTFILE_H

#include <assert.h>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>  
#include <utility>
#include <malloc.h>
#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <time.h>
#include <algorithm>
#include <cstring>
#include <functional>
#include <vector>
#include <time.h>
#include <sys/types.h>   
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/mman.h>
#include "my_hash_map.h"

typedef unsigned int UINT;
typedef long long __int64;

#define LOADBUF 1024*128

template <class T>
bool CheckMemAlloc(T *gBuffer, int nCnt)
{
#ifdef __GNUC__
	if (malloc_usable_size(gBuffer)<nCnt*sizeof(T))
		return false;
	else return true;
#else
	if (_msize(gBuffer)!=nCnt*sizeof(T))
		return false;
	else return true;
#endif
}

//Vector内存清理
template <class T>
void ClearVector(vector<T>& vt)
{
	vector<T> vecTemp;
	vecTemp.swap(vt);
}

//Hashmap内存清理
template <class K, class V>
void ClearHashmap(hash_map<K,V>& hp)
{
	hash_map<K,V> hpTemp;
	hpTemp.swap(hp);
}

//读取倒排文件
template<class IndexType, class InvertType>
bool ReadInvert(FILE *fp, IndexType strIndexNode, vector<InvertType>& vecInvert, int nReadSize=-1)
{
	bool bRt=true;
	time_t timep;//时间变量
	time (&timep);//获取当前时间
	char cBuffer[1024]={0};

	int nSize;
	int nCount=strIndexNode.count;
	__int64 nAddr=strIndexNode.offset;
	
	//文件定位
	if (fseek(fp,nAddr,0)==-1)
	{
		bRt=false;
		cout<<"nAddr:"<<nAddr<<endl;
		printf("[ ERROR ] [Msg:fseek] [Line:%d] -%s\n", __LINE__, ctime(&timep));
		return bRt;
	}
	
	//分配容器空间
	if (nCount>nReadSize && nReadSize!=-1) nSize=nReadSize;
	else nSize=nCount;
	vecInvert.resize(nSize);

	//Read
	int nReadCnt;
	nReadCnt=fread(&vecInvert[0],sizeof(InvertType),nSize,fp);
	if (nReadCnt==-1 || nReadCnt!=nSize)
	{
		bRt=false;
		sprintf(cBuffer,"fread: nReadCnt=%d, nSize=%d", nReadCnt, nSize);
		printf("[ ERROR ] [Msg: %s] [Line:%d] -%s\n", cBuffer, __LINE__, ctime(&timep));
		return bRt;	
	}

	return bRt;
}

//载入索引到hashmap
template<class K, class IndexType>
bool LoadIndex_Hashmap(const string& sIndexFile, hash_map<K,IndexType>& hmIndex)
{
	bool bRt=true;
	time_t timep;//时间变量
	time (&timep);//获取当前时间
	
	//打开文件
	FILE *fpIndex;
	if ((fpIndex=fopen(sIndexFile.c_str(),"rb"))==NULL)
	{
		bRt=false;
		printf("[ ERROR ] [Msg:Open File %s] [Line:%d] -%s\n",sIndexFile.c_str(), __LINE__, ctime(&timep));
		return bRt;
	}

	IndexType *tempinfo = new IndexType[LOADBUF];
	int nReadCnt;

	while (!feof(fpIndex))
	{
		nReadCnt=fread(tempinfo,sizeof(IndexType),LOADBUF,fpIndex);
		if (nReadCnt==-1)
		{
			printf("[ ERROR ] [Msg:fread] [Line:%d] -%s\n", __LINE__, ctime(&timep));
			return false;
		}
		
		for (int i=0;i<nReadCnt;i++)
		{	
			hmIndex.insert(pair<K,IndexType>(tempinfo[i].item_id,tempinfo[i]));
		}
		if (nReadCnt<LOADBUF) break;		
		memset(tempinfo,0,LOADBUF);//清零		
	}

	delete []tempinfo;
	tempinfo=NULL;
	fclose(fpIndex);

	return bRt;
}

//载入索引到vector
template<class IndexType>
bool LoadIndex_Vector(const string& sIndexFile, vector<IndexType>& vecIndex)
{
	bool bRt=true;
	time_t timep;//时间变量
	time (&timep);//获取当前时间

	//打开文件
	FILE *fpIndex;
	if ((fpIndex=fopen(sIndexFile.c_str(),"rb"))==NULL)
	{
		bRt=false;
		printf("[ ERROR ] [Msg:Open File %s] [Line:%d] -%s\n",sIndexFile.c_str(), __LINE__, ctime(&timep));
		return bRt;
	}

	IndexType *tempinfo = new IndexType[LOADBUF];
	int nReadCnt;

	while (!feof(fpIndex))
	{
		nReadCnt=fread(tempinfo,sizeof(IndexType),LOADBUF,fpIndex);
		if (nReadCnt==-1)
		{
			printf("[ ERROR ] [Msg:fread] [Line:%d] -%s\n", __LINE__, ctime(&timep));
			return false;
		}

		for (int i=0;i<nReadCnt;i++)
		{	
			vecIndex.push_back(tempinfo[i]);
		}
		if (nReadCnt<LOADBUF) break;		
		memset(tempinfo,0,LOADBUF);//清零		
	}

	delete []tempinfo;
	tempinfo=NULL;
	fclose(fpIndex);

	return bRt;
}


//载入索引到vector & hashmap
template<class K, class IndexType>
bool LoadIndex_Vector_Hashmap(const string& sIndexFile, vector<IndexType>& vecIndex, hash_map<K,IndexType>& hmIndex)
{
	bool bRt=true;
	time_t timep;//时间变量
	time (&timep);//获取当前时间

	//打开文件
	FILE *fpIndex;
	if ((fpIndex=fopen(sIndexFile.c_str(),"rb"))==NULL)
	{
		bRt=false;
		printf("[ ERROR ] [Msg:Open File %s] [Line:%d] -%s\n",sIndexFile.c_str(), __LINE__, ctime(&timep));
		return bRt;
	}

	IndexType *tempinfo = new IndexType[LOADBUF];
	int nReadCnt;

	while (!feof(fpIndex))
	{
		nReadCnt=fread(tempinfo,sizeof(IndexType),LOADBUF,fpIndex);
		if (nReadCnt==-1)
		{
			printf("[ ERROR ] [Msg:fread] [Line:%d] -%s\n", __LINE__, ctime(&timep));
			return false;
		}

		for (int i=0;i<nReadCnt;i++)
		{	
			vecIndex.push_back(tempinfo[i]);
			hmIndex.insert(pair<K,IndexType>(tempinfo[i].m_nID,tempinfo[i]));
		}
		if (nReadCnt<LOADBUF) break;		
		memset(tempinfo,0,LOADBUF);//清零		
	}

	delete []tempinfo;
	tempinfo=NULL;
	fclose(fpIndex);

	return bRt;
}

//同时载入索引（hashmap）和倒排到内存
template<class K,class IndexType,class InvertType>
bool LoadIndexAndInvert_Hashmap(const string& sIndexFile, hash_map<K,IndexType>& hmIndex,
								const string& sInvertFile, vector<InvertType>& vecInvert)
{
	bool bRt=true;
	time_t timep;//时间变量
	time (&timep);//获取当前时间

	//索引文件
	FILE *fpIndex;	
	if ((fpIndex=fopen(sIndexFile.c_str(),"rb"))==NULL)//
	{
		bRt=false;
		printf("[ ERROR ] [Msg:Open File %s] [Line:%d] -%s\n", sIndexFile.c_str(), __LINE__, ctime(&timep));
		return bRt;
	}

	//倒排文件
	FILE *fpInvert;	
	if ((fpInvert=fopen(sInvertFile.c_str(),"rb"))==NULL)//
	{
		bRt=false;
		printf("[ ERROR ] [Msg:Open File %s] [Line:%d] -%s\n", sInvertFile.c_str(), __LINE__, ctime(&timep));
		return bRt;
	}

	IndexType *tempinfo1 = new IndexType[LOADBUF];
	InvertType *tempinfo2 = new InvertType[LOADBUF];
	int nReadCnt;
	vector<InvertType> vecReadBuf;
	int nFrom=0;
	
	//索引
	while (!feof(fpIndex))
	{
		nReadCnt=fread(tempinfo1, sizeof(IndexType), LOADBUF, fpIndex);
			
		for (size_t i=0; i<nReadCnt; ++i)
		{				
			tempinfo1[i].m_nAddr=nFrom;
			hmIndex.insert(pair<int,IndexType>(tempinfo1[i].m_nID,tempinfo1[i]));
			nFrom+=tempinfo1[i].m_nCount;
		}
		if (nReadCnt<LOADBUF) break;		
		memset(tempinfo1,0,LOADBUF);//清零		
	}
	
	//倒排
	while (!feof(fpInvert))
	{
		nReadCnt=fread(tempinfo2, sizeof(InvertType), LOADBUF, fpInvert);
			
		for (size_t i=0; i<nReadCnt; ++i)
		{				
			vecInvert.push_back(tempinfo2[i]);
		}
		if (nReadCnt<LOADBUF) break;		
		memset(tempinfo2,0,LOADBUF);//清零		
	}

	fclose(fpIndex);
	fclose(fpInvert);

	delete []tempinfo1;
	tempinfo1=NULL;
	
	delete []tempinfo2;
	tempinfo2=NULL;
	
	return bRt;
}

#endif

