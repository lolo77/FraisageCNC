// Driver.h: interface for the KDriver class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DRIVER_H__21456481_2F42_41D8_9B3E_A80F7F1658ED__INCLUDED_)
#define AFX_DRIVER_H__21456481_2F42_41D8_9B3E_A80F7F1658ED__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

enum
{
	CONFIG_DRV_STEP_X = 0,
	CONFIG_DRV_DIST_X,
	CONFIG_DRV_INV_X,

	CONFIG_DRV_STEP_Y,
	CONFIG_DRV_DIST_Y,
	CONFIG_DRV_INV_Y,
	CONFIG_DRV_ERR_Y,

	CONFIG_DRV_STEP_Z,
	CONFIG_DRV_DIST_Z,
	CONFIG_DRV_INV_Z,
	
	CONFIG_DRV_SPEED_MOVE,
	CONFIG_DRV_SPEED_WORK,

	CONFIG_DRV_POS_Z_MOVE_MARGIN,

	CONFIG_DRV_MAX
};

enum
{
	STATUS_WORKING = 0,
	STATUS_MOVE_UP,
	STATUS_MOVE_TO,
	STATUS_MOVE_DOWN,
	STATUS_ERROR,
	STATUS_FINISHED,
	STATUS_MANUAL_MOVE,
	STATUS_SUPER_INIT_TOOL_LENGTH,
	STATUS_SUPER_NORMAL
};


class KPort;
class KEntity;

class KDriver
{
private:

	class KPosZ
	{
	public:
		int		m_iColor;
		double	m_fPosZStart;
		double	m_fPosZEnd;
		double	m_fPosZInc;
		bool	m_bReverse;
	};

	static char m_szConfigKeys[CONFIG_DRV_MAX][16];

	KPort *m_pPort;

	int m_iStepX;
	int m_iStepY;
	int m_iStepZ;

	double m_dPosAxeX;
	double m_dPosAxeY;
	double m_dPosAxeZ;

	double m_dPosX;
	double m_dPosY;
	double m_dPosZ;

	double m_dResX;
	double m_dResY;
	double m_dResZ;
	double m_dResMin;

	double m_dManualX;
	double m_dManualY;
	double m_dManualZ;

	double m_dPosZWork;
	double m_dPosZWorkInc;
	double m_dPosZWorkEnd;
	double m_dPosZMove;
	double m_dDXFMaxZ;
	double m_dPosZInit;

	int		m_iColorCount;
	KPosZ	*m_pPosZ;

	ticks m_Ticks;

	double m_dConfig[CONFIG_DRV_MAX];

	int		m_iPathCur;
	KEntity *m_pEntityCur;
	int		m_iStatus;
	int		m_iSuperStatus;

	bool	m_bDrill; // false : Follows the path above the surface (no work)
	bool	m_bPause;

	bool IsArrived(double x, double y, double z);
	KPosZ* GetPosZ(int iColor);

	double GetTickCountSinceLastCall();
	bool MicroMove(int iX, int iY, int iZ);
	void ComputeAxis(double dx, double dy, double dz);

public:

	void LoadStatus(char *szFileName);
	void SaveStatus(char *szFileName);

	int  GetStatus()		{ return m_iStatus;	}
	int  GetSuperStatus()	{ return m_iSuperStatus; }
	void Finish();

	double GetPosX()	{ return m_dPosX;	}
	double GetPosY()	{ return m_dPosY;	}
	double GetPosZ()	{ return m_dPosZ;	}

	double GetPosZInit()	{ return m_dPosZInit;	}

	double GetPosAxeX()	{ return m_dPosAxeX;}
	double GetPosAxeY()	{ return m_dPosAxeY;}
	double GetPosAxeZ()	{ return m_dPosAxeZ;}

	int	 GetPathCur()	{ return m_iPathCur;	}
	int	 GetEntityCurInd()	{ return (NULL == m_pEntityCur) ? -1 : m_pEntityCur->GetId();	}
	KEntity* GetEntityCur()	{ return m_pEntityCur; }

	void SkipCurrentPath()	{ m_iPathCur++; m_pEntityCur = NULL;	}

	void Reset();
	void Start();
	void TogglePause()	{ m_bPause ^= true;				}
	void ToggleDrill()	{ m_bDrill ^= true;				}
	void StartToolMeasure();

	bool IsDrill()		{ return m_bDrill;				}

	bool MoveTo(double dX, double dY, double dZ, double dSpeedMmPerSec);
	void MoveToStops(double dSpeed);

	int  LoadConfig(char *szConfigFileName);
	int	 LoadConfigDXF(char *szConfigFileName);
	void ToNextStep();

	void InitXY(double x, double y)	{ m_dPosX = x; m_dPosY = y; ComputeAxis(m_dPosX, m_dPosY, m_dPosZ); m_iStepX = (int)((double)m_dPosAxeX / m_dResX); m_iStepY = (int)((double)m_dPosAxeY / m_dResY); }
	void InitZ(double z)			{ m_dPosZ = z; m_dPosAxeZ = z; m_iStepZ = (int)((double)z / m_dResZ); }
	void LongMoveTo(double dX, double dY, double dZ);

	KDriver(KPort *port);
	~KDriver();
};

#endif // !defined(AFX_DRIVER_H__21456481_2F42_41D8_9B3E_A80F7F1658ED__INCLUDED_)
