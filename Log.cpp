// Log.cpp: implementation of the KLog class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Log.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define CRLF "\n"


char g_szLog[4096];


KLog *KLog::ms_pInstance = NULL;
char KLog::ms_szFile[128];


KLog::KLog()
{

}


KLog::~KLog()
{

}


void KLog::Log(char *szStr)
{
	if (szStr == NULL)
		return;

	FILE *f = fopen(ms_szFile, "a");
	if (NULL != f)
	{
		fwrite(szStr, strlen(szStr), 1, f);
		fwrite(CRLF, strlen(CRLF), 1, f);

//		printf("%s\n", szStr);

		fclose(f);
	}
}


KLog *KLog::GetInstance(char *szFile)
{
	if (NULL != szFile)
	{
		strcpy(ms_szFile, szFile);

		// Vide le fichier
		FILE *f = fopen(ms_szFile, "w");
		if (f != NULL)
			fclose(f);
	}

	if (NULL == ms_pInstance)
	{
		ms_pInstance = new KLog();
	}

	return ms_pInstance;
}


void KLog::DestroyInstance()
{
	if (NULL != ms_pInstance)
	{
		delete(ms_pInstance);
		ms_pInstance = NULL;
	}
}