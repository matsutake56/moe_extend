////////////////////////////////////////////////////////////////////////
//�Q�[�}�[�X�R�A�擾�N���X
////////////////////////////////////////////////////////////////////////
#include "GetGamerData.h"

//�R���X�g���N�^
GetGamerData::GetGamerData()
{
	this->GamerCardServer = "gamercard.xbox.com";
}

//�f�X�g���N�^
GetGamerData::~GetGamerData()
{
}

//�Q�[�}�[�J�[�hHTML�\�[�X�擾
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

	// WinSock.dll������������
	nRet = WSAStartup(wVersionRequested, &wsaData);
	if (nRet)
	{
		//fprintf(stderr,"\nWSAStartup(): %d\n", nRet);
		WSACleanup();
		return strRet;
	}

	// WinSock�̃o�[�W�����𒲂ׂ�

	if (wsaData.wVersion != wVersionRequested)
	{
		//fprintf(stderr,"\nWinSock version not supported\n");
		WSACleanup();
		return strRet;
	}

	// .gif�t�@�C�������.jpg�t�@�C���̃��_�C���N�V�������@�\�����邽�߂ɁAstdout���o�C�i�����[�h�ɐݒ肷��
	_setmode(_fileno(stdout), _O_BINARY);

	// GetHTTP()���Ăяo���Ă��ׂĂ̏��������s����
	strRet = GetHTML(this->GamerCardServer,strCardFile);

	// WinSock���������
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
	
	// �z�X�g�̌���
	LPHOSTENT lpHostEntry;
	lpHostEntry = gethostbyname(this->GamerCardServer.c_str());
	if (lpHostEntry == NULL)
	{
		//PRINTERROR("gethostbyname()");
		return strHTML;
	}

	// �T�[�o�[�A�h���X�\���̂𖄂߂�
	SOCKADDR_IN sa;
	sa.sin_family = AF_INET;
	sa.sin_addr = *((LPIN_ADDR)*lpHostEntry->h_addr_list);
	sa.sin_port = htons(80);	//HTTP�|�[�g

	// TCP/IP�X�g���[���\�P�b�g���쐬����
	SOCKET	Socket;
	Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (Socket == INVALID_SOCKET)
	{
		//PRINTERROR("socket()"); 
		return strHTML;
	}

	// ���̃\�P�b�g�Ŏg�p����C�x���g�I�u�W�F�N�g���쐬����
	WSAEVENT hEvent;
	hEvent = WSACreateEvent();
	if (hEvent == WSA_INVALID_EVENT)
	{
		//PRINTERROR("WSACreateEvent()");
		closesocket(Socket);
		return strHTML;
	}

	// �\�P�b�g���u���b�L���O�ɂ��A�l�b�g���[�N�C�x���g���֘A�t����
	int nRet;
	nRet = WSAEventSelect(Socket,hEvent,FD_READ|FD_CONNECT|FD_CLOSE);
	if (nRet == SOCKET_ERROR)
	{
		//PRINTERROR("EventSelect()");
		closesocket(Socket);
		WSACloseEvent(hEvent);
		return strHTML;
	}

	// �ڑ���v������
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
	
	// �񓯊��l�b�g���[�N�C�x���g����������
	char szBuffer[4096];
	char szBufferZ[8192];
	WSANETWORKEVENTS events;
	while(1)
	{
		// ������������̂�ҋ@����
		DWORD dwRet;
		dwRet = WSAWaitForMultipleEvents(1, &hEvent, FALSE,10000,FALSE);
		if (dwRet == WSA_WAIT_TIMEOUT)
		{
			break;
		}

		// �ǂ̃C�x���g�������������𔻕ʂ���
		nRet = WSAEnumNetworkEvents(Socket, hEvent, &events);
		if (nRet == SOCKET_ERROR)
		{
			//PRINTERROR("WSAEnumNetworkEvents()");
			break;
		}

		// �C�x���g����������

		// �ڑ��C�x���g���ǂ����H
		if (events.lNetworkEvents & FD_CONNECT)
		{
			// http�v���𑗐M����
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

		// �ǂݎ��C�x���g���ǂ����H
		if (events.lNetworkEvents & FD_READ)
		{
			// �f�[�^��ǂݎ��Astdout�ɏ�������
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

		// �I���C�x���g���ǂ����H
		if (events.lNetworkEvents & FD_CLOSE)
		{
			break;
		}

		// �������݃C�x���g���ǂ����H
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
