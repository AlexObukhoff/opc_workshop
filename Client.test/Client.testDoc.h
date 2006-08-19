// Client.testDoc.h : interface of the CClienttestDoc class
//


#pragma once

struct tagDsc
{
	CString tagName;				// Строка имени тэга
	OPCHANDLE client_handle;	// 
	int itemNo;					//  Индекс тэга в списке тэгов в ListView

	FILETIME last_time;
	VARIANT  last_value;
	WORD	 last_quality;
};

class CClienttestDoc : public CDocument,
	public COPCReceiveData
{
protected: // create from serialization only
	CClienttestDoc();
	DECLARE_DYNCREATE(CClienttestDoc)

// Attributes
public:
	OPCClient m_OPCClient;

	// Теги, на которые мы уже подписались
	// Слева - хэндл ОРС, справа - номер item в ListCtrl
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

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	
	// Устанавливаем выделение. Передаем номер итема в ListCtrl
	void SetSelection(int item_index);

// Implementation
public:
	virtual ~CClienttestDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	vector<tagDsc> m_TagList;
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

	afx_msg void OnRefreshValue();

	virtual void OnCloseDocument();
};


