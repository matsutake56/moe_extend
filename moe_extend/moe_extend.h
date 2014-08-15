#include "GetGamerData.h"
#include <string>

using namespace std ;

extern "C" __declspec(dllexport) HGLOBAL __cdecl request(HGLOBAL h, long *len);
extern "C" __declspec(dllexport) BOOL __cdecl load(HGLOBAL h, long len);
extern "C" __declspec(dllexport) BOOL __cdecl unload();

string SaoriMain(const string);

extern "C" __declspec(dllexport) BOOL __cdecl debug();