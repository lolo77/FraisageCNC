// Optimiser.cpp: implementation of the Optimiser class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Entity.h"
#include "Path.h"
#include "Optimiser.h"
#include "Log.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

double   *KOptimiser::ms_tabLength = NULL;
KEntity **KOptimiser::ms_tabEntity = NULL;
int KOptimiser::ms_iPathCount = 0;

KOptimiser optimiser;


KOptimiser::~KOptimiser()
{
	if (NULL != ms_tabLength)
	{
		delete[] ms_tabLength;
		ms_tabLength = NULL;
	}

	if (NULL != ms_tabEntity)
	{
		delete[] ms_tabEntity;
		ms_tabEntity = NULL;
	}
}


void KOptimiser::MarkDeadLocks()
{
	KEntity *p1 = KEntities::GetFirstGross();
	
	while (NULL != p1)
	{
		if (NULL == p1->FindAdjacent(p1->GetDebX(), p1->GetDebY()))
		{
			p1->SetDeadLockDeb();
			LOG3("#%d is Deadlock on Deb (%f ; %f)", p1->GetId(), p1->GetDebX(), p1->GetDebY());
		}

		if (NULL == p1->FindAdjacent(p1->GetFinX(), p1->GetFinY()))
		{
			p1->SetDeadLockFin();
			if (p1->IsDeadLockDeb())
			{
				p1->SetStandAlone();
				p1->SetPathHead();
				LOG1("#%d is StandAlone and PathHead", p1->GetId());
			}
			else
			{
				LOG3("#%d is Deadlock on Fin (%f ; %f)", p1->GetId(), p1->GetFinX(), p1->GetFinY());
			}
		}
		
		p1 = KEntities::GetNextGross(p1);
	}
}


void KOptimiser::LinkEntitiesFromDeadLocks()
{
	bool bDebConnected = false;

	KEntity *p1 = KEntities::GetFirstDeadLock();
	if (NULL != p1)
		p1->SetPathHead();

	while (NULL != p1)
	{
		p1->SetTraite();
		
		KEntity *p2 = p1;
		while (p2 != NULL)
		{
			KEntity *p3 = p2->FindAdjacent(p2->GetDebX(), p2->GetDebY(), &bDebConnected);
			if (p3 != NULL)
			{
				p2->SetNextEntity(p3);
				p2->SetTraite();
				p2->SetReverse();

				LOG2("Linked %d on Deb %d (reversed)", p3->GetId(), p2->GetId());

				if (!bDebConnected)
				{
					p3->SetReverse();
					LOG1("   --> %d is reversed", p3->GetId());
				}

				p2 = p3;
			}
			else
			{
				p3 = p2->FindAdjacent(p2->GetFinX(), p2->GetFinY(), &bDebConnected);
				p2->SetTraite();

				if (p3 != NULL)
				{
					p2->SetNextEntity(p3);
					
					LOG2("Linked %d on Fin %d", p3->GetId(), p2->GetId());

					if (!bDebConnected)
					{
						p3->SetReverse();
						LOG1("   --> %d is reversed", p3->GetId());
					}

				}
				else
				{
					LOG("End of path reached");

					p1 = KEntities::GetFirstDeadLock();
					if (NULL != p1)
						p1->SetPathHead();
				}

				p2 = p3;
			}
		}
	}
}


void KOptimiser::LinkEntitiesRemaining()
{
	bool bDebConnected = false;
	KEntity *p1 = KEntities::GetFirstGross();

	if (NULL != p1)
		p1->SetPathHead();

	while (NULL != p1)
	{
		p1->SetTraite();
		KEntity *p2 = p1->FindAdjacent(p1->GetFinX(), p1->GetFinY(), &bDebConnected);

		if (NULL == p2)
		{
			p2 = p1->FindAdjacent(p1->GetDebX(), p1->GetDebY(), &bDebConnected);

			if (NULL == p2)
			{
				LOG("End of path reached");

				if (!bDebConnected)
				{
					p1->SetReverse();
					LOG1("(#%d reversed)", p1->GetId());
				}
				

				p1 = KEntities::GetFirstGross();
				if (NULL != p1)
					p1->SetPathHead();
			}
			else
			{
				LOG2("Linked %d on Deb %d (reversed)", p2->GetId(), p1->GetId());

				p1->SetNextEntity(p2);
				p1->SetReverse();
				p1 = p2;
			}
		}
		else
		{
			LOG2("Linked %d on Fin %d", p2->GetId(), p1->GetId());

			p1->SetNextEntity(p2);
			p1 = p2;
		}
	}
}


void KOptimiser::Optimise()
{
	LOG("MarkDeadLocks");
	MarkDeadLocks();			 // Marque les élements de terminaison ou orphelins
	LOG("LinkEntitiesFromDeadLocks");
	LinkEntitiesFromDeadLocks(); // Lie les chemins non bouclés
	LOG("LinkEntitiesRemaining");
	LinkEntitiesRemaining();     // Lie les chemins bouclés
	KPath::BuildPath();			 //	Crée la liste des chemins
	KPath::LinkRewind();		 // Lie les entités à l'envers au sein de chaque chemin
	KPath::SortPath();			 // Trie la liste des chemins
}
