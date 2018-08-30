#ifndef WINNERTREE_H
#define WINNERTREE_H
#include <vector>
#include <algorithm>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#ifndef _WIN32
#include <unistd.h>
#define O_BINARY 0
#else
#include <io.h>
#define O_LARGEFILE 0
#endif


#define  INFINITE_DISTANCE 65536

using namespace std;

template<class INVERT>
inline int DEFULT_MINUS_INVERT(INVERT&l,INVERT&r)
{
	if (l.nDocId!=r.nDocId)
	{
		return l.nDocId-r.nDocId>0?INFINITE_DISTANCE:-INFINITE_DISTANCE;
	}	

	return l.nOffset-r.nOffset;
}

template<class INVERT>
inline INVERT& DEFAULT_INVERT_MINUS_INT(INVERT& l,int nMinus)
{
	l.nOffset-=nMinus;
	return l;
}

template<class TYPE>
inline	bool CMP_LESS_DEFAULT(const TYPE& l,const TYPE& r)
{
	return l<r;
}




//���ð�����ʵ��K·�ļ��鲢�����㷨
//************************************
// Method:    K_MergeFile
// FullName:  <NODE>::K_MergeFile
// Access:    public 
// Returns:   int
// Qualifier:
// Parameter: const char *pchFileIn �����ļ�����
// Parameter: const char *pchFileOut����ļ�����

// Parameter: *CMP_NODE             �ȽϺ���
// Parameter: int nFreeMemSize      �趨�鲢ʹ�������ڴ�ռ�Ĵ�С
//************************************
template <class NODE>
int  K_MergeFile(const char *pchFileIn,            const char *pchFileOut,
				 bool(*CMP_NODE)(const NODE& L, const NODE& R), int nFreeMemSize);



//���ð�����ʵ��K·�ļ��鲢�����㷨,��K�������ļ����й鲢
//************************************
// Method:    K_MergeFile
// FullName:  <NODE>::K_MergeFile
// Access:    public 
// Returns:   int
// Qualifier:
// Parameter: vector<char*> &vecFileNames Ҫ�鲢�������ļ������б�
// Parameter: const char *pchFileOut����ļ�����

// Parameter: *CMP_NODE             �ȽϺ���
// Parameter: int nFreeMemSize      �趨�鲢ʹ�������ڴ�ռ�Ĵ�С
//************************************
template <class NODE>
int  K_MergeFile(vector<string> &vecFileNames,   const char *pchFileOut,
				 bool(*CMP_NODE)(const NODE& L, const NODE& R), int nCacheNodeCnt);




//��ЧK·�����󽻼�
//************************************
// Method:    K_Intersect
// FullName:  <NODE>::K_Intersect
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: vector<NODE*>&vecNodePtr    �����׵�ַ����
// Parameter: vector<NODE*>&vecNodeEndPtr ���н�����ַ���顣
// Parameter: vector<NODE> *pVecDest      ָ��鲢���ŵ�������������vector��push_back������

// Parameter: *CMP_NODE                   �ȽϺ�������С��

//************************************
template <class NODE>
NODE*  K_Intersect(vector<NODE*>&vecNodePtr,
				  vector<NODE*>&vecNodeEndPtr,
				  NODE *pNodeDest,
				  bool(*CMP_NODE)(const NODE& L, const NODE& R),
				  void(*ADD_NODE)(NODE& L, NODE& R));




//��ЧK·���ű������󽻼� ���ص��ű����У�ÿ���ڵ��ƫ�����������ĵ��е�ƫ��λ��

//************************************
// Method:    K_InvertIntersect
// FullName:  <NODE>::K_InvertIntersect
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: vector<NODE*>&vecNodePtr    �����׵�ַ����
// Parameter: vector<NODE*>&vecNodeEndPtr ���н�����ַ���顣
// Parameter: vector<NODE> *pVecDest      ָ��鲢���ŵ�������ȫ���������е�Ԫ�ء���ÿ��Ԫ�ص�ƫ�Ƶ�ַ�������鴮�ĵ�һ���֡�

// Parameter: *MINUS_INVERT               ���������������ıȽϺ�����

// Parameter: *INVERT_MINUS_INT           ����Ԫ�ؼ�ȥ������Ŀ����Ϊ���ҵ��ĵ���ǰ��Ԫ�صĵ��Žڵ㡣

//************************************
template <class NODE>
NODE*  K_InvertIntersect(vector<NODE*>&vecNodePtr,
				  vector<NODE*>&vecNodeEndPtr,
				  NODE*        pNodeDest,
				  int (*MINUS_INVERT)(NODE&l,NODE&r),
				  NODE&(*INVERT_MINUS_INT)(NODE&l,int nMinus));







//������������������ʵ���ڴ���K·�鲢
template <class NODE>
class CWinnerTree
{
public:

	//CMP_ELEMENTָ��ȽϺ���ָ�� ��Ϊ�յ���OPERATOR <
	CWinnerTree(bool(*CMP_ELEMENT)(const NODE& L, const NODE& R))
	{	
		M_CMP_ELEMENT=CMP_ELEMENT;	
		m_pNodeDest=NULL;
		m_pNodePrDest=NULL;
	}


	//����һ����ȫ������������ʼ����һ�Žڵ㡣
	//************************************
	// Method:    MakeTree
	// FullName:  CWinnerTree<NODE>::MakeTree
	// Access:    public 
	// Returns:   void
	// Qualifier:
	// Parameter: vector<NODE*>&vecNodePtr    �����׵�ַ����
	// Parameter: vector<NODE*>&vecNodeEndPtr ���н�����ַ���顣
	// Parameter: vector<NODE> *pVecDest      ָ��鲢���ŵ�������������vector��push_back������
	//************************************
	//�鲢����в�������� ���� K_Merge(vector<NODE>* )�ɶ�ʹ��
	bool CreateTree(vector<NODE*>&vecNodePtr,vector<NODE*>&vecNodeEndPtr,NODE *pNodeDest=NULL);

	//�ڴ��е�K·�󲢼������m_pVecDest��
	inline NODE*  K_Merge(NODE *);




   //�鲢���д�����ţ���K_Merge(vector<pair<NODE,int> >* )�ɶ�ʹ��
	bool CreateTree(vector<NODE*>&vecNodePtr,vector<NODE*>&vecNodeEndPtr,pair<NODE,int>  *pNodePrDest);

	//�ڴ��е�K·�󲢼������m_pVecDest��
	inline pair<NODE,int> *   K_Merge(pair<NODE,int> * );

     //��Ԫ����
	friend	int  K_MergeFile<NODE>(const char *pchFileIn,            const char *pchFileOut,
		bool(*CMP_NODE)(const NODE& L, const NODE& R), int nFreeMemSize);
	//��Ԫ����
	friend int  K_MergeFile<NODE>(vector<string> &vecFileNames,   const char *pchFileOut,
		bool(*CMP_NODE)(const NODE& L, const NODE& R), int nCacheNodeCnt);
public:
	~CWinnerTree(void){};
	
private:
	inline bool CMP_NODE(const pair<NODE*,int> &nodeL, const pair<NODE*,int>& nodeR)
	{
// 		if (nodeL.first==NULL)//��һ�е�ĩβ����NULL��ʾ���޴�
// 		{
// 			return 1;
// 		}
// 		else
// 		{
// 			if (nodeR.first==NULL)
// 			{
// 				return -1;
// 			}		
// 		}
// 		int minus=*(nodeL.first)-*(nodeR.first);
// 		if (minus==0)
// 		{
// 			return nodeL.second-nodeR.second;
// 		}
// 		return minus;
		return  M_CMP_ELEMENT(*(nodeL.first),*(nodeR.first))||
                (!M_CMP_ELEMENT(*(nodeR.first),*(nodeL.first))&& (nodeL.second<nodeR.second) );
		
	}

	void MakeTree();	

	//�ڲ�ʹ�ú���
	bool K_MergeFile(vector<int> &vecTempFileHandle,int hWriteFileHandle,int nReadCnt,NODE*pOutPtr);	

private:
	vector<pair<NODE*,int> > m_vecTree;//��ȫ�������ṹ

	vector<NODE*> m_vecRunBeginPtr;      //����ÿһ����ʼ��ַ
	vector<NODE*> m_vecRunTracks;        //��ǰ���ٵ��ĸ���Node
	vector<NODE*> m_vecRunEnds;         //Run����ָֹ��
	vector<int>   m_vecStopMark;        //ÿ��Rund����ֹ��׼
	vector<int>   m_vecRunInTreePos;    //ÿ��Run�Ŷ�Ӧ��TREE�е�λ�á�

	NODE *m_pNodeDest;          //��Ź鲢����ĵط���
	pair<NODE,int>  *m_pNodePrDest;          //��Ź鲢����ĵط� pair Ϊ����͹鲢�Ӻţ�

	//	NODE *m_pMaxNodePtr//;
	int m_nLevel;                      //����
	int m_nLastLevelBegin;             //���һ��ڵ���ʼ��
	size_t m_nStoppedRun;                 //�鲢��ֹͣ������

	bool (*M_CMP_ELEMENT)(const NODE&L,const NODE& R);//Ԫ�رȽϺ���
	
private:
	//��ʱ����������ٶ����
	//pair<NODE*,int> resPair;
	int nRun;
	int nTrack;//���ټ�����
	
};



//�ļ��鲢����
//************************************
// Method:    K_MergeFile
// FullName:  <NODE>::K_MergeFile
// Access:    public 
// Returns:   int
// Qualifier:
// Parameter: const char *pchFileIn �����ļ�����
// Parameter: const char *pchFileOut����ļ�����

// Parameter: *CMP_NODE             �ȽϺ���
// Parameter: int nFreeMemSize      �趨�鲢ʹ�������ڴ�ռ�Ĵ�С
//************************************
template <class NODE>
int  K_MergeFile(const char *pchFileIn,const char *pchFileOut,bool(*CMP_NODE)(const NODE& L,const NODE& R),int nFreeMemSize)
{

	int fdSrc;
	
    fdSrc = ::open(pchFileIn, O_RDWR|O_BINARY|O_LARGEFILE, 0644);

	if (fdSrc == -1)
	{	
		fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
		return -1;
	}

	//��ô�С
	struct stat64 statInfo;
	if (fstat64(fdSrc, &statInfo ) < 0) 
	{
		fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
		//::close(fdSrc);
		return -1;
	}

	long long  llFileSize;

	llFileSize = statInfo.st_size;		//�ļ�����

	int fdDest;
	fdDest = ::open(pchFileOut, O_CREAT|O_RDWR|O_TRUNC|O_BINARY|O_LARGEFILE, 0644);

	if (fdDest == -1)
	{	
		fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
		return -1;
	}


	//������ʱ�ļ�Ŀ¼·��
	string strDirectPrefix;
	const char*p=pchFileOut+strlen(pchFileOut);
	while (p!=pchFileOut&&*(p-1)!='\\'&&*(p-1)!='/')
	{
		--p;
	}
	if (p!=pchFileOut)
	{
		strDirectPrefix=string(pchFileOut,p);
	}




	if (llFileSize%sizeof(NODE)!=0)//�ļ���СӦ���ǽṹ���������
	{
		fprintf(stderr,"FileSize error .\n");
		return -1;
	}


	long long llTotalEleCnt;
	//int nTotalEleCnt;//����Ԫ�ظ���
	int nEachColCnt;//ÿ���鲢��Ԫ�ظ���
	int nColCnt;   //�и���
	size_t nReadCnt;//ʵ�ʶ�ȡ����

	llTotalEleCnt=llFileSize/sizeof(NODE);
	nEachColCnt=nFreeMemSize/sizeof(NODE);
	if (llTotalEleCnt%nEachColCnt==0)
	{
		nColCnt=llTotalEleCnt/nEachColCnt;
	}
	else
	{
		nColCnt=llTotalEleCnt/nEachColCnt+1;
	}


	if (nColCnt<=0)
	{
		printf("�ļ���СΪ0.\n"); 
		return -1;
	}
	if (nEachColCnt<=0)
	{
		printf("�鲢�ռ䲻��0.\n"); 
		return -1;
	}
	int i;
	if (nColCnt==1)//�ڴ��ܷ��¸��ļ�
	{		
		vector<NODE> vecBuf;//��������	
		vecBuf.resize(nEachColCnt);
		
		if((nReadCnt=read(fdSrc,(void*)&vecBuf[0],llTotalEleCnt*sizeof(NODE)))<0
			||nReadCnt!=llTotalEleCnt*sizeof(NODE))
		{
			fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
			return -1;
		}
		vecBuf.erase(vecBuf.begin()+nReadCnt/sizeof(NODE),vecBuf.end());

		sort(vecBuf.begin(),vecBuf.end(),CMP_NODE);	
		
		if ((nReadCnt=write(fdDest,(void*)&vecBuf[0],llTotalEleCnt*sizeof(NODE)))<0
			||nReadCnt!=llTotalEleCnt*sizeof(NODE))
		{
			fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
			return -1;
		}

	}
	else//����������
	{
		//0.���������ֶ������ļ�
		vector<NODE> vecBuf;//��������	
		vecBuf.resize(nEachColCnt);
		vector<string> vecTempFileName;
		vector<int> vecTempFileHandle;
		vecTempFileName.resize(nColCnt,"0");
		vecTempFileHandle.resize(nColCnt);
		int nWriteCnt;
		char chBuf[1024];

		for (i=0;i<nColCnt;++i )
		{
			sprintf(chBuf,"%d",i);
			vecTempFileName[i]=strDirectPrefix;//�����ļ�����		
			vecTempFileName[i]+=chBuf;
			vecTempFileHandle[i] = ::open(vecTempFileName[i].c_str(), O_CREAT|O_RDWR|O_TRUNC|O_BINARY|O_LARGEFILE, 0644);
			if (vecTempFileHandle[i] ==  -1)
			{		
				fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
				return -1;
			}
		}

		//1��ȡ������ д��ʱ�ֶ��ļ�
		for ( i=0; i<nColCnt; ++i)
		{

			
			if((nReadCnt=read(fdSrc,(void*)&vecBuf[0],nEachColCnt*sizeof(NODE)))<0)
			{
				fprintf(stderr,"file:%s , line: %d, error info: %s\n", __FILE__, __LINE__, strerror(errno));
				return -1;
			}

// 			for (int k=0;k<128;++k)
// 			{
// 			  printf("%-10d%-10d",vecBuf[k].a,vecBuf[k].b);
// 			}
// 			getchar();
			if (i!=nColCnt-1 && nReadCnt!=nEachColCnt*sizeof(NODE))//��ȡ����������
			{
				fprintf(stderr,"file:%s , line: %d, error info: %s\n", __FILE__, __LINE__, strerror(errno));
				return -1;
			}
			if (i==nColCnt-1)//���һ�п��ܶ�ȡ�ıȽ�ǰ�������
			{
				vecBuf.erase(vecBuf.begin()+nReadCnt/sizeof(NODE),vecBuf.end());
			}

			sort(vecBuf.begin(),vecBuf.end(),CMP_NODE);
			
			if ((nWriteCnt=write(vecTempFileHandle[i],(void*)&vecBuf[0],(int)nReadCnt))==-1
				|| (int)nReadCnt!=nWriteCnt)
			{
				fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
				return -1;
			}
		}

		//׼���鲢
		int nEachMergeCnt=nEachColCnt/(nColCnt+1);//�鲢���õ�ÿ��������Ԫ�ظ���
		vecBuf.resize(nEachColCnt);

		NODE* outPtrBegin;//�����������ʼλ��

		vector<NODE*> vecNodePtr;
		vector<NODE*> vecNodeEndPtr;
		vecNodePtr.resize(nColCnt);
		vecNodeEndPtr.resize(nColCnt);

		for (i=0; i<nColCnt; ++i)
		{
			vecNodePtr[i]=&vecBuf[0]+i*nEachMergeCnt;	
			lseek64(vecTempFileHandle[i],0,SEEK_SET);
			//SetFilePointer(vecTempFileHandle[i],0,NULL,FILE_BEGIN);
			if((nReadCnt=read(vecTempFileHandle[i],(void*)vecNodePtr[i],nEachMergeCnt*sizeof(NODE)))<0)
			{
				fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
				return -1;
				//return pair<NODE*,int>(NULL,-1);//��ʾ��ȡ�ļ�ʧ��
			}
			vecNodeEndPtr[i]=vecNodePtr[i]+nReadCnt/sizeof(NODE);
		}
		outPtrBegin=vecNodeEndPtr[nColCnt-1];

		CWinnerTree<NODE> winnerTree(CMP_NODE);
		winnerTree.CreateTree(vecNodePtr,vecNodeEndPtr);
		winnerTree.K_MergeFile(vecTempFileHandle,fdDest,nEachMergeCnt,outPtrBegin);

		for (i=0; i<nColCnt; ++i)
		{
			close(vecTempFileHandle[i]);
			unlink(vecTempFileName[i].c_str());
		}

	}
	close(fdSrc);
	close(fdDest);
	return 0;
}

//Ҫ������ļ��Ѿ��ź��� 
template <class NODE>
int  K_MergeFile(vector<string> &vecFileNames, const char *pchFileOut,
				 bool(*CMP_NODE)(const NODE& L, const NODE& R), int nCacheNodeCnt)
{


	vector<int> vecTempFileHandle;
	vecTempFileHandle.resize(vecFileNames.size());
	for (int i=0;i<vecFileNames.size();++i )
	{		
// 		vecTempFileHandle[i] = 
// 			CreateFile(
// 			vecFileNames[i].c_str(),
// 			GENERIC_READ,
// 			FILE_SHARE_READ|FILE_SHARE_DELETE,
// 			0,
// 			OPEN_EXISTING,
// 			0,
// 			0);
// 		if (vecTempFileHandle[i] == INVALID_HANDLE_VALUE)
// 		{		
// 			printf("Could not create file handle object !(%d).\n",GetLastError()); 
// 			return -1;
// 		}
		vecTempFileHandle[i] = open(vecFileNames[i].c_str(), O_RDWR|O_BINARY|O_LARGEFILE, 0644);
		if (vecTempFileHandle[i] ==  -1)
		{		
			fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
				return -1;
		}
	}

// 	HANDLE hDestFile;
// 	hDestFile = 
// 		CreateFile(
// 		pchFileOut,
// 		GENERIC_WRITE,
// 		0,
// 		0,
// 		CREATE_ALWAYS,
// 		0,
// 		0);
// 	if (hDestFile == INVALID_HANDLE_VALUE)
// 	{		
// 		printf("Could not create file handle object !(%d).\n",GetLastError()); 
// 		return -1;
// 	}
	int fdDest;
	fdDest = ::open(pchFileOut, O_CREAT|O_RDWR|O_TRUNC|O_BINARY|O_LARGEFILE, 0644);

	if (fdDest == -1)
	{	
		fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
		return -1;
	}


	//׼���鲢
	vector<NODE> vecBuf;//��������	
	
	int nEachMergeCnt=nCacheNodeCnt/(vecFileNames.size()+1);//�鲢���õ�ÿ��������Ԫ�ظ���
	vecBuf.resize(nCacheNodeCnt);

	NODE* outPtrBegin;//�����������ʼλ��

	vector<NODE*> vecNodePtr;
	vector<NODE*> vecNodeEndPtr;
	vecNodePtr.resize(vecFileNames.size());
	vecNodeEndPtr.resize(vecFileNames.size());


    unsigned int nReadCnt;
	for (int i=0;i<vecFileNames.size();++i)
	{
		vecNodePtr[i]=&vecBuf[0]+i*nEachMergeCnt;	
		//SetFilePointer(vecTempFileHandle[i],0,NULL,FILE_BEGIN);
		//if(!ReadFile(vecTempFileHandle[i],vecNodePtr[i],nEachMergeCnt*sizeof(NODE),&nReadCnt,NULL))
		if((nReadCnt=read(vecTempFileHandle[i],vecNodePtr[i],nEachMergeCnt*sizeof(NODE)))==-1)
		{
			//printf("ReadFile error %d.\n",GetLastError());
			fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
			return -1;
			//return pair<NODE*,int>(NULL,-1);//��ʾ��ȡ�ļ�ʧ��
		}
		vecNodeEndPtr[i]=vecNodePtr[i]+nReadCnt/sizeof(NODE);
	}
	outPtrBegin=vecNodeEndPtr[vecFileNames.size()-1];

	CWinnerTree<NODE> winnerTree(CMP_NODE);
	winnerTree.CreateTree(vecNodePtr,vecNodeEndPtr);
	winnerTree.K_MergeFile(vecTempFileHandle,fdDest,nEachMergeCnt,outPtrBegin);

	for (int i=0;i<vecFileNames.size();++i)
	{
		//CloseHandle(vecTempFileHandle[i]);
		close(vecTempFileHandle[i]);
		//DeleteFile(vecTempFileName[i].c_str());
	}
	//CloseHandle(hDestFile);
	close(fdDest);
	return 0;
}

template<class NODE>
bool CWinnerTree<NODE>::CreateTree(vector<NODE*>&vecNodePtr,vector<NODE*>&vecNodeEndPtr,NODE *pNodeDest)
{
	m_nStoppedRun=0;	
	if (vecNodePtr.size()<1)
	{ return false;	}
	m_vecRunTracks=vecNodePtr;
	m_vecRunEnds=vecNodeEndPtr;
	m_vecRunBeginPtr=vecNodePtr;
	m_pNodeDest=pNodeDest;

	m_vecStopMark.resize(vecNodePtr.size(),0);		

	MakeTree();
	return true;

}
template<class NODE>
bool CWinnerTree<NODE>:: CreateTree(vector<NODE*>&vecNodePtr,vector<NODE*>&vecNodeEndPtr,pair<NODE,int> *pNodePrDest)
{
	m_nStoppedRun=0;	
	if (vecNodePtr.size()<1)
	{ return false;	}
	m_vecRunTracks=vecNodePtr;
	m_vecRunEnds=vecNodeEndPtr;
	m_vecRunBeginPtr=vecNodePtr;
	m_pNodePrDest=pNodePrDest;

	m_vecStopMark.resize(vecNodePtr.size(),0);		

	MakeTree();
	return true;

}

template<class NODE>
inline NODE* CWinnerTree<NODE>::K_Merge(NODE *)
{
	//resPair=m_vecTree[0];
	
	while (m_vecRunTracks.size()!=m_nStoppedRun)
	{
		//m_pVecDest->push_back(*(m_vecTree[0].first));
		*m_pNodeDest++=*(m_vecTree[0].first);
		nRun=m_vecTree[0].second;
		nTrack=m_vecRunInTreePos[m_vecTree[0].second];//���ټ�����

		//����һ��������ײ�
		if (++(m_vecRunTracks[nRun])==m_vecRunEnds[nRun])//һ����ֹ��
		{

			MakeTree();//һ����ֹ���¹�����
			//++m_nStoppedRun;
		}
		else
		{
			m_vecTree[nTrack].first=m_vecRunTracks[nRun];
			m_vecTree[nTrack].second=nRun;
			//���������бȽ�
			while (nTrack!=0)
			{
				if (nTrack%2==0)//��ż��Ϊ������
				{
					m_vecTree[(nTrack>>1)-1]=CMP_NODE(m_vecTree[nTrack],m_vecTree[nTrack-1])?m_vecTree[nTrack]:m_vecTree[nTrack-1];				
					nTrack>>=1;
					nTrack-=1;
				}
				else
				{
					m_vecTree[nTrack>>1]=CMP_NODE(m_vecTree[nTrack],m_vecTree[nTrack+1])?m_vecTree[nTrack]:m_vecTree[nTrack+1];				
					//nTrack-=1;
					nTrack>>=1;
				}	
			}
		}

	}	
	return m_pNodeDest;
}
template<class NODE>
inline pair<NODE,int> * CWinnerTree<NODE>::K_Merge(pair<NODE,int> *)
{
	pair<NODE,int> resPair;
	int second;
	while (m_nStoppedRun!=m_vecRunTracks.size())
	{


		second=m_vecTree[0].second;
		resPair.first=*(m_vecTree[0].first);
		resPair.second=second;
		//m_pVecDestPair->push_back(resPair);
		*m_pNodePrDest++=resPair;

		nRun=second;
		nTrack=m_vecRunInTreePos[second];//���ټ�����

		//����һ��������ײ�
		if (++(m_vecRunTracks[nRun])==m_vecRunEnds[nRun])//һ����ֹ��
		{
			// 			m_vecTree[nTrack].first=NULL;
			// 			m_vecTree[nTrack].second=nRun;
			MakeTree();//һ����ֹ���¹�����
			//++m_nStoppedRun;
		}
		else
		{
			m_vecTree[nTrack].first=m_vecRunTracks[nRun];
			m_vecTree[nTrack].second=nRun;
			//���������бȽ�
			while (nTrack!=0)
			{
				if (nTrack%2==0)//��ż��Ϊ������
				{
					m_vecTree[(nTrack>>1)-1]=CMP_NODE(m_vecTree[nTrack],m_vecTree[nTrack-1])?m_vecTree[nTrack]:m_vecTree[nTrack-1];				
					nTrack>>=1;
					nTrack-=1;
				}
				else
				{
					m_vecTree[nTrack>>1]=CMP_NODE(m_vecTree[nTrack],m_vecTree[nTrack+1])?m_vecTree[nTrack]:m_vecTree[nTrack+1];				
					//nTrack-=1;
					nTrack>>=1;
				}	
			}
		}
	}

	return m_pNodePrDest;
}

template<class NODE>
bool CWinnerTree<NODE>::K_MergeFile(vector<int> &vecTempFileHandle,int hWriteFileHandle,int nReadCnt,NODE*pOutPtr)
{
	int nOutCnt=0;

	while (m_vecRunTracks.size()!=m_nStoppedRun)//�����ļ�û�й鲢��ȫ
	{
		pOutPtr[nOutCnt++]=*(m_vecTree[0].first);

		if (nOutCnt==nReadCnt)//д��������
		{
			size_t nWriteCnt;
			if ((nWriteCnt=write(hWriteFileHandle,pOutPtr,nOutCnt*sizeof(NODE)))<0
				|| nOutCnt*sizeof(NODE)!=nWriteCnt)
			{
				//printf("WriteFile error %d.\n",GetLastError());
				fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
				return false;
			}
			nOutCnt=0;
		}

		nRun=m_vecTree[0].second;
		nTrack=m_vecRunInTreePos[m_vecTree[0].second];//���ټ�����

		//����һ��������ײ�
		++(m_vecRunTracks[nRun]);
		if (m_vecRunTracks[nRun]==m_vecRunEnds[nRun])//һ����ֹ��
		{
			// 			m_vecTree[nTrack].first=NULL;
			// 			m_vecTree[nTrack].second=nRun;
			size_t nRealReadCnt=0;
			if ((nRealReadCnt=read(vecTempFileHandle[nRun],(void*)m_vecRunBeginPtr[nRun],nReadCnt*sizeof(NODE)))<0)
			//if(!ReadFile(vecTempFileHandle[nRun],m_vecRunBeginPtr[nRun],nReadCnt*sizeof(NODE),&nRealReadCnt,NULL))
			{
				//printf("ReadFile error %d.\n",GetLastError());
				fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
				return false;
				//return pair<NODE*,int>(NULL,-1);//��ʾ��ȡ�ļ�ʧ��
			}

			if (nRealReadCnt==0)//һ���ļ�������¹�����
			{
				MakeTree();//һ����ֹ���¹�����
				continue;
			}
			else
			{
				m_vecRunTracks[nRun]=m_vecRunBeginPtr[nRun];
				m_vecRunEnds[nRun]=m_vecRunTracks[nRun]+nRealReadCnt/sizeof(NODE);

			}			
			//++m_nStoppedRun;
		}

		m_vecTree[nTrack].first=m_vecRunTracks[nRun];
		m_vecTree[nTrack].second=nRun;
		//���������бȽ�
		while (nTrack!=0)
		{
			if (nTrack%2==0)//��ż��Ϊ������
			{
				m_vecTree[(nTrack>>1)-1]=CMP_NODE(m_vecTree[nTrack],m_vecTree[nTrack-1])?m_vecTree[nTrack]:m_vecTree[nTrack-1];				
				nTrack>>=1;
				nTrack-=1;
			}
			else
			{
				m_vecTree[nTrack>>1]=CMP_NODE(m_vecTree[nTrack],m_vecTree[nTrack+1])?m_vecTree[nTrack]:m_vecTree[nTrack+1];				
				//nTrack-=1;
				nTrack>>=1;
			}	
		}

	}

	if (nOutCnt)//����������ûд���ļ���
	{
		size_t nWriteCnt;
		if ((nWriteCnt=write(hWriteFileHandle,pOutPtr,nOutCnt*sizeof(NODE)))<0
			||nOutCnt*sizeof(NODE)!=nWriteCnt)
		{
			//printf("WriteFile error %d.\n",GetLastError());
			fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
			return false;
		}			
	}



	return true;
}

template<class NODE>
void CWinnerTree<NODE>::MakeTree()
{	
	for (size_t i=0;i<m_vecRunTracks.size();++i)
	{
		if ((m_vecRunTracks[i]==NULL||m_vecRunTracks[i]==m_vecRunEnds[i])&&m_vecStopMark[i]==0)
		{
			m_vecStopMark[i]=1;
			++m_nStoppedRun;
		}	
	}

	int nLeftRun=m_vecRunTracks.size()-m_nStoppedRun;
	if (nLeftRun==0)
	{
		return ;
	}

	//____________________��������ڵ�������������������Ҷ�ӽڵ�� ��ȫ������
	unsigned int i=1;
	m_nLevel=0;
	while(nLeftRun>(int)i)
	{
		++m_nLevel;
		i<<=1;
	}	

	++m_nLevel;//��ȫ�������Ĳ���
	m_vecTree.resize(2*nLeftRun-1);//���һ�㲻��ʱ ���ڵ���Ϊ 2*run-1
	m_nLastLevelBegin=m_vecTree.size()-nLeftRun;		


	//____________________��д����Ҷ�ӽڵ㣬����������RUN�ĵ�һ��Ԫ��
	typename vector<pair<NODE*,int> >::iterator itTree=m_vecTree.begin()+m_nLastLevelBegin;
	int nCnt=-1;
	m_vecRunInTreePos.clear();
	for (i=0;i<m_vecRunTracks.size();++i)
	{
		if ((m_vecRunTracks[i]!=NULL&&m_vecRunTracks[i]!=m_vecRunEnds[i]))			
		{
			(*itTree).first=m_vecRunTracks[i];
			(*itTree).second=i;
			++itTree;
			++nCnt;

		}
		m_vecRunInTreePos.push_back(nCnt+m_nLastLevelBegin);
	}

	if (nLeftRun<=1)
	{
		return;
	}

	//____________________��д�����ڵ�
	int nR=m_vecTree.size()-1;
	int nL=nR-1;
	while (nL>0) 
	{
		m_vecTree[nL>>1]=CMP_NODE(m_vecTree[nL],m_vecTree[nR])?m_vecTree[nL]:m_vecTree[nR];	//��ڵ�С
		nR-=2;
		nL-=2;
	}

}

template <class NODE>
NODE*  K_Intersect(vector<NODE*>&vecNodePtr,
				  vector<NODE*>&vecNodeEndPtr,
				  NODE*        pNodeDest,
				  bool(*CMP_NODE)(const NODE& L, const NODE& R),
				  void(*ADD_NODE)(NODE& L, NODE& R))
{

	size_t i,j;
	if (vecNodePtr.empty())
	{
		return pNodeDest;
	}
	for (j=0;j<vecNodePtr.size();++j)
	{
		if (vecNodePtr[j]==vecNodeEndPtr[j])
		{
			return pNodeDest;
		}				
	}
// 	if (vecNodePtr.size()==1)
// 	{
// 		(*pVecDest).assign(vecNodePtr[0],vecNodeEndPtr[0]);
// 		return ;
// 	}

	if (vecNodePtr.size()==1)
	{		
		NODE* pNode=vecNodePtr[0];
		if (vecNodePtr[0]==pNodeDest)
		{
			return vecNodeEndPtr[0];
		}

		for (;pNode!=vecNodeEndPtr[0];++pNode,++pNodeDest)
		{
			*pNodeDest=*pNode;
		}
		return pNodeDest;
	}


	

// 	for (i=1;i<vecNodePtr.size();++i) //{--for--}�Ҹ���ͷ�� ����ֵ��
// 	{
// 		if (CMP_NODE(nMax,*(vecNodePtr[i])))
// 		{
// 			nMax=*(vecNodePtr[i]);
// 			nMaxMark=i;
// 		}
// 	}


	vector<NODE*> vecCurColMark=vecNodePtr;
	NODE *pCurNode;
	NODE *pEndNode;
	bool bSuccess;
	NODE nMax;
	int nMaxMark=0;
	nMax=*(vecCurColMark[nMaxMark]);
	while (true)
	{			
		bSuccess=true;
		for (i=(nMaxMark+1)%vecNodePtr.size();i!=nMaxMark;)
		{
			pCurNode=vecCurColMark[i];
			pEndNode=vecNodeEndPtr[i];

			while (pCurNode!=pEndNode&&CMP_NODE(*pCurNode,nMax))//ȥ��һ����ͬ
			{
				++pCurNode;
			}
			vecCurColMark[i]=pCurNode;

			if (pCurNode==pEndNode)                             //û�ҵ�
			{
				return pNodeDest;
			}
			else
			{
				if (CMP_NODE(nMax,*pCurNode))                          //��һ��Ԫ�ز��ǹ����ģ��Ե�ǰ�е�ǰԪ��Ϊ��׼
				{
					nMaxMark=i;
					nMax=*pCurNode;
					bSuccess=false;
						break;
				}
				else if(ADD_NODE)
					ADD_NODE(nMax, *pCurNode);

			}
			++i;
			i=i%vecNodePtr.size();
		}

		if (bSuccess)                 //�ҵ�һ����������
		{			
			          
			*pNodeDest++=nMax;
			//pVecDest->push_back(nMax);   
			for (j=0;j<vecCurColMark.size();++j)
			{
				if (++vecCurColMark[j]==vecNodeEndPtr[j])
				{
					return pNodeDest;
				}				
			}			
			nMax=*(vecCurColMark[nMaxMark]);	
// 			nMax=*vecCurColMark[0];			
// 			for (i=1;i<vecCurColMark.size();++i) //{--for--}�Ҹ���ͷ�� ����ֵ��
// 			{			
// 				cout<<"\ni "<<i<<":*(vecCurColMark[i]) "<<"Id "<<vecCurColMark[i]->nDocID<<" off "<<vecCurColMark[i]->nOffset;
// 				if (CMP_NODE(nMax,*(vecCurColMark[i])))
// 				{
// 					nMax=*(vecCurColMark[i]);
// 					nMaxMark=i;
// 				}
// 			}
		}

	}
    return pNodeDest;
}

;

template <class NODE>
NODE*  K_InvertIntersect(vector<NODE*>&vecNodePtr,
						vector<NODE*>&vecNodeEndPtr,
						NODE*        pNodeDest,
					    int (*MINUS_INVERT)(NODE&l,NODE&r),
						NODE&(*INVERT_MINUS_INT)(NODE&l,int nMinus))
{

	int i,j;
	if (vecNodePtr.empty())
	{
		return pNodeDest;
	}
	for (j=0;j<vecNodePtr.size();++j)
	{
		if (vecNodePtr[j]==vecNodeEndPtr[j])
		{
			return pNodeDest;
		}				
	}

	
	if (vecNodePtr.size()==1)
	{		
		NODE* pNode=vecNodePtr[0];
		if (vecNodePtr[0]==pNodeDest)
		{
			return vecNodeEndPtr[0];
		}

		for (;pNode!=vecNodeEndPtr[0];++pNode,++pNodeDest)
		{
			*pNodeDest=*pNode;
		}
		return pNodeDest;
	}


	vector<NODE*> vecCurColMark=vecNodePtr;
	NODE *pCurNode;
	NODE *pEndNode;
	bool bSuccess;
	int nMatchGap;
	int nMinus;
	NODE nMax;
	int nMaxMark=0;
	nMax=*(vecCurColMark[nMaxMark]);
	while (true)
	{
		bSuccess=true;
		for (i=(nMaxMark+1)%vecNodePtr.size();i!=nMaxMark;)
		{
			pCurNode=vecCurColMark[i];
			pEndNode=vecNodeEndPtr[i];
			nMatchGap=i-nMaxMark;			
			while (pCurNode!=pEndNode&&MINUS_INVERT(*pCurNode,nMax)<nMatchGap)//ȥ��һ����ͬ
			{
				++pCurNode;
			}
		
			vecCurColMark[i]=pCurNode;

			if (pCurNode==pEndNode)                             //û�ҵ�
			{
				return pNodeDest;
			}
			else
			{
				if (MINUS_INVERT(*pCurNode,nMax)!=nMatchGap)                          //��һ��Ԫ�ز��ǹ����ģ��Ե�ǰ�е�ǰԪ��Ϊ��׼
				{
					nMaxMark=i;
					nMax=*pCurNode;
					bSuccess=false;
					break;
				}

			}
			++i;
			i=i%vecNodePtr.size();
		}

		if (bSuccess)                     //�ҵ�һ����������
		{
			 INVERT_MINUS_INT(nMax,i);    //ȡƥ��ĵ�һ���ֵ�ƫ��
			// nMaxMark=0;      
			 *pNodeDest++ = nMax;

			// pVecDest->push_back(nMax);   

			for (j=0;j<vecCurColMark.size();++j)
			{
				if (++vecCurColMark[j]==vecNodeEndPtr[j])
				{
					return pNodeDest;
				}	
			}

			nMax=*vecCurColMark[nMaxMark];			
// 			for (i=1;i<vecCurColMark.size();++i) //{--for--}�Ҹ���ͷ�� ����ֵ��
// 			{
// 				if (MINUS_INVERT(*vecCurColMark[i],nMax)> i-nMaxMark)
// 				{
// 					nMax=*(vecCurColMark[i]);
// 					nMaxMark=i;
// 				}
// 			}

		}

	}

}
#endif
