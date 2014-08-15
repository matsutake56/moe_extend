////////////////////////////////////////////////////////////////////////
//ゲーマースコア取得クラス
////////////////////////////////////////////////////////////////////////
#include "GetGamerData.h"

//コンストラクタ
GetGamerData::GetGamerData()
{
	this->GamerCardServer = "gamercard.xbox.com";
}

//デストラクタ
GetGamerData::~GetGamerData()
{
}

//ゲーマーカードHTMLソース取得
string GetGamerData::GetCardSource(const string &strGamerTag)
{
	string strRet = "";
	string strCardFile;

	WORD wVersionRequested = WINSOCK_VERSION;
	WSADATA wsaData;
	int nRet;

	if(strGamerTag.size() == 0 || strGamerTag.size()  >= 16)
	{
		return strRet;
	}

	strCardFile = "";
	strCardFile += this->UrlEncode(strGamerTag);
	strCardFile += ".card";

	long lRet = 0;

	// WinSock.dllを初期化する
	nRet = WSAStartup(wVersionRequested, &wsaData);
	if (nRet)
	{
		//fprintf(stderr,"\nWSAStartup(): %d\n", nRet);
		WSACleanup();
		return strRet;
	}

	// WinSockのバージョンを調べる

	if (wsaData.wVersion != wVersionRequested)
	{
		//fprintf(stderr,"\nWinSock version not supported\n");
		WSACleanup();
		return strRet;
	}

	// .gifファイルおよび.jpgファイルのリダイレクションを機能させるために、stdoutをバイナリモードに設定する
	_setmode(_fileno(stdout), _O_BINARY);

	// GetHTTP()を呼び出してすべての処理を実行する
	strRet = GetHTML(this->GamerCardServer,strCardFile);

	// WinSockを解放する
	WSACleanup();
	
	return strRet;
}

string GetGamerData::GetGamerScore(const string &strSrc)
{
	std::string::size_type szRet = string::npos;
	string strSearchBegin = "<img alt=\"Gamerscore\" src=\"/xweb/lib/images/G_Icon_External.gif\" /></span><span class=\"XbcFRAR\">";
	string strSearchEnd   = "</span>";
	string strBuf;
	string strRet = "";

	szRet = strSrc.rfind(strSearchBegin);
	if (szRet != string::npos)
	{
		strBuf = strSrc.substr(szRet+strSearchBegin.size());
		szRet = strBuf.find(strSearchEnd);
		if (szRet != string::npos)
		{
			strRet = strBuf.substr(0,szRet);
		}
	}
	return strRet;
}


vstring GetGamerData::GetGameHistory(const string &strSrc)
{
	vstring vstrRet; 
	std::string::size_type szRet = string::npos;
	string strSearchBegin = "<img height=\"32\" width=\"32\" title=\"";
	string strSearchEnd   = "\"";
	string strBuf = strSrc;
	string strTitle;

	while (1)
	{
		szRet = strBuf.find(strSearchBegin);
		if (szRet == string::npos) break;
		
		strBuf = strBuf.substr(szRet+strSearchBegin.size());

		szRet = strBuf.find(strSearchEnd);
		if (szRet == string::npos) break;
		
		//strTitle = ConvUtf8toSJis(strBuf.substr(0,szRet));
		strTitle = strBuf.substr(0,szRet);
		if (strTitle.length() > 0) vstrRet.push_back(strTitle);
		
		strBuf = strBuf.substr(szRet+strSearchEnd.size());
	}

	return vstrRet;

}

string GetGamerData::GetHTML(const string& strServerName, const string& strFileName)
{
	string strGET  = "";
	string strHTML = "";
	
	// ホストの検索
	LPHOSTENT lpHostEntry;
	lpHostEntry = gethostbyname(this->GamerCardServer.c_str());
	if (lpHostEntry == NULL)
	{
		//PRINTERROR("gethostbyname()");
		return strHTML;
	}

	// サーバーアドレス構造体を埋める
	SOCKADDR_IN sa;
	sa.sin_family = AF_INET;
	sa.sin_addr = *((LPIN_ADDR)*lpHostEntry->h_addr_list);
	sa.sin_port = htons(80);	//HTTPポート

	// TCP/IPストリームソケットを作成する
	SOCKET	Socket;
	Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (Socket == INVALID_SOCKET)
	{
		//PRINTERROR("socket()"); 
		return strHTML;
	}

	// このソケットで使用するイベントオブジェクトを作成する
	WSAEVENT hEvent;
	hEvent = WSACreateEvent();
	if (hEvent == WSA_INVALID_EVENT)
	{
		//PRINTERROR("WSACreateEvent()");
		closesocket(Socket);
		return strHTML;
	}

	// ソケットを非ブロッキングにし、ネットワークイベントを関連付ける
	int nRet;
	nRet = WSAEventSelect(Socket,hEvent,FD_READ|FD_CONNECT|FD_CLOSE);
	if (nRet == SOCKET_ERROR)
	{
		//PRINTERROR("EventSelect()");
		closesocket(Socket);
		WSACloseEvent(hEvent);
		return strHTML;
	}

	// 接続を要求する
	nRet = connect(Socket,(LPSOCKADDR)&sa,sizeof(SOCKADDR_IN));
	if (nRet == SOCKET_ERROR)
	{
		nRet = WSAGetLastError();
		if (nRet == WSAEWOULDBLOCK)
		{
			//fprintf(stderr,"\nConnect would block");
		}
		else
		{
			//PRINTERROR("connect()");
			closesocket(Socket);
			WSACloseEvent(hEvent);
			return strHTML;
		}
	}
	
	// 非同期ネットワークイベントを処理する
	char szBuffer[4096];
	char szBufferZ[8192];
	WSANETWORKEVENTS events;
	while(1)
	{
		// 何か発生するのを待機する
		DWORD dwRet;
		dwRet = WSAWaitForMultipleEvents(1, &hEvent, FALSE,10000,FALSE);
		if (dwRet == WSA_WAIT_TIMEOUT)
		{
			break;
		}

		// どのイベントが発生したかを判別する
		nRet = WSAEnumNetworkEvents(Socket, hEvent, &events);
		if (nRet == SOCKET_ERROR)
		{
			//PRINTERROR("WSAEnumNetworkEvents()");
			break;
		}

		// イベントを処理する

		// 接続イベントかどうか？
		if (events.lNetworkEvents & FD_CONNECT)
		{
			// http要求を送信する
			strGET = "";
			strGET += "GET ";
			strGET += strFileName;
			strGET += " HTTP/1.1\r\nHost: ";
			strGET += strServerName;
			strGET += "\r\nKeep-Alive: timeout=5, max=20\r\nConnection: close\r\n\r\n";
			nRet = send(Socket, strGET.c_str(), strGET.size(), 0);
			if (nRet == SOCKET_ERROR)
			{
				//PRINTERROR("send()");
				break;
			}
		}

		// 読み取りイベントかどうか？
		if (events.lNetworkEvents & FD_READ)
		{
			// データを読み取り、stdoutに書き込む
			nRet = recv(Socket, szBuffer, sizeof(szBuffer), 0);
			if (nRet == SOCKET_ERROR)
			{
				//PRINTERROR("recv()");
				break;
			}
			ZeroMemory(szBufferZ,sizeof(szBufferZ));
			memcpy(szBufferZ,szBuffer,nRet);
			strHTML.append(szBufferZ);
		}

		// 終了イベントかどうか？
		if (events.lNetworkEvents & FD_CLOSE)
		{
			break;
		}

		// 書き込みイベントかどうか？
		if (events.lNetworkEvents & FD_WRITE)
		{
		}

	}
	closesocket(Socket);	
	WSACloseEvent(hEvent);
	return strHTML;
}

string GetGamerData::UrlEncode(const string &str) {
    std::ostringstream os;

    for (std::string::size_type i = 0; i < str.size(); i++) {
        char c = str[i];
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') ||
            c == '-' || c == '_' || c == '.' || c == '~') {
            os << c;
        } else {
            char s[4];
            sprintf_s(s, sizeof(s), "%%%02x", c & 0xff);
            os << s;
        }
    }

    return os.str();
}

string GetGamerData::ConvUtf8toSJis(const string &strSrc) 
{
	string strRet="";
	int		intLen = 0;
	char    strSJis[2048];
	wchar_t wstrBuf[1024];
	ZeroMemory( strSJis, sizeof(strSJis) );
	ZeroMemory( wstrBuf, sizeof(wstrBuf) );
	intLen = MultiByteToWideChar( CP_UTF8, 0, strSrc.c_str(), -1, wstrBuf, NULL);
	if (intLen * sizeof(wchar_t) < sizeof(wstrBuf))
	{
		if (MultiByteToWideChar( CP_UTF8, 0, strSrc.c_str(), -1, wstrBuf, sizeof(wstrBuf)))
		{
			intLen = WideCharToMultiByte( CP_ACP , 0, wstrBuf, -1, strSJis, NULL, NULL, NULL);
			if (intLen < sizeof(strSJis))
			{
				if (WideCharToMultiByte( CP_ACP , 0, wstrBuf, -1, strSJis, sizeof(strSJis), NULL, NULL)) strRet = strSJis;
			}
		}
	}
	return strRet;
}
