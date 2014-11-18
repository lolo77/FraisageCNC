// Port.cpp: implementation of the KPort class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Port.h"
#include "DXFReader.h"
#include "Log.h"

/*

	LPT Pin -> Reg/Bit reference

    Pin		Register	Name
    1		/C0			Control bit 0 (inverted)
	2		D0			Data bit 0
	3		D1			Data bit 1
	4		D2			Data bit 2
	5		D3			Data bit 3
	6		D4			Data bit 4
	7		D5			Data bit 5
	8		D6			Data bit 6
	9		D7			Data bit 7
	10		S6			Status bit 6
	11		/S7			Status bit 7 (inverted)
	12		S5			Status bit 5
	13		S4			Status bit 4
	14		/C1			Control bit 1 (inverted)
	15		S3			Status bit 3
	16		C2			Control bit 2
	17		/C3			Control bit 3 (inverted)
	18 to 25			Ground (masse)




	LPT Reg/Bit -> Pin reference

    Pin		Register	Name
    1		/C0			Control bit 0 (inverted)
	14		/C1			Control bit 1 (inverted)
	16		C2			Control bit 2
	17		/C3			Control bit 3 (inverted)
<none>		C4			0
<none>		C5			0
<none>		C6			0
<none>		C7			0

	2		D0			Data bit 0
	3		D1			Data bit 1
	4		D2			Data bit 2
	5		D3			Data bit 3
	6		D4			Data bit 4
	7		D5			Data bit 5
	8		D6			Data bit 6
	9		D7			Data bit 7

<none>		S0			0
<none>		S1			0
<none>		S2			0  
	15		S3			Status bit 3
	13		S4			Status bit 4
	12		S5			Status bit 5
	10		S6			Status bit 6
	11		/S7			Status bit 7 (inverted)

*/

#define IOCTL_PP_WRITE_DATA         0x002C0004
#define IOCTL_PP_WRITE_CONTROL      0x002C0008
#define IOCTL_PP_READ_STATUS        0x002C000C

#define PP_DEVICE_NAME              L"\\\\.\\$VDMLPT%d"
#define PP_DEVICE_NAME_SIZE         (sizeof(PP_DEVICE_NAME)+13*sizeof(WCHAR))

#define PP_OFFSET_DATA      0
#define PP_OFFSET_STATUS    1
#define PP_OFFSET_CONTROL   2


HANDLE __inline WINAPI PPOpen(ULONG DeviceId)
{
    WCHAR DeviceName[PP_DEVICE_NAME_SIZE];
    HANDLE h;

    wsprintfW(DeviceName,PP_DEVICE_NAME,DeviceId+1);

    h = CreateFileW(
        DeviceName,
        GENERIC_READ|GENERIC_WRITE,
        FILE_SHARE_READ|FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    if (INVALID_HANDLE_VALUE==h) {
        h = NULL;
    }

    return h;
}


BOOL __inline WINAPI PPWrite(HANDLE hDevice,UCHAR Offset,UCHAR Value)
{
    ULONG CtlCode;
    ULONG Transferred;
    ULONG FakeByteForDriverBug;

    if ( (Offset!=PP_OFFSET_DATA) && (Offset!=PP_OFFSET_CONTROL) ) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    CtlCode = (Offset==PP_OFFSET_DATA) ? IOCTL_PP_WRITE_DATA : IOCTL_PP_WRITE_CONTROL;

    return DeviceIoControl(
        hDevice,
        CtlCode,
        &Value,
        sizeof(Value),
        &FakeByteForDriverBug,
        sizeof(FakeByteForDriverBug),
        &Transferred,
        NULL);
}


BOOL __inline WINAPI PPRead(HANDLE hDevice,PUCHAR pValue)
{
    ULONG Transferred;

    return DeviceIoControl(
        hDevice,
        IOCTL_PP_READ_STATUS,
        NULL,
        0,
        pValue,
        sizeof(UCHAR),
        &Transferred,
        NULL);
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

char KPort::m_szConfigKeys[CONFIG_MAX][16] = {	"X ", 
												"XDir ",
												"Y ",
												"YDir ",
												"Z ",
												"ZDir ",
												"Rot ",
												"XStart ",
												"XStop ",
												"YStart ",
												"YStop ",
												"ZStart ",
												"ZStop ",
												"ZTool ",
												"ARU "};

KPort::KPort(int iDevice)
{
	m_cPortValue = 0;
	m_cPortValueOld = 0;
	m_cAxisSensors = 0;
	m_bSimul = (iDevice < 0) ? true : false;
	m_hPort = NULL;

	if (!m_bSimul)
	{
		LOG1("### Opening LPT%d port", iDevice + 1);
		m_hPort = PPOpen(iDevice);

		if (NULL != m_hPort)
		{
			PPWrite(m_hPort, PP_OFFSET_DATA, 0x00);
			PPWrite(m_hPort, PP_OFFSET_CONTROL, 0x00);
		
			// Test présence carte CNC3AX
			if (IsCarteCNC3AX())
			{
				LOG("### CNC3AX card detected !");
				EnableMotors(MOTOR_NONE);
			}
			else
			{
				LOG("### CNC3AX card -not- detected !");
				m_bSimul = true;
			}
		}
	}
	else
	{
		LOG("### Simulation Mode");
	}

}


KPort::~KPort()
{
	if (m_hPort != NULL)
	{
		PPWrite(m_hPort, PP_OFFSET_DATA, 0);
		PPWrite(m_hPort, PP_OFFSET_CONTROL, 0);

		EnableMotors(MOTOR_NONE);

		CloseHandle(m_hPort);
		m_hPort = NULL;
	}
}


void KPort::LoadStatus(FILE *f)
{
	fread(&m_cAxisSensors, sizeof(m_cAxisSensors), 1, f);
	fread(&m_cPortValue, sizeof(m_cPortValue), 1, f);
}


void KPort::SaveStatus(FILE *f)
{
	fwrite(&m_cAxisSensors, sizeof(m_cAxisSensors), 1, f);
	fwrite(&m_cPortValue, sizeof(m_cPortValue), 1, f);
}


void KPort::EnableMotors(int iMotor)
{
	if (m_bSimul)
		return;

	unsigned char data = 0;

	if (iMotor & MOTOR_X)
	{
		LOG("### Enable motor X");
		data |= 0x01;
	}
	else
	{
		LOG("### Disable motor X");
	}

	if (iMotor & MOTOR_Y)
	{
		LOG("### Enable motor Y");
		data |= 0x02;
	}
	else
	{
		LOG("### Disable motor Y");
	}

	if (iMotor & MOTOR_Z)
	{
		LOG("### Enable motor Z");
		data |= 0x04;
	}
	else
	{
		LOG("### Disable motor Z");
	}

	// Pin C2 (0x04) only is not inverted
	// Negative logic involves inverting all the others (/C0 (0x01) and /C1 (0x02))
	data ^= 0x03;

	PPWrite(m_hPort, PP_OFFSET_CONTROL, data);
}


bool KPort::IsCarteCNC3AX()
{
	unsigned char d1, d2;

	// Test if pin 17 (C3) is connected to pin 10 (S6)

	PPWrite(m_hPort, PP_OFFSET_CONTROL, 0x00);
	Input();
	d1 = m_cPortValue;

	PPWrite(m_hPort, PP_OFFSET_CONTROL, 0x08);
	Input();
	d2 = m_cPortValue;

	return ((d1 & 0x40) == 0) && ((d2 & 0x40) != 0);
}


int KPort::LoadConfig(char *szConfigFileName)
{
	FILE *f;

	LOG("\n Port : LoadConfig\n");

	f = fopen(szConfigFileName, "r");

	if (NULL == f)
	{
		LOG1("\nOpen error on file '%s'\n", szConfigFileName);
		return -1;
	}

	int i = 0;
	memset(m_iConfig, 0xff, sizeof(m_iConfig));

	char szBuf[MAX_LINE];
	while (fgets(szBuf, MAX_LINE, f) != 0)
	{
		if (szBuf[0] == '#')
			continue;

		for (int j = 0; j < CONFIG_MAX; j++)
		{
			if (strncmp(szBuf, m_szConfigKeys[j], strlen(m_szConfigKeys[j])) == 0)
			{
				char * szVal = szBuf + strlen(m_szConfigKeys[j]);
				StringTrim(szVal, strlen(szVal));
				int iBit = -1;

				if (szVal[0] != 0)
				{

					iBit = atoi(szVal);

					if (iBit >= 0)
						m_iConfig[j] = 1 << iBit;
				}

				LOG3("Port Found Key '%s' = '%d' -> '%02X'", m_szConfigKeys[j], iBit, (unsigned char)m_iConfig[j]);
				break;
			}
		}
		
	}

	fclose(f);

	return 0;
}



void KPort::Output(bool bX, bool bXDir, bool bY, bool bYDir, bool bZ, bool bZDir, bool bRotChange, bool bRotValue)
{
	unsigned char cData = 0;
	int iMotorsEnabled = 0;

	if ((bX) && (m_iConfig[CONFIG_OUT_X] != 0xFF))
	{
		// TODO : half step motors cannot be disabled
		iMotorsEnabled |= MOTOR_X;
		cData |= m_iConfig[CONFIG_OUT_X];
	}

	if ((bXDir) && (m_iConfig[CONFIG_OUT_X_DIR] != 0xFF))
		cData |= m_iConfig[CONFIG_OUT_X_DIR];

	if ((bY) && (m_iConfig[CONFIG_OUT_Y] != 0xFF))
	{
		iMotorsEnabled |= MOTOR_Y;
		cData |= m_iConfig[CONFIG_OUT_Y];
	}

	if ((bYDir) && (m_iConfig[CONFIG_OUT_Y_DIR] != 0xFF))
		cData |= m_iConfig[CONFIG_OUT_Y_DIR];

	if ((bZ) && (m_iConfig[CONFIG_OUT_Z] != 0xFF))
	{
		iMotorsEnabled |= MOTOR_Z;
		cData |= m_iConfig[CONFIG_OUT_Z];
	}

	if ((bZDir) && (m_iConfig[CONFIG_OUT_Z_DIR] != 0xFF))
		cData |= m_iConfig[CONFIG_OUT_Z_DIR];

	if ((bRotChange) && (bRotValue) && (m_iConfig[CONFIG_OUT_ROT] != 0xFF))
		cData |= m_iConfig[CONFIG_OUT_ROT];

//	LOG1("Out %02X", (unsigned char)cData);

	if (m_bSimul)
		return;

//	EnableMotors(iMotorsEnabled);
	PPWrite(m_hPort, PP_OFFSET_DATA, (unsigned char)cData);
	PPWrite(m_hPort, PP_OFFSET_DATA, (unsigned char)0);
}



void KPort::Input(int iDeltaX, int iDeltaY, int iDeltaZ)
{
	if (m_bSimul)
		return;

	PPRead(m_hPort, (unsigned char*)&m_cPortValue);

	// Invert all but Pin 11 (/BUSY) (closed contact put zero bit)
	m_cPortValue ^= 0x7F;

	// Read twice the same info to be sure
	if (m_cPortValue != m_cPortValueOld)
	{
		m_cPortValueOld = m_cPortValue;
		return;
	}

//	LOG1("Input %02X",(unsigned char*)m_cPortValue);


	// Update X Start
	if ((m_iConfig[CONFIG_IN_X_START] != m_iConfig[CONFIG_IN_X_STOP]) && (m_iConfig[CONFIG_IN_X_START] != 0xff))
	{
		if ((m_cPortValue & m_iConfig[CONFIG_IN_X_START]) != 0)
			m_cAxisSensors |= MASK_AXIS_X_START;
		else
			m_cAxisSensors &= 0xff ^ MASK_AXIS_X_START;
	}

	// Update X Stop
	if ((m_iConfig[CONFIG_IN_X_START] != m_iConfig[CONFIG_IN_X_STOP]) && (m_iConfig[CONFIG_IN_X_STOP] != 0xff))
	{
		if ((m_cPortValue & m_iConfig[CONFIG_IN_X_STOP]) != 0)
			m_cAxisSensors |= MASK_AXIS_X_STOP;
		else
			m_cAxisSensors &= 0xff ^ MASK_AXIS_X_STOP;
	}

	// Update Y Start
	if ((m_iConfig[CONFIG_IN_Y_START] != m_iConfig[CONFIG_IN_Y_STOP]) && (m_iConfig[CONFIG_IN_Y_START] != 0xff))
	{
		if ((m_cPortValue & m_iConfig[CONFIG_IN_Y_START]) != 0)
			m_cAxisSensors |= MASK_AXIS_Y_START;
		else
			m_cAxisSensors &= 0xff ^ MASK_AXIS_Y_START;
	}

	// Update Y Stop
	if ((m_iConfig[CONFIG_IN_Y_START] != m_iConfig[CONFIG_IN_Y_STOP]) && (m_iConfig[CONFIG_IN_Y_STOP] != 0xff))
	{
		if ((m_cPortValue & m_iConfig[CONFIG_IN_Y_STOP]) != 0)
			m_cAxisSensors |= MASK_AXIS_Y_STOP;
		else
			m_cAxisSensors &= 0xff ^ MASK_AXIS_Y_STOP;
	}

	// Update Z Start
	if ((m_iConfig[CONFIG_IN_Z_START] != m_iConfig[CONFIG_IN_Z_STOP]) && (m_iConfig[CONFIG_IN_Z_START] != 0xff))
	{
		if ((m_cPortValue & m_iConfig[CONFIG_IN_Z_START]) != 0)
			m_cAxisSensors |= MASK_AXIS_Z_START;
		else
			m_cAxisSensors &= 0xff ^ MASK_AXIS_Z_START;
	}

	// Update Z Stop
	if ((m_iConfig[CONFIG_IN_Z_START] != m_iConfig[CONFIG_IN_Z_STOP]) && (m_iConfig[CONFIG_IN_Z_STOP] != 0xff))
	{
		if ((m_cPortValue & m_iConfig[CONFIG_IN_Z_STOP]) != 0)
			m_cAxisSensors |= MASK_AXIS_Z_STOP;
		else
			m_cAxisSensors &= 0xff ^ MASK_AXIS_Z_STOP;
	}
		

	// Update sensor status according to the last moving direction
	// Used if only one bit represents both start and end of an axis
	//
	// *** If a sensor is engaged at program startup, make sure
	//     its status was saved into the file "status.dat"

	// X Composite
	if ((m_iConfig[CONFIG_IN_X_START] == m_iConfig[CONFIG_IN_X_STOP]) && (m_iConfig[CONFIG_IN_X_START] != 0xff))
	{
		if ((m_cPortValue & m_iConfig[CONFIG_IN_X_START]) != 0)
		{
			// New event : sensor engaged
			if (((m_cAxisSensors & MASK_AXIS_X_START) == 0) &&
				((m_cAxisSensors & MASK_AXIS_X_STOP) == 0))
			{
				if (iDeltaX > 0)
					m_cAxisSensors |= MASK_AXIS_X_STOP;

				if (iDeltaX < 0)
					m_cAxisSensors |= MASK_AXIS_X_START;

				// == 0 -> ignore sensors
				// TODO : ask the user to know which sensor is actually engaged
			}
			else
			{
				// Event already detected
				// Nothing to do
			}
		}
		else
		{
			// New event : sensor release
			if (((m_cAxisSensors & MASK_AXIS_X_START) != 0) ||
				((m_cAxisSensors & MASK_AXIS_X_STOP) != 0))
			{
				m_cAxisSensors &= 0xff ^ MASK_AXIS_X_START;
				m_cAxisSensors &= 0xff ^ MASK_AXIS_X_STOP;
			}
		}
	}

	// Y Composite
	if ((m_iConfig[CONFIG_IN_Y_START] == m_iConfig[CONFIG_IN_Y_STOP]) && (m_iConfig[CONFIG_IN_Y_START] != 0xff))
	{
		if ((m_cPortValue & m_iConfig[CONFIG_IN_Y_START]) != 0)
		{
			// New event : sensor engaged
			if (((m_cAxisSensors & MASK_AXIS_Y_START) == 0) &&
				((m_cAxisSensors & MASK_AXIS_Y_STOP) == 0))
			{
				if (iDeltaY > 0)
					m_cAxisSensors |= MASK_AXIS_Y_STOP;

				if (iDeltaY < 0)
					m_cAxisSensors |= MASK_AXIS_Y_START;

				// == 0 -> ignore sensors
				// TODO : ask the user to know which sensor is actually engaged
			}
			else
			{
				// Event already detected
				// Nothing to do
			}
		}
		else
		{
			// New event : sensor release
			if (((m_cAxisSensors & MASK_AXIS_Y_START) != 0) ||
				((m_cAxisSensors & MASK_AXIS_Y_STOP) != 0))
			{
				m_cAxisSensors &= 0xff ^ MASK_AXIS_Y_START;
				m_cAxisSensors &= 0xff ^ MASK_AXIS_Y_STOP;
			}
		}
	}

	// Z Composite
	if ((m_iConfig[CONFIG_IN_Z_START] == m_iConfig[CONFIG_IN_Z_STOP]) && (m_iConfig[CONFIG_IN_Z_START] != 0xff))
	{
		if ((m_cPortValue & m_iConfig[CONFIG_IN_Z_START]) != 0)
		{
			// New event : sensor engaged
			if (((m_cAxisSensors & MASK_AXIS_Z_START) == 0) &&
				((m_cAxisSensors & MASK_AXIS_Z_STOP) == 0))
			{
				if (iDeltaZ > 0)
					m_cAxisSensors |= MASK_AXIS_Z_STOP;

				if (iDeltaZ < 0)
					m_cAxisSensors |= MASK_AXIS_Z_START;

				// == 0 -> ignore sensors
				// TODO : ask the user to know which sensor is actually engaged
			}
			else
			{
				// Event already detected
				// Nothing to do
			}
		}
		else
		{
			// New event : sensor release
			if (((m_cAxisSensors & MASK_AXIS_Z_START) != 0) ||
				((m_cAxisSensors & MASK_AXIS_Z_STOP) != 0))
			{
				m_cAxisSensors &= 0xff ^ MASK_AXIS_Z_START;
				m_cAxisSensors &= 0xff ^ MASK_AXIS_Z_STOP;
			}
		}
	}
}