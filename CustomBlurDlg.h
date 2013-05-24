#pragma once


// CCustomBlurDlg dialog

class CCustomBlurDlg : public CDialog
{
	DECLARE_DYNAMIC(CCustomBlurDlg)

public:
	CCustomBlurDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCustomBlurDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG3 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	int m_arr0;
	int m_arr1;
	int m_arr2;
	int m_arr3;
	int m_arr4;
	int m_arr5;
	int m_arr6;
	int m_arr7;
	int m_arr8;
};
