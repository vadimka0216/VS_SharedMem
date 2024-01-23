//#include <iostream>
#include <windows.h>
#include <fstream>
//#include "audiere.h"
#include <process.h>
//#include <string.h>
//#include <time.h>
//=== ����������� ��� Lua ��������� ============================================================================//
#define LUA_LIB
#define LUA_BUILD_AS_DLL
 
//=== ������������ ����� LUA ===================================================================================//
extern "C" {
#include "lauxlib.h"
#include "lua.h"
}

//int *LastPFileMap=0;//��������� ����������� ��������� ���� ������
//using namespace std;

bool FileIsExist(std::string filePath)
{
	bool isExist=false;
	std::ifstream fin(filePath.c_str());
	if (fin.is_open()) isExist=true;
	fin.close();
	
	return isExist;	
};


//=== ����������� ����� ����� ��� DLL ==========================================================================//
BOOL APIENTRY DllMain(HANDLE hModule, DWORD  fdwReason, LPVOID lpReserved)
{
   //������� ������� ������������� ���� �������� ��������� fdwReason, ������������� ������� DllMain ��� ��� �������������   
   switch (fdwReason)
   {
      case DLL_PROCESS_ATTACH: // ����������� DLL
      	{
		  	//RakBotProc=SetWindowLongA(FindWindowA(NULL,"RakBot 0.8.2.1"), -4, LONG(WindowProc)); 
	        break;
    	}
      case DLL_PROCESS_DETACH: // ���������� DLL
         break;
      case DLL_THREAD_ATTACH:  // �������� ������ ������
	    break;
      case DLL_THREAD_DETACH:  // ���������� ������
         break;
   }	
   return TRUE;
}
 
//=== ���������� �������, ���������� �� LUA ====================================================================//

static int GetPID(lua_State *L)
{
	lua_pushnumber(L, _getpid());
	return(1);
};
 
static int ConsoleCmd(lua_State *L)
{
	system((char*)luaL_checkstring(L, 1)); 	
 	lua_pushnumber(L, 1);
 	return(1);
 }
 
/*static int _CreateFileMapping(lua_State *L)//1 �������� - ��� ����������� ����������� ������
{
	HANDLE hFileMapping=hFileMapping = CreateFileMapping((HANDLE)0xFFFFFFFF,
  NULL, PAGE_READWRITE, 0, 100, (LPCSTR*)luaL_checkstring(L, 1));
	return 1;
}
 
 
static int _OpenFileMapping(lua_State *L)
{
	char *SharedName=(char*)luaL_checkstring(L, 1);
	if (SharedName==0) lua_pushnumber(L, -1);
	HANDLE hFileMapping=hFileMapping=OpenFileMapping(PAGE_READWRITE, false,
  SharedName);
  	
  	
  	lua_pushnumber(L, 1);
	return 1;
}*/
static int getValueShared(lua_State *L)//1 ��������
{
	char *SharedName=(char*)luaL_checkstring(L, 1);
	if (SharedName==0)
	{
		lua_pushnumber(L, -1); 
		return(1);
	}
	HANDLE hFileMapping=0; int *lpFileMap=0;
	hFileMapping=OpenFileMapping(/*PAGE_READWRITE*/FILE_MAP_ALL_ACCESS, false, SharedName);
	if (hFileMapping==0)
	{
		hFileMapping = CreateFileMapping((HANDLE)0xFFFFFFFF,
  			NULL, PAGE_READWRITE, 0, sizeof(int), SharedName);
  		if (hFileMapping==0)
  		{
  			lua_pushnumber(L, -1); 
			return(1);
		}
  		
  		lpFileMap = (int*)MapViewOfFile(hFileMapping, 
  			FILE_MAP_READ | FILE_MAP_WRITE, 0, 0,  sizeof(int));
  		(*lpFileMap)=0;//�.�. ������ ��� ��������
		//MessageBoxA(0, "CreateFileMap", "Info", 0);
	}
	else
	{
		lpFileMap = (int*)MapViewOfFile(hFileMapping, 
  			/*FILE_MAP_READ | FILE_MAP_WRITE*/FILE_MAP_ALL_ACCESS, 0, 0, sizeof(int));
	}
	
	//CloseHandle(hFileMapping);// ??
	//LastPFileMap=lpFileMap;

	lua_pushnumber(L, (int)*lpFileMap);
	UnmapViewOfFile(lpFileMap);//??
	return(1);
};

static int setValueShared(lua_State *L)//2 ���������
{
	char *SharedName=(char*)luaL_checkstring(L, 1);
	int value=luaL_checknumber(L, 2);
	
	if (SharedName==0)
	{
		lua_pushnumber(L, 0);
		return(1);
	}
	
	HANDLE hFileMapping=0; int *lpFileMap=0;
	hFileMapping=OpenFileMapping(FILE_MAP_ALL_ACCESS, false, SharedName);
	if (hFileMapping==0)
	{
		hFileMapping = CreateFileMapping((HANDLE)0xFFFFFFFF,
  			NULL, PAGE_READWRITE, 0, sizeof(int), SharedName);
  		if (hFileMapping==0)
  		{
  			lua_pushnumber(L, 0); 
			return(1);
		}
  		
  		
  		lpFileMap = (int*)MapViewOfFile(hFileMapping, 
  			FILE_MAP_READ | FILE_MAP_WRITE, 0, 0,  sizeof(int));
  		*lpFileMap=value;//�.�. ������ ��� ��������
	}
	else
	{
		lpFileMap = (int*)MapViewOfFile(hFileMapping, 
  			FILE_MAP_ALL_ACCESS, 0, 0, sizeof(int));
	}
	*lpFileMap=value;
	
	//CloseHandle(hFileMapping);// ??
	//LastPFileMap=lpFileMap;

	lua_pushnumber(L, 1);
	UnmapViewOfFile(lpFileMap);//??
	return(1);
};

static int deleteShared(lua_State *L)
{
	char *SharedName=(char*)luaL_checkstring(L, 1);
	if (SharedName==0)
	{
		lua_pushnumber(L, 0);
		return (1);
	}
	
	HANDLE hFileMapping=OpenFileMapping(PAGE_READWRITE, false, SharedName);
	if (hFileMapping!=0)
	{
		LPVOID lpFileMap = MapViewOfFile(hFileMapping, 
  			FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, sizeof(int));
		UnmapViewOfFile(lpFileMap);
		CloseHandle(hFileMapping);
	}
	
	lua_pushnumber(L, 1);
	return(1);
}

 
/*static int setValueLastShared(lua_State *L)
{
	if (LastPFileMap==0) 
	{
		lua_pushnumber(L, 0);
		return (1);
	}

	(*LastPFileMap)=luaL_checknumber(L, 1);
	
	lua_pushnumber(L, 1);
	return (1);
}

/*static int getValueLastShared(lua_State *L)
{
	if (LastPFileMap==0) 
	{
		lua_pushnumber(L, -1);
		return (1);
	}
	
	lua_pushnumber(L, (*LastPFileMap));
	return (1);
}*/
 
//=== ����������� ������������� � dll �������, ����� ��� ����� "������" ��� Lua ================================//
static struct luaL_reg ls_lib[] = {
//   { "getZ", fgetZ }, // �� ������� Lua ��� ������� ����� ����� �������� ���: QluaCSharpConnector.TestFunc(); ����� ����� ������� ����� ������ ��������
//   { "AudioRun", AudioRun },
//  { "AudioStop", AudioStop },
   { "GetPID", GetPID },
//   {"ConsoleCmd", ConsoleCmd},
   {"getValueShared", getValueShared},//����������� � 20�???
   {"setValueShared", setValueShared},//����������� � 20�???
   {"deleteShared", deleteShared},
  // {"setValueLastShared", setValueLastShared},
   //{"getValueLastShared", getValueLastShared},
   //{"CreateFileMapping", _CreateFileMapping},
  // {"OpenFileMapping", _OpenFileMapping},
   {NULL,NULL}//������ ��������,����� �� ����� ������������ ����������!(������� �����)
};
 
//=== ����������� �������� ����������, �������� � ������� Lua ==================================================//
 extern "C" LUALIB_API int luaopen_vch777(lua_State *L) {
    luaL_openlib(L, "vch777", ls_lib, 0);
    return 0;
}
