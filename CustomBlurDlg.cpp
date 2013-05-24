// CustomBlurDlg.cpp : implementation file
//

#include "stdafx.h"
#include "diblook.h"
#include "CustomBlurDlg.h"
#include "afxdialogex.h"


// CCustomBlurDlg dialog

IMPLEMENT_DYNAMIC(CCustomBlurDlg, CDialog)

CCustomBlurDlg::CCustomBlurDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCustomBlurDlg::IDD, pParent)
	, m_arr0(0)
	, m_arr1(0)
	, m_arr2(0)
	, m_arr3(0)
	, m_arr4(0)
	, m_arr5(0)
	, m_arr6(0)
	, m_arr7(0)
	, m_arr8(0)
{

}

CCustomBlurDlg::~CCustomBlurDlg()
{
}

void CCustomBlurDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_arr0);
	DDX_Text(pDX, IDC_EDIT2, m_arr1);
	DDX_Text(pDX, IDC_EDIT3, m_arr3);
	DDX_Text(pDX, IDC_EDIT4, m_arr3);
	DDX_Text(pDX, IDC_EDIT5, m_arr4);
	DDX_Text(pDX, IDC_EDIT6, m_arr5);
	DDX_Text(pDX, IDC_EDIT7, m_arr6);
	DDX_Text(pDX, IDC_EDIT8, m_arr7);
	DDX_Text(pDX, IDC_EDIT9, m_arr8);
}


BEGIN_MESSAGE_MAP(CCustomBlurDlg, CDialog)
END_MESSAGE_MAP()


// CCustomBlurDlg message handlers
