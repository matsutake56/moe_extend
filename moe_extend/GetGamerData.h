#include "winsock2.h"
#include <stdio.h>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <fcntl.h>
#include <io.h>

using namespace std ;

typedef vector<std::string> vstring;

//ゲーマーデータ取得クラス定義
class GetGamerData
{

public:
	GetGamerData();
	~GetGamerData();

	string	GetCardSource   (const string&);					//ゲーマーカードHTMLソース取得
	
	string	GetGamerScore   (const string&);					//ゲーマースコア取得
	vstring	GetGameHistory  (const string&);					//ゲーム履歴取得

private:
	string	GamerCardServer;									//カード取得鯖
	string	GetHTML         (const string& , const string&);    //HTMLソース取得
	string	UrlEncode       (const string&);				    //URLエンコード
	
	string	ConvUtf8toSJis	(const string&);

};