#pragma once


// DlgHistogram dialog

class DlgHistogram : public CDialog
{
	DECLARE_DYNAMIC(DlgHistogram)

public:
	DlgHistogram(CWnd* pParent = NULL);   // standard constructor
	virtual ~DlgHistogram();

// Dialog Data
	enum { IDD = IDD_DIALOG2 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
