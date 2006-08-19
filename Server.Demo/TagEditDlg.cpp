// TagEditDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Server.Demo.h"
#include "TagEditDlg.h"
#include ".\tageditdlg.h"

// CTagEditDlg dialog

IMPLEMENT_DYNAMIC(CTagEditDlg, CDialog)

CTagEditDlg::CTagEditDlg(DemoTagDescriptor tag, CWnd* pParent /*=NULL*/)
	: CDialog(CTagEditDlg::IDD, pParent)
	, edit_tag (tag)
	, m_TagName(_T(""))
	, m_TagDatatype(0)
	, m_TagValue(_T(""))
{
}

CTagEditDlg::~CTagEditDlg()
{
}

void CTagEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_TAG_NAME, m_TagName);
	DDX_CBIndex(pDX, IDC_TAG_DATATYPE, m_TagDatatype);
	DDX_Text(pDX, IDC_TAG_VALUE, m_TagValue);
	DDX_Control(pDX, IDC_TAG_DATATYPE, m_TagTypeCtrl);
}


BEGIN_MESSAGE_MAP(CTagEditDlg, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()


// CTagEditDlg message handlers

BOOL CTagEditDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_TagName = edit_tag.tag_name.c_str();
	m_TagValue = edit_tag.tag_value.c_str();

	CWnd *list = GetDlgItem(IDC_TAG_DATATYPE);
	if (! list->IsKindOf(RUNTIME_CLASS(CComboBox))) {
		//return FALSE;
	}

	CComboBox *box = &m_TagTypeCtrl;
	int ret = box->InsertString(VT_EMPTY, "VT_EMPTY");
	ret = box->InsertString(VT_NULL, "VT_NULL");
	ret = box->InsertString(VT_I2, "VT_I2");
	ret = box->InsertString(VT_I4, "VT_I4");
	ret = box->InsertString(VT_R4, "VT_R4");
	ret = box->InsertString(VT_R8, "VT_R8");

	m_TagDatatype = edit_tag.tag_type;

	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CTagEditDlg::OnBnClickedOk()
{
	UpdateData(TRUE);

	edit_tag.tag_type = (VARENUM)m_TagDatatype;

	edit_tag.tag_name = (LPCSTR)m_TagName;
	edit_tag.tag_value = (LPCSTR)m_TagValue;

	OnOK();
}
