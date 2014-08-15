#include "moe_extend.h"

using namespace std ;
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}


HGLOBAL request(HGLOBAL h, long *len)
{
	string strBufIn  = "";
	string strBufOut = "SAORI/1.0 200 OK\nResult: 1\r\nCharset: UTF-8\r\n\r\n";
	string strSearchBegin = "";
	string strSearchEnd = "\r\n";
	std::string::size_type szPos = string::npos;
	string strBuf;
	string strRet = "";
	HGLOBAL hRet    =0;
	CHAR sInRet[4096];

	try
	{
		ZeroMemory(sInRet,sizeof(sInRet));

		if (*len >= sizeof(sInRet) -1)
		{
			strBufOut = "SAORI/1.0 400 Bad Request\r\nResult: 0\r\nCharset: UTF-8\r\n\r\n";
		}
		else
		{
			memcpy(sInRet,h,*len);
		}
		//ŠJ•ú
		GlobalFree(h);

		//‰ðÍ
		strBufIn = sInRet;
		strSearchBegin = "Argument1: ";
		szPos = strBufIn.find(strSearchBegin);
		if (szPos  != string::npos)
		{
			strBuf = strBufIn.substr(szPos+strSearchBegin.size());
			szPos = strBuf.find(strSearchEnd);
			if (szPos  != string::npos)
			{
				strBuf = strBuf.substr(0,szPos);
			}
		}
		if (strBuf != "")
		{
			strRet = SaoriMain(strBuf);
			if (strRet != "")
			{
				strBufOut.append("SAORI/1.0 200 OK\nResult: 1\r\n").append(strRet).append("\r\nCharset: UTF-8\r\n\r\n");
			}
		}
	}
	catch(...)
	{
		strBufOut = "SAORI/1.0 500 Internal Server Error\r\nResult: 0\r\nCharset: UTF-8\r\n\r\n";
	}
	//Out
	hRet = GlobalAlloc(GMEM_FIXED,strBufOut.size());
	memcpy(hRet,strBufOut.c_str(),strBufOut.size());
	*len = strBufOut.size();
	return hRet;
}

BOOL  load(HGLOBAL h, long len)
{
	return TRUE;
}

BOOL  unload()
{
	return TRUE;
}

string SaoriMain(string strGamerTag)
{
	std::ostringstream osRet;
	string	strCardSource;
	vstring vstrGames;
	int		intIndex = 0;

	GetGamerData* objGGD;
	
	objGGD = new GetGamerData();
	
	strCardSource = objGGD->GetCardSource(strGamerTag);

	osRet << "Value" << intIndex << ": " << objGGD->GetGamerScore(strCardSource) << "\r\n";

	vstrGames = objGGD->GetGameHistory(strCardSource);
	for( vstring::const_iterator vstriGame = vstrGames.begin(); vstriGame != vstrGames.end(); vstriGame++)
	{
		osRet << "Value" << ++intIndex << ": " << *vstriGame << "\r\n";
	}

	delete objGGD;

	return osRet.str();
}

BOOL  debug()
{
	string strRet;
	strRet = SaoriMain("Matutake56");
	return TRUE;
}

