// Extents.cpp: implementation of the KExtents class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Extents.h"
#include "Log.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

KExtents::KExtents()
{
	m_dX1 = 10000000000;
	m_dY1 = 10000000000;
	m_dX2 = -10000000000;
	m_dY2 = -10000000000;
}


void KExtents::Adjust(KExtents &p)
{
	m_dX1 = MIN(m_dX1, p.m_dX1);
	m_dY1 = MIN(m_dY1, p.m_dY1);
	m_dX2 = MAX(m_dX2, p.m_dX2);
	m_dY2 = MAX(m_dY2, p.m_dY2);
}


double KExtents::GetHalfPerimeter()
{
	return (m_dX2 - m_dX1 + m_dY2 - m_dY1);
}
