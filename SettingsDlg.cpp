// SettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "tcpcom32.h"
#include "SettingsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//#define TCPCOM32_USE_REGISTRY
//#define TCPCOM32_REG_KEY "SOFTWARE\\mep\\TcpCom32\\Settings\\%s"

#define DEFAULT_LOCAL_IP "localhost"
#define DEFAULT_CLIENT_MODE "FALSE"
#define DEFAULT_PROXY_ENABLED "FALSE"
#define DEFAULT_PROXY_IP ""
#define DEFAULT_PROXY_PORT ""
#define DEFAULT_AUTH_ENABLED "FALSE"
#define DEFAULT_AUTHSCHEME "Basic"
#define DEFAULT_USERNAME ""
#define DEFAULT_PASSWORD ""
#define DEFAULT_WORKGROUP ""
#define DEFAULT_WORKSTATION ""
#define DEFAULT_LOG_ENABLED "FALSE"
#define DEFAULT_LOG_DIR "C:\\WINDOWS\\Temp"

#define DEFAULT_ISENABLED "FALSE"
#define DEFAULT_BAUD_RATE "57600"
#define DEFAULT_BYTE_SIZE "8"
#define DEFAULT_PARITY "NOPARITY"
#define DEFAULT_STOP_BITS "1"
#define DEFAULT_F_BINARY "FALSE"
#define DEFAULT_F_PARITY "FALSE"
#define DEFAULT_F_OUTX_CTS_FLOW "FALSE"
#define DEFAULT_F_OUTX_DSR_FLOW "FALSE"
#define DEFAULT_F_DTR_CONTROL "DTR_CONTROL_DISABLE"
#define DEFAULT_F_DSR_SENSITIVITY "FALSE"
#define DEFAULT_F_RTS_CONTROL "RTS_CONTROL_DISABLE"
#define DEFAULT_F_TX_CONTINUE_ON_XOFF "FALSE"
#define DEFAULT_F_OUTX "FALSE"
#define DEFAULT_F_INX "FALSE"
#define DEFAULT_F_ERROR_CHAR "FALSE"
#define DEFAULT_F_NULL "FALSE"
#define DEFAULT_F_ABORT_ON_ERROR "FALSE"
#define DEFAULT_F_XON_LIM "0"
#define DEFAULT_F_XOFF_LIM "0"
#define DEFAULT_F_XON_CHAR "0"
#define DEFAULT_F_XOFF_CHAR "0"
#define DEFAULT_ERROR_CHAR "0"
#define DEFAULT_EOF_CHAR "0"
#define DEFAULT_EVT_CHAR "0"
#define DEFAULT_LOCAL_PORT "232"

/////////////////////////////////////////////////////////////////////////////
// CSettingsDlg dialog


CSettingsDlg::CSettingsDlg()
{
	//{{AFX_DATA_INIT(CSettingsDlg)
	//}}AFX_DATA_INIT
}

BOOL CSettingsDlg::ReadConfigString(LPSTR lpszSection, LPSTR lpszName, LPSTR lpszBuffer, DWORD dwBufferSize)
{
DWORD dwLength = GetPrivateProfileString (
	TEXT( lpszSection ), 
	TEXT( lpszName ), 
	TEXT( "\r\n" ), 
	lpszBuffer, dwBufferSize,
	TEXT( m_iniFilePath )
);

return strcmp(lpszBuffer, "\r\n") ? TRUE : FALSE;
}

BOOL CSettingsDlg::ReadConfigDefaultString(LPSTR lpszSection, LPSTR lpszName, LPSTR DefaultValue, LPSTR lpszBuffer, DWORD dwBufferSize)
{
if (ReadConfigString(lpszSection, lpszName, lpszBuffer, dwBufferSize))
   return TRUE;
else
   {
   strncpy(lpszBuffer, DefaultValue, dwBufferSize);
   return FALSE;
   }
}

void CSettingsDlg::SetIniFilePath(CString iniFilePath)
{
	m_iniFilePath = iniFilePath;
}

LPSTR CSettingsDlg::GetLocalIpAddress(LPSTR lpszBuffer, int nBufferSize)
{
	if (!ReadConfigString("Server", "IpAddress", lpszBuffer, nBufferSize)) {
		strcpy(lpszBuffer, DEFAULT_LOCAL_IP);
	}
	return lpszBuffer;
}

BOOL CSettingsDlg::IsLogEnabled()
{
	char buf[8];

	if (!ReadConfigString("Logging", "LogEnabled", buf, 8)) {
		strcpy(buf, DEFAULT_LOG_ENABLED);
	}
	return !_stricmp(buf, "TRUE");
}

LPSTR CSettingsDlg::GetLogDir(LPSTR lpszBuffer, int nBufferSize)
{
	if (!ReadConfigString("Logging", "LogDir", lpszBuffer, nBufferSize)) {
		strcpy(lpszBuffer, DEFAULT_LOG_DIR);
	}
	return lpszBuffer;
}

BOOL CSettingsDlg::IsComConfigured(LPSTR lpszPortName)
{
return this->IsIniSection(lpszPortName);
}


BOOL CSettingsDlg::IsIniSection(LPSTR lpszSectionName)
{
TCHAR Tmp[4];
bool retval = (GetPrivateProfileSection(TEXT(lpszSectionName), TEXT(Tmp), _countof(Tmp), m_iniFilePath) == 0 ? false : true);
return retval;
}



BOOL CSettingsDlg::GetPortConfig(LPSTR lpszPortName, DCB *lpDcb, int *TcpPort)
{
char buf[32];
BOOL RetVal;

DCB dcb;
memset(&dcb, 0, sizeof(DCB));
dcb.DCBlength = sizeof(DCB);

ReadConfigDefaultString(lpszPortName, "IsEnabled", DEFAULT_ISENABLED, buf, 32);
RetVal = !_stricmp(buf, "TRUE");

ReadConfigDefaultString(lpszPortName, "BaudRate", DEFAULT_BAUD_RATE, buf, 32);
dcb.BaudRate = atoi(buf);

ReadConfigDefaultString(lpszPortName, "ByteSize", DEFAULT_BYTE_SIZE, buf, 32);
dcb.ByteSize = atoi(buf);

ReadConfigDefaultString(lpszPortName, "Parity", DEFAULT_PARITY, buf, 32);
if (!_stricmp(buf, "NOPARITY")) {
	dcb.Parity = NOPARITY;
} else if (!_stricmp(buf, "MARKPARITY")) {
	dcb.Parity = MARKPARITY;
} else if (!_stricmp(buf, "EVENPARITY")) {
	dcb.Parity = EVENPARITY;
} else if (!_stricmp(buf, "ODDPARITY")) {
	dcb.Parity = ODDPARITY;
} else if (!_stricmp(buf, "SPACEPARITY")) {
	dcb.Parity = SPACEPARITY;
} else {
	return FALSE;
}

ReadConfigDefaultString(lpszPortName, "StopBits", DEFAULT_STOP_BITS, buf, 32);
if (!_stricmp(buf, "1")) {
	dcb.StopBits = ONESTOPBIT;
} else if (!_stricmp(buf, "1.5")) {
	dcb.StopBits = ONE5STOPBITS;
} else if (!_stricmp(buf, "2")) {
	dcb.StopBits = TWOSTOPBITS;
} else {
	return FALSE;
}

ReadConfigDefaultString(lpszPortName, "fBinary", DEFAULT_F_BINARY, buf, 32);
dcb.fBinary = !_stricmp(buf, "TRUE");

ReadConfigDefaultString(lpszPortName, "fParity", DEFAULT_F_PARITY, buf, 32);
dcb.fParity = !_stricmp(buf, "TRUE");

ReadConfigDefaultString(lpszPortName, "fOutxCtsFlow", DEFAULT_F_OUTX_CTS_FLOW, buf, 32);
dcb.fOutxCtsFlow = !_stricmp(buf, "TRUE");

ReadConfigDefaultString(lpszPortName, "fOutxDsrFlow", DEFAULT_F_OUTX_DSR_FLOW, buf, 32);
dcb.fOutxDsrFlow = !_stricmp(buf, "TRUE");

ReadConfigDefaultString(lpszPortName, "fDtrControl", DEFAULT_F_DTR_CONTROL, buf, 32);
if (!_stricmp(buf, "DTR_CONTROL_DISABLE")) {
	dcb.fDtrControl = DTR_CONTROL_DISABLE;
} else if (!_stricmp(buf, "DTR_CONTROL_ENABLE")) {
	dcb.fDtrControl = DTR_CONTROL_ENABLE;
} else if (!_stricmp(buf, "DTR_CONTROL_HANDSHAKE")) {
	dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;
} else {
	return FALSE;
}

ReadConfigDefaultString(lpszPortName, "fDsrSensitivity", DEFAULT_F_DSR_SENSITIVITY, buf, 32);
dcb.fDsrSensitivity = !_stricmp(buf, "TRUE");

ReadConfigDefaultString(lpszPortName, "fRtsControl", DEFAULT_F_RTS_CONTROL, buf, 32);
if (!_stricmp(buf, "RTS_CONTROL_DISABLE")) {
	dcb.fRtsControl = RTS_CONTROL_DISABLE;
} else if (!_stricmp(buf, "RTS_CONTROL_ENABLE")) {
	dcb.fRtsControl = RTS_CONTROL_ENABLE;
} else if (!_stricmp(buf, "RTS_CONTROL_HANDSHAKE")) {
	dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
} else if (!_stricmp(buf, "RTS_CONTROL_TOGGLE")) {
	dcb.fRtsControl = RTS_CONTROL_TOGGLE;
} else {
	return FALSE;
}

ReadConfigDefaultString(lpszPortName, "fTXContinueOnXoff", DEFAULT_F_TX_CONTINUE_ON_XOFF, buf, 32);
dcb.fTXContinueOnXoff = !_stricmp(buf, "TRUE");

ReadConfigDefaultString(lpszPortName, "fOutX", DEFAULT_F_OUTX, buf, 32);
dcb.fOutX = !_stricmp(buf, "TRUE");
      
ReadConfigDefaultString(lpszPortName, "fInX", DEFAULT_F_INX, buf, 32);
dcb.fInX = !_stricmp(buf, "TRUE");
      
ReadConfigDefaultString(lpszPortName, "fErrorChar", DEFAULT_F_ERROR_CHAR, buf, 32);
dcb.fErrorChar = !_stricmp(buf, "TRUE");
      
ReadConfigDefaultString(lpszPortName, "fNull", DEFAULT_F_NULL, buf, 32);
dcb.fNull = !_stricmp(buf, "TRUE");
      
ReadConfigDefaultString(lpszPortName, "fAbortOnError", DEFAULT_F_ABORT_ON_ERROR, buf, 32);
dcb.fAbortOnError = !_stricmp(buf, "TRUE");
      
ReadConfigDefaultString(lpszPortName, "XonLim", DEFAULT_F_XON_LIM, buf, 32);
dcb.XonLim = atoi(buf);
      
ReadConfigDefaultString(lpszPortName, "XoffLim", DEFAULT_F_XOFF_LIM, buf, 32);
dcb.XoffLim = atoi(buf);
      
ReadConfigDefaultString(lpszPortName, "XonChar", DEFAULT_F_XON_CHAR, buf, 32);
dcb.XonChar = atoi(buf);
      
ReadConfigDefaultString(lpszPortName, "XoffChar", DEFAULT_F_XOFF_CHAR, buf, 32);
dcb.XoffChar = atoi(buf);
      
ReadConfigDefaultString(lpszPortName, "ErrorChar", DEFAULT_ERROR_CHAR, buf, 32);
dcb.ErrorChar = atoi(buf);
      
ReadConfigDefaultString(lpszPortName, "EofChar", DEFAULT_EOF_CHAR, buf, 32);
dcb.EofChar = atoi(buf);
      
ReadConfigDefaultString(lpszPortName, "EvtChar", DEFAULT_EVT_CHAR, buf, 32);
dcb.EvtChar = atoi(buf);
      
ReadConfigDefaultString(lpszPortName, "TcpPort", DEFAULT_LOCAL_PORT, buf, 32);
*TcpPort = atoi(buf);

*lpDcb = dcb;

return RetVal;
}
