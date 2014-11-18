#include "stdafx.h"
#include "DXFReader.h"
#include "Entity.h"
#include "Log.h"



bool KDXFReader::OpenFile(char *szName)
{
	m_fHandle = fopen(szName, "r");
	return (m_fHandle != NULL);
}


void KDXFReader::CloseFile()
{
	if (NULL == m_fHandle)
		return;
	
	fclose(m_fHandle);
	m_fHandle = NULL;
}


bool KDXFReader::ReadItem()
{
	m_bRead = false;
	memset(m_szKey, 0, MAX_LINE);
	memset(m_szVal, 0, MAX_LINE);
	if (fgets(m_szKey, MAX_LINE, m_fHandle) != NULL)
	{
		StringTrim(m_szKey, MAX_LINE);
		if (fgets(m_szVal, MAX_LINE, m_fHandle) != NULL)
		{
			StringTrim(m_szVal, MAX_LINE);
			m_bRead = true;
		}
	}
	
	return m_bRead;
}


void KDXFReader::ReadArc()
{
	KArc *p = KEntityFactory::BuildArc();

	while (m_bRead)
	{
		m_bRead = ReadItem();

		if (strcmp(m_szKey, KEY_KEYWORD) == 0)
			break;

		p->Parse(this);
	}

	p->ParseDone();

}


void KDXFReader::ReadCircle()
{
	KCircle *p = KEntityFactory::BuildCircle();

	while (m_bRead)
	{
		m_bRead = ReadItem();

		if (strcmp(m_szKey, KEY_KEYWORD) == 0)
			break;

		p->Parse(this);
	}

	p->ParseDone();

}


void KDXFReader::ReadLine()
{
	KLine *p = KEntityFactory::BuildLine();

	while (m_bRead)
	{
		m_bRead = ReadItem();

		if (strcmp(m_szKey, KEY_KEYWORD) == 0)
			break;

		p->Parse(this);
	}

	p->ParseDone();

}


void KDXFReader::ReadPoint()
{
	KPoint *p = KEntityFactory::BuildPoint();

	while (m_bRead)
	{
		m_bRead = ReadItem();

		if (strcmp(m_szKey, KEY_KEYWORD) == 0)
			break;

		p->Parse(this);
	}

	p->ParseDone();

}


int KDXFReader::Parse()
{
	m_bRead = ReadItem();
	while ((m_bRead) && ((strcmp(m_szKey, KEY_SECTION) != 0) || (strcmp(m_szVal, VAL_SECTION_ENTITIES) != 0)))
	{
		m_bRead = ReadItem();
	}

	if (!m_bRead)
	{
		return -1;
	}
	
	m_bRead = ReadItem();
	while ((m_bRead) && ((strcmp(m_szKey, KEY_KEYWORD) != 0) || (strcmp(m_szVal, VAL_KEYWORD_ENDSEC) != 0)))
	{
		if (strcmp(m_szKey, KEY_KEYWORD) == 0)
		{
			if (strcmp(m_szVal, VAL_ARC) == 0)
				ReadArc();
			else
			if (strcmp(m_szVal, VAL_CIRCLE) == 0)
				ReadCircle();
			else
			if (strcmp(m_szVal, VAL_LINE) == 0)
				ReadLine();
			else
			if (strcmp(m_szVal, VAL_POINT) == 0)
				ReadPoint();
		}
	}

	return 0;
}