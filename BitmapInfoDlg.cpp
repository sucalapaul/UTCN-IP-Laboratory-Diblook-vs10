// BitmapInfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "diblook.h"
#include "BitmapInfoDlg.h"
#include "afxdialogex.h"


// CBitmapInfoDlg dialog

IMPLEMENT_DYNAMIC(CBitmapInfoDlg, CDialog)

CBitmapInfoDlg::CBitmapInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBitmapInfoDlg::IDD, pParent)
	, m_LUT(_T(""))
	, m_BPS(_T(""))
	, m_Width(_T(""))
	, m_Height(_T(""))
{

}

CBitmapInfoDlg::~CBitmapInfoDlg()
{
}

void CBitmapInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_LUT);
	DDX_Text(pDX, IDC_STATIC_BPS, m_BPS);
	DDX_Text(pDX, IDC_STATIC_WIDTH, m_Width);
	DDX_Text(pDX, IDC_STATIC_HEIGHT, m_Height);
}


BEGIN_MESSAGE_MAP(CBitmapInfoDlg, CDialog)
END_MESSAGE_MAP()


// CBitmapInfoDlg message handlers
