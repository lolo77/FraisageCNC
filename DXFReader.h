#ifndef __DXFREADER_H__
#define __DXFREADER_H__


#define MAX_LINE				128

#define KEY_SECTION				"2"
#define KEY_KEYWORD				"0"
#define KEY_X0					"10"
#define KEY_Y0					"20"
#define KEY_Z0					"30"
#define KEY_X1					"11"
#define KEY_Y1					"21"
#define KEY_Z1					"31"
#define KEY_RADIUS				"40"
#define KEY_ANGLE_START			"50"
#define KEY_ANGLE_END			"51"
#define KEY_COLOR				"62"

#define VAL_SECTION_ENTITIES	"ENTITIES"
#define VAL_KEYWORD_ENDSEC		"ENDSEC"
#define VAL_ARC					"ARC"
#define VAL_CIRCLE				"CIRCLE"
#define VAL_LINE				"LINE"
#define VAL_POINT				"POINT"

class KDXFReader
{
	protected:
	FILE *m_fHandle;
	bool m_bRead;
	
	public:
	
	char m_szKey[MAX_LINE];
	char m_szVal[MAX_LINE];
	
	bool OpenFile(char *szName);
	void CloseFile();
	bool ReadItem();

	void ReadArc();
	void ReadCircle();
	void ReadLine();
	void ReadPoint();
	int  Parse();
};

#endif