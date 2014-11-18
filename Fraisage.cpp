// Fraisage.cpp : Defines the entry point for the console application.
//


#include "stdafx.h"
#include "Entity.h"
#include "DXFReader.h"
#include "Optimiser.h"
#include "Path.h"
#include "Port.h"
#include "Driver.h"
#include "Log.h"


void gotoxy(int col, int row)
{
	// Source : http://www.commentcamarche.net/forum/affich-2936037-c-probleme-gotoxy
	COORD coord;
	coord.X = col;
	coord.Y = row;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
} 


int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		printf("Usage : Fraisage <Fichier DXF> <Fichier de profondeurs>\n");
		return -1;
	}

	KLog *log = KLog::GetInstance("log.txt");

	KEntities entities;
	KDXFReader reader;

	if (!reader.OpenFile(argv[1]))
	{
		printf("Fichier '%s' introuvable.\n", argv[1]);
		return -2;
	}

	LOG1("Traitement du fichier %s", argv[1]);

	if (reader.Parse() < 0)
	{
		printf("Erreur : pas de section ENTITIES.\n");
		return -3;
	}

	reader.CloseFile();

	KEntity *p = KEntities::ms_pFirst;
	while (NULL != p)
	{
		p->Dump();
		p = p->GetNextAll();
	}

	LOG1("\n Count = %d\n", KEntities::ms_iCount);

	KOptimiser::Optimise();

	if (KPath::GetCount() > 0)
	{
		double dx, dy;
		dx = KPath::GetPath(0)->GetEntityEnd()->GetFinX();
		dy = KPath::GetPath(0)->GetEntityEnd()->GetFinY();

		for (int i = 1; i < KPath::GetCount(); i++)
		{
			double _dx, _dy;
			_dx = KPath::GetPath(i)->GetEntity()->GetDebX();
			_dy = KPath::GetPath(i)->GetEntity()->GetDebY();

			double d = sqrt((_dx-dx) * (_dx-dx) + (_dy-dy) * (_dy-dy));
			LOG3("Path #%d -> #%d : move %f", i-1, i, d);

			dx = KPath::GetPath(i)->GetEntityEnd()->GetFinX();
			dy = KPath::GetPath(i)->GetEntityEnd()->GetFinY();
		}
	}

	LOG("");

	KPort port(0);
	if (port.LoadConfig("default.cfg") < 0)
	{
		printf("Erreur lors du chargement du fichier de configuration [Port].\n");
		return -4;
	}

	KDriver driver(&port);

	if (driver.LoadConfig("default.cfg") < 0)
	{
		printf("Erreur lors du chargement du fichier de configuration [Driver].\n");
		return -5;
	}

	if (driver.LoadConfigDXF(argv[2]) < 0)
	{
		printf("Erreur lors du chargement du fichier de configuration DXF [Driver].\n");
		return -6;
	}

	char *cStatusFile = "status.dat";
	driver.LoadStatus(cStatusFile);

	gotoxy(0,0);
	printf("Echap : Quitter\n");
	printf("F2    : RAZ capteur\n");
	printf("F3    : Mesure Outil\n");
	printf("F4    : Init (X,Y) = (0,0)\n");
	printf("F5    : Init Z = 0\n");
	printf("F6    : Init Z = %f\n", driver.GetPosZInit());
//	printf("F6    : Init Origine (X,Y) par détection Centre Cercle\n");
//	printf("F7    : Init Origine (X,Y) par détection Centre Arc (3 points)\n");
//	printf("F8    : Init Origine (X,Y) par intersection de 2 droites (4 points)\n"); 
	printf("F8    : Chemin suivant\n");
	printf("F9    : Fraisage sans contact / avec contact\n");
	printf("F10   : Fraisage/Pause\n");
	printf("R     : Déplacement relatif (X,Y,Z)\n");
	printf("A     : Déplacement absolu  (X,Y,Z)\n");

	char k = 0;

	bool bWorking = false;

	DWORD dwTotalTime = 0;
	DWORD dwStartTime;
	double dPathCurviligne = 0;
	double dPathCurviligneWork = 0;
	int iCurPath = -1;
	KEntity *pCurEntity = NULL;

	int iRefreshTick = 0;

	while (k != 27)
	{
		if (kbhit())
		{
			k = getch();
			if (k == 0)
			{
				k = getch();

				if (driver.GetStatus() == STATUS_FINISHED)
				{
					if (k == 68)
					{
						LOG1("Driver Start %u", GetTickCount());

						dwStartTime = GetTickCount();

						bWorking = true;

						KEntities::Init();
						driver.Reset();
						driver.Start();

						// DISABLE Z
//						port.EnableMotors(MOTOR_XY);

						// Start Spin rotation
						port.Output(false, false, false, false, false, false, true, true);

					}

					if (k == 60) // F2
					{
						port.ResetCapteurs();
					}

					if (k == 61) // F3
					{
						driver.StartToolMeasure();
					}

					if (k == 62) // F4
					{
						driver.InitXY(0,0);
					}

					if (k == 63) // F5
					{
						driver.InitZ(0);
					}

					if (k == 64) // F6
					{
						driver.InitZ(driver.GetPosZInit());
					}

					if (k == 67) // F9
					{
						driver.ToggleDrill();
					}
				}
				else
				if ((driver.GetStatus() != STATUS_FINISHED) && (driver.GetStatus() != STATUS_ERROR))
				{
					if (k == 68) // F10
					{
						bWorking ^= true;

						dwTotalTime += GetTickCount() - dwStartTime;

						LOG1("Driver Pause Toggle %u", GetTickCount());

						driver.TogglePause();
					}

					if (k == 66) // F8
					{
						driver.SkipCurrentPath();
					}
				}
			}
			else
			{
				if (driver.GetStatus() == STATUS_FINISHED)
				{
					if ((k == 'A') || (k == 'a'))
					{
						gotoxy(5,16);
						printf("Déplacement Absolu\nSaisissez les coordonnées X,Y,Z : ");
						double x = 0;
						double y = 0;
						double z = 0;
						int i = scanf("%lf,%lf,%lf", &x, &y, &z);

						gotoxy(5,16);
						printf("                  \n                                                     ");

						if (i == 3)
						{
							driver.LongMoveTo(x,y,z);
						}
					}

					if ((k == 'R') || (k == 'r'))
					{
						gotoxy(5,16);
						printf("Déplacement Relatif\nSaisissez les coordonnées X,Y,Z : ");
						double x = 0;
						double y = 0;
						double z = 0;
						
						int i = scanf("%lf,%lf,%lf", &x, &y, &z);

						gotoxy(5,16);
						printf("                   \n                                                     ");

						if (i == 3)
							driver.LongMoveTo(x + driver.GetPosX(), y + driver.GetPosY(), z + driver.GetPosZ());
					}
				}
				
				if ((driver.GetSuperStatus() == STATUS_SUPER_INIT_TOOL_LENGTH) ||
					(driver.GetStatus() != STATUS_FINISHED))
				{
					if (k == 27)
					{
						driver.Finish();
						k = 1; // To avoid exiting
					}
				}
			}
		}

		if ((iRefreshTick & 0x0F) == 0)
		{

			gotoxy(50,0);
			if (driver.IsDrill())
			{
				printf("Fraisage avec contact");
			}
			else
			{
				printf("Fraisage sans contact");
			}

			gotoxy(0,11);
			printf("Pos X,Y,Z : %f %f %f / %f %f %f                  ", driver.GetPosX(), driver.GetPosY(), driver.GetPosZ(), driver.GetPosAxeX(), driver.GetPosAxeY(), driver.GetPosAxeZ());

			gotoxy(0,12);
			printf("Chemin %d/%d ; longueur : %f mm - Entite %d         ", driver.GetPathCur()+1, KPath::GetCount(), ((driver.GetPathCur() >= 0) && (driver.GetPathCur() < KPath::GetCount())) ? (KPath::GetPath(driver.GetPathCur())->GetLength()) : 0, driver.GetEntityCurInd());

			gotoxy(0,13);
			printf("%f mm parcourus en usinage", dPathCurviligneWork);
			
			gotoxy(0,14);
			printf("%f mm parcourus en déplacement", dPathCurviligne - dPathCurviligneWork);

			gotoxy(0,15);
			printf("%f mm parcourus au total", dPathCurviligne);

			gotoxy(0,18);
			printf("Capteurs X1,X2 ; Y1,Y2 ; Z1,Z2 ; Tool : %d,%d ; %d,%d ; %d,%d ; %d", port.IsXStart(), port.IsXStop(), port.IsYStart(), port.IsYStop(), port.IsZStart(), port.IsZStop(), port.IsZTool());
			
		}

		if (driver.GetStatus() == STATUS_ERROR)
		{
			gotoxy(0,19);
			printf("Mouvement interrompu par une erreur - Appuyez sur une touche pour continuer");
			driver.Finish();
			getch();
			gotoxy(0,19);
			printf("                                                                                   ");
		}
		
		double dOldX = driver.GetPosX();
		double dOldY = driver.GetPosY();

		driver.ToNextStep();

		double dX = driver.GetPosX() - dOldX;
		double dY = driver.GetPosY() - dOldY;

		double dDist = sqrt(dX*dX + dY*dY);

		dPathCurviligne += dDist;

		if (driver.GetStatus() == STATUS_WORKING)
		{
			dPathCurviligneWork += dDist;
		}

		if ((bWorking) && (driver.GetStatus() == STATUS_FINISHED))
		{
			bWorking = false;

			dwTotalTime += GetTickCount() - dwStartTime;

			LOG1("Driver Finished %u", GetTickCount());
			LOG5("Working Time : %u (%02d:%02d:%02d, %03d)", dwTotalTime, (dwTotalTime / 3600000), (dwTotalTime / 60000) % 60, (dwTotalTime / 1000) % 60, dwTotalTime % 1000);
		}

/*
		if ((driver.GetStatus() != STATUS_FINISHED) && (driver.GetStatus() != STATUS_ERROR))
		{
			LOG3("POSXYZ %d %d %d", (int)((double)driver.GetPosX()*1000.0), (int)((double)driver.GetPosY()*1000.0), (int)((double)driver.GetPosZ()*1000.0));
		}*/

		iRefreshTick++;
	}

	LOG("\n\n");
	LOG1("%f mm parcourus en usinage", dPathCurviligneWork);
	LOG1("%f mm parcourus en deplacement", dPathCurviligne - dPathCurviligneWork);
	LOG1("%f mm parcourus au total", dPathCurviligne);

	// Stop Spin rotation
	port.Output(false, false, false, false, false, false, true, false);

	driver.SaveStatus(cStatusFile);

	KPath::DestroyPath();
	return 0;
}

