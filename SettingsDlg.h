// SettingsDlg.h : header file
//

#if !defined(AFX_SETTINGSDLG_H__C79CE2DE_F30B_4A13_8A7F_CE281C345DA3__INCLUDED_)
#define AFX_SETTINGSDLG_H__C79CE2DE_F30B_4A13_8A7F_CE281C345DA3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CSettingsDlg dialog

class CSettingsDlg
{
// Construction
public:
	CSettingsDlg();   // standard constructor
	
	void SetIniFilePath(CString iniFilePath);
   	
	LPSTR GetLocalIpAddress(LPSTR lpszBuffer, int nBufferSize);
	
	BOOL IsLogEnabled();

	LPSTR GetLogDir(LPSTR lpszBuffer, int nBufferSize);

	BOOL GetPortConfig(LPSTR lpszPortName, DCB *lpDcb, int* TcpPort);
   BOOL IsComConfigured(LPSTR lpszPortName);
   BOOL IsIniSection(LPSTR lpszSectionName);
   
private:
	CString m_iniFilePath;
	BOOL ReadConfigString(LPSTR lpszSection, LPSTR lpszName, LPSTR lpszBuffer, DWORD dwBufferSize);
   BOOL CSettingsDlg::ReadConfigDefaultString(LPSTR lpszSection, LPSTR lpszName, LPSTR DefaultValue, LPSTR lpszBuffer, DWORD dwBufferSize);

// Implementation
protected:
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETTINGSDLG_H__C79CE2DE_F30B_4A13_8A7F_CE281C345DA3__INCLUDED_)
