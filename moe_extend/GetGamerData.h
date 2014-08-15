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

//�Q�[�}�[�f�[�^�擾�N���X��`
class GetGamerData
{

public:
	GetGamerData();
	~GetGamerData();

	string	GetCardSource   (const string&);					//�Q�[�}�[�J�[�hHTML�\�[�X�擾
	
	string	GetGamerScore   (const string&);					//�Q�[�}�[�X�R�A�擾
	vstring	GetGameHistory  (const string&);					//�Q�[�������擾

private:
	string	GamerCardServer;									//�J�[�h�擾�I
	string	GetHTML         (const string& , const string&);    //HTML�\�[�X�擾
	string	UrlEncode       (const string&);				    //URL�G���R�[�h
	
	string	ConvUtf8toSJis	(const string&);

};