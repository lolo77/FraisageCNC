#include "stdafx.h"
#include "DXFReader.h"
#include "Entity.h"
#include "Extents.h"
#include "Log.h"


//-------------------------------------------------------------------------------
// KEntity
//-------------------------------------------------------------------------------


KEntity::KEntity()
{
	m_iId			= 0;
	m_dDebX			= 0;
	m_dDebY			= 0;
	m_dFinX			= 0;
	m_dFinY			= 0;
	m_dPosZ			= 0;
	m_iColor		= 0;
	m_iFlags		= 0;
	m_pNextAll		= NULL;
	m_pNextEntity	= NULL;
	m_pPrevEntity	= NULL;
}


KEntity::~KEntity()
{
	m_pNextEntity	= NULL;
	m_pPrevEntity	= NULL;

	if (NULL != m_pNextAll)
	{
		delete m_pNextAll;
		m_pNextAll = NULL;
	}
}


void KEntity::Init()
{
	
}


double KEntity::GetShortestDistance(double x, double y)
{
	double dx = m_dFinX - x;
	double dy = m_dFinY - y;
	double dDistFin = sqrt(dx*dx + dy*dy);

	dx = m_dDebX - x;
	dy = m_dDebY - y;
	double dDistDeb = sqrt(dx*dx + dy*dy);

	return MIN(dDistDeb, dDistFin);
}


void KEntity::GetStartPos(double &dX, double &dY)
{
	bool bReverse = IsReverse() ^ IsStartFromEnd();

	if (bReverse)
	{
		dX = m_dFinX;
		dY = m_dFinY;		
	}
	else
	{
		dX = m_dDebX;
		dY = m_dDebY;
	}
}


void KEntity::GetEndPos(double &dX, double &dY)
{
	bool bReverse = IsReverse() ^ IsStartFromEnd();

	if (bReverse)
	{
		dX = m_dDebX;
		dY = m_dDebY;
	}
	else
	{
		dX = m_dFinX;
		dY = m_dFinY;
	}
}


void KEntity::Parse(KDXFReader *pReader)
{
	if (strcmp(pReader->m_szKey, KEY_COLOR) == 0)
		m_iColor = atoi(pReader->m_szVal);
}


void KEntity::Dump()
{
	LOG8("\nEntity #%d <%f,%f ; %f,%f ; %d ; %s ; %f> ", m_iId, m_dDebX, m_dDebY, m_dFinX, m_dFinY, m_iColor, (IsReverse()) ? "true" : "false", GetLength());
}


void KEntity::ParseDone()
{
	if ((fabs(m_dDebX - m_dFinX) < DELTA_MIN) &&
	    (fabs(m_dDebY - m_dFinY) < DELTA_MIN))
	{
		LOG1("ParseDone Entity #%d - Marqued as STANDALONE", m_iId);
		AddFlag(FLAG_DEADLOCK_DEB | FLAG_DEADLOCK_FIN | FLAG_STANDALONE | FLAG_PATH_HEAD);
	}
}


KEntity *KEntity::FindAdjacent(double dX, double dY, bool *bDebConnected)
{
	KEntity *p = KEntities::ms_pFirst;

	while (p != NULL)
	{
		if (p != this)/* && (!p->IsTraite()) && (!p->IsStandAlone()))*/
		{
			if ((fabs(p->m_dDebX - dX) < DELTA_MIN) &&
				(fabs(p->m_dDebY - dY) < DELTA_MIN))
			{
				LOG2("%d is connected to %d DEB", m_iId, p->m_iId);

				if ((fabs(p->m_dFinX - dX) < DELTA_MIN) &&
					(fabs(p->m_dFinY - dY) < DELTA_MIN))
				{
					LOG2("%d is connected to %d FIN", m_iId, p->m_iId);
				}

				if ((!p->IsTraite()) && (!p->IsStandAlone()))
				{

					if (NULL != bDebConnected)
						*bDebConnected = true;

					return p;
				}
			}

			if ((fabs(p->m_dFinX - dX) < DELTA_MIN) &&
				(fabs(p->m_dFinY - dY) < DELTA_MIN))
			{
				LOG2("%d is connected to %d FIN", m_iId, p->m_iId);

				if ((fabs(p->m_dDebX - dX) < DELTA_MIN) &&
					(fabs(p->m_dDebY - dY) < DELTA_MIN))
				{
					LOG2("%d is connected to %d DEB", m_iId, p->m_iId);
				}

				if ((!p->IsTraite()) && (!p->IsStandAlone()))
				{
					if (NULL != bDebConnected)
						*bDebConnected = false;

					return p;
				}
			}
		}

		p = p->m_pNextAll;
	}

	return NULL;
}


//-------------------------------------------------------------------------------
//  KEntities
//-------------------------------------------------------------------------------


KEntity *KEntities::ms_pFirst = NULL;
int KEntities::ms_iCount = 0;


void KEntities::Insert(KEntity *p)
{
	p->m_pNextAll = ms_pFirst;
	p->m_iId = ms_iCount;
	ms_pFirst = p;
	ms_iCount++;	
}


KEntities::~KEntities()
{
	if (NULL != ms_pFirst)
	{
		delete ms_pFirst;
		ms_pFirst = NULL;
	}
}


void KEntities::Init()
{
	KEntity *p = ms_pFirst;

	while (NULL != p)
	{
		p->Init();

		p = p->m_pNextAll;
	}
}


KEntity* KEntities::GetFirstGross()
{
	KEntity *p = ms_pFirst;

	while (NULL != p)
	{
		if ((!p->IsDeadLock()) && (!p->IsTraite()) && (!p->IsStandAlone()))
			return p;

		p = p->m_pNextAll;
	}

	return NULL;
}


KEntity* KEntities::GetFirstGross(KEntity *pFrom)
{
	KEntity *p = pFrom;

	while (NULL != p)
	{
		if ((!p->IsDeadLock()) && (!p->IsTraite()) && (!p->IsStandAlone()))
			return p;

		p = p->m_pNextAll;
	}

	return NULL;
}


KEntity* KEntities::GetNextGross(KEntity *pFrom)
{
	KEntity *p = pFrom;

	if (NULL != p)
		p = p->m_pNextAll;

	while (NULL != p)
	{
		if ((!p->IsDeadLock()) && (!p->IsTraite()) && (!p->IsStandAlone()))
			return p;

		p = p->m_pNextAll;
	}

	return NULL;
}



KEntity* KEntities::GetFirstDeadLock()
{
	KEntity *p = ms_pFirst;

	while (NULL != p)
	{
		if ((p->IsDeadLock()) && (!p->IsTraite()) && (!p->IsStandAlone()))
			return p;

		p = p->m_pNextAll;
	}

	return NULL;
}


KEntity* KEntities::GetNextDeadLock(KEntity *pFrom)
{
	KEntity *p = pFrom;

	if (NULL != p)
		p = p->m_pNextAll;

	while (NULL != p)
	{
		if ((p->IsDeadLock()) && (!p->IsTraite()) && (!p->IsStandAlone()))
			return p;

		p = p->m_pNextAll;
	}

	return NULL;
}


bool KEntities::IsZeroCoord(double x, double y)
{
	return ((fabs(x) < DELTA_MIN) && (fabs(y) < DELTA_MIN));
}


//-------------------------------------------------------------------------------
// KPoint
//-------------------------------------------------------------------------------

void KPoint::Init()
{
	KEntity::Init();
}


double KPoint::GetLength()
{
	return 0;
}


void KPoint::Parse(KDXFReader *pReader)
{
	KEntity::Parse(pReader);

	if (strcmp(pReader->m_szKey, KEY_X0) == 0)
	{
		m_dDebX = atof(pReader->m_szVal);
		m_dFinX = m_dDebX;
	}

	if (strcmp(pReader->m_szKey, KEY_Y0) == 0)
	{
		m_dDebY = atof(pReader->m_szVal);
		m_dFinY = m_dDebY;
	}
}


void KPoint::ParseDone()
{
	KEntity::ParseDone();
}


void KPoint::Dump()
{
	KEntity::Dump();
	LOG("Point");
}


void KPoint::ComputeExtents(KExtents &p)
{
	p.m_dX1 = m_dDebX;
	p.m_dY1 = m_dDebY;
	p.m_dX2 = m_dDebX;
	p.m_dY2 = m_dDebY;
}


bool KPoint::ComputeNextPosition(double &dX, double &dY, double dLen)
{
	// Always arrived
	return true;
}


//-------------------------------------------------------------------------------
// KLine
//-------------------------------------------------------------------------------

void KLine::Init()
{
	KEntity::Init();
}


double KLine::GetLength()
{
	double dx = m_dFinX - m_dDebX;
	double dy = m_dFinY - m_dDebY;
	
	return sqrt(dx*dx + dy*dy);
}


void KLine::Parse(KDXFReader *pReader)
{
	KEntity::Parse(pReader);

	if (strcmp(pReader->m_szKey, KEY_X0) == 0)
		m_dDebX = atof(pReader->m_szVal);

	if (strcmp(pReader->m_szKey, KEY_Y0) == 0)
		m_dDebY = atof(pReader->m_szVal);

	if (strcmp(pReader->m_szKey, KEY_X1) == 0)
		m_dFinX = atof(pReader->m_szVal);

	if (strcmp(pReader->m_szKey, KEY_Y1) == 0)
		m_dFinY = atof(pReader->m_szVal);
}


void KLine::ParseDone()
{
	KEntity::ParseDone();
}


void KLine::Dump()
{
	KEntity::Dump();
	LOG("Line");
}


void KLine::ComputeExtents(KExtents &p)
{
	p.m_dX1 = MIN(m_dDebX, m_dFinX);
	p.m_dY1 = MIN(m_dDebY, m_dFinY);
	p.m_dX2 = MAX(m_dDebX, m_dFinX);
	p.m_dY2 = MAX(m_dDebY, m_dFinY);
}


bool KLine::ComputeNextPosition(double &dX, double &dY, double dLen)
{
	double x,y;
	GetEndPos(x,y);

	x -= dX;
	y -= dY;

	double tmp = sqrt(x*x + y*y);
	if (tmp >= dLen)
	{
		tmp /= dLen;

		x /= tmp;
		y /= tmp;

		x += dX;
		y += dY;

		dX = x;
		dY = y;

//		LOG5("LINE ComputeNextPosition Id %d  next %f %f", m_iId, x, y, dX, dY);
		return false;
	}
	else
	{
		GetEndPos(dX,dY);

//		LOG5("LINE ComputeNextPosition Id %d  next %f %f (arrived)", m_iId, x, y, dX, dY);
		return true;
	}

}


//-------------------------------------------------------------------------------
// KCircle
//-------------------------------------------------------------------------------


double KCircle::GetLength()
{
	return PI*m_dRayon*2;
}


double KCircle::GetShortestDistance(double x, double y)
{
	double dx = x - m_dCenX;
	double dy = y - m_dCenY;
	double dDist = sqrt(dx*dx + dy*dy);
	double dCoef = m_dRayon / dDist;
	dx *= dCoef;
	dy *= dCoef;

	double dCx = dx + m_dCenX;
	double dCy = dy + m_dCenY;

	dx = dCx - x;
	dy = dCy - y;

	double dDistMin = sqrt(dx*dx + dy*dy);

	return dDistMin;
}


void KCircle::Parse(KDXFReader *pReader)
{
	KEntity::Parse(pReader);

	if (strcmp(pReader->m_szKey, KEY_X0) == 0)
		m_dCenX = atof(pReader->m_szVal);

	if (strcmp(pReader->m_szKey, KEY_Y0) == 0)
		m_dCenY = atof(pReader->m_szVal);

	if (strcmp(pReader->m_szKey, KEY_RADIUS) == 0)
		m_dRayon = atof(pReader->m_szVal);
}


void KCircle::Init()
{
	// Calcul du point de départ et d'arrivée de la trajectoire.
	// Arbitraire à l'Est.
	m_dDebX = m_dCenX + m_dRayon;
	m_dDebY = m_dCenY;

	m_dFinX = m_dDebX;
	m_dFinY = m_dDebY;

	m_dWorkingAngle = ANGLE_UNINITIALIZED; // not initialized

	KEntity::Init();
}


void KCircle::ParseDone()
{
	Init();

	KEntity::ParseDone();
}


void KCircle::Dump()
{
	KEntity::Dump();
	LOG3("Circle <%f,%f ; %f> ", m_dCenX, m_dCenY, m_dRayon);
}


void KCircle::ComputeExtents(KExtents &p)
{
	p.m_dX1 = m_dCenX - m_dRayon;
	p.m_dY1 = m_dCenY - m_dRayon;
	p.m_dX2 = m_dCenX + m_dRayon;
	p.m_dY2 = m_dCenY + m_dRayon;
}


bool KCircle::ComputeNextPosition(double &dX, double &dY, double dLen)
{
	// Init
	if (m_dWorkingAngle == ANGLE_UNINITIALIZED)
		m_dWorkingAngle = 0;

	double x = m_dCenX + cos(m_dWorkingAngle) * m_dRayon;
	double y = m_dCenY + sin(m_dWorkingAngle) * m_dRayon;

	double dArcAngle = dLen / m_dRayon;

//	LOG9("CIRCLE %f %f %f : angle %f + %f ; from %f %f to %f %f", m_dCenX, m_dCenY, m_dRayon, m_dWorkingAngle * 180.0 / PI, dArcAngle * 180.0 / PI, dX, dY, x, y);

	bool bArrived = (m_dWorkingAngle >= PI*2);

	if (bArrived)
	{
		GetEndPos(dX, dY);
	}
	else
	{
		dX = x;
		dY = y;
	}

	m_dWorkingAngle += dArcAngle; // radians

	return bArrived;

}


//-------------------------------------------------------------------------------
// KArc
//-------------------------------------------------------------------------------

void KArc::Init()
{
	m_dWorkingAngle = ANGLE_UNINITIALIZED; // not initialized
	KEntity::Init();
}


double KArc::GetLength()
{
	double dAng = m_dAngFin - m_dAngDeb;
	if (dAng < 0.0)
		dAng += 360.0;

	return KCircle::GetLength() * dAng / 360.0;
}


void KArc::Parse(KDXFReader *pReader)
{
	KCircle::Parse(pReader);

	if (strcmp(pReader->m_szKey, KEY_ANGLE_START) == 0)
		m_dAngDeb = atof(pReader->m_szVal);

	if (strcmp(pReader->m_szKey, KEY_ANGLE_END) == 0)
		m_dAngFin = atof(pReader->m_szVal);
}


void KArc::ParseDone()
{
	// Calcul du point de départ et d'arrivée de la trajectoire.
	m_dDebX = m_dCenX + cos(m_dAngDeb * PI/180.0) * m_dRayon;
	m_dDebY = m_dCenY + sin(m_dAngDeb * PI/180.0) * m_dRayon;

	m_dFinX = m_dCenX + cos(m_dAngFin * PI/180.0) * m_dRayon;
	m_dFinY = m_dCenY + sin(m_dAngFin * PI/180.0) * m_dRayon;

//	LOG5("Arc #%d DebX, DebY, FinX, FinY = %f, %f, %f, %f", m_iId, m_dDebX, m_dDebY, m_dFinX, m_dFinY);

	KEntity::ParseDone();
}


void KArc::Dump()
{
	KCircle::Dump();
	LOG2("Arc <%f,%f> ", m_dAngDeb, m_dAngFin);
}


void KArc::ComputeExtents(KExtents &p)
{
	double dDeb = m_dAngDeb;
	double dFin = m_dAngFin;
	
	if (dFin < dDeb)
		dFin += 360;
	
	p.m_dX1 = MIN(m_dDebX, m_dFinX);
	p.m_dY1 = MIN(m_dDebY, m_dFinY);
	p.m_dX2 = MAX(m_dDebX, m_dFinX);
	p.m_dY2 = MAX(m_dDebY, m_dFinY);

	if (dFin >= 360)
	{
		p.m_dX2 = m_dCenX + m_dRayon;
	}
	
	if ((dDeb <= 90) && (dFin >= 90))
	{
		p.m_dY2 = m_dCenY + m_dRayon;
	}
	
	if ((dDeb <= 180) && (dFin >= 180))
	{
		p.m_dX1 = m_dCenX - m_dRayon;
	}
	
	if ((dDeb <= 270) && (dFin >= 270))
	{
		p.m_dY1 = m_dCenY - m_dRayon;
	}

}


bool KArc::ComputeNextPosition(double &dX, double &dY, double dLen)
{
	double dAngDeb;
	double dAngFin;
	double dDirection;

	bool bReverse = (IsReverse()) ^ IsStartFromEnd();

	if (bReverse)
	{
		dAngDeb = m_dAngFin * PI / 180.0;
		dAngFin = m_dAngDeb * PI / 180.0;
		dDirection = -1.0;

		if (dAngFin > dAngDeb)
			dAngDeb += PI*2;
	}
	else
	{
		dAngDeb = m_dAngDeb * PI / 180.0;
		dAngFin = m_dAngFin * PI / 180.0;
		dDirection = 1.0;

		if (dAngFin < dAngDeb)
			dAngFin += PI*2;
	}

	if (m_dWorkingAngle == ANGLE_UNINITIALIZED)
		m_dWorkingAngle = dAngDeb;

	double x = m_dCenX + cos(m_dWorkingAngle) * m_dRayon;
	double y = m_dCenY + sin(m_dWorkingAngle) * m_dRayon;

	bool bArrived = false;

	if (bReverse)
		bArrived = (m_dWorkingAngle <= dAngFin);
	else
		bArrived = (m_dWorkingAngle >= dAngFin);

	if (bArrived)
	{
		GetEndPos(x, y);
	}

//	LOG8("\nARC  %f %f %f : angle %f  from %f %f to %f %f", m_dCenX, m_dCenY, m_dRayon, m_dWorkingAngle * 180.0 / PI, x, y, dX, dY);
//	LOG3("  -> %f %f %s", dAngDeb*180.0/PI, dAngFin*180.0/PI, IsReverse() ? "Reverse" : "");
	
	dX = x;
	dY = y;

	double dArcAngle = dLen / m_dRayon;
	m_dWorkingAngle += dArcAngle * dDirection; // radians

	if (bReverse)
	{
		if (m_dWorkingAngle < dAngFin)
			m_dWorkingAngle = dAngFin;
	}
	else
	{
		if (m_dWorkingAngle > dAngFin)
			m_dWorkingAngle = dAngFin;
	}

	if (m_dWorkingAngle < 0)
		m_dWorkingAngle += PI * 2;

	return bArrived;
}


//-------------------------------------------------------------------------------
// KEntityFactory
//-------------------------------------------------------------------------------


KArc *KEntityFactory::BuildArc()
{
	KArc *p = new KArc();
	KEntities::Insert(p);

	return p;
}


KCircle *KEntityFactory::BuildCircle()
{
	KCircle *p = new KCircle();
	KEntities::Insert(p);

	return p;
}


KLine *KEntityFactory::BuildLine()
{
	KLine *p = new KLine();
	KEntities::Insert(p);

	return p;
}

KPoint *KEntityFactory::BuildPoint()
{
	KPoint *p = new KPoint();
	KEntities::Insert(p);

	return p;
}
