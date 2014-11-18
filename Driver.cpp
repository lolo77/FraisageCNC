// Driver.cpp: implementation of the KDriver class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Entity.h"
#include "Driver.h"
#include "Path.h"
#include "Port.h"
#include "DXFReader.h"
#include "Log.h"

char KDriver::m_szConfigKeys[CONFIG_DRV_MAX][16] = {"XStep ", 
													"XDist ",
													"XInvert ",
													"YStep ", 
													"YDist ",
													"YInvert ",
													"YError ",
													"ZStep ", 
													"ZDist ",
													"ZInvert ",
													"SpeedMove ", 
													"SpeedWork ",
													"ZPosMoveMargin "};


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////



KDriver::KDriver(KPort *p)
{
	m_pPort			= p;
	m_iStepX		= 0;
	m_iStepY		= 0;
	m_iStepZ		= 0;
	m_dPosX			= 0;
	m_dPosY			= 0;
	m_dPosZ			= 0;
	m_dPosAxeX		= 0;
	m_dPosAxeY		= 0;
	m_dPosAxeZ		= 0;
	m_dResX			= 0;
	m_dResY			= 0;
	m_dResZ			= 0;
	m_dManualX		= 0;
	m_dManualY		= 0;
	m_dManualZ		= 0;
	m_iColorCount	= 0;
	m_pPosZ			= NULL;
	m_dPosZWork		= 0;
	m_dPosZMove		= 0;
	m_dPosZWorkEnd	= 0;
	m_dPosZWorkInc	= 0;
	m_dDXFMaxZ		= 0;
	m_bDrill		= true;
	memset(m_dConfig, 0xff, sizeof(m_dConfig));
	Reset();
}


KDriver::~KDriver()
{
	if (m_pPosZ != NULL)
	{
		delete[] m_pPosZ;
		m_pPosZ = NULL;
	}
}


void KDriver::LoadStatus(char *szFileName)
{
	FILE *f;

	LOG("\n Driver : LoadStatus\n");

	f = fopen(szFileName, "r");

	if (NULL == f)
	{
		LOG1("\nErreur de lecture du fichier '%s'\n", szFileName);
		return;
	}

	fread(&m_dPosX, sizeof(m_dPosX), 1, f);
	fread(&m_dPosY, sizeof(m_dPosY), 1, f);
	fread(&m_dPosZ, sizeof(m_dPosZ), 1, f);

	m_pPort->LoadStatus(f);

	ComputeAxis(m_dPosX, m_dPosY, m_dPosZ);

	if (m_dPosAxeX >= 0)
		m_iStepX = (int)((double)(m_dPosAxeX / m_dResX) + 0.5);
	else
		m_iStepX = (int)((double)(m_dPosAxeX / m_dResX) - 0.5);

	if (m_dPosAxeY >= 0)
		m_iStepY = (int)((double)(m_dPosAxeY / m_dResY) + 0.5);
	else
		m_iStepY = (int)((double)(m_dPosAxeY / m_dResY) - 0.5);

	if (m_dPosAxeZ >= 0)
		m_iStepZ = (int)((double)(m_dPosAxeZ / m_dResZ) + 0.5);
	else
		m_iStepZ = (int)((double)(m_dPosAxeZ / m_dResZ) - 0.5);

	fclose(f);
}


void KDriver::SaveStatus(char *szFileName)
{
	if (m_pPort->IsSimul())
		return;

	FILE *f;

	LOG("\n Driver : SaveStatus\n");

	f = fopen(szFileName, "w");

	if (NULL == f)
	{
		LOG1("\nErreur d'ouverture du fichier '%s' en écriture\n", szFileName);
		return;
	}

	fwrite(&m_dPosX, sizeof(m_dPosX), 1, f);
	fwrite(&m_dPosY, sizeof(m_dPosY), 1, f);
	fwrite(&m_dPosZ, sizeof(m_dPosZ), 1, f);

	m_pPort->SaveStatus(f);

	fclose(f);
}


int KDriver::LoadConfig(char *szConfigFileName)
{
	FILE *f;

	LOG("\n Driver : LoadConfig\n");

	f = fopen(szConfigFileName, "r");

	if (NULL == f)
	{
		LOG1("\nErreur de lecture du fichier '%s'\n", szConfigFileName);
		return -1;
	}

	int i = 0;
	memset(m_dConfig, 0xff, sizeof(m_dConfig));

	char szBuf[MAX_LINE];
	while (fgets(szBuf, MAX_LINE, f) != 0)
	{
		if (szBuf[0] == '#')
			continue;

		for (int j = 0; j < CONFIG_DRV_MAX; j++)
		{
			if (strncmp(szBuf, m_szConfigKeys[j], strlen(m_szConfigKeys[j])) == 0)
			{
				char * szVal = szBuf + strlen(m_szConfigKeys[j]);
				StringTrim(szVal, strlen(szVal));

				double dVal = -1;

				if (szVal[0] != 0)
				{
					dVal = atof(szVal);

					m_dConfig[j] = dVal;
				}

				LOG3("Driver Found Key '%s' = '%f' -> '%f'", m_szConfigKeys[j], dVal, m_dConfig[j]);
				break;
			}
		}
		
	}

	fclose(f);

	m_dResX = m_dConfig[CONFIG_DRV_DIST_X] / m_dConfig[CONFIG_DRV_STEP_X];
	m_dResY = m_dConfig[CONFIG_DRV_DIST_Y] / m_dConfig[CONFIG_DRV_STEP_Y];
	m_dResZ = m_dConfig[CONFIG_DRV_DIST_Z] / m_dConfig[CONFIG_DRV_STEP_Z];

	m_dResMin = MIN(m_dResX, m_dResY);
	m_dResMin = MIN(m_dResMin, m_dResZ);

	LOG4("Résolutions : %f %f %f ; Min : %f", m_dResX, m_dResY, m_dResZ, m_dResMin);

	return 0;
}


int KDriver::LoadConfigDXF(char *szConfigFileName)
{
	FILE *f;

	double fPosZInit = 0;

	LOG("\n Driver : LoadConfigDXF\n");

	f = fopen(szConfigFileName, "r");

	if (NULL == f)
	{
		LOG1("\nErreur de lecture du fichier '%s'\n", szConfigFileName);
		return -1;
	}

	char szBuf[MAX_LINE];
	m_iColorCount = 0;
	while (fgets(szBuf, MAX_LINE, f) != 0)
	{
		if ((szBuf[0] == '#') || (szBuf[0] == 'Z'))
			continue;

		m_iColorCount++;	
	}

	m_pPosZ  = new KPosZ[m_iColorCount];

	fseek(f, 0, SEEK_SET);
	int iCur = 0;

	while (fgets(szBuf, MAX_LINE, f) != 0)
	{
		if (szBuf[0] == '#')
			continue;

		StringTrim(szBuf, strlen(szBuf));

		unsigned int i = 0;
		while ((i < strlen(szBuf)) && (szBuf[i] != ':'))
			i++;

		if (i < strlen(szBuf))
		{
			szBuf[i] = 0;
			if (szBuf[0] == 'Z')
			{
				int iRet = sscanf(szBuf+i+1, "%lf", &fPosZInit);
				if (iRet != 1)
				{
					fPosZInit = 0;
					LOG1("Driver DXF Key error. Entry not loaded Z : '%s'", szBuf+i+1);
					continue;
				}

				LOG2("Driver DXF Found Key Z : '%s' = '%f'", szBuf+i+1, fPosZInit);
			}
			else
			{
				int iCol = atoi(szBuf);
				double fPosStart, fPosEnd, fPosInc;
				char sFlag[16];
				memset(sFlag,0, sizeof(sFlag));
				int iRet = sscanf(szBuf+i+1, "%lf;%lf;%lf;%5s", &fPosStart, &fPosEnd, &fPosInc, sFlag);
				if (iRet == 1)
				{
					fPosEnd = fPosStart;
					fPosInc   = 1;
				}

				if ((iRet != 1) && (iRet != 3) && (iRet != 4))
				{
					LOG2("Driver DXF Key error. Entry not loaded '%s' : '%s'", szBuf, szBuf+i+1);
					continue;
				}

				fPosStart += fPosZInit;
				fPosEnd   += fPosZInit;

				LOG6("Driver DXF Found Key '%s' = %d : '%s' : Start=%f ; End=%f ; Inc=%f", szBuf, iCol, szBuf+i+1, fPosStart, fPosEnd, fPosInc);

				if (fPosInc < 0)
					fPosInc *= -1;

				// Pour eviter un usinage infini
				if (fPosInc == 0)
				{
					fPosInc = 1;

					if (fPosStart != fPosEnd)
					{
						LOG2("ERREUR ! Profondeur de passe nulle pour '%s'. Init à %f", szBuf, fPosInc);
					}
				}

				if (fPosStart < fPosEnd)
				{
					double d = fPosStart;
					fPosStart = fPosEnd;
					fPosEnd = d;
				}

				m_pPosZ[iCur].m_iColor		= iCol;
				m_pPosZ[iCur].m_fPosZStart	= fPosStart;
				m_pPosZ[iCur].m_fPosZEnd	= fPosEnd;
				m_pPosZ[iCur].m_fPosZInc	= fPosInc;
				m_pPosZ[iCur].m_bReverse	= (strncmp("inv", sFlag, 3) == 0);

				if (fPosStart > m_dDXFMaxZ)
					m_dDXFMaxZ = fPosStart;

				iCur++;
			}
		}
		
		
	}

	fclose(f);

	LOG1("DXF Z Max = %lf", m_dDXFMaxZ);

	m_dPosZInit = fPosZInit;

	return 0;
}


void KDriver::ComputeAxis(double dx, double dy, double dz)
{
	m_dPosAxeX = dx + dy * (m_dConfig[CONFIG_DRV_ERR_Y]/100.0);
	m_dPosAxeY = dy;
	m_dPosAxeZ = dz;
}


double KDriver::GetTickCountSinceLastCall()
{
	ticks t = getticks();

	double dDiff = (double)t.QuadPart - (double)m_Ticks.QuadPart;

	m_Ticks = t;

	return dDiff;
}



void KDriver::Reset()
{
	m_iPathCur = 0;
	m_pEntityCur = NULL;

	// DEBUG
/*	m_iPathCur = 0;
	m_pEntityCur = KPath::ms_tabPath[m_iPathCur].m_pEntity;
	while ((m_pEntityCur != NULL) && (m_pEntityCur->GetId() != 67))
		m_pEntityCur = m_pEntityCur->m_pNextEntity;*/

	m_iStatus = STATUS_FINISHED;
	m_iSuperStatus = STATUS_SUPER_NORMAL;
	m_dPosZWork = -1;
	m_dPosZMove = m_dConfig[CONFIG_DRV_POS_Z_MOVE_MARGIN] + m_dDXFMaxZ;

	m_bPause = false;
	m_pPort->Input();
	m_pPort->Input();

	m_Ticks = getticks();

	LOG("\nDriver Status init to FINISHED\n");
}


void KDriver::Start()
{
	m_iStatus = STATUS_MOVE_UP;
	
	m_pPort->EnableMotors(MOTOR_ALL);
}


void KDriver::Finish()
{
	m_iSuperStatus	= STATUS_SUPER_NORMAL;
	m_iStatus		= STATUS_FINISHED;
	
	m_pPort->EnableMotors(MOTOR_NONE);
}


void KDriver::StartToolMeasure()
{
	m_iSuperStatus = STATUS_SUPER_INIT_TOOL_LENGTH;
	// Go back to origin (while sensors are not engaged)
	m_dManualX = -1000000;
	m_dManualY = -1000000;
	m_dManualZ = m_dPosZ;
	m_iStatus = STATUS_MANUAL_MOVE;;

	m_pPort->EnableMotors(MOTOR_ALL);
}


void KDriver::ToNextStep()
{
	if ((m_bPause) || (m_iStatus == STATUS_FINISHED))
		return;

	if ((NULL == m_pEntityCur) && (m_iPathCur < KPath::GetCount()))
	{
		KPath::GetPath(m_iPathCur)->Init();
		m_pEntityCur = KPath::GetPath(m_iPathCur)->GetEntity();

		KPosZ *pPos = GetPosZ(m_pEntityCur->GetColor());
		while ((pPos == NULL) || ((pPos != NULL) && (pPos->m_fPosZStart < 0)) && (m_iPathCur < KPath::GetCount()))
		{
			m_pEntityCur = m_pEntityCur->GetNextEntityToDo();
			if (NULL == m_pEntityCur)
			{
				m_iPathCur++;

				if (m_iPathCur < KPath::GetCount())
				{
					KPath::GetPath(m_iPathCur)->Init();
					m_pEntityCur = KPath::GetPath(m_iPathCur)->GetEntity();
				}
				else
				{
					// No more path to do
					break;
				}
			}

			if (NULL != m_pEntityCur)
			{
				pPos = GetPosZ(m_pEntityCur->GetColor());
			}
		}
		
		if (m_iPathCur >= KPath::GetCount())
		{
			if (m_iStatus == STATUS_WORKING)
			{
				LOG("No more path. Was Working. Status changed to MOVE_UP");
				m_iStatus = STATUS_MOVE_UP;
			}
			else
			if (m_iStatus != STATUS_MOVE_UP)
			{
				LOG("No more path. Was not working. Status changed to FINISHED");
				Finish();
			}
		}
		else
		{
			if (pPos != NULL)
			{
				m_dPosZWork		= pPos->m_fPosZStart;
				m_dPosZWorkEnd	= pPos->m_fPosZEnd;
				m_dPosZWorkInc	= pPos->m_fPosZInc;
				KPath::GetPath(m_iPathCur)->SetStartFromEnd(pPos->m_bReverse);
				m_pEntityCur = KPath::GetPath(m_iPathCur)->GetEntity();
			
				// Init at first work deep
				m_dPosZWork -= m_dPosZWorkInc;
				if (m_dPosZWork < m_dPosZWorkEnd)
					m_dPosZWork = m_dPosZWorkEnd;

				// Do not drill -> stay above the matter (no work)
				if (!IsDrill())
				{
					m_dPosZWork = pPos->m_fPosZStart + m_dConfig[CONFIG_DRV_POS_Z_MOVE_MARGIN];
					m_dPosZWorkEnd = m_dPosZWork;
					m_dPosZWorkInc = 0;
				}
			}

			LOG2("Entity #%d : PosZ Work = %f", m_pEntityCur->GetId(), m_dPosZWork);
		}
	}

	double dX = m_dPosX;
	double dY = m_dPosY;
	double dZ = m_dPosZ;
	double dNorm = 0;
	bool bArrived = false;

	switch(m_iStatus)
	{
	case STATUS_MOVE_DOWN :
		if (!MoveTo(m_dPosX, m_dPosY, (m_dPosZ > m_dPosZWork) ? m_dPosZ-m_dResZ : (m_dPosZ < m_dPosZWork) ? m_dPosZ+m_dResZ : m_dPosZ, m_dConfig[CONFIG_DRV_SPEED_WORK]))
		{
			if (m_iSuperStatus == STATUS_SUPER_INIT_TOOL_LENGTH)
			{
				if (m_pPort->IsZTool())
				{
//					InitXY();
					InitZ(0);
					LOG("Tool found - Reinit X,Y,Z to zero.");
					Finish();
				}
				else
				if (m_pPort->IsZStart())
				{
					LOG("Tool not found (Z MOVE) !!");
					Finish();
				}

			}
			else
			{
				LOG1("\nError while STATUS_MOVE_DOWN at start of path %d\n", m_iPathCur);
				m_iStatus = STATUS_ERROR;
			}
		}
		else
		if (IsArrived(m_dPosX, m_dPosY, m_dPosZWork))
		{
			if (m_iSuperStatus == STATUS_SUPER_INIT_TOOL_LENGTH)
			{
				LOG("Tool not found (XY MOVE) !!");
				Finish();
			}
			else
			{
				LOG1("Starting working on path %d. Status changed to WORKING", m_iPathCur);
				m_iStatus = STATUS_WORKING;
			}
		}

		break;

	case STATUS_MOVE_UP :
		if (!MoveTo(m_dPosX, m_dPosY, (m_dPosZ > m_dPosZMove) ? m_dPosZ-m_dResZ : (m_dPosZ < m_dPosZMove) ? m_dPosZ+m_dResZ : m_dPosZ, m_dConfig[CONFIG_DRV_SPEED_MOVE]))
		{
			LOG1("\nError while STATUS_MOVE_UP at end of path %d\n", m_iPathCur);
			m_iStatus = STATUS_ERROR;
		}
		else
		if (IsArrived(m_dPosX, m_dPosY, m_dPosZMove))
		{
			if (m_iPathCur >= KPath::GetCount())
			{
				Finish();
				LOG("\nSTATUS_MOVE_UP complete. Status changed to FINISHED\n");
			}
			else
			{
				m_iStatus = STATUS_MOVE_TO;
				LOG1("\nGoing to StartPoint of path %d. Status changed to MOVE_TO\n", m_iPathCur);
			}
		}
		break;

	case STATUS_WORKING :
		
		bArrived = m_pEntityCur->ComputeNextPosition(dX, dY, m_dResMin);
		if (bArrived)
		{
			// Send the last step
			if (!MoveTo(dX, dY, m_dPosZ, m_dConfig[CONFIG_DRV_SPEED_WORK]))
			{
				m_iStatus = STATUS_ERROR;
				LOG("Error while WORKING");
			}
			else
			{

				LOG1("\nEntity #%d complete", m_pEntityCur->GetId());
				m_pEntityCur = m_pEntityCur->GetNextEntityToDo();
				if (NULL != m_pEntityCur)
				{
					m_pEntityCur->Init();
					LOG1("Next entity : %d", m_pEntityCur->GetId());
				}
				else
				{
					if (m_dPosZWork <= m_dPosZWorkEnd)
					{
						LOG("Path terminated. Status changed to MOVE_UP");
						m_iStatus = STATUS_MOVE_UP;
						m_iPathCur++;
					}
					else
					{
						// Increment deep pass						
						m_dPosZWork -= m_dPosZWorkInc;
						
						// Limit to m_dPosZWorkEnd
						if (m_dPosZWork < m_dPosZWorkEnd)
							m_dPosZWork = m_dPosZWorkEnd;

						LOG1("Path terminated. Increasing deep to %f. Status changed to MOVE_DOWN", m_dPosZWork);

						m_iStatus = STATUS_MOVE_DOWN;

						if (KPath::GetPath(m_iPathCur)->IsClosed())
						{
							LOG1("Path %d is closed : Status changed to STATUS_MOVE_DOWN", m_iPathCur);
							// Repeat the same path with the new PosZ from the beginning.
						}
						else
						{
							// Path is not a closed figure :
							// Go back from the end
							LOG1("Path %d is not closed : Status changed to STATUS_MOVE_DOWN, going back", m_iPathCur);

							KPath::GetPath(m_iPathCur)->ToggleStartFromEnd();
						}

						m_pEntityCur = KPath::GetPath(m_iPathCur)->GetEntity();
						m_pEntityCur->Init();

						LOG1("Continue on entity #%d", m_pEntityCur->GetId());
					}
				}
			}
		}
		else
		{
			if (!MoveTo(dX, dY, m_dPosZ, m_dConfig[CONFIG_DRV_SPEED_WORK]))
			{
				m_iStatus = STATUS_ERROR;
				LOG("Error while WORKING");
			}
		}
		break;

	case STATUS_MOVE_TO :
		m_pEntityCur->GetStartPos(dX, dY);
		
//		LOG4("StartPos %f %f ; CurPos %f %f", dX, dY, m_dPosX, m_dPosY);

		dX -= m_dPosX;
		dY -= m_dPosY;

		dNorm = sqrt(dX*dX + dY*dY);
		if (dNorm < m_dResMin)
		{
			LOG("Arrived (Norm < ResMin). Status changed to MOVE_DOWN");
			m_iStatus = STATUS_MOVE_DOWN;
			break;
		}

		dX *= m_dResMin;
		dY *= m_dResMin;

		dX /= dNorm;
		dY /= dNorm;

		dX += m_dPosX;
		dY += m_dPosY;

//		LOG3("NextPos %f %f (res %f)", dX, dY, m_dResMin);

		if (!MoveTo(dX, dY, m_dPosZ, m_dConfig[CONFIG_DRV_SPEED_MOVE]))
		{
			m_iStatus = STATUS_ERROR;
			LOG("Error while MOVE_TO");
		}

		m_pEntityCur->GetStartPos(dX, dY);

		if (IsArrived(dX, dY, m_dPosZ))
		{
			LOG("Arrived at StartPos. Status changed to MOVE_DOWN");
			m_iStatus = STATUS_MOVE_DOWN;
		}
		break;

	case STATUS_MANUAL_MOVE:
		dX = m_dManualX;
		dY = m_dManualY;
		dZ = m_dManualZ;
		
//		LOG4("CurPos %f %f ; Going to %f %f", m_dPosX, m_dPosY, dX, dY);

		dX -= m_dPosX;
		dY -= m_dPosY;
		dZ -= m_dPosZ;

		dNorm = sqrt(dX*dX + dY*dY + dZ*dZ);

		if (dNorm < m_dResMin)
		{
			LOG("Already at destination.");
			m_iStatus = STATUS_FINISHED;
			break;
		}

		dX *= m_dResMin;
		dY *= m_dResMin;
		dZ *= m_dResMin;

		dX /= dNorm;
		dY /= dNorm;
		dZ /= dNorm;

		dX += m_dPosX;
		dY += m_dPosY;
		dZ += m_dPosZ;

//		LOG3("NextPos %f %f (res %f)", dX, dY, m_dResMin);

		if (!MoveTo(dX, dY, dZ, m_dConfig[CONFIG_DRV_SPEED_MOVE]))
		{
			if (m_iSuperStatus == STATUS_SUPER_INIT_TOOL_LENGTH)
			{
				if (m_pPort->IsXStart() && m_pPort->IsYStart())
				{
					// Arrived at origin
					// Move down until Tool sensor is engaged

					m_iStatus = STATUS_MOVE_DOWN;
					m_dPosZWork = -100000;
				}
			}
			else
			{
				m_iStatus = STATUS_ERROR;
				LOG("Error while MOVE_TO_MANUAL");
			}
		}
		else // Already at position ?
			if (m_iSuperStatus == STATUS_SUPER_INIT_TOOL_LENGTH)
			{
				if (m_pPort->IsXStart() && m_pPort->IsYStart())
				{
					// Arrived at origin
					// Move down until Tool sensor is engaged

					m_iStatus = STATUS_MOVE_DOWN;
					m_dPosZWork = -100000;
				}
			}

		dX = m_dManualX;
		dY = m_dManualY;
		dZ = m_dManualZ;

		if (IsArrived(dX, dY, dZ))
		{
			LOG("Arrived at StartPos. Status changed to FINISHED");
			m_iStatus = STATUS_FINISHED;

			m_pPort->EnableMotors(MOTOR_NONE);
		}
		break;

	default:
		m_iStatus = STATUS_ERROR;
	}
}


bool KDriver::MoveTo(double dX, double dY, double dZ, double dSpeedMmPerSec)
{
//	LOG6("Path %04d ; Entity #%04d ; Status = %02d ; MoveTo %f %f %f", m_iPathCur, (m_pEntityCur != NULL) ? m_pEntityCur->GetId() : 9999, m_iStatus, dX, dY, dZ);

	ComputeAxis(dX, dY, dZ);

	double dStepX = m_dPosAxeX / m_dResX;
	double dStepY = m_dPosAxeY / m_dResY;
	double dStepZ = m_dPosAxeZ / m_dResZ;

	// Sometimes, double divisions makes limits :
	// ex : (0.1 + 0.1 + 0.1 + 0.1 + 0.1) / 5 < 0.1
	// So add 0.5 before truncation (cast double -> int)
	if (dStepX >= 0)
		dStepX += 0.5;
	else
		dStepX -= 0.5;

	if (dStepY >= 0)
		dStepY += 0.5;
	else
		dStepY -= 0.5;

	if (dStepZ >= 0)
		dStepZ += 0.5;
	else
		dStepZ -= 0.5;

	if ((abs((int)dStepX - m_iStepX) >= 2) ||
		(abs((int)dStepY - m_iStepY) >= 2) ||
		(abs((int)dStepZ - m_iStepZ) >= 2))
	{
		LOG6("MoveTo :     step %f %f %f  (int) %d %d %d", dStepX, dStepY, dStepZ, (int)dStepX, (int)dStepY, (int)dStepZ);
		LOG6("      -> cur step %d %d %d ; %f %f %f", m_iStepX, m_iStepY, m_iStepZ, dStepX - (double)m_iStepX, dStepY - (double)m_iStepY, dStepZ - (double)m_iStepZ);
		LOG("MoveTo overflow !");
		m_iStatus = STATUS_ERROR;
		return false;
	}

	int iDeltaX = (int)(dStepX) - m_iStepX;
	int iDeltaY = (int)(dStepY) - m_iStepY;
	int iDeltaZ = (int)(dStepZ) - m_iStepZ;

	bool bComplete = true;
	bool bX = true;
	bool bY = true;
	bool bZ = true;

	if ((m_pPort->IsXStart()) && (iDeltaX < 0))
	{
		bX = false;
		bComplete = false;
		iDeltaX = 0;
	}

	if ((m_pPort->IsXStop()) && (iDeltaX > 0))
	{
		bX = false;
		bComplete = false;
		iDeltaX = 0;
	}

	if ((m_pPort->IsYStart()) && (iDeltaY < 0))
	{
		bY = false;
		bComplete = false;
		iDeltaY = 0;
	}

	if ((m_pPort->IsYStop()) && (iDeltaY > 0))
	{
		bY = false;
		bComplete = false;
		iDeltaY = 0;
	}

	if (((m_pPort->IsZStart()) || ((m_pPort->IsZTool()) && (m_iSuperStatus == STATUS_SUPER_INIT_TOOL_LENGTH))) && (iDeltaZ < 0))
	{
		bZ = false;
		bComplete = false;
		iDeltaZ = 0;
	}

	if ((m_pPort->IsZStop()) && (iDeltaZ > 0))
	{
		bZ = false;
		bComplete = false;
		iDeltaZ = 0;
	}

	if (bX)
	{
		m_iStepX = (int)(dStepX);
		m_dPosX = dX;
	}

	if (bY)
	{
		m_iStepY = (int)(dStepY);
		m_dPosY = dY;
	}

	if (bZ)
	{
		m_iStepZ = (int)(dStepZ);
		m_dPosZ = dZ;
	}
	
	if ((iDeltaX != 0) || (iDeltaY != 0) || (iDeltaZ != 0))
	{
		// Init current tick before adding diffs
		
		if (!MicroMove(iDeltaX, iDeltaY, iDeltaZ))
			bComplete = false;
		
		double dStepPerSec = dSpeedMmPerSec / m_dResMin;
		double dNanos = 1000000000.0 / dStepPerSec;
		
		double dTicks = GetTickCountSinceLastCall();
		while (dTicks < dNanos)
		{
			dTicks += GetTickCountSinceLastCall();
		}

//		LOG2("Step duration : %f ns ; Ticks = %f", dNanos, dTicks);
		
		if (!m_pPort->IsSimul())
		{
			// Update inputs after step delay
			m_pPort->Input(iDeltaX, iDeltaY, iDeltaZ);
		}	
	}

	return bComplete;
}


void KDriver::LongMoveTo(double dX, double dY, double dZ)
{
	m_dManualX = dX;
	m_dManualY = dY;
	m_dManualZ = dZ;

	m_iStatus = STATUS_MANUAL_MOVE;

	m_pPort->EnableMotors(MOTOR_ALL);
}


bool KDriver::MicroMove(int iX, int iY, int iZ)
{
	bool bComplete = true;

	if (m_pPort->IsEmergencyStop())
		return false;

	if ((iX != 0) || (iY != 0) || (iZ != 0))
	{
		m_pPort->Output((iX != 0), ((iX > 0) ^ (int)m_dConfig[CONFIG_DRV_INV_X]) != 0, 
						(iY != 0), ((iY > 0) ^ (int)m_dConfig[CONFIG_DRV_INV_Y]) != 0, 
						(iZ != 0), ((iZ > 0) ^ (int)m_dConfig[CONFIG_DRV_INV_Z]) != 0, 
						false, false);

//		LOG6("MicroMove %d %d %d (%f %f %f)", iX, iY, iZ, m_dPosX, m_dPosY, m_dPosZ);
	}

	return bComplete;
}


bool KDriver::IsArrived(double x, double y, double z)
{
	if ((fabs(x - m_dPosX) < m_dResX) &&
		(fabs(y - m_dPosY) < m_dResY) &&
		(fabs(z - m_dPosZ) < m_dResZ))
	{
		return true;
	}

	return false;
}


KDriver::KPosZ* KDriver::GetPosZ(int iColor)
{
	int i = 0;
	KPosZ *pPos = m_pPosZ;

	while (i < m_iColorCount)
	{
		if (pPos->m_iColor == iColor)
		{
			return pPos;
		}

		pPos++;
		i++;
	}

	return NULL;
}