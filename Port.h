// Port.h: interface for the KPort class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PORT_H__5BE7B105_E4C0_496C_B60A_7CDEFC7B21CA__INCLUDED_)
#define AFX_PORT_H__5BE7B105_E4C0_496C_B60A_7CDEFC7B21CA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


enum
{
	CONFIG_OUT_X = 0,
	CONFIG_OUT_X_DIR,

	CONFIG_OUT_Y,
	CONFIG_OUT_Y_DIR,

	CONFIG_OUT_Z,
	CONFIG_OUT_Z_DIR,

	CONFIG_OUT_ROT,

	CONFIG_IN_X_START,
	CONFIG_IN_X_STOP,

	CONFIG_IN_Y_START,
	CONFIG_IN_Y_STOP,

	CONFIG_IN_Z_START,
	CONFIG_IN_Z_STOP,

	CONFIG_IN_Z_TOOL,

	CONFIG_IN_EMERGENCY_STOP,

	CONFIG_MAX
};


#define MOTOR_X	0x01
#define MOTOR_Y	0x02
#define MOTOR_Z	0x04

#define MOTOR_NONE	0
#define MOTOR_ALL	(MOTOR_X | MOTOR_Y | MOTOR_Z)

#define MOTOR_XY	(MOTOR_X | MOTOR_Y)


#define MASK_AXIS_X_START	0x01
#define MASK_AXIS_X_STOP	0x02
#define MASK_AXIS_Y_START	0x04
#define MASK_AXIS_Y_STOP	0x08
#define MASK_AXIS_Z_START	0x10
#define MASK_AXIS_Z_STOP	0x20


class KPort
{
private:

	static char m_szConfigKeys[CONFIG_MAX][16];

	HANDLE m_hPort;

	unsigned char m_cPortValue;
	unsigned char m_cPortValueOld;
	unsigned char m_cAxisSensors;	

	bool m_bSimul;

	unsigned char m_iConfig[CONFIG_MAX];

public:

	void LoadStatus(FILE *f);
	void SaveStatus(FILE *f);

	bool IsCarteCNC3AX();
	bool IsSimul()				{ return m_bSimul;	}

	void ResetCapteurs()		{ m_cAxisSensors = 0; }

	void EnableMotors(int iMotors = MOTOR_ALL); // Combine defines MOTOR_x

	void Output(bool bX, bool bXDir, bool bY, bool bYDir, bool bZ, bool bZDir, bool bRotChange, bool bRotValue);

	void Input(int iDeltaX = 0, int iDeltaY = 0, int iDeltaZ = 0);

	bool IsBitPortSet(int iFlag)	{ if (m_iConfig[iFlag] == 0xFF) return false; else return ((m_cPortValue & m_iConfig[iFlag]) != 0);	}
	bool IsBitSensorSet(int iMask)	{ return ((m_cAxisSensors & iMask) != 0);	}

	bool IsXStart()				{ return IsBitSensorSet(MASK_AXIS_X_START);		}
	bool IsXStop()				{ return IsBitSensorSet(MASK_AXIS_X_STOP);		}

	bool IsYStart()				{ return IsBitSensorSet(MASK_AXIS_Y_START);		}
	bool IsYStop()				{ return IsBitSensorSet(MASK_AXIS_Y_STOP);		}

	bool IsZStart()				{ return IsBitSensorSet(MASK_AXIS_Z_START);		}
	bool IsZStop()				{ return IsBitSensorSet(MASK_AXIS_Z_STOP);		}

	bool IsZTool()				{ return IsBitPortSet(CONFIG_IN_Z_TOOL);		}

	bool IsEmergencyStop()		{ return IsBitPortSet(CONFIG_IN_EMERGENCY_STOP);}

	KPort(int iDevice);
	virtual ~KPort();

	int LoadConfig(char *szConfigFileName);
};



#endif // !defined(AFX_PORT_H__5BE7B105_E4C0_496C_B60A_7CDEFC7B21CA__INCLUDED_)
