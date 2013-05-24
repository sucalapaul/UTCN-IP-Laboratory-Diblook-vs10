// Hystogram.cpp : implementation file
//

#include "stdafx.h"
#include "diblook.h"
#include "Hystogram.h"
#include "afxdialogex.h"


// CHystogram dialog

IMPLEMENT_DYNAMIC(CHystogram, CDialogEx)

CHystogram::CHystogram(CWnd* pParent /*=NULL*/)
	: CDialogEx(CHystogram::IDD, pParent)
{

}

CHystogram::~CHystogram()
{
}

void CHystogram::DoDataExchange(CDataExchange* pDX)
{
	CStatic::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CHystogram, CStatic)
END_MESSAGE_MAP()


// CHystogram message handlers
