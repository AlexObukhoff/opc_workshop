// Client.testDoc.h : interface of the CClienttestDoc class
//

#pragma once

struct tagDsc
{
	CString tagName;				// tag name string
	OPCHANDLE client_handle;	// 
	int itemNo;					//  tag index in ListView

	FILETIME last_time;
	VARIANT  last_value;
	WORD	 last_quality;

	VARTYPE type;
	int counter;
	bool active; // flag - "use this tag for loading tests"

	tagDsc() {
		counter = 0;
		active = true;
	}
};

// load-test config
//  we should inherit from CObject to allow MFC serialization of it
class CLoad_test_config : public CObject
{
//	DECLARE_SERIAL(CLoad_test_config);
public:
	CLoad_test_config();

//	virtual void Serialize(CArchive& ar);

	bool use_read_op;		// Enable read in test
	bool use_write_op;		// Enable write in test
	unsigned int timeout;	// delay between read/write groups
};

class CClienttestDoc : public CDocument,
	public COPCReceiveData
{
protected: // create from serialization only
	CClienttestDoc();
	DECLARE_DYNCREATE(CClienttestDoc)

	void StopLoadTest();
// Attributes
public:

	// tag already subscribed
	// key - ÎÐÑ handle , value - item index in ListCtrl
	map<OPCHANDLE, int> tag_list_items;

	tagDsc *selection;
protected:
	// COPCReceiveData:
	void newData(LPCTSTR name, DWORD clientID, FILETIME &time, VARIANT &value, WORD Quality );
	void log_message( LogLevel fatal, const char* fmt, ... );
	void StatusChanged( int status, const char *message );

	// COPCReceiveData-related members
	string m_LastMessage;
	int status;

	BOOL m_LoadTestRunning;

	// Load test syncronization
	HANDLE hThread;
	HANDLE hThreadStarted;
	HANDLE hStopThread;
// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	
	// Selecting. Pass item index in ListCtrl
	void SetSelection(int item_index);

	// Thread finction for load test processing
	void DoLoadTest();
// Implementation
public:
	virtual ~CClienttestDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	vector<tagDsc> m_TagList;

	CLoad_test_config m_LoadTestConfig;
protected:
	int AddTag(CString & tagName);
	int ConnectAndCreateGroup(CString &ServerName, CString &host, CString &GroupName);

	void LoadFile(const char *fname);
	void SaveFile(const char *fname);
// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnConnect();
	afx_msg void OnUpdateConnect(CCmdUI *pCmdUI);
	afx_msg void OnDisconnect();
	afx_msg void OnUpdateDisconnect(CCmdUI *pCmdUI);
	afx_msg void OnAddTag();
	afx_msg void OnUpdateAddTag(CCmdUI *pCmdUI);

	afx_msg void OnAddTagList();

	afx_msg void OnWriteValue();
	afx_msg void OnUpdateWriteValue(CCmdUI *pCmdUI);

	afx_msg void OnLoadTest();
	afx_msg void OnUpdateLoadTest(CCmdUI *pCmdUI);

	afx_msg void OnRefreshValue();

	virtual void OnCloseDocument();
};


extern OPCClient m_OPCClient;
