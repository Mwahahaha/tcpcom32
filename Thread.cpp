// Thread.cpp: thread main procs.
// Last Revision: y2008 m01 d29
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "ServerSocket.h"
#include "SettingsDlg.h"
#include "PortContext.h"
#include "tcpcom32Dlg.h"
#include "base64.h"
#include "NTLM.h"
#include "HTTP.h"
#include "Thread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CSettingsDlg theSettingsDlg;


BOOL StartThread(LPTHREAD_START_ROUTINE lpRoutine, LPVOID lpParam, LPHANDLE lpEvent, LPHANDLE lpThread, LPDWORD lpThreadId)
{
	// Clear output values
	if (lpEvent) {
		*lpEvent = NULL;
	}
	if (lpThread) {
		*lpThread = NULL;
	}
	if (lpThreadId) {
		*lpThreadId = 0;
	}

	// Create termination event
	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (NULL == hEvent) {
		MessageBox(NULL, "Error creating termination event", "ERROR", MB_OK);
		return FALSE;
	}
	ResetEvent(hEvent);

	// Create thread
	DWORD dwThreadId;
	HANDLE hThread = CreateThread(NULL, 0, lpRoutine, lpParam, 0, &dwThreadId);
	if (NULL == hThread) {
		CloseHandle(hEvent);
		MessageBox(NULL, "Error starting thread", "ERROR", MB_OK);
		return FALSE;
	}

	// Set output values
	if (lpEvent) {
		*lpEvent = hEvent;
	}
	if (lpThread) {
		*lpThread = hThread;
	}
	if (lpThreadId) {
		*lpThreadId = dwThreadId;
	}

	return TRUE;
}

DWORD WINAPI StopThreadProc(LPVOID lpParam)
{
HANDLE *lpArgs = (HANDLE *)lpParam;
HANDLE hEvent = lpArgs[0];
HANDLE hThread = lpArgs[1];	

if (NULL != hEvent) 
   {
	if (0 == SetEvent(hEvent)) 
      {
		MessageBox(NULL, "Error setting termination event", "ERROR", MB_OK);
		return -1;
		}
	}

if (NULL != hThread) 
   {
	DWORD dwExitCode = STILL_ACTIVE;
	while (GetExitCodeThread(hThread, &dwExitCode)
            && STILL_ACTIVE == dwExitCode) 
      {
		Sleep(100);
		} 
	}

return 0;
}

void ShutdownMainThread(CTcpcom32Dlg* lpTcpcom32Dlg)
{
	// Stop main thread
   for (int ii = 0; ii < MX_COM_PORT; ii++)
      {
      if (lpTcpcom32Dlg->m_hEvent[ii] && lpTcpcom32Dlg->m_hThread[ii]) {
         LPHANDLE lpArgs = new HANDLE[2];
         lpArgs[0] = lpTcpcom32Dlg->m_hEvent[ii];
         lpArgs[1] = lpTcpcom32Dlg->m_hThread[ii];
         StopThreadProc(lpArgs);
         lpTcpcom32Dlg->m_hEvent[ii] = NULL;
         lpTcpcom32Dlg->m_hThread[ii] = NULL;
         }
      }
}

DWORD WINAPI InitializeThreadProc(LPVOID lpParam)
   {
   CTcpcom32Dlg *lpTcpcom32Dlg = (CTcpcom32Dlg *)lpParam;
   char buf[256];
   CTreeCtrl& tree = lpTcpcom32Dlg->m_treeview;

   // Clear initialized flag
   lpTcpcom32Dlg->m_isInitialized = FALSE;

   // Disable initialize
   CMenu *lpMenu = lpTcpcom32Dlg->GetMenu()->GetSubMenu(MENU_SYSTEM);
   lpMenu->EnableMenuItem(ID_SYSTEM_SETTINGS, MF_DISABLED | MF_GRAYED);
   lpMenu->EnableMenuItem(ID_SYSTEM_RESTART, MF_DISABLED | MF_GRAYED);

   // Status line
   lpTcpcom32Dlg->SendMessage(WM_THREAD_NOTIFY, (WPARAM)"Initializing...", MAKELONG(CMD_SET_STATUS, 0));


   // Stop threads

   for (int ii = 1; ii < MX_COM_PORT; ii++)
      {
      if (lpTcpcom32Dlg->m_ctx[ii] != NULL)
         {
         LPHANDLE lpArgs = new HANDLE[2];
         lpArgs[0] = lpTcpcom32Dlg->m_ctx[ii]->hEvent;
         lpArgs[1] = lpTcpcom32Dlg->m_ctx[ii]->hThread;
         StopThreadProc(lpArgs);
         }
      }
   

   // Stop main thread
   ShutdownMainThread(lpTcpcom32Dlg);

   // Re-initialize treeview
   tree.DeleteAllItems();

   gethostname(buf, 256);
   HTREEITEM htroot = tree.InsertItem(TEXT(buf), BMP_ROOT, BMP_ROOT);
   
   int TcpPort;
   char szPortName[12];
   char szPortNameCOM[12];
   HTREEITEM hti;
   for (int ii = 1; ii < MX_COM_PORT; ii++)
      {
      snprintf(szPortName, _countof(szPortName), "COM%d", ii);
      snprintf(szPortNameCOM, _countof(szPortNameCOM), "\\\\.\\COM%d", ii);
      if (CAsyncPort::IsInstalled(TEXT(szPortNameCOM))) 
         {
         DCB* Dcb = new DCB();
        
         if (theSettingsDlg.IsComConfigured(szPortName)
             && TRUE == theSettingsDlg.GetPortConfig(szPortName, Dcb, &TcpPort))
            {
            hti = tree.InsertItem(TEXT(szPortName), BMP_PORT_DISABLED, BMP_PORT_DISABLED, htroot);
            LPVOID *lpArgs = new LPVOID[8];
            lpArgs[0] = lpTcpcom32Dlg;
            lpArgs[1] = (LPVOID*)ii;
            lpArgs[2] = (LPVOID*)TcpPort;
            lpArgs[3] = (LPVOID*)Dcb;
            lpArgs[4] = (LPVOID*)hti;
            StartThread(AcceptThreadProc, lpArgs, &(lpTcpcom32Dlg->m_hEvent[ii]), &(lpTcpcom32Dlg->m_hThread[ii]), &(lpTcpcom32Dlg->m_dwThreadId[ii]));
            }
         else 
            hti = tree.InsertItem(TEXT(szPortName), BMP_PORT_CLOSED, BMP_PORT_CLOSED, htroot);

         tree.SetItemData(hti, 0);
         }
      }
   tree.Expand(htroot, TVE_EXPAND);
   
   // Status line
   lpTcpcom32Dlg->SendMessage(WM_THREAD_NOTIFY, (WPARAM)"Initialized", MAKELONG(CMD_SET_STATUS, 0));

   // Enable initialize
   lpMenu->EnableMenuItem(ID_SYSTEM_SETTINGS, MF_ENABLED);
   lpMenu->EnableMenuItem(ID_SYSTEM_RESTART, MF_ENABLED);

   // Initialized
   lpTcpcom32Dlg->m_isInitialized = TRUE;

   return 0;
   }


#define MX_TCP_CONNECTION 64

DWORD WINAPI AcceptThreadProc(LPVOID lpParam)
{
// Decode lpParam
LPVOID *lpArgs = (LPVOID *)lpParam;
CTcpcom32Dlg *lpTcpcom32Dlg = (CTcpcom32Dlg *)lpArgs[0];
int ComPort = (int)lpArgs[1];
int TcpPort = (int)lpArgs[2];
DCB* Dcb = (DCB*)lpArgs[3];
HTREEITEM hti = (HTREEITEM)lpArgs[4];
char Err[256];
CClientSocket* TcpConnection[MX_TCP_CONNECTION];

for (int ii = 0; ii < MX_TCP_CONNECTION; ii++)
   TcpConnection[ii] = NULL;

// Initialize Windows Sockets
WSADATA data;
WSAStartup(MAKEWORD(1,1), & data);

// Minimize on start
if (lpTcpcom32Dlg->m_dwCmdShow == SW_HIDE)
	lpTcpcom32Dlg->SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);

// Status line && trayicon
lpTcpcom32Dlg->SendMessage(WM_THREAD_NOTIFY, (WPARAM)"Running", MAKELONG(CMD_SET_STATUS,0));
lpTcpcom32Dlg->SendMessage(WM_THREAD_NOTIFY, (WPARAM)0, MAKELONG(CMD_TRAYICON,0));

// Open server socket
CServerSocket serverSocket;
char buf[256];
if (-1 == serverSocket.Open(theSettingsDlg.GetLocalIpAddress(buf, 128), TcpPort)) 
   {
   snprintf(Err, _countof(Err), "Error opening server socket %d", TcpPort);
	MessageBox(NULL, Err, "ERROR", MB_OK);
	return -1;
	}
serverSocket.SetNonBlocking();

// Lancement du thread de gestion du port
LPVOID *Args = new LPVOID[8];
Args[0] = TcpConnection;
Args[1] = lpTcpcom32Dlg;
Args[2] = (LPVOID*)ComPort;
Args[3] = (LPVOID*)Dcb;
Args[4] = (LPVOID*)hti;
StartThread(PortThreadProc, Args, (LPHANDLE)(Args + 5), (LPHANDLE)(Args + 6), (LPDWORD)(Args + 7));
bool ConnectionAccepted;
// Main loop
for (BOOL loop = TRUE; loop; ) 
   {
   // Check termination
   if (WAIT_OBJECT_0 == WaitForSingleObject(lpTcpcom32Dlg->m_hEvent[ComPort], 0)) {
      loop = FALSE;
      break;
      }

   ConnectionAccepted = false;
	// Wait incoming connections
	if (-1 == serverSocket.WaitForPendingData(1, 0))
		continue;

   for (int ii = 0; ii < MX_TCP_CONNECTION; ii++)
      {
      if (TcpConnection[ii] == NULL)
         {
         ConnectionAccepted = true;
         CClientSocket* lpSocket = new CClientSocket();
         if (0 == serverSocket.Accept(lpSocket))
            {
            TcpConnection[ii] = lpSocket;
            }
         break;
         }
      }	
   }
// Close server socket
serverSocket.Close();
return 0;
}

DWORD WINAPI PortThreadProc(LPVOID lpParam)
{
#define SHOW_PORT_ACTIVITY	

#define BUFLEN				8192
#define SPARE_BUF			0
#define SPARE_LPPARAM		1
#define SPARE_EXTRA			2
#define MEMSZ_URL			256
#define MEMSZ_IP			128
#define MEMSZ_PORT			16
#define MEMSZ_USERNAME		64
#define MEMSZ_PASSWORD		64
#define MEMSZ_WORKGROUP		64
#define MEMSZ_WORKSTATION	64
#define MEMSZ_AUTH			1024
#define MEMSZ_AUTHSCHEME	32
#define MEMSZ_CONNECTION	32

// Decode lpParam
LPVOID *lpArgs = (LPVOID *)lpParam;
CClientSocket** TcpConnection = (CClientSocket**)lpArgs[0];
CTcpcom32Dlg *lpTcpcom32Dlg = (CTcpcom32Dlg *)lpArgs[1];
int ComPort = (int)lpArgs[2];
DCB* Dcb = (DCB*)lpArgs[3];
HTREEITEM hti = (HTREEITEM)lpArgs[4];
HANDLE hEvent = (HANDLE)(lpArgs[5]);
HANDLE hThread = (HANDLE)(lpArgs[6]);
DWORD dwThreadId = (DWORD)(lpArgs[7]);

char szPortName[12];
char szPortNameCOM[12];
snprintf(szPortName, _countof(szPortName), "COM%d", ComPort);
snprintf(szPortNameCOM, _countof(szPortNameCOM), "\\\\.\\COM%d", ComPort);


// Allocate buffer
char *buf = new char[BUFLEN+BUFLEN];
char *mem = buf + BUFLEN;

// Create context
CPortContext* ctx = new CPortContext();
lpTcpcom32Dlg->m_ctx[ComPort] = ctx;
ctx->hEvent = hEvent;
ctx->hThread = hThread;
ctx->htiPort = hti;
ctx->dwThreadId = dwThreadId;
ctx->lpSpare[SPARE_LPPARAM] = lpParam;
ctx->lpSpare[SPARE_BUF] = buf;
ctx->lpDcb = Dcb;
strcpy(ctx->szPortName, szPortName);

CTreeCtrl& tree = lpTcpcom32Dlg->m_treeview;
tree.SetItemData(hti, (DWORD_PTR)ctx);


// Startup Windows Sockets Library
WSADATA data;
WSAStartup(MAKEWORD(1,1), & data);
   
DWORD dwMessageLength = 0;
DWORD dwExtraLength = 0;

// Open asynch port
CAsyncPort *lpPort = new CAsyncPort();
ctx->lpPort = lpPort;

if (-1 == lpPort->OpenConnection(szPortNameCOM, *Dcb)) 
   {
	lpTcpcom32Dlg->SendMessage(WM_THREAD_NOTIFY, (WPARAM)(ctx->htiPort),
		MAKELONG(CMD_SET_IMAGE,BMP_PORT_CLOSED));
   for (int ii = 0; ii < MX_TCP_CONNECTION; ii++)
      {
      if (TcpConnection[ii] != NULL)
         {
         TcpConnection[ii]->Close();
         }
      }
	ctx->Cleanup();
	return -1;
}

if (theSettingsDlg.IsLogEnabled())
   lpPort->Debug(theSettingsDlg.GetLogDir(buf, 256));
   
// Update treeview & trayicon
lpTcpcom32Dlg->SendMessage(WM_THREAD_NOTIFY, (WPARAM)(ctx), MAKELONG(CMD_OPEN_PORT,0));
lpTcpcom32Dlg->SendMessage(WM_THREAD_NOTIFY, (WPARAM)(ctx->htiPort), MAKELONG(CMD_SET_IMAGE,BMP_PORT_OPEN));
lpTcpcom32Dlg->SendMessage(WM_THREAD_NOTIFY, (WPARAM)0, MAKELONG(CMD_TRAYICON,0));
   
LPBYTE bytes = (LPBYTE)buf;
int portlen = -1;
int socklen = -1;
#ifdef SHOW_PORT_ACTIVITY
int lastbmp = BMP_PORT_OPEN;
DWORD dwPortTicks = GetTickCount();
DWORD dwSockTicks = GetTickCount();
#endif // SHOW_PORT_ACTIVITY
while (TRUE) 
   {
	// Check thread termination
	if (WAIT_OBJECT_0 == WaitForSingleObject(hEvent, 0)) 
		break;

#ifdef SHOW_PORT_ACTIVITY
	DWORD dwTicks = GetTickCount();
	if (dwTicks - dwPortTicks < 1000) 
      {
		if (dwTicks - dwSockTicks < 1000) 
         {
			if (BMP_PORT_OPEN_BOTH != lastbmp) 
				lpTcpcom32Dlg->SendMessage(WM_THREAD_NOTIFY, (WPARAM)(ctx->htiPort), MAKELONG(CMD_SET_IMAGE,(lastbmp=BMP_PORT_OPEN_BOTH)));
			} 
      else if (BMP_PORT_OPEN_LOCAL != lastbmp) 
			lpTcpcom32Dlg->SendMessage(WM_THREAD_NOTIFY, (WPARAM)(ctx->htiPort), MAKELONG(CMD_SET_IMAGE,(lastbmp=BMP_PORT_OPEN_LOCAL)));
		} 
   else if (dwTicks - dwSockTicks < 1000) 
      {
		if (BMP_PORT_OPEN_REMOTE != lastbmp) 
			lpTcpcom32Dlg->SendMessage(WM_THREAD_NOTIFY, (WPARAM)(ctx->htiPort), MAKELONG(CMD_SET_IMAGE,(lastbmp=BMP_PORT_OPEN_REMOTE)));
		} 
   else if (BMP_PORT_OPEN != lastbmp) 
		lpTcpcom32Dlg->SendMessage(WM_THREAD_NOTIFY, (WPARAM)(ctx->htiPort), MAKELONG(CMD_SET_IMAGE,(lastbmp=BMP_PORT_OPEN)));

#endif // SHOW_PORT_ACTIVITY

	// poll asynch port
	if (lpPort->BufferedDataSize() > 0) 
      {
		portlen = lpPort->ReadBuffer(bytes, BUFLEN, TRUE);
		if (portlen > 0) 
         {
			ctx->dwPortInBytes += portlen;

         for (int ii = 0; ii < MX_TCP_CONNECTION; ii++)
            {
            if (TcpConnection[ii] != NULL
               && portlen != TcpConnection[ii]->Send(buf, portlen))
               {
               TcpConnection[ii]->Close();
               TcpConnection[ii] = NULL;
               }
            }
			ctx->dwSockOutBytes += portlen;
#ifdef SHOW_PORT_ACTIVITY
			dwPortTicks = dwTicks;
			lpTcpcom32Dlg->SendMessage(WM_THREAD_NOTIFY, (WPARAM)(ctx), MAKELONG(CMD_SET_STATS,0));
#endif // SHOW_PORT_ACTIVITY
			continue;
			}
		}
	if (-1 == lpPort->IsConnected()) 
		break;

	// poll socket
   for (int ii = 0; ii < MX_TCP_CONNECTION; ii++)
      {
      if (TcpConnection[ii] != NULL)
         {
         socklen = TcpConnection[ii]->Recv(buf, BUFLEN, 0, 1000);

         if (0 == socklen)
            {
            TcpConnection[ii]->Close();
            TcpConnection[ii] = NULL;
            }
         if (socklen > 0)
            {
            ctx->dwSockInBytes += socklen;
            if (socklen != lpPort->WriteBuffer(bytes, socklen))
               {
               TcpConnection[ii]->Close();
               TcpConnection[ii] = NULL;
               }
            ctx->dwPortOutBytes += socklen;
#ifdef SHOW_PORT_ACTIVITY
            dwSockTicks = dwTicks;
            lpTcpcom32Dlg->SendMessage(WM_THREAD_NOTIFY, (WPARAM)(ctx), MAKELONG(CMD_SET_STATS, 0));
#endif // SHOW_PORT_ACTIVITY
            }
         }
      }
   lpPort->SaveLog(true);
	}
lpTcpcom32Dlg->SendMessage(WM_THREAD_NOTIFY, (WPARAM)(ctx), MAKELONG(CMD_CLOSE_PORT,0));
lpTcpcom32Dlg->SendMessage(WM_THREAD_NOTIFY, (WPARAM)0, MAKELONG(CMD_TRAYICON,0));

// Close asynch port
lpPort->CloseConnection();

for (int ii = 0; ii < MX_TCP_CONNECTION; ii++)
   {
   if (TcpConnection[ii] != NULL)
      {
      TcpConnection[ii]->Close();
      TcpConnection[ii] = NULL;
      }
   }
// Clenup context
ctx->Cleanup();

delete lpTcpcom32Dlg->m_ctx[ComPort];
lpTcpcom32Dlg->m_ctx[ComPort] = NULL;

return 0;

#undef SHOW_PORT_ACTIVITY
#undef BUFLEN
#undef SPARE_BUF
#undef SPARE_LPPARAM
#undef SPARE_EXTRA
#undef MEMSZ_URL
#undef MEMSZ_IP
#undef MEMSZ_PORT
#undef MEMSZ_USERNAME
#undef MEMSZ_PASSWORD
#undef MEMSZ_WORKGROUP
#undef MEMSZ_WORKSTATION
#undef MEMSZ_AUTH
#undef MEMSZ_AUTHSCHEME
#undef MEMSZ_CONNECTION
}

