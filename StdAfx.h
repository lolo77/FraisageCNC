// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__C73C3552_4A97_4067_91DF_8ED79EE5FBF7__INCLUDED_)
#define AFX_STDAFX_H__C73C3552_4A97_4067_91DF_8ED79EE5FBF7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <conio.h>
#include <windows.h>
#include <winioctl.h>
#include <winerror.h>


#define PI	3.1415926535897932384626433832795

#define DELTA_MIN 0.00001

#define MIN(a,b)	((a < b) ? a : b)
#define MAX(a,b)	((a > b) ? a : b)

void StringTrim(char *sz, int len);



typedef LARGE_INTEGER ticks;
#define RDTSC __asm __emit 0fh __asm __emit 031h /* hack for VC++ 5.0 */

static __inline ticks getticks(void)
{
     ticks retval;

     __asm
	 {
	  RDTSC
	  mov retval.HighPart, edx
	  mov retval.LowPart, eax
     }

     return retval;
}


// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__C73C3552_4A97_4067_91DF_8ED79EE5FBF7__INCLUDED_)
