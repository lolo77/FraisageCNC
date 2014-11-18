// Path.cpp: implementation of the KPath class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Entity.h"
#include "Extents.h"
#include "Path.h"
#include "Log.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

KPath *KPath::ms_tabPath = NULL;
int KPath::ms_iCount = 0;


KPath::KPath()
{
	m_pEntityHead = NULL;
	m_pEntityTail = NULL;
	m_dExtents = 0;
	m_dLength = 0;
	m_bClosed = false;
	m_bStartFromEndInit = false;
	m_bStartFromEnd = false;
}


KPath::~KPath()
{

}


void KPath::Reset()
{
	m_bStartFromEnd = m_bStartFromEndInit;

	KEntity *p = m_pEntityHead;
	while (NULL != p)
	{
		p->SetStartFromEnd(m_bStartFromEnd);
		p->Init();
		p = p->GetNextEntity();
	}
}


void KPath::Init()
{
	KEntity *p = m_pEntityHead;
	while (NULL != p)
	{
		p->SetStartFromEnd(m_bStartFromEnd);
		p->Init();
		p = p->GetNextEntity();
	}

}


void KPath::BuildPath()
{
	ms_iCount = 0;
	KEntity *p = KEntities::ms_pFirst;
	
	while (p != NULL)
	{
		if (p->IsPathHead())
			ms_iCount++;
		
		p = p->GetNextAll();
	}

	LOG1("\n%d chemin(s)\n", ms_iCount);

	ms_tabPath = new KPath[ms_iCount];

	int i = 0;
	p = KEntities::ms_pFirst;
	while (p != NULL)
	{
		if (p->IsPathHead())
		{
			ms_tabPath[i].m_pEntityHead = p;
			ms_tabPath[i].ComputeExtents();
			ms_tabPath[i].ComputeLength();


			LOG4("Path %d : Extents = %f ; Length = %f ; Color = %d", i, ms_tabPath[i].m_dExtents, ms_tabPath[i].m_dLength, p->GetColor());
			i++;
		}
		
		p = p->GetNextAll();
	}
}


void KPath::LinkRewind()
{
	LOG("\nLinkRewind\n");
	int iPath = 0;
	while (iPath < ms_iCount)
	{
		KEntity *p = GetPath(iPath)->m_pEntityHead;
		KEntity *pPrev = NULL;
		while (p != NULL)
		{
			p->SetPrevEntity(pPrev);
			
			pPrev = p;
			p = p->GetNextEntity();
		}

		GetPath(iPath)->m_pEntityTail = pPrev;

		double dX1, dY1, dX2, dY2;
		GetPath(iPath)->m_pEntityHead->GetStartPos(dX1, dY1);
		pPrev->GetEndPos(dX2, dY2);

		if (KEntities::IsZeroCoord(dX1-dX2, dY1-dY2))
		{
			ms_tabPath[iPath].m_bClosed = true;

			LOG6("Path %3d is closed (Extents %11f ; Length %11f ; Color %2d ; Head #%4d ; Tail #%4d)", iPath, GetPath(iPath)->GetExtents(), GetPath(iPath)->GetLength(), GetPath(iPath)->GetEntity()->GetColor(), GetPath(iPath)->m_pEntityHead->GetId(), pPrev->GetId());
		} else
		{
			// Path not looped
			LOG6("Path %3d is open   (Extents %11f ; Length %11f ; Color %2d ; Head #%4d ; Tail #%4d)", iPath, GetPath(iPath)->GetExtents(), GetPath(iPath)->GetLength(), GetPath(iPath)->GetEntity()->GetColor(), GetPath(iPath)->m_pEntityHead->GetId(), pPrev->GetId());
		}

		iPath++;
	}
}


void KPath::SortPath()
{
	qsort(ms_tabPath, ms_iCount, sizeof(KPath), (int(*)(const void*,const void*))KPath::Compare);
	LOG("\nChemins triés par surface\n");

	LOG("\nOptimisation des déplacements\n");

	double dCurExtent;
	int iPath = 0;
	while (iPath < ms_iCount)
	{
		dCurExtent = GetPath(iPath)->GetExtents();

		// Prend les coordonnees du bout du chemin
		KEntity *p = GetPath(iPath)->GetEntityEnd();
		double dx = p->GetFinX();
		double dy = p->GetFinY();

		// Recherche le chemin de même "extents" avec les coordonnées de début
		// les plus proches
		int i = iPath+1;
		int iFound = -1;
		double dDist = 10000000000;
		while ((i < ms_iCount) && (GetPath(i)->GetExtents() >= dCurExtent - 0.1) && (GetPath(i)->GetExtents() <= dCurExtent + 0.1))
		{
			KEntity *_p = GetPath(i)->GetEntity();
			double _dx = _p->GetDebX();
			double _dy = _p->GetDebY();

			double d = (_dx-dx) * (_dx-dx) + (_dy-dy) * (_dy-dy);
			if (d < dDist)
			{
				dDist = d;
				iFound = i;
			}

			i++;
		}

		if (iFound >= 0)
		{
			KPath path;
			memcpy(&path, &ms_tabPath[iPath+1], sizeof(KPath));
			memcpy(&ms_tabPath[iPath+1], &ms_tabPath[iFound], sizeof(KPath));
			memcpy(&ms_tabPath[iFound], &path, sizeof(KPath));

			LOG3("Inverted pathes %d with %d : dist = %lf", iPath, iFound, sqrt(dDist));
		}

		iPath++;
	}
}


void KPath::DestroyPath()
{
	if (NULL != ms_tabPath)
	{
		delete[] ms_tabPath;
		ms_tabPath = NULL;
	}
}


void KPath::ComputeLength()
{
	m_dLength = 0;

	KEntity *p = m_pEntityHead;
	while (NULL != p)
	{
		m_dLength += p->GetLength();
		p = p->GetNextEntity();
	}
}


void KPath::ComputeExtents()
{
	KExtents e;
	KExtents eTmp;
	KEntity  *p = m_pEntityHead;
	LOG1("Compute Extents from #%d", p->GetId());

	while (p != NULL)
	{
		p->ComputeExtents(eTmp);
		LOG5("Extents of #%d : (%f,%f ; %f,%f)", p->GetId(), eTmp.m_dX1, eTmp.m_dY1, eTmp.m_dX2, eTmp.m_dY2);
		e.Adjust(eTmp);
		
		p = p->GetNextEntity();
	}
	
	m_dExtents = e.GetHalfPerimeter();
}


int KPath::Compare(KPath *p1, KPath *p2)
{
	return (p2->m_dExtents < p1->m_dExtents) ? 1 : -1;
}


void KPath::ToggleStartFromEnd()
{
	m_bStartFromEnd ^= true;
	Init();
}