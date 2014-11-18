// Log.h: interface for the KLog class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOG_H__69E63737_8DD3_43E5_8CC3_BC6892E93C10__INCLUDED_)
#define AFX_LOG_H__69E63737_8DD3_43E5_8CC3_BC6892E93C10__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#define LOG_ENABLED


class KLog  
{
	static KLog *ms_pInstance;

	static char ms_szFile[128];

public:
	KLog();
	virtual ~KLog();

	void Log(char *szStr);

	static KLog *GetInstance(char *szFile = NULL);
	static void DestroyInstance();

};

extern char g_szLog[4096];

#ifdef LOG_ENABLED

#define LOG(s)				{ KLog::GetInstance()->Log(s);	}
#define LOG1(s, a1)			{ sprintf(g_szLog, s, a1); KLog::GetInstance()->Log(g_szLog);	}
#define LOG2(s, a1, a2)		{ sprintf(g_szLog, s, a1, a2); KLog::GetInstance()->Log(g_szLog);	}
#define LOG3(s, a1, a2, a3)	{ sprintf(g_szLog, s, a1, a2, a3); KLog::GetInstance()->Log(g_szLog);	}
#define LOG4(s, a1, a2, a3, a4)	{ sprintf(g_szLog, s, a1, a2, a3, a4); KLog::GetInstance()->Log(g_szLog);	}
#define LOG5(s, a1, a2, a3, a4, a5)	{ sprintf(g_szLog, s, a1, a2, a3, a4, a5); KLog::GetInstance()->Log(g_szLog);	}
#define LOG6(s, a1, a2, a3, a4, a5, a6)	{ sprintf(g_szLog, s, a1, a2, a3, a4, a5, a6); KLog::GetInstance()->Log(g_szLog);	}
#define LOG7(s, a1, a2, a3, a4, a5, a6, a7)	{ sprintf(g_szLog, s, a1, a2, a3, a4, a5, a6, a7); KLog::GetInstance()->Log(g_szLog);	}
#define LOG8(s, a1, a2, a3, a4, a5, a6, a7, a8)	{ sprintf(g_szLog, s, a1, a2, a3, a4, a5, a6, a7, a8); KLog::GetInstance()->Log(g_szLog);	}
#define LOG9(s, a1, a2, a3, a4, a5, a6, a7, a8, a9)	{ sprintf(g_szLog, s, a1, a2, a3, a4, a5, a6, a7, a8, a9); KLog::GetInstance()->Log(g_szLog);	}

#else

#define LOG(s)
#define LOG1(s, a1)
#define LOG2(s, a1, a2)
#define LOG3(s, a1, a2, a3)
#define LOG4(s, a1, a2, a3, a4)
#define LOG5(s, a1, a2, a3, a4, a5)
#define LOG6(s, a1, a2, a3, a4, a5, a6)
#define LOG7(s, a1, a2, a3, a4, a5, a6, a7)
#define LOG8(s, a1, a2, a3, a4, a5, a6, a7, a8)
#define LOG9(s, a1, a2, a3, a4, a5, a6, a7, a8, a9)

#endif

#endif // !defined(AFX_LOG_H__69E63737_8DD3_43E5_8CC3_BC6892E93C10__INCLUDED_)
