#ifndef __ENTITY_H__
#define __ENTITY_H__

class KDXFReader;
class KExtents;

//-------------------------------------------------------------------------------

#define FLAG_DEADLOCK_DEB	0x00000001
#define FLAG_DEADLOCK_FIN	0x00000002
#define FLAG_STANDALONE		0x00000004
#define FLAG_TRAITE			0x00000008
#define FLAG_REVERSE		0x00000010
#define FLAG_PATH_HEAD		0x00000020
#define FLAG_START_FROM_END 0x00000040

#define ANGLE_UNINITIALIZED	1000

//-------------------------------------------------------------------------------

class KEntity
{

protected:

	friend class KEntities;

	int		m_iId;

	double	m_dDebX, m_dDebY;
	double	m_dFinX, m_dFinY;
	double	m_dPosZ;
	int		m_iColor;
	int		m_iFlags;

	KEntity *m_pNextAll;    /* Liste globale                     */
	KEntity *m_pNextEntity; /* Liste des entités du même tracé   */
	KEntity *m_pPrevEntity; /* Liste des entités du même tracé - Ordre inverse   */


public:

	int  GetId()			{ return m_iId;					}
	int  GetColor()			{ return m_iColor;				}

	double GetDebX()		{ return m_dDebX;				}
	double GetDebY()		{ return m_dDebY;				}
	double GetFinX()		{ return m_dFinX;				}
	double GetFinY()		{ return m_dFinY;				}

	void AddFlag(int i)		{ m_iFlags |= i;				}
	void ClrFlag(int i)		{ m_iFlags &= i ^ 0xFFFFFFFF;	}
	bool HasFlag(int i)		{ return (m_iFlags & i) != 0;	}

	bool IsDeadLock()		{ return HasFlag(FLAG_DEADLOCK_DEB | FLAG_DEADLOCK_FIN);	}
	bool IsDeadLockDeb()	{ return HasFlag(FLAG_DEADLOCK_DEB);	}
	bool IsDeadLockFin()	{ return HasFlag(FLAG_DEADLOCK_FIN);	}
	bool IsStandAlone()		{ return HasFlag(FLAG_STANDALONE);		}
	bool IsTraite()			{ return HasFlag(FLAG_TRAITE);			}
	bool IsReverse()		{ return HasFlag(FLAG_REVERSE);			}
	bool IsPathHead()		{ return HasFlag(FLAG_PATH_HEAD);		}
	bool IsStartFromEnd()	{ return HasFlag(FLAG_START_FROM_END);	}	

	void SetDeadLockDeb()	{ AddFlag(FLAG_DEADLOCK_DEB);			}	
	void SetDeadLockFin()	{ AddFlag(FLAG_DEADLOCK_FIN);			}
	void SetStandAlone()	{ AddFlag(FLAG_STANDALONE);				}
	void SetTraite()		{ AddFlag(FLAG_TRAITE);					}
	void SetReverse()		{ AddFlag(FLAG_REVERSE);				}
	void SetPathHead()		{ AddFlag(FLAG_PATH_HEAD);				}
	void SetStartFromEnd(bool b=true)	{ if (b) AddFlag(FLAG_START_FROM_END); else ClrFlag(FLAG_START_FROM_END); }	
	
	KEntity();
	virtual ~KEntity();
	
//	bool	GetAdjacentTo(KEntity *p);
	KEntity *FindAdjacent(double dX, double dY, bool *bDebConnected = NULL);

	virtual void Init();
	virtual double GetLength() = 0;
	virtual double GetShortestDistance(double x, double y);
	virtual void Parse(KDXFReader *pReader);
	virtual void ParseDone();
	virtual void ComputeExtents(KExtents &p) = 0;

	void GetStartPos(double &dX, double &dY);
	void GetEndPos(double &dX, double &dY);
	KEntity *GetNextEntity()		{ return m_pNextEntity;	}
	KEntity *GetPrevEntity()		{ return m_pPrevEntity;	}
	KEntity *GetNextAll()			{ return m_pNextAll;	}
	KEntity *GetNextEntityToDo()	{ if (IsStartFromEnd()) return GetPrevEntity(); else return GetNextEntity();	}

	void SetNextEntity(KEntity *p)	{ m_pNextEntity = p;	}
	void SetPrevEntity(KEntity *p)	{ m_pPrevEntity = p;	}
	void SetNextAll(KEntity *p)		{ m_pNextAll = p;	}

	virtual bool ComputeNextPosition(double &dX, double &dY, double dLen) = 0;

	virtual void Dump();
};


//-------------------------------------------------------------------------------

class KPoint : public KEntity
{
	public:
	
	virtual void Init();
	virtual double GetLength();
	virtual void Parse(KDXFReader *pReader);
	virtual void ParseDone();
	virtual void ComputeExtents(KExtents &p);

	virtual bool ComputeNextPosition(double &dX, double &dY, double dLen);

	virtual void Dump();
};


//-------------------------------------------------------------------------------

class KLine : public KEntity
{
	public:
	
	virtual void Init();
	virtual double GetLength();
	virtual void Parse(KDXFReader *pReader);
	virtual void ParseDone();
	virtual void ComputeExtents(KExtents &p);

	virtual bool ComputeNextPosition(double &dX, double &dY, double dLen);

	virtual void Dump();
};

//-------------------------------------------------------------------------------

class KCircle : public KEntity
{
	public:

	double m_dCenX, m_dCenY;
	double m_dRayon;

	double m_dWorkingAngle;
	
	virtual void Init();
	virtual double GetLength();
	virtual double GetShortestDistance(double x, double y);
	virtual void Parse(KDXFReader *pReader);
	virtual void ParseDone();
	virtual void ComputeExtents(KExtents &p);

	virtual bool ComputeNextPosition(double &dX, double &dY, double dLen);

	virtual void Dump();
};

//-------------------------------------------------------------------------------

class KArc : public KCircle
{
	public:

	double m_dAngDeb, m_dAngFin;
	
	virtual void Init();
	virtual double GetLength();
	virtual void Parse(KDXFReader *pReader);
	virtual void ParseDone();
	virtual void ComputeExtents(KExtents &p);

	virtual bool ComputeNextPosition(double &dX, double &dY, double dLen);

	virtual void Dump();
};

//-------------------------------------------------------------------------------

class KEntities
{
	public:

	static KEntity *ms_pFirst;
	static int ms_iCount;

	static void Init();
	static void Insert(KEntity *p);
	static KEntity* GetFirstGross();
	static KEntity* GetFirstGross(KEntity *pFrom);
	static KEntity* GetNextGross(KEntity *pFrom);
	static KEntity* GetFirstDeadLock();
	static KEntity* GetNextDeadLock(KEntity *pFrom);
	
	static bool IsZeroCoord(double x, double y);

	~KEntities();
};

//-------------------------------------------------------------------------------

class KEntityFactory
{
public:

	static KArc    *BuildArc();
	static KCircle *BuildCircle();
	static KLine   *BuildLine();
	static KPoint  *BuildPoint();
};

//-------------------------------------------------------------------------------

#endif