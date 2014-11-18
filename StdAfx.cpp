// stdafx.cpp : source file that includes just the standard includes
//	Fraisage.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

void StringTrim(char *sz, int len)
{
	int iIn, iOut;
	
	iIn = 0;
	iOut = 0;
	while (iIn < len)
	{
		if (sz[iIn] > 0x20)
		{
			sz[iOut] = sz[iIn];
			iOut++;
		}
		
		iIn++;
	}
	
	while (iOut < len)
	{
		sz[iOut] = 0;
		iOut++;
	}
}