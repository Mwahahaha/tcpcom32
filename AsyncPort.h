// AsyncPort.h: interface for the CAsyncPort class.
// Last Revision: y2003 m09 d12
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//	This class models a generic buffered asyncronous serial port.
//	Implementation is thread safe. For each instance a thread is 
//	created to wait for asyncronous input data. 
//////////////////////////////////////////////////////////////////////


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <windows.h>

#ifndef ASYNCPORT_H_INCLUDED
#define ASYNCPORT_H_INCLUDED



#define LOG_SILENT_TIME 50 // Temps avant quavegarde des logs en ms
class CAsyncPort
{
protected:

	enum
	{
		IN_BUFFER_SIZE = 4096,		// input buffer size in bytes (4 Kb)
		IN_QUEUE_SIZE = 1024,		// input queue size in bytes (1 Kb)
		OUT_QUEUE_SIZE = 1024		// input queue size in bytes (1 Kb)
	};

protected:

	char m_szCommDevName[MAX_PATH];
   char m_szLogDir[MAX_PATH];
   char m_szBufferIn[5* IN_QUEUE_SIZE];
   char m_szBufferOut[5* IN_QUEUE_SIZE];
   int m_szInBuffLen;
   int m_szOutBuffLen;
   LARGE_INTEGER m_TimeLastIn;
   LARGE_INTEGER m_TimeFirstIn;
   LARGE_INTEGER m_TimeLastOut;
   LARGE_INTEGER m_TimeFirstOut;
   LARGE_INTEGER m_Freq;
   SYSTEMTIME m_DateIn;
   SYSTEMTIME m_DateOut;

	HANDLE m_hCommDev;
	COMMCONFIG m_commConfig;
	COMMPROP m_commProp;
	COMMTIMEOUTS m_commTimeOuts;
	OVERLAPPED m_ovlRead;
	OVERLAPPED m_ovlWrite;
	BOOL m_isConnected;
	HANDLE m_hWatchThread;
	DWORD m_dwThreadID;
	BYTE m_baInBuf[IN_BUFFER_SIZE];
	unsigned m_nInBufIndex;
	unsigned m_nInBufSize;
	char m_szMutexName[19];				// ex. "CAsyncPortXXXXXXXX"

	int ParseSetupString(const char * pszSetupString, DWORD * pdwBaudRate, BYTE * pbByteSize, BYTE * pbParity, BYTE * pbStopBits, char nSeparator = ',');
	int SetupConnection(DCB dcb);
	int ReadCommBlock(char * pszBlock, int nMaxLength);
	int WriteCommBlock(BYTE * pByte, DWORD dwBytesToWrite);
	int WatchCommDev();
	friend DWORD WINAPI CommWatchProc(LPVOID lpParam);
   BOOL WriteLogIn(int ComTime);
   BOOL WriteLogOut(int ComTime);
   BOOL WriteLog(const char* FileHeader, char* Buffer, int szBuffer, SYSTEMTIME* ComDate, float ComTime);

public:

	CAsyncPort();
	virtual ~CAsyncPort();
   void SaveLog(bool CheckTime);

	static int IsInstalled(const char * pszCommDevName);

	int OpenConnection(const char * pszPortName, const char * pszSetupString = "9600,8,N,1");
	int OpenConnection(const char * pszPortName, DWORD dwBaudRate, BYTE bByteSize, BYTE bParity, BYTE bStopBits);
	int OpenConnection(const char * pszPortName, DCB dcb);
	int CloseConnection();
	int SetupConnection(DWORD dwBaudRate, BYTE bByteSize, BYTE bParity, BYTE bStopBits);
	BOOL IsConnected();
	int ClearInputBuffer();
	int ReadBuffer(BYTE * bpBuffer, unsigned nBytes, BOOL nExtractFlag);
	int WriteBuffer(BYTE * bpBuffer, unsigned nBytes);
	int DeleteBuffer(unsigned nBytes);
	unsigned BufferedDataSize();
	int ShowConfigDialog(HWND hWnd = NULL);
	const char *GetCommDevName();
	LPSTR GetSetupString(LPSTR pszBuffer, int nBufferSize);
	BOOL GetCommState(DCB *lpDcb);
	static LPSTR FormatDcb(DCB *lpDcb, LPSTR lpszString, int nStringSize);
	int Debug(const char * pszLogDir);
};

#endif // ASYNCPORT_H_INCLUDED

