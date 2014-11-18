// Optimiser.h: interface for the Optimiser class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OPTIMISER_H__1CBBC0F3_FDA4_488D_A34A_A7FA94A1D97E__INCLUDED_)
#define AFX_OPTIMISER_H__1CBBC0F3_FDA4_488D_A34A_A7FA94A1D97E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class KOptimiser  
{
	static int       ms_iPathCount;
	static double   *ms_tabLength;
	static KEntity **ms_tabEntity;

public:

	~KOptimiser();

	static void MarkDeadLocks();
	static void LinkEntitiesFromDeadLocks();
	static void LinkEntitiesRemaining();
	static void Optimise();

};

#endif // !defined(AFX_OPTIMISER_H__1CBBC0F3_FDA4_488D_A34A_A7FA94A1D97E__INCLUDED_)
