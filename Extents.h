// Extents.h: interface for the KExtents class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EXTENTS_H__A3AA8CB2_61B1_43CC_B364_2E6946F7C342__INCLUDED_)
#define AFX_EXTENTS_H__A3AA8CB2_61B1_43CC_B364_2E6946F7C342__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class KExtents
{
	public:
	double m_dX1;
	double m_dY1;
	double m_dX2;
	double m_dY2;
	
	KExtents();
	void Adjust(KExtents &p);
	double GetHalfPerimeter();
};

#endif // !defined(AFX_EXTENTS_H__A3AA8CB2_61B1_43CC_B364_2E6946F7C342__INCLUDED_)
