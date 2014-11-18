// Path.h: interface for the KPath class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PATH_H__AC6A7DCD_8E85_485D_AB34_AE3DEE214962__INCLUDED_)
#define AFX_PATH_H__AC6A7DCD_8E85_485D_AB34_AE3DEE214962__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class KEntity;


class KPath
{
	KEntity *m_pEntityHead;
	KEntity *m_pEntityTail;
	double	m_dExtents;
	double	m_dLength;
	bool	m_bClosed;
	bool	m_bStartFromEndInit;
	bool	m_bStartFromEnd;

	static KPath *ms_tabPath;
	static int   ms_iCount;

public:

	KPath();
	virtual ~KPath();
	
	void	Reset();
	void	Init();

	void    ComputeLength();
	void    ComputeExtents();
	bool	IsClosed()				{ return m_bClosed;		}
	double  GetExtents()			{ return m_dExtents;	}
	double  GetLength()				{ return m_dLength;		}

	void	ToggleStartFromEnd();
	void	SetStartFromEnd(bool b = true)	{ if (m_bStartFromEnd != b) ToggleStartFromEnd();	}

	KEntity* GetEntity()			{ if (m_bStartFromEnd) return m_pEntityTail; else return m_pEntityHead;	}

	KEntity* GetEntityEnd()			{ if (m_bStartFromEnd) return m_pEntityHead; else return m_pEntityTail;	}

	static int  GetCount()			{ return ms_iCount;		}
	static KPath* GetPath(int i)	{ if ((i < 0) || (i >= ms_iCount)) return NULL; else return ms_tabPath + i;	}


	static void BuildPath();
	static void LinkRewind();
	static void DestroyPath();
	static void SortPath();

	static int	 Compare(KPath *p1, KPath *p2);
};


#endif // !defined(AFX_PATH_H__AC6A7DCD_8E85_485D_AB34_AE3DEE214962__INCLUDED_)
