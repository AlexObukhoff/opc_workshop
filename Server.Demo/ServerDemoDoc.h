// Server Demo document class
//

#pragma once

#include <string>
#include <vector>

#include "..\Server.dll\OPCServerObject.h"

class CServerDemoDoc;

class DemoDataReceiver :
	public COPCReceiveData
{
public:

	CServerDemoDoc *pDoc; // действительный обработчик событий

	virtual void newData(LPCTSTR name, DWORD /*clientId*/, FILETIME &/*time*/, VARIANT &value, WORD Quality );
	/// after calls newData, called newItemIsReceived
	virtual void newItemIsReceived( DWORD count ) { count; }
	virtual void log_message( LogLevel fatal, const char* fmt, ... )
	{
		switch( fatal )
		{
		case LogDebugInfo:	printf("DBG:\t"); break;
		case LogMessage:	printf("LOG:\t"); break;
		case LogWarning:	printf("WAR:\t"); break;
		case LogError:		printf("ERR:\t"); break;
		};
//		static char buffer[4096];
		va_list args;
		va_start(args, fmt);
//			vsprintf (buffer, fmt, args);
			vprintf (fmt, args);
		va_end(args);
	}

	virtual void StatusChanged( int , const char * ) { };

};

class DemoTagDescriptor
{
public:
	std::string tag_name;
	std::string tag_value;
	VARENUM tag_type;
	int itemNo;
};

#define DEMO_CMD_MASK			0xF0000000
#define DEMO_TAGNO_MASK			0x0FFFFFFF
#define	DEMO_CMD_ADD_TAG		0x10000000
#define DEMO_CMD_EDIT_TAG		0x20000000
#define DEMO_CMD_DELETE_TAG		0x30000000
#define DEMO_CMD_VALUE_CHANGED	0x40000000

class CServerDemoDoc : public CDocument
{
protected: 
	CServerDemoDoc(); // create from serialization only
	DECLARE_DYNCREATE(CServerDemoDoc)

// Attributes
public:

	DemoTagDescriptor *selection;
protected:
	void AddTag(DemoTagDescriptor &tag_dsc);

	// OPC server

	// define server object 
	COPCServerObject *server;

	DemoDataReceiver	receiver;
// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	
	// Устанавливаем выделение. Передаем номер итема в ListCtrl
	void SetSelection(int item_index);
	int GetSelection(void) {
		if (selection == NULL) {
			return -1;
		} else {
			return selection->itemNo;
		}
	}

// Implementation
public:
	virtual ~CServerDemoDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	std::vector<DemoTagDescriptor> m_TagList;
protected:
	int AddTag(CString & tagName);
	int ConnectAndCreateGroup(CString &ServerName, CString &GroupName);

	void LoadFile(const char *fname);
	void SaveFile(const char *fname);
// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnAddTag();
	afx_msg void OnUpdateAddTag(CCmdUI *pCmdUI);
	afx_msg void OnDeleteTag();
	afx_msg void OnUpdateDeleteTag(CCmdUI *pCmdUI);
	afx_msg void OnEditTag();
	afx_msg void OnUpdateEditTag(CCmdUI *pCmdUI);
	afx_msg void OnOpenCsv();
	afx_msg void OnUpdateOpenCsv(CCmdUI *pCmdUI);
	afx_msg void OnSaveCsv();
	afx_msg void OnUpdateSaveCsv(CCmdUI *pCmdUI);
	virtual void OnCloseDocument();

	void DataChanged(LPCTSTR name, VARIANT &value, WORD quality);
};


