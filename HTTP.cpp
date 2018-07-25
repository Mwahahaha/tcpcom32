// HTTP.cpp: HTTP common procs
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ClientSocket.h"
#include "HTTP.h"


//////////////////////////////////////////////////////////////////////
// ReadHttpHeader
//
DWORD ReadHttpHeader(CClientSocket* lpSocket, LPVOID lpBuffer, DWORD dwBufferSize, LPDWORD lpHeaderLength, LPDWORD lpExtraLength, DWORD dwTimeoutMillis)
{
	char *buf = (char *)lpBuffer;
	int maxlen = dwBufferSize;
	int length = 0;
	DWORD eor = 0x00000000;
	int count, parsed;
	DWORD dwTicks = GetTickCount();
	while (0x0D0A0D0A != eor && length < maxlen) {
		count = lpSocket->Recv(buf + length, maxlen - length, 0, 10000);
		parsed = 0;
		if (count > 0) {
			for (parsed = 0; 0x0D0A0D0A != eor && parsed < count; parsed ++, length ++) {
				eor = ((eor & 0x00FFFFFF) << 8) + buf[length];
			}
		} else if (0 == count) {
			return READHTTP_CONNCLOSED;
		} else if (GetTickCount() - dwTicks > dwTimeoutMillis) {
			return READHTTP_TIMEDOUT;
		}
	}
	// CR LF CR LF not found
	if (0x0D0A0D0A != eor) {
		ASSERT(length >= maxlen);
		return READHTTP_BUFFTOOSMALL;
	}

	*lpHeaderLength = length;
	if (lpExtraLength) {
		*lpExtraLength = (count - parsed);
	}
	return READHTTP_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
// FormatRequest
//
LPSTR FormatRequest(LPSTR lpszMethod, LPSTR lpszUrl, LPSTR lpszHost, LPSTR lpszHeaders, LPSTR lpszRequest, int nRequestSize)
{
	wsprintf(lpszRequest,
		"%s %s HTTP/1.1\r\n"
		"Host: %s\r\n"
		"User-Agent: TcpCom32/1.0\r\n"
		"Accept: */*\r\n"
		"Accept-Encoding: *\r\n"
		"Proxy-Connection: keep-alive\r\n"
		"%s"
		"\r\n",
		lpszMethod, 
		lpszUrl,
		lpszHost,
		lpszHeaders ? lpszHeaders : ""
	);

	return lpszRequest;
}

//////////////////////////////////////////////////////////////////////
// FormatResponse
//
LPSTR FormatResponse(int nErrCode, LPSTR lpszText, LPSTR lpszResponse, int nResponseSize)
{
	static char *days[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	static char *months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

	time_t rawtime;
	time ( &rawtime );
	struct tm *timeinfo = localtime ( &rawtime );	
	
	wsprintf(lpszResponse,
		"HTTP/1.1 %d %s\r\n"
		"Date: %s, %02d %s %d %02d:%02d:%02d GMT\r\n"
		"Server: TcpCom32/1.0\r\n"
		"Cache-Control: no-cache\r\n"
		"Pragma: no-cache\r\n"
//		"Connection: keep-alive\r\n"
		"\r\n",
		nErrCode, 
		lpszText,
		days[timeinfo->tm_wday],
		timeinfo->tm_mday,
		months[timeinfo->tm_mon],
		timeinfo->tm_year + 1900,
		timeinfo->tm_hour,
		timeinfo->tm_min,
		timeinfo->tm_sec
	);

	return lpszResponse;
}


//////////////////////////////////////////////////////////////////////
// ParseResponse
//
BOOL ParseResponse(LPDWORD lpErrCode, LPSTR lpmszAuth, LPDWORD lpContentLength, LPSTR lpszConnection, LPSTR lpszResponse, int nResponseSize)
{
	if (lpmszAuth) {
		*lpmszAuth = 0;
	}
	if (lpContentLength) {
		*lpContentLength = 0;
	}
	if (lpszConnection) {
		*lpszConnection = 0;
	}
	int nErrCode = 0;
	char *cur = lpszResponse;
	char *nxt, *hdr;
	while (nxt = strstr(cur, "\r\n")) {
		*nxt = 0; // close request line (kill '\r')
		if (hdr = strchr(cur, ' ')) {
			*hdr = 0; // close header name
			for (hdr ++; ' ' == *hdr; hdr ++); // skip white spaces
			if (!_stricmp(cur, "HTTP/1.0")) {
				if (cur = strchr(hdr, ' ')) {
					*cur = 0;
					nErrCode = atoi(hdr);
				}
			} else if (!_stricmp(cur, "HTTP/1.1")) {
				if (cur = strchr(hdr, ' ')) {
					*cur = 0;
					nErrCode = atoi(hdr);
				}
			} else if (!_stricmp(cur, "WWW-Authenticate:")) {
				if (lpmszAuth) {
					strcpy(lpmszAuth, hdr);
					lpmszAuth += 1 + strlen(hdr);
					*lpmszAuth = 0;
				}
			} else if (!_stricmp(cur, "Proxy-Authenticate:")) {
				if (lpmszAuth) {
					strcpy(lpmszAuth, hdr);
					lpmszAuth += 1 + strlen(hdr);
					*lpmszAuth = 0;
				}
			} else if (!_stricmp(cur, "Content-Length:")) {
				if (lpContentLength) {
					*lpContentLength = atoi(hdr);
				}
			} else if (!_stricmp(cur, "Connection:")) {
				if (lpszConnection) {
					strcpy(lpszConnection, hdr);
				}
			} else if (!_stricmp(cur, "Proxy-Connection:")) {
				if (lpszConnection) {
					strcpy(lpszConnection, hdr);
				}
			}
		}
		cur = nxt + 2;
	}

	if (nErrCode) {
		*lpErrCode = nErrCode;
		return TRUE;
	}
	return FALSE;
}
