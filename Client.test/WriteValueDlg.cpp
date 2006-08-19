// WriteValueDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Client.test.h"
#include "WriteValueDlg.h"
#include ".\writevaluedlg.h"

VARIANT CreateVariant(CString str, VARIANT vtype)
{
	CComVariant ret; ret.Clear();
	ret.vt = vtype.vt;

	switch(ret.vt) {
		case VT_BSTR: 
			ret.bstrVal = CComBSTR(str).Detach();	
			break;
		case VT_I1:  
			ret.bVal = (BYTE)_tstoi(str);	
			break;
		case VT_I2:  
			ret.iVal = (short)_tstoi(str);	
			break;
		case VT_I4:  
			ret.lVal = (long)_tstol(str);	
			break;
		case VT_I8:  
			ret.llVal = (LONGLONG)_tstoi64(str);	
			break;
		case VT_R4:  
			ret.fltVal = (float)_tstof(str);	
			break;
		case VT_R8:  
			ret.dblVal = _tstof(str);	
			break;
		case VT_UI1: 
			ret.bVal = (BYTE)_tstoi(str);	
			break;
		case VT_UI2: 
			ret.uiVal = (USHORT)_tstoi(str);	
			break;
		case VT_UI4: 
			ret.ulVal = _tstol(str);	
			break;
		case VT_UI8: 
			ret.ullVal = _tstoi64(str);	
			break;
		default:
			ret.vt = VT_EMPTY;
	}

	return ret;
}

// CWriteValueDlg dialog

IMPLEMENT_DYNAMIC(CWriteValueDlg, CDialog)
CWriteValueDlg::CWriteValueDlg(OPCClient &client, CWnd* pParent /*=NULL*/)
	: CDialog(CWriteValueDlg::IDD, pParent)
	, m_OPCClient(client)
	, m_TagName(_T(""))
	, m_TagValue(_T(""))
	, m_tag_client_id(0)
{
}

CWriteValueDlg::~CWriteValueDlg()
{
}

void CWriteValueDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_TAG_NAME, m_TagName);
	DDX_Text(pDX, IDC_TAG_VALUE, m_TagValue);
}


BEGIN_MESSAGE_MAP(CWriteValueDlg, CDialog)
	ON_BN_CLICKED(IDC_WRITE, OnBnClickedWrite)
END_MESSAGE_MAP()


// CWriteValueDlg message handlers

void CWriteValueDlg::OnBnClickedWrite()
{
	UpdateData(TRUE);

	if (m_tag_client_id == 0) {
		AfxMessageBox("Can't write value: invalid client handle");
		return;
	}

	FILETIME now_time;
	CoFileTimeNow(&now_time);

	VARIANT value = CreateVariant(m_TagValue, m_last_value);
	if (value.vt == VT_EMPTY) {
		AfxMessageBox("Coudn't convert value to required type");
		return;
	}

	if (! m_OPCClient.WriteValue(m_tag_client_id, now_time, value, QUAL_GOOD)) {
		AfxMessageBox("Coudn't write value to server");
		return;
	}
}
