// moe_extend_test.cpp : �R���\�[�� �A�v���P�[�V�����̃G���g�� �|�C���g���`���܂��B
//

#include "stdafx.h"


int _tmain(int argc, _TCHAR* argv[])
{
	HMODULE hLib;
	TCHAR sTest[64];
	ZeroMemory(sTest,sizeof(sTest));

	HGLOBAL (__stdcall *func)();

	hLib =LoadLibrary(_T("moe_extend.dll"));
	if (hLib  != NULL ) 
	{
		func = (HGLOBAL (__stdcall *)())GetProcAddress( hLib, "debug" );
		if( func != NULL )
		{
			func();
		}
		FreeLibrary(hLib);
	}

	return 0;
}