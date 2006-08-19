// ServerConnectDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Client.test.h"
#include "ServerConnectDlg.h"
#include ".\serverconnectdlg.h"

//+----------------------------------------------------------------------------
//
// Function   : CCOMCategoriesComboBox::BuildList
//
// Synopsis	  : Build the list box of the combo box using ICatInformation and 
//				"(Default)" string of the COM object
//				Sets the 32-bit value associated with the specified item in
//				the combo box to be a CString's pointer containing the CLSID
//				
//----------------------------------------------------------------------------- 
int getArrayObjectNameFromCategory( GUID &category, std::map<std::string, std::string> &names )
{
	ICatInformation * pICatInformation = NULL;
	HRESULT hr = CoCreateInstance(	CLSID_StdComponentCategoriesMgr,
		NULL, 
		CLSCTX_INPROC_SERVER, 
		IID_ICatInformation, 
		(void**) &pICatInformation );

	if ( SUCCEEDED(hr) )
	{
		IEnumGUID * pIEnumGUID = 0;

		HRESULT hr = pICatInformation->EnumClassesOfCategories(
			1,
			&category, 
			0,
			0,
			&pIEnumGUID );
		if ( SUCCEEDED(hr) )
		{
			pIEnumGUID->Reset();

			ULONG lFetched = 0;
			CLSID ClsID;
			CRegKey regKey;

			LPOLESTR lpClsIDString = NULL;

			while ( S_OK == pIEnumGUID->Next( 1, &ClsID, &lFetched ) )
			{
				CString sKey( _T("CLSID\\") );
				CString pstrItemData;

				if ( S_OK == StringFromIID( ClsID, &lpClsIDString ) )
				{
					sKey += lpClsIDString;
					pstrItemData = /*new CString(*/ lpClsIDString /*)*/;
					CoTaskMemFree( lpClsIDString );
				}

				// Open the registry key
				if ( ERROR_SUCCESS == regKey.Open(	HKEY_CLASSES_ROOT, 
					sKey,
					KEY_QUERY_VALUE) )
				{
					ULONG	dwCount = 256;
					TCHAR	sValue[256] = {0};

					// Read "(Default)" string value
					if ( ERROR_SUCCESS == regKey.QueryStringValue( (LPCTSTR)NULL,
						sValue,
						&dwCount ) )
					{
						// names[ Name ] = CLSID;
						CLSID clsid = CLSID_NULL;
						CLSIDFromString( CT2W(pstrItemData), &clsid );
						LPOLESTR progID = NULL;
						ProgIDFromCLSID( clsid, &progID );
						names.insert( std::pair<string,string>( (LPCTSTR)sValue, (LPCTSTR)CW2A(progID) ) );
					}

					regKey.Close();
				}					
			}

			pIEnumGUID->Release();
		}

		pICatInformation->Release();
	}
	return names.size();
}

// CServerConnectDlg dialog

IMPLEMENT_DYNAMIC(CServerConnectDlg, CDialog)
CServerConnectDlg::CServerConnectDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CServerConnectDlg::IDD, pParent)
	, m_GroupName(_T(""))
	, m_OPCServer(_T(""))
	, m_HostName(_T(""))
{
}

CServerConnectDlg::~CServerConnectDlg()
{
}

void CServerConnectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SELECT_SERVER, m_ServerList);
	DDX_Text(pDX, IDC_GROUP_NAME, m_GroupName);
	DDX_Text(pDX, IDC_HOST_NAME, m_HostName);
}


BEGIN_MESSAGE_MAP(CServerConnectDlg, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CServerConnectDlg message handlers

BOOL CServerConnectDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	getArrayObjectNameFromCategory( const_cast<GUID&>( CATID_OPCDAServer20 )  , m_OPCServerNames );

	CStringArray options;
	std::map<string,string>::iterator it;
	for( it = m_OPCServerNames.begin(); it != m_OPCServerNames.end(); ++it )
	{
		m_ServerList.AddString(it->second.c_str());

	}
	return TRUE;  
}

void CServerConnectDlg::OnBnClickedOk()
{
	int selection = m_ServerList.GetCurSel();

	if (selection != -1) {
		m_ServerList.GetLBText(selection, m_OPCServer);
	} else {
		m_ServerList.GetWindowText(m_OPCServer);
		if( m_OPCServer == "" ) {
			AfxMessageBox("Выберите OPC Server");
			return;
		}
	}

	OnOK();
}
