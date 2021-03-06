// dibview.cpp : implementation of the CDibView class
//
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1998 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"
#include "diblook.h"

#include "dibdoc.h"
#include "dibview.h"
#include "dibapi.h"
#include "mainfrm.h"
#include "dibfft.h"

#include "HRTimer.h"
#include "math.h"
#include "BitmapInfoDlg.h"
#include "DlgHistogram.h"
#include "CustomBlurDlg.h"
#include "fft.cpp"

#define PI	3.14159265358979323846	/* pi to machine precision, defined in math.h */
#define TWOPI	(2.0*PI)

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define BEGIN_PROCESSING() INCEPUT_PRELUCRARI()

#define END_PROCESSING(Title) SFARSIT_PRELUCRARI(Title)

#define INCEPUT_PRELUCRARI() \
	CDibDoc* pDocSrc=GetDocument();										\
	CDocTemplate* pDocTemplate=pDocSrc->GetDocTemplate();				\
	CDibDoc* pDocDest=(CDibDoc*) pDocTemplate->CreateNewDocument();		\
	BeginWaitCursor();													\
	HDIB hBmpSrc=pDocSrc->GetHDIB();									\
	HDIB hBmpDest = (HDIB)::CopyHandle((HGLOBAL)hBmpSrc);				\
	if ( hBmpDest==0 ) {												\
	pDocTemplate->RemoveDocument(pDocDest);							\
	return;															\
	}																	\
	BYTE* lpD = (BYTE*)::GlobalLock((HGLOBAL)hBmpDest);					\
	BYTE* lpS = (BYTE*)::GlobalLock((HGLOBAL)hBmpSrc);					\
	int iColors = DIBNumColors((char *)&(((LPBITMAPINFO)lpD)->bmiHeader)); \
	RGBQUAD *bmiColorsDst = ((LPBITMAPINFO)lpD)->bmiColors;	\
	RGBQUAD *bmiColorsSrc = ((LPBITMAPINFO)lpS)->bmiColors;	\
	BYTE * lpDst = (BYTE*)::FindDIBBits((LPSTR)lpD);	\
	BYTE * lpSrc = (BYTE*)::FindDIBBits((LPSTR)lpS);	\
	int dwWidth  = ::DIBWidth((LPSTR)lpS);\
	int dwHeight = ::DIBHeight((LPSTR)lpS);\
	int w=WIDTHBYTES(dwWidth*((LPBITMAPINFOHEADER)lpS)->biBitCount);	\
	HRTimer my_timer;	\
	my_timer.StartTimer();	\
	int bitCount = ((LPBITMAPINFOHEADER)lpS)->biBitCount; \

#define BEGIN_SOURCE_PROCESSING \
	CDibDoc* pDocSrc=GetDocument();										\
	BeginWaitCursor();													\
	HDIB hBmpSrc=pDocSrc->GetHDIB();									\
	BYTE* lpS = (BYTE*)::GlobalLock((HGLOBAL)hBmpSrc);					\
	int iColors = DIBNumColors((char *)&(((LPBITMAPINFO)lpS)->bmiHeader)); \
	RGBQUAD *bmiColorsSrc = ((LPBITMAPINFO)lpS)->bmiColors;	\
	BYTE * lpSrc = (BYTE*)::FindDIBBits((LPSTR)lpS);	\
	int dwWidth  = ::DIBWidth((LPSTR)lpS);\
	int dwHeight = ::DIBHeight((LPSTR)lpS);\
	int w=WIDTHBYTES(dwWidth*((LPBITMAPINFOHEADER)lpS)->biBitCount);	\



#define END_SOURCE_PROCESSING	\
	::GlobalUnlock((HGLOBAL)hBmpSrc);								\
	EndWaitCursor();												\
	/////////////////////////////////////////////////////////////////////////////


//---------------------------------------------------------------
#define SFARSIT_PRELUCRARI(Titlu)	\
	double elapsed_time_ms = my_timer.StopTimer();	\
	CString Title;	\
	Title.Format("%s - Proc. time = %.2f ms", Titlu, elapsed_time_ms);	\
	::GlobalUnlock((HGLOBAL)hBmpDest);								\
	::GlobalUnlock((HGLOBAL)hBmpSrc);								\
	EndWaitCursor();												\
	pDocDest->SetHDIB(hBmpDest);									\
	pDocDest->InitDIBData();										\
	pDocDest->SetTitle((LPCSTR)Title);									\
	CFrameWnd* pFrame=pDocTemplate->CreateNewFrame(pDocDest,NULL);	\
	pDocTemplate->InitialUpdateFrame(pFrame,pDocDest);	\

/////////////////////////////////////////////////////////////////////////////
// CDibView

IMPLEMENT_DYNCREATE(CDibView, CScrollView)

	BEGIN_MESSAGE_MAP(CDibView, CScrollView)
		//{{AFX_MSG_MAP(CDibView)
		ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
		ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
		ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
		ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
		ON_MESSAGE(WM_DOREALIZE, OnDoRealize)
		ON_COMMAND(ID_PROCESSING_PARCURGERESIMPLA, OnProcessingParcurgereSimpla)
		//}}AFX_MSG_MAP

		// Standard printing commands
		ON_COMMAND(ID_FILE_PRINT, CScrollView::OnFilePrint)
		ON_COMMAND(ID_FILE_PRINT_PREVIEW, CScrollView::OnFilePrintPreview)
		ON_COMMAND(ID_PROCESSING_NEWPROCESSING, &CDibView::OnProcessingNewprocessing)
		ON_COMMAND(ID_PROCESSING_CONSTRAST, &CDibView::OnProcessingConstrast)
		ON_COMMAND(ID_PROCESSING_BRIGHTNESS, &CDibView::OnProcessingBrightness)
		ON_COMMAND(ID_PROCESSING_WAVES, &CDibView::OnProcessingWaves)
		ON_COMMAND(ID_PROCESSING_DISPLAYBITMAPINFO, &CDibView::OnProcessingDisplaybitmapinfo)
		ON_COMMAND(ID_BW_APPLY, &CDibView::OnBwApply)
		ON_COMMAND(ID_PROCESSING_GRAYSCALE, &CDibView::OnProcessingGrayscale)
		ON_COMMAND(ID_PROCESSING_HYSTOGRAM, &CDibView::OnProcessingHystogram)
		ON_COMMAND(ID_PROCESSING_LABELING, &CDibView::OnProcessingLabeling)
		ON_WM_LBUTTONDBLCLK()
		ON_COMMAND(ID_PROCESSING_CONTOUR, &CDibView::OnProcessingContour)
		ON_COMMAND(ID_PROCESSING_FFT, &CDibView::OnProcessingFft)
		ON_COMMAND(ID_FFT_MEANFILTER, &CDibView::OnFftMeanfilter)
		ON_COMMAND(ID_FFT_GAUSSIANFILTER, &CDibView::OnFftGaussianfilter)
		ON_COMMAND(ID_FFT_EDGEDETECTION, &CDibView::OnFftEdgedetection)
		ON_COMMAND(ID_FFT_HIGHPASSFILTER, &CDibView::OnFftHighpassfilter)
		ON_COMMAND(ID_FFT_HIGHPASSFILTER2, &CDibView::OnFftHighpassfilter2)
		ON_COMMAND(ID_FFT_EDGEDETECTION2, &CDibView::OnFftEdgedetection2)
		ON_COMMAND(ID_FFT_CUSTOMBLUR, &CDibView::OnFftCustomblur)
		ON_COMMAND(ID_FFT_FFT, &CDibView::OnFftFft)
		ON_COMMAND(ID_FFT_MAGNITUDE, &CDibView::OnFftMagnitude)
		ON_COMMAND(ID_FFT_LOWPASS1, &CDibView::OnFftLowpass1)
		ON_COMMAND(ID_PROCESSING_GAUSSIANFILTER, &CDibView::OnProcessingGaussianfilter)
		ON_COMMAND(ID_FILTER_SALTPEPPER, &CDibView::OnFilterSaltpepper)
		ON_COMMAND(ID_FILTER_GAUSSIAN1, &CDibView::OnFilterGaussian1)
		ON_COMMAND(ID_FILTER_GAUSSIAN2, &CDibView::OnFilterGaussian2)
		ON_COMMAND(ID_PROCESSING_EDGEDETECTION, &CDibView::OnProcessingEdgedetection)
		ON_COMMAND(ID_PROCESSING_TEST, &CDibView::OnProcessingTest)
		ON_COMMAND(ID_PROCESSING_PROJECT, &CDibView::OnProcessingProject)
	END_MESSAGE_MAP()

	/////////////////////////////////////////////////////////////////////////////
	// CDibView construction/destruction

	CDibView::CDibView()
	{
	}

	CDibView::~CDibView()
	{
	}

	/////////////////////////////////////////////////////////////////////////////
	// CDibView drawing

	void CDibView::OnDraw(CDC* pDC)
	{
		CDibDoc* pDoc = GetDocument();

		HDIB hDIB = pDoc->GetHDIB();
		if (hDIB != NULL)
		{
			LPSTR lpDIB = (LPSTR) ::GlobalLock((HGLOBAL) hDIB);
			int cxDIB = (int) ::DIBWidth(lpDIB);         // Size of DIB - x
			int cyDIB = (int) ::DIBHeight(lpDIB);        // Size of DIB - y
			::GlobalUnlock((HGLOBAL) hDIB);
			CRect rcDIB;
			rcDIB.top = rcDIB.left = 0;
			rcDIB.right = cxDIB;
			rcDIB.bottom = cyDIB;
			CRect rcDest;
			if (pDC->IsPrinting())   // printer DC
			{
				// get size of printer page (in pixels)
				int cxPage = pDC->GetDeviceCaps(HORZRES);
				int cyPage = pDC->GetDeviceCaps(VERTRES);
				// get printer pixels per inch
				int cxInch = pDC->GetDeviceCaps(LOGPIXELSX);
				int cyInch = pDC->GetDeviceCaps(LOGPIXELSY);

				//
				// Best Fit case -- create a rectangle which preserves
				// the DIB's aspect ratio, and fills the page horizontally.
				//
				// The formula in the "->bottom" field below calculates the Y
				// position of the printed bitmap, based on the size of the
				// bitmap, the width of the page, and the relative size of
				// a printed pixel (cyInch / cxInch).
				//
				rcDest.top = rcDest.left = 0;
				rcDest.bottom = (int)(((double)cyDIB * cxPage * cyInch)
					/ ((double)cxDIB * cxInch));
				rcDest.right = cxPage;
			}
			else   // not printer DC
			{
				rcDest = rcDIB;
			}
			::PaintDIB(pDC->m_hDC, &rcDest, pDoc->GetHDIB(),
				&rcDIB, pDoc->GetDocPalette());
		}
	}

	/////////////////////////////////////////////////////////////////////////////
	// CDibView printing

	BOOL CDibView::OnPreparePrinting(CPrintInfo* pInfo)
	{
		// default preparation
		return DoPreparePrinting(pInfo);
	}

	/////////////////////////////////////////////////////////////////////////////
	// CDibView commands


	LRESULT CDibView::OnDoRealize(WPARAM wParam, LPARAM)
	{
		ASSERT(wParam != NULL);
		CDibDoc* pDoc = GetDocument();
		if (pDoc->GetHDIB() == NULL)
			return 0L;  // must be a new document

		CPalette* pPal = pDoc->GetDocPalette();
		if (pPal != NULL)
		{
			CMainFrame* pAppFrame = (CMainFrame*) AfxGetApp()->m_pMainWnd;
			ASSERT_KINDOF(CMainFrame, pAppFrame);

			CClientDC appDC(pAppFrame);
			// All views but one should be a background palette.
			// wParam contains a handle to the active view, so the SelectPalette
			// bForceBackground flag is FALSE only if wParam == m_hWnd (this view)
			CPalette* oldPalette = appDC.SelectPalette(pPal, ((HWND)wParam) != m_hWnd);

			if (oldPalette != NULL)
			{
				UINT nColorsChanged = appDC.RealizePalette();
				if (nColorsChanged > 0)
					pDoc->UpdateAllViews(NULL);
				appDC.SelectPalette(oldPalette, TRUE);
			}
			else
			{
				TRACE0("\tSelectPalette failed in CDibView::OnPaletteChanged\n");
			}
		}

		return 0L;
	}

	void CDibView::OnInitialUpdate()
	{
		CScrollView::OnInitialUpdate();
		ASSERT(GetDocument() != NULL);

		SetScrollSizes(MM_TEXT, GetDocument()->GetDocSize());
	}


	void CDibView::OnActivateView(BOOL bActivate, CView* pActivateView,
		CView* pDeactiveView)
	{
		CScrollView::OnActivateView(bActivate, pActivateView, pDeactiveView);

		if (bActivate)
		{
			ASSERT(pActivateView == this);
			OnDoRealize((WPARAM)m_hWnd, 0);   // same as SendMessage(WM_DOREALIZE);
		}
	}

	void CDibView::OnEditCopy()
	{
		CDibDoc* pDoc = GetDocument();
		// Clean clipboard of contents, and copy the DIB.

		if (OpenClipboard())
		{
			BeginWaitCursor();
			EmptyClipboard();
			SetClipboardData (CF_DIB, CopyHandle((HANDLE) pDoc->GetHDIB()) );
			CloseClipboard();
			EndWaitCursor();
		}
	}



	void CDibView::OnUpdateEditCopy(CCmdUI* pCmdUI)
	{
		pCmdUI->Enable(GetDocument()->GetHDIB() != NULL);
	}


	void CDibView::OnEditPaste()
	{
		HDIB hNewDIB = NULL;

		if (OpenClipboard())
		{
			BeginWaitCursor();

			hNewDIB = (HDIB) CopyHandle(::GetClipboardData(CF_DIB));

			CloseClipboard();

			if (hNewDIB != NULL)
			{
				CDibDoc* pDoc = GetDocument();
				pDoc->ReplaceHDIB(hNewDIB); // and free the old DIB
				pDoc->InitDIBData();    // set up new size & palette
				pDoc->SetModifiedFlag(TRUE);

				SetScrollSizes(MM_TEXT, pDoc->GetDocSize());
				OnDoRealize((WPARAM)m_hWnd,0);  // realize the new palette
				pDoc->UpdateAllViews(NULL);
			}
			EndWaitCursor();
		}
	}


	void CDibView::OnUpdateEditPaste(CCmdUI* pCmdUI)
	{
		pCmdUI->Enable(::IsClipboardFormatAvailable(CF_DIB));
	}

	void CDibView::OnProcessingParcurgereSimpla() 
	{
		// TODO: Add your command handler code here
		BEGIN_PROCESSING();

		// Makes a grayscale image by equalizing the R, G, B components from the LUT
		for (int k=0;  k < iColors ; k++)
			bmiColorsDst[k].rgbRed=bmiColorsDst[k].rgbGreen=bmiColorsDst[k].rgbBlue=k;


		//  Goes through the bitmap pixels and performs their negative	
		for (int i=0;i<dwHeight;i++)
			for (int j=0;j<dwWidth;j++)
			{	
				lpDst[i*w+j]= 255 - lpSrc[i*w+j]; //makes image negative
			}

			END_PROCESSING("Negativ imagine");
	}




	void CDibView::OnProcessingNewprocessing()
	{
		BEGIN_PROCESSING(); 

		// Makes a grayscale image by equalizing the R, G, B components from the LUT 
		for (int k=0;  k < iColors ; k++) 
			bmiColorsDst[k].rgbRed=bmiColorsDst[k].rgbGreen= 
			bmiColorsDst[k].rgbBlue=k; 

		// Goes through the bitmap pixels and performs their negative   
		for (int i=0;i<dwHeight;i++) 
			for (int j=0;j<dwWidth;j++) 
				lpDst[i*w+j]= 255 - lpSrc[i*w+j]; //makes image negative 

		END_PROCESSING("Operation name"); 


	}


	void CDibView::OnProcessingConstrast()
	{
		// TODO: Add your command handler code here

		BEGIN_PROCESSING(); 

		// Makes a grayscale image by equalizing the R, G, B components from the LUT 
		for (int k=0;  k < iColors ; k++) 
			bmiColorsDst[k].rgbRed=bmiColorsDst[k].rgbGreen= 
			bmiColorsDst[k].rgbBlue=k; 
		bitCount;
		// Goes through the bitmap pixels and performs their negative   
		int tmpPixel;
		for (int i=0;i<dwHeight;i++) 
			for (int j=0;j<dwWidth;j++) 
			{
				tmpPixel = lpSrc[i*w+j] * 2; //makes image negative 
				if ( tmpPixel > 255 )
					lpDst[i*w+j] = 255;
				else
					lpDst[i*w+j] = tmpPixel;
			}


			END_PROCESSING("Operation name"); 
	}


	void CDibView::OnProcessingBrightness()
	{
		// TODO: Add your command handler code here
	}


	void CDibView::OnProcessingWaves()
	{
		BEGIN_PROCESSING(); 

		// Makes a grayscale image by equalizing the R, G, B components from the LUT 
		for (int k=0;  k < iColors ; k++) 
			bmiColorsDst[k].rgbRed=bmiColorsDst[k].rgbGreen= 
			bmiColorsDst[k].rgbBlue=k; 

		int tmpPos;
		float a = 10;
		float p = 50;
		// Goes through the bitmap pixels and performs their negative   
		for (int i=0;i<dwHeight;i++) 
			for (int j=0;j<dwWidth;j++) 
			{
				tmpPos = i + a * sin( (double) j/(p*3)) * dwWidth+j;
				if (tmpPos< 0)
				{
					tmpPos = 0;
				}
				if (tmpPos > dwHeight)
				{ 
					tmpPos = dwHeight;
				}
				lpDst[i*w+j]= lpSrc[(byte)tmpPos]; //makes image negative 
			}

			END_PROCESSING("Operation name"); 
	}


	void CDibView::OnProcessingDisplaybitmapinfo()
	{
		BEGIN_SOURCE_PROCESSING;

		CBitmapInfoDlg dlgBmpHeader;
		LPBITMAPINFO pBitmapInfoSrc = (LPBITMAPINFO) lpS;
		CString buffer;

		buffer.Format("Bits/px:\t%d\r\n", pBitmapInfoSrc->bmiHeader.biBitCount);
		dlgBmpHeader.m_LUT+=buffer;

		buffer.Format("Width:\t%dpx\r\n", pBitmapInfoSrc->bmiHeader.biWidth);
		dlgBmpHeader.m_LUT+=buffer;

		buffer.Format("Height:\t%dpx\r\n", pBitmapInfoSrc->bmiHeader.biHeight);
		dlgBmpHeader.m_LUT+=buffer;

		buffer.Format("\r\n\r\nLUT:\r\n");
		dlgBmpHeader.m_LUT+=buffer;

		for (int i=0; i<iColors; i++)
		{
			buffer.Format("%3d.\t%3d\t%3d\t%3d\r\n", i, 
				bmiColorsSrc[i].rgbRed,
				bmiColorsSrc[i].rgbGreen,
				bmiColorsSrc[i].rgbBlue);
			dlgBmpHeader.m_LUT+=buffer;
		}

		dlgBmpHeader.DoModal();

		END_SOURCE_PROCESSING;
	}


	void CDibView::OnBwApply()
	{
		// TODO: Add your command handler code here

		BEGIN_PROCESSING(); 

		int threshold = 32;

		// Makes a grayscale image by equalizing the R, G, B components from the LUT 
		for (int k=0;  k < iColors ; k++) 
			bmiColorsDst[k].rgbRed=bmiColorsDst[k].rgbGreen= 
			bmiColorsDst[k].rgbBlue=k; 
		bitCount;
		// Goes through the bitmap pixels and performs their negative   
		int tmpPixel;
		for (int i=0;i<dwHeight;i++) 
			for (int j=0;j<dwWidth;j++) 
			{
				tmpPixel = lpSrc[i*w+j];
				if ( tmpPixel > threshold )
					lpDst[i*w+j] = 255;
				else
					lpDst[i*w+j] = 0;
			}


			END_PROCESSING("Operation name"); 
	}


	void CDibView::OnProcessingGrayscale()
	{
		// TODO: Add your command handler code here
		BEGIN_PROCESSING();

		if (bitCount < 24) {

			// Makes a grayscale image by equalizing the R, G, B components from the LUT
			for (int k=0;  k < iColors ; k++) {
				bmiColorsDst[k].rgbRed=bmiColorsDst[k].rgbGreen=bmiColorsDst[k].rgbBlue=k;
			}
		} 
		else
		{
			BYTE red, green, blue, avg;
			for (int i=0; i<dwHeight; i++)
			{
				for (int j=0; j<dwWidth; j++)
				{
					red = lpSrc[i*w+3*j+2];
					green = lpSrc[i*w+3*j+1];
					blue = lpSrc[i*w+3*j];

					avg = (red+green+blue)/3;

					lpDst[i*w+3*j+2] = avg;
					lpDst[i*w+3*j+1] = avg;
					lpDst[i*w+3*j] = avg;
				}
			}

		}

		END_PROCESSING("Negativ imagine");
	}


	void CDibView::OnProcessingHystogram()
	{
		// TODO: Add your command handler code here
	}

	int q[10000];
	int arr[10000];
	int ipair = 0;

	int qstart=0;
	int qstop = 0;
	void newPair(int a, int b)
	{
		q[ipair] = a;
		q[ipair+1] = b;
		ipair = ipair + 2;
	}

	void CDibView::OnProcessingLabeling()
	{
		BEGIN_PROCESSING(); 


		int start = 0;
		int stop = 0;
		int label = 1;
		int a,b,c,d,x;

		int *labels = (int*) malloc(w*dwHeight * sizeof(int));

		for (int i=0; i<dwHeight; i++)
		{
			for (int j=0; j<dwWidth; j++)
			{
				labels[i*w+j] = lpSrc[i*w+j];
			}
		}


		for (int i=1; i = dwHeight -1 ; i++)
		{
			for (int j=1; j<dwWidth-1; j++)
			{
				if ( lpSrc[i*dwWidth + j] == 0 )
				{
					a = i*dwWidth + j-1;
					b = (i-1)*dwWidth + j-1;
					c = (i-1)*dwWidth + j;
					d = (i-1)*dwWidth + j+1;
					x = i*dwWidth + j;

					//a = i*dwWidth + j-1;
					//b = (i+1)*dwWidth + j-1;
					//c = (i+1)*dwWidth + j;
					//d = (i+1)*dwWidth + j+1;
					//x = i*dwWidth + j;
					if (lpSrc[b]!=0)
					{
						if (lpSrc[a]!=0)
						{
							if (lpSrc[c]!=0)
							{
								if (lpSrc[d]!=0)
								{
									//8
									label++;
									labels[x] = label;
								}
								else
								{
									//7
									//d=0
									labels[x] = labels[d];
								}
							}
							else
							{
								//6
								//c=0
								labels[x] = labels[c];
							}
						}
						else
						{
							//a=0
							if (lpSrc[c]!=0)
							{
								//4, 5
								if (lpSrc[d]!=0)
								{
									//5
									labels[x] = labels[a];
								}
								else
								{
									//4
									labels[x] = labels[a];
									newPair(labels[a], labels[d]);
								}
							}
							else
							{
								//3
								labels[x]=labels[a];
								newPair(labels[a], labels[c]);
							}
						}
					}
					else
					{
						//b=0
						if (lpSrc[c]!=0 && lpSrc[d]==0)
						{
							// 1
							lpSrc[x] = lpSrc[b];
							newPair(labels[b], labels[d]);
						}
						else
						{
							//2
							labels[x] = labels[b];
						}

					}



					//a[start] = i*dwWidth + j;
					//lpDst[i*dwWidth + j] = label;
					//start++;

					////dreapta
					//if (lpSrc[i*dwWidth + j+1] == 0 && lpDst[i*dwWidth + j+1] == 0) //e obiect, si nu a mai fost atins
					//{
					//	a[start] = i*dwWidth + j+1;
					//	lpDst[i*dwWidth + j+1] = label;
					//	start++;
					//}				
					//
					////jos
					//if (lpSrc[(i+1)*dwWidth + j] == 0 && lpDst[(i+1)*dwWidth + j] == 0) //e obiect, si nu a mai fost atins
					//{
					//	a[start] =(i+1)*dwWidth + j;
					//	lpDst[(i+1)*dwWidth + j] = label;
					//	start++;
					//}
				}

				//red = lpSrc[i*w+3*j+2];
				//green = lpSrc[i*w+3*j+1];
				//blue = lpSrc[i*w+3*j];

				//avg = (red+green+blue)/3;
				//	
				//lpDst[i*w+3*j+2] = avg;
				//lpDst[i*w+3*j+1] = avg;
				//lpDst[i*w+3*j] = avg;
			}
		}

		for (int i=0; i<dwHeight; i++)
		{
			for (int j=1; j<dwWidth; j++)
			{
				lpDst[i*dwWidth + j] = labels[i*dwWidth + j];
			}
		}

		int max = label;

		for (int i = 0; i<10000; i++)
		{
			arr[i] = 0;
		}

		srand(time(NULL));
		for (int k=1; k<254; k++)
		{
			bmiColorsDst[k].rgbRed = rand() % 256;
			bmiColorsDst[k].rgbGreen = rand() % 256;
			bmiColorsDst[k].rgbBlue = rand() % 256; 
		}


		for (int i = 0; i < ipair-1; i++)
		{
			int mic, mare;
			if (q[i]<q[i+1])
			{
				mic = q[i];
				mare = q[i+1];
			}
			else
			{
				mic = q[i+1];
				mare = q[i];
			}

			if (arr[mare]!=0)
			{
				if (arr[mic] == 0)
				{
					bmiColorsDst[mic] = bmiColorsDst[mare];
					arr[mic]=arr[mare];
				}
				else
				{
					for (int j = 0; j<10000; j++)
					{
						if (arr[j] == arr[mare])
						{
							arr[j] = arr[mic];
							bmiColorsDst[j] = bmiColorsDst[mic];
						}
					}
					bmiColorsDst[arr[mare]] = bmiColorsDst[mic];
				}
			}
			else
			{
				bmiColorsDst[mare] = bmiColorsDst[mic];
				arr[mare] = mic;
			}
		}



		// TODO: Add your command handler code here


		END_PROCESSING("Negativ imagine");
	}


	//void CDibView::OnLButtonDblClk(UINT nFlags, CPoint point)
	//{			   
	//	BEGIN_SOURCE_PROCESSING; 

	//	//obtain the scroll position (because of scroll bars' positions 
	//	//the coordinates may be shifted) and adjust the position 
	//	CPoint pos  =  GetScrollPosition()+ point ; 

	//	//point contains the window's client area coordinates 
	//	//the y axis is inverted because of the way bitmaps  
	//	//are represented in memory 
	//	int  x =  pos.x ; 
	//	int  y =  dwHeight -pos.y-1;

	//	//test if the position is inside the image 
	//	if (x>0 && x< dwWidth && y >0 && y<dwHeight )  
	//	{ 
	//		//prepare a CString for formating the output message     
	//		CString info; 
	//		info.Format("x=%d, y=%d, color=%d", x, y, lpSrc [y*w+x]); 
	//		AfxMessageBox (info); 
	//	} 

	//	END_SOURCE_PROCESSING; 

	//	//call the superclass' method 
	//	CScrollView::OnLButtonDblClk (nFlags, point );

	//}

	void CDibView::OnLButtonDblClk(UINT nFlags, CPoint point)  
	{ 
		BEGIN_PROCESSING(); 

		CDC dc; //memory DC 
		dc.CreateCompatibleDC(0); //create it compatible with the screen 

		CBitmap ddBitmap;//to hold a device dependent bitmap compatible with 
		//the screen 

		//create a DDB, compatible with the screen 
		//and initialize it with the data from the source DIB 
		HBITMAP hDDBitmap =  
			CreateDIBitmap(::GetDC(0), &((LPBITMAPINFO)lpS)->bmiHeader, CBM_INIT, 
			lpSrc, (LPBITMAPINFO)lpS, DIB_RGB_COLORS); 

		//attach the handle to the CBitmap object 
		ddBitmap.Attach(hDDBitmap);

		//select the DDB into the memory DC 
		//so that all drawing will be performed on the DDB 
		CBitmap* pTempBmp = dc.SelectObject(&ddBitmap); 

		//from this point onward, all drawing done using the DC object
		//will be made on the DDB 
		//obtain the scroll position (because of scroll bars' positions 
		//the coordinates may be shifted) and adjust the position 
		CPoint pos = GetScrollPosition()+point; 

		//create a green pen for drawing 
		CPen pen(PS_SOLID, 1, RGB(0,255,0)); 

		//select the pen on the device context 
		CPen *pTempPen = dc.SelectObject(&pen); 
		//draw a text 
		dc.TextOut(pos.x,pos.y, "test"); 
		//and a line 
		dc.MoveTo(pos.x,pos.y); 
		dc.LineTo(pos.x, pos.y-20); 

		//select back the old pen 
		dc.SelectObject(pTempPen); 
		//and the old bitmap 
		dc.SelectObject(pTempBmp); 

		int  x =  pos.x ; 
		int  y =  dwHeight-pos.y-1;

		//AREA
		for (int i=0; i<dwHeight; i++)
		{
			for (int j=0; j<dwWidth; j++)
			{
				//labels[i*w+j] = lpSrc[i*w+j];
			}
		}

		//copy the pixel data from the device dependent bitmap 
		//to the destination DIB 
		GetDIBits(dc.m_hDC, ddBitmap, 0, dwHeight, lpDst, (LPBITMAPINFO)lpD, 
			DIB_RGB_COLORS); 

		END_PROCESSING("line");  
	}

	void CDibView::OnProcessingContour()
	{
		BEGIN_PROCESSING();
		//int x[8] = {1,  1,  0, -1, -1, -1, 0, 1};
		//int y[8] = {0, -1, -1, -1,  0,  1, 1, 1};
		int x[8] = {1, 1, 0, -1, -1, -1, 0, 1};
		int y[8] = {0, 1, 1, 1, 0, -1, -1, -1};
		int dir, first, second, last, lastbo, ic;
		int nextPixel;
		bool stop;

		int chain_codes[100000];

		for (int i = 0; i < 100000; i++)
		{
			chain_codes[i] = 0;
		}
		stop = false;

		for (int i=0; i<dwHeight; i++)
		{
			for (int j=0; j<dwWidth; j++)
			{
				if (lpSrc[i*w+j] == 0)
				{
					//am gasit inceput de contur nou
					first = i*w+j;
					second = -1;
					last = -2;
					lastbo = -2;
					dir = 7;
					ic = 1;

					while (true)
					{
						if (dir%2 == 0)
						{
							dir = (dir + 7)%8;
						}
						else
						{
							dir = (dir + 6)%8;
						}

						//ma rotesc
						for (int k = dir; k < dir+8; k++)
						{
							int km = k%8;
							//taranism
							i = i+x[km];
							j = j+y[km];
							nextPixel = i*w + j;
							if (lpSrc[ nextPixel ] == 0) //pixelu' vecin 
							{
								dir = km;
								chain_codes[ic] = dir;
								break;
							}
						}

						ic++;
						//urmatorul pixel din contur
						if (ic == 2)
						{
							second = nextPixel;
						}
						//lpDst[nextPixel] = 128;
						lastbo = last;
						last = nextPixel;

						if ( (first == lastbo && second == last) || ic > 100)
						{
							stop = true;
							break;
						}
					}
				}

				if (stop)
				{
					break;
				}
			}
			if (stop)
			{
				break;
			}
		}

		int posx = first;
		for (int i = 0; i<100; i++)
		{
			lpDst[posx] = 128;
			dir = chain_codes[i];
			posx = posx + x[dir]*w + y[dir];
		}


		END_PROCESSING("Contour");
	}


	void CDibView::OnProcessingFft()
	{
		// TODO: Add your command handler code here
	}


	void CDibView::OnFftMeanfilter()
	{
		// TODO: Add your command handler code here
		BEGIN_PROCESSING();

		int filter[10] = {1, 1, 1,   1, 1, 1,   1, 1, 1,   9};
		float pixel;

		for (int i=1; i<dwHeight-1; i++)
		{
			for (int j=1; j<dwWidth-1; j++)
			{
				pixel = lpSrc[(i-1)*w+j-1]*filter[0] + lpSrc[(i-1)*w+j+0]*filter[1] + lpSrc[(i-1)*w+j+1]*filter[2] +
						lpSrc[(i+0)*w+j-1]*filter[3] + lpSrc[(i+0)*w+j+0]*filter[4] + lpSrc[(i+0)*w+j+1]*filter[5] +
						lpSrc[(i+1)*w+j-1]*filter[6] + lpSrc[(i+1)*w+j+0]*filter[7] + lpSrc[(i+1)*w+j+1]*filter[8];
				pixel = pixel / filter[9];

				if (pixel < 0)
				{
					pixel = 0;
				}
				if (pixel > 255)
				{
					pixel = 255;
				}
				
				lpDst[i*w+j] = pixel;
			}
		}

		END_PROCESSING("Mean");
	}


	void CDibView::OnFftGaussianfilter()
	{
		// TODO: Add your command handler code here
				BEGIN_PROCESSING();

		int filter[10] = {1, 2, 1,   2, 4, 2,   1, 2, 1,   16};
		float pixel;

		for (int i=1; i<dwHeight-1; i++)
		{
			for (int j=1; j<dwWidth-1; j++)
			{
				pixel = lpSrc[(i-1)*w+j-1]*filter[0] + lpSrc[(i-1)*w+j+0]*filter[1] + lpSrc[(i-1)*w+j+1]*filter[2] +
						lpSrc[(i+0)*w+j-1]*filter[3] + lpSrc[(i+0)*w+j+0]*filter[4] + lpSrc[(i+0)*w+j+1]*filter[5] +
						lpSrc[(i+1)*w+j-1]*filter[6] + lpSrc[(i+1)*w+j+0]*filter[7] + lpSrc[(i+1)*w+j+1]*filter[8];
				pixel = pixel / filter[9];

				if (pixel < 0)
				{
					pixel = 0;
				}
				if (pixel > 255)
				{
					pixel = 255;
				}
				
				lpDst[i*w+j] = pixel;
			}
		}

		END_PROCESSING("Gaussian");
	}


	void CDibView::OnFftEdgedetection() //Laplace
	{
		// TODO: Add your command handler code here
		BEGIN_PROCESSING();

		int filter[10] = {0, -1, 0,   -1, 4, -1,   0, -1, 0,   1};
		float pixel;

		for (int i=1; i<dwHeight-1; i++)
		{
			for (int j=1; j<dwWidth-1; j++)
			{
				pixel = lpSrc[(i-1)*w+j-1]*filter[0] + lpSrc[(i-1)*w+j+0]*filter[1] + lpSrc[(i-1)*w+j+1]*filter[2] +
						lpSrc[(i+0)*w+j-1]*filter[3] + lpSrc[(i+0)*w+j+0]*filter[4] + lpSrc[(i+0)*w+j+1]*filter[5] +
						lpSrc[(i+1)*w+j-1]*filter[6] + lpSrc[(i+1)*w+j+0]*filter[7] + lpSrc[(i+1)*w+j+1]*filter[8];
				pixel = pixel / filter[9];

				if (pixel < 0)
				{
					pixel = 0;
				}
				if (pixel > 255)
				{
					pixel = 255;
				}
				
				lpDst[i*w+j] = pixel;
			}
		}

		END_PROCESSING("laplace");
	}

	

	void CDibView::OnFftEdgedetection2()
	{
		// TODO: Add your command handler code here
		BEGIN_PROCESSING();

		int filter[10] = {-1, -1, -1,   -1, 8, -1,   -1, -1, -1,   1};
		float pixel;

		for (int i=1; i<dwHeight-1; i++)
		{
			for (int j=1; j<dwWidth-1; j++)
			{
				pixel = lpSrc[(i-1)*w+j-1]*filter[0] + lpSrc[(i-1)*w+j+0]*filter[1] + lpSrc[(i-1)*w+j+1]*filter[2] +
						lpSrc[(i+0)*w+j-1]*filter[3] + lpSrc[(i+0)*w+j+0]*filter[4] + lpSrc[(i+0)*w+j+1]*filter[5] +
						lpSrc[(i+1)*w+j-1]*filter[6] + lpSrc[(i+1)*w+j+0]*filter[7] + lpSrc[(i+1)*w+j+1]*filter[8];
				pixel = pixel / filter[9];

				if (pixel < 0)
				{
					pixel = 0;
				}
				if (pixel > 255)
				{
					pixel = 255;
				}
				
				lpDst[i*w+j] = pixel;
			}
		}

		END_PROCESSING("laplace");
	}


	void CDibView::OnFftHighpassfilter()
	{
		// TODO: Add your command handler code here
		BEGIN_PROCESSING();

		int filter[10] = {0, -1, 0,   -1, 5, -1,   0, -1, 0,   1};
		float pixel;

		for (int i=1; i<dwHeight-1; i++)
		{
			for (int j=1; j<dwWidth-1; j++)
			{
				pixel = lpSrc[(i-1)*w+j-1]*filter[0] + lpSrc[(i-1)*w+j+0]*filter[1] + lpSrc[(i-1)*w+j+1]*filter[2] +
						lpSrc[(i+0)*w+j-1]*filter[3] + lpSrc[(i+0)*w+j+0]*filter[4] + lpSrc[(i+0)*w+j+1]*filter[5] +
						lpSrc[(i+1)*w+j-1]*filter[6] + lpSrc[(i+1)*w+j+0]*filter[7] + lpSrc[(i+1)*w+j+1]*filter[8];
				pixel = pixel / filter[9];

				if (pixel < 0)
				{
					pixel = 0;
				}
				if (pixel > 255)
				{
					pixel = 255;
				}
				
				lpDst[i*w+j] = pixel;
			}
		}

		END_PROCESSING("Mean");
	}


	void CDibView::OnFftHighpassfilter2()
	{
		// TODO: Add your command handler code here
		BEGIN_PROCESSING();

		int filter[10] = {-1, -1, -1,   -1, 9, -1,   -1, -1, -1,   1};
		float pixel;

		for (int i=1; i<dwHeight-1; i++)
		{
			for (int j=1; j<dwWidth-1; j++)
			{
				pixel = lpSrc[(i-1)*w+j-1]*filter[0] + lpSrc[(i-1)*w+j+0]*filter[1] + lpSrc[(i-1)*w+j+1]*filter[2] +
						lpSrc[(i+0)*w+j-1]*filter[3] + lpSrc[(i+0)*w+j+0]*filter[4] + lpSrc[(i+0)*w+j+1]*filter[5] +
						lpSrc[(i+1)*w+j-1]*filter[6] + lpSrc[(i+1)*w+j+0]*filter[7] + lpSrc[(i+1)*w+j+1]*filter[8];
				pixel = pixel / filter[9];

				if (pixel < 0)
				{
					pixel = 0;
				}
				if (pixel > 255)
				{
					pixel = 255;
				}
				
				lpDst[i*w+j] = pixel;
			}
		}

		END_PROCESSING("Mean");
	}


	void CDibView::OnFftCustomblur()
	{
		// TODO: Add your command handler code here
		BEGIN_PROCESSING();

		CCustomBlurDlg dlg;

		dlg.DoModal();
		float pixel;

		for (int i=1; i<dwHeight-1; i++)
		{
			for (int j=1; j<dwWidth-1; j++)
			{
				pixel = lpSrc[(i-1)*w+j-1]*dlg.m_arr0 + lpSrc[(i-1)*w+j+0]*dlg.m_arr1 + lpSrc[(i-1)*w+j+1]*dlg.m_arr2 +
						lpSrc[(i+0)*w+j-1]*dlg.m_arr3 + lpSrc[(i+0)*w+j+0]*dlg.m_arr4 + lpSrc[(i+0)*w+j+1]*dlg.m_arr5 +
						lpSrc[(i+1)*w+j-1]*dlg.m_arr6 + lpSrc[(i+1)*w+j+0]*dlg.m_arr7 + lpSrc[(i+1)*w+j+1]*dlg.m_arr8;
				int sum = dlg.m_arr0 + dlg.m_arr1 + dlg.m_arr2 + dlg.m_arr3 + dlg.m_arr4 + dlg.m_arr5 + dlg.m_arr6 + dlg.m_arr7 + dlg.m_arr8;
				if (sum < 1)
				{
					sum = 1;
				}

				pixel = pixel / sum;

				if (pixel < 0)
				{
					pixel = 0;
				}
				if (pixel > 255)
				{
					pixel = 255;
				}
				
				lpDst[i*w+j] = pixel;
			}
		}

		END_PROCESSING("Custom blur");
	}




	void CDibView::OnFftFft()
	{
		// TODO: Add your command handler code here
		BEGIN_PROCESSING();
		double *real = new double[dwWidth*dwHeight];
		double *imag = new double[dwWidth*dwHeight];
		fftimage(dwWidth, dwHeight, lpSrc, (BYTE*)0, real, imag);

		ifftimage(dwWidth, dwHeight, real, imag, lpDst, (BYTE*)0);
		END_PROCESSING("FFT");
	}




	void CDibView::OnFftMagnitude()
	{
		// TODO: Add your command handler code here

		BEGIN_PROCESSING();
		double *real = new double[dwWidth*dwHeight];
		double *imag = new double[dwWidth*dwHeight];
		double *dst = new double[dwWidth*dwHeight];

		for (int i=0; i<dwHeight; i++)
		{
			for (int j=0; j<dwWidth; j++)
			{
				lpSrc[i*w+j] = ((i+j)&1)? -lpSrc[i*w+j] : lpSrc[i*w+j];
			}
		}

		fftimage(dwWidth, dwHeight, lpSrc, (BYTE*)0, real, imag);

		double pixel;
		double max = 0;
		
		for (int i=0; i<dwHeight; i++)
		{
			for (int j=0; j<dwWidth; j++)
			{
				pixel = real[i*w+j] * real[i*w+j] + imag[i*w+j] * imag[i*w+j];
				pixel = sqrt(pixel);
				pixel = log(pixel);
				dst[i*w+j] = pixel;
				if (pixel > max)
				{
					max = pixel;
				}
			}
		}

		for (int i=0; i<dwHeight; i++)
		{
			for (int j=0; j<dwWidth; j++)
			{
				pixel = (dst[i*w+j] / max) * 255;
				if (pixel > 255)
					pixel = 255;
				if (pixel < 0)
					pixel = 0;

				lpDst[i*w+j] = pixel;
			}
		}

		//ifftimage(dwWidth, dwHeight, real, imag, lpDst, (BYTE*)0);
		END_PROCESSING("FFT");

	}


	void CDibView::OnFftLowpass1()
	{
		// TODO: Add your command handler code here
		
		BEGIN_PROCESSING();
		double *real = new double[dwWidth*dwHeight];
		double *imag = new double[dwWidth*dwHeight];
		double *dst = new double[dwWidth*dwHeight];

		double r = 30;

		for (int i=0; i<dwHeight; i++)
		{
			for (int j=0; j<dwWidth; j++)
			{
				dst[i*w+j] = ((i+j)&1)? -lpSrc[i*w+j] : lpSrc[i*w+j];
			}
		}

		fftimage(dwWidth, dwHeight, lpSrc, (BYTE*)0, real, imag);


		for (int i=0; i<dwHeight; i++)
		{
			for (int j=0; j<dwWidth; j++)
			{
				if ( pow((double)(dwHeight/2 - i) , 2) + pow((double)(dwWidth/2 - j) , 2) > pow(r, 2) )
				{
					//real[i*w+j] = 0;
					//imag[i*w+j] = 0;
				}
			}
		}

		ifftimage(dwWidth, dwHeight, real, imag, dst, (DOUBLE*)0);

		
		for (int i=0; i<dwHeight; i++)
		{
			for (int j=0; j<dwWidth; j++)
			{
				if (dst[i*w+j] > 255)
					dst[i*w+j] = 255;
				if (dst[i*w+j] < 0)
					dst[i*w+j] = 0;

				lpDst[i*w+j] = ((i+j)&1)? -dst[i*w+j] : dst[i*w+j];
			}
		}

		END_PROCESSING("low pass 1");
	}




	void CDibView::OnProcessingGaussianfilter()
	{
		//deprecated
	}


	void CDibView::OnFilterSaltpepper()
	{
		BEGIN_PROCESSING();

		int size = 3;
		int size2 = size/2;

		int a[10];//*a = new int[size*size];

		for (int i = size2; i<dwHeight-size2; i++)
		{
			for (int j = size2; j<dwWidth-size2; j++)
			{
				int pos = 0;
				//pun matricea de pixeli intr-un vector
				for (int ii = i-size2; ii <= i+size2; ii++)
					for (int jj = j-size2; jj <= j+size2; jj++)
					{
						a[pos] = lpSrc[ii*w+jj];
						pos++;
					}

				//sortez vectorul
				bool sorted = false;
				while (!sorted)
				{
					sorted = true;
					for (int iii = 0; iii<pos-1; iii++)
					{
						if (a[iii] > a[iii+1])
						{
							int aux = a[iii];
							a[iii] = a[iii+1];
							a[iii+1] = aux;
							sorted = false;
						}
					}
				}
				lpDst[i*w+j] = a[pos/2];
			}
		}



		END_PROCESSING("Gaussian Filter");
	}


	void CDibView::OnFilterGaussian1()
	{
		
		BEGIN_PROCESSING();

		int size = 3;
		int size2 = size/2;
		double g[3][3];
		double sigma = 0.8;
		double sum = 0;

		for (int i = 0; i<size; i++)
		{
			for (int j = 0; j<size; j++)
			{
				double kk = exp( -(((size2-i)*(size2-i) + (size2-j)*(size2-j)) / 2.0*sigma*sigma) );
				g[i][j] = 1/(2.0*3.14*sigma*sigma) * kk;
				sum = sum + g[i][j];
			}
		}

		for (int i = size2; i<dwHeight-size2; i++)
		{
			for (int j = size2; j<dwWidth-size2; j++)
			{
				//each pixel in source
				float newpixel = 0;
				for (int ii = 0; ii < size; ii ++)
				{
					for (int jj = 0; jj < size; jj++)
					{
						newpixel += lpSrc[ (i+ii-size2)*w + (j+jj-size2) ] * g[ii][jj];
					}
				}
				lpDst[ i*w + j ] = newpixel / sum;
			}
		}



				
		END_PROCESSING("Gaussian Filter");

	}


	void CDibView::OnFilterGaussian2()
	{		
		BEGIN_PROCESSING();

		int size = 3;
		int size2 = size/2;
		double g[3];
		double sigma = 0.8;
		double sum = 0;

		//convolution matrix
		for (int i = 0; i<size; i++)
		{
			double kk = exp( -(( (size2-i)*(size2-i) ) / 2.0*sigma*sigma) );
			g[i] = 1/(2.0*3.14*sigma) * kk;
			sum = sum + g[i]*2;
		}
		//half the value of middle point; it will be added twice
		g[size2] = g[size2]/2;
		sum = sum - g[size2];

		for (int i = size2; i<dwHeight-size2; i++)
		{
			for (int j = size2; j<dwWidth-size2; j++)
			{
				//each pixel in source
				float newpixel = 0;
				for (int ii = 0; ii < size; ii ++)
				{
					newpixel += lpSrc[ (i+ii-size2)*w + (j) ] * g[ii];
					newpixel += lpSrc[ (i)*w + (j+ii-size2) ] * g[ii];
				}
				lpDst[ i*w + j ] = newpixel / sum;
			}
		}
				
		END_PROCESSING("Gaussian Filter");
	}

	//void computeConvolution(int* kernel, float* buffer, int size)
	//{
	//
	//}

	struct pixel {
		int x, y, val;
		struct pixel *pnext;
	};

	struct pixel *front = NULL;
	struct pixel *back = NULL;

	struct pixel* enqueue (int x, int y, int val)
	{
		struct pixel *ptr = (struct pixel*)malloc(sizeof(struct pixel));
		ptr->x = x;
		ptr->y = y;
		ptr->val = val;
		ptr->pnext = NULL;

		if (front == NULL)
		{
			front = back = ptr;
		}
		else 
		{
			back->pnext = ptr;
			back = ptr;
		}

		return back;
	}

	struct pixel* dequeue ()
	{
		if (front == NULL)
		{
			return NULL;
		}
		pixel *ptr;
		ptr = front;
		front = front->pnext;
		if (front == NULL)
		{
			back = NULL;
		}
		return ptr;
	}

	void CDibView::OnProcessingEdgedetection()
	{
		BEGIN_PROCESSING();

		//gaussian filter
		int size = 7;
		int size2 = size/2;
		double g[7][7];
		double sigma = 0.8;
		double sum = 0;

		int *hist = (int*) malloc(256 * sizeof(int));
		int *bufferSrc = (int*) malloc(w*dwHeight * sizeof(int));

		memset(hist, 255, 255);

		for (int i = 0; i<size; i++)
		{
			for (int j = 0; j<size; j++)
			{
				double kk = exp( -(((size2-i)*(size2-i) + (size2-j)*(size2-j)) / 2.0*sigma*sigma) );
				g[i][j] = 1/(2.0*3.14*sigma*sigma) * kk;
				sum = sum + g[i][j];
			}
		}
		//convolution for gaussian filter
		for (int i = size2; i<dwHeight-size2; i++)
		{
			for (int j = size2; j<dwWidth-size2; j++)
			{
				//each pixel in source
				float newpixel = 0;
				for (int ii = 0; ii < size; ii ++)
				{
					for (int jj = 0; jj < size; jj++)
					{
						newpixel += lpSrc[ (i+ii-size2)*w + (j+jj-size2) ] * g[ii][jj];
					}
				}
				bufferSrc[ i*w + j ] = newpixel / sum;
			}
		}


		//compute gradient and magnitude

		int sobelKernelX[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
		int sobelKernelY[3][3] = {{1, 2, 1}, {0, 0, 0}, {-1, -2, -1}};
		
		float *bufferX = (float*) malloc(w*dwHeight * sizeof(float));
		float *bufferY = (float*) malloc(w*dwHeight * sizeof(float));

		size = 3;
		size2 = size/2;

		int sumX = 0, sumY = 0;

		/*for (int i = 0; i < size; i++)
		{
			for (int j = 0; i < size; i++)
			{
				sumX += sobelKernelX[i][j];
				sumY += sobelKernelY[i][j];
			}
		}*/
		
		sumX = 4;
		sumY = 4;
		memset(bufferX, 0, w*dwHeight);
		memset(bufferY, 0, w*dwHeight);

		//compute vector components
		for (int i = size2; i<dwHeight-size2; i++)
		{
			for (int j = size2; j<dwWidth-size2; j++)
			{
				//each pixel in source
				float newPixelX = 0;
				float newPixelY = 0;

				for (int ii = 0; ii < size; ii ++)
				{
					for (int jj = 0; jj < size; jj++)
					{
						newPixelX += bufferSrc[ (i+ii-size2)*w + (j+jj-size2) ] * sobelKernelX[ii][jj];
						newPixelY += bufferSrc[ (i+ii-size2)*w + (j+jj-size2) ] * sobelKernelY[ii][jj];
					}
				}
				bufferX[ i*w + j ] = newPixelX / sumX;
				bufferY[ i*w + j ] = newPixelY / sumY;
			}
		}


		//vector module and direction
		for (int i = 0; i<dwHeight; i++)
		{
			for (int j = 0; j<dwWidth; j++)
			{
				float module = sqrt( bufferX[i*w + j]*bufferX[i*w + j] + bufferY[i*w + j]*bufferY[i*w + j] );
				float direction = atan( bufferY[i*w + j] / bufferX[i*w + j] ) * 180 / PI;

				if (i < 4 || i > dwHeight-5 || j < 4 || j > dwWidth-5)
				{
					module = 0;
				}


				bufferX[i*w + j] = module;
				bufferY[i*w + j] = direction;
				bufferSrc[i*w + j] = module;

				//lpDst[i*w + j] = module;
			}
		}
		
		//Non maxima suppression of the gradient's module
		//Adica subtiez edge-urile
		for (int i = 1; i<dwHeight-1; i++)
		{
			for (int j = 1; j<dwWidth-1; j++)
			{
				float direction = bufferY[i*w + j];

				if ( (direction > 67.5 && direction < 112.5) || (direction > 247.5 && direction < 292.5) )
				{
					//case 0
					if ( bufferX[i*w + j] < bufferX[(i-1)*w +j+0] || bufferX[i*w + j] < bufferX[(i+1)*w +j+0] )
					{
						bufferSrc[i*w+j] = 0;
					}
				}

				if ( (direction >= 22.5 && direction <= 67.5) || (direction >= 202.5 && direction <= 247.5) )
				{
					//case 1
					if ( bufferX[i*w + j] < bufferX[(i-1)*w +j+1] || bufferX[i*w + j] < bufferX[(i+1)*w +j-1] )
					{
						bufferSrc[i*w+j] = 0;
					}
				}

				if ( (direction > 337.5 || direction < 22.5) || (direction > 157.5 && direction < 202.5) )
				{
					//case 2
					if ( bufferX[i*w + j] < bufferX[(i+0)*w +j-1] || bufferX[i*w + j] < bufferX[(i+0)*w +j+1] )
					{
						bufferSrc[i*w+j] = 0;
					}
				}

				if ( (direction >= 112.5 && direction <= 157.5) || (direction >= 292.5 && direction <= 337.5) )
				{
					//case 3
					if ( bufferX[i*w + j] < bufferX[(i-1)*w +j-1] || bufferX[i*w + j] < bufferX[(i+1)*w +j+1] )
					{
						bufferSrc[i*w+j] = 0;
					}
				}

				//Compute hystogram
				int intensity = bufferSrc[i*w+j]/5.7;
				if (intensity > 255)
					intensity = 255;
				hist[ intensity ]++;
				
				lpDst[i*w + j] = bufferSrc[i*w+j];
				

			}
		}

		//Adaptive thresholding
		float p = 0.1; //quantity of edges
		float t = 0.4; //min threshold
		int d = 2;	//distance to a weak pixel to be taken as connected
		int weak = 40;
		int weakS = 150;
		int noNoEdge = (1-p) * (dwHeight * w - hist[0]);

		int count = 0;
		int threshold = 1;
		while (count < noNoEdge && threshold < 255)
		{
			count = count + hist[threshold];
			threshold++;
		}

		int minThreshold = threshold * t;

		for (int i = 1; i<dwHeight-1; i++)
		{
			for (int j = 1; j<dwWidth-1; j++)
			{
				lpDst[i*w + j] = 0;
				if ( bufferSrc[i*w+j] > threshold )
				{
					lpDst[i*w + j] = 255;
				} 
				else if(bufferSrc[i*w+j] > minThreshold)
				{
					lpDst[i*w + j] = weak;
				}
			}
		}


		for (int i = 1; i<dwHeight-1; i++)
		{
			for (int j = 1; j<dwWidth-1; j++)
			{
				if (lpDst[i*w + j] == 255)
				{
					enqueue(i, j, 255);
					while (front != NULL)
					{
						pixel *p = dequeue();
						int x = p->x;
						int y = p->y;
						lpDst[x*w + y] = weakS;
						free(p);
						for (int ii = x-d; ii < x+d; ii++)
						{
							if (ii > 0 && ii < dwHeight) //check for out of bounds
							{
								for (int jj = y-d; jj < y+d; jj++)
								{
									if (jj > 0 && jj < dwWidth) //out of bounds
									{
										if (lpDst[ii*w + jj] == weak)
										{
											enqueue(ii, jj, weak);
										}
									}
								}
							
							}
						}
					} //while
					lpDst[i*w + j] = 255;

				}
			}
		}


		free(bufferX);
		free(bufferY);
		free(bufferSrc);
		free(hist);
		
		END_PROCESSING("Edge detection");
	}

	/*
 FFT/IFFT routine. (see pages 507-508 of Numerical Recipes in C)

 Inputs:
	data[] : array of complex* data points of size 2*NFFT+1.
		data[0] is unused,
		* the n'th complex number x(n), for 0 <= n <= length(x)-1, is stored as:
			data[2*n+1] = real(x(n))
			data[2*n+2] = imag(x(n))
		if length(Nx) < NFFT, the remainder of the array must be padded with zeros

	nn : FFT order NFFT. This MUST be a power of 2 and >= length(x).
	isign:  if set to 1, 
				computes the forward FFT
			if set to -1, 
				computes Inverse FFT - in this case the output values have
				to be manually normalized by multiplying with 1/NFFT.
 Outputs:
	data[] : The FFT or IFFT results are stored in data, overwriting the input.
*/

void four1(double data[], int nn, int isign)
{
    int n, mmax, m, j, istep, i;
    double wtemp, wr, wpr, wpi, wi, theta;
    double tempr, tempi;
    
    n = nn << 1;
    j = 1;
    for (i = 1; i < n; i += 2) {
	if (j > i) {
	    tempr = data[j];     data[j] = data[i];     data[i] = tempr;
	    tempr = data[j+1]; data[j+1] = data[i+1]; data[i+1] = tempr;
	}
	m = n >> 1;
	while (m >= 2 && j > m) {
	    j -= m;
	    m >>= 1;
	}
	j += m;
    }
    mmax = 2;
    while (n > mmax) {
	istep = 2*mmax;
	theta = TWOPI/(isign*mmax);
	wtemp = sin(0.5*theta);
	wpr = -2.0*wtemp*wtemp;
	wpi = sin(theta);
	wr = 1.0;
	wi = 0.0;
	for (m = 1; m < mmax; m += 2) {
	    for (i = m; i <= n; i += istep) {
		j =i + mmax;
		tempr = wr*data[j]   - wi*data[j+1];
		tempi = wr*data[j+1] + wi*data[j];
		data[j]   = data[i]   - tempr;
		data[j+1] = data[i+1] - tempi;
		data[i] += tempr;
		data[i+1] += tempi;
	    }
	    wr = (wtemp = wr)*wpr - wi*wpi + wr;
	    wi = wi*wpr + wtemp*wpi + wi;
	}
	mmax = istep;
    }
}


	typedef struct punct{
	int x;
	int y;
	};

	bool punctEgal(punct a, punct b){
		return ((a.x==b.x)&&(a.y==b.y));
	}

	void CDibView::OnProcessingProject()
		{


		BEGIN_PROCESSING();



		int threshold = 200;

		for (int i=1; i<dwHeight-1; i++)
		{
			for (int j=1; j<dwWidth-1; j++)
			{
				if (lpSrc[i*w+j] > threshold)
					lpSrc[i*w+j] = 255;
				else
					lpSrc[i*w+j] = 0;

			}
		}
		
		//int filter[10] = {0, -1, 0,   -1, 4, -1,   0, -1, 0,   1};
		//float pixel;

		//for (int i=1; i<dwHeight-1; i++)
		//{
		//	for (int j=1; j<dwWidth-1; j++)
		//	{
		//		pixel = lpSrc[(i-1)*w+j-1]*filter[0] + lpSrc[(i-1)*w+j+0]*filter[1] + lpSrc[(i-1)*w+j+1]*filter[2] +
		//				lpSrc[(i+0)*w+j-1]*filter[3] + lpSrc[(i+0)*w+j+0]*filter[4] + lpSrc[(i+0)*w+j+1]*filter[5] +
		//				lpSrc[(i+1)*w+j-1]*filter[6] + lpSrc[(i+1)*w+j+0]*filter[7] + lpSrc[(i+1)*w+j+1]*filter[8];
		//		pixel = pixel / filter[9];

		//		if (pixel < 0)
		//		{
		//			pixel = 0;
		//		}
		//		if (pixel > 255)
		//		{
		//			pixel = 255;
		//		}
		//		
		//		lpDst[i*w+j] = pixel;
		//	}
		//}
		
		int x, y;
		bool found = false;
		
		int dx[] ={1,1,0,-1,-1,-1,0,1};
		int dy[] = {0,1,1,1,0,-1,-1,-1};
		int p[1000];
		punct p0,p1,pn1,pn;
		memset(lpDst,255,w*dwHeight);

		for (int i=1; i<dwHeight-1; i++)
		{
			for (int j=1; j<dwWidth-1; j++)
			{
				if (lpSrc[i*w+j] == 0 )
				{
					x = i;
					y = j;
					p0.x = j;
					p0.y = i;

					found = true;
					break;
				}
			}
			if (found)
				break;
		}

		int dir = 7;
		int toate = 1;
		p1 = p0;
		pn1 = p0;
		pn = p1;
		p1.x = -1;
		p1.y = -1;
		int a = 7;
		int b = 7;
		int count = 0;
		int jump = 0;

		int Nx;
		int NFFT;
		double *X;

		X = (double *) malloc((4000) * sizeof(double)); //2*NFFT+1

		//determine rest of the points
		while((!( (toate > 2) && (punctEgal(p0,pn1)) &&(punctEgal(p1,pn)) ))&&(toate < 1500))
		{
			if(dir % 2 == 1){
				//cazul (e)
				dir = (dir + 6) % 8 ;
			}
			else{
				//cazul (d)
				dir = (dir + 7) % 8;
		
			}
			//next direction
			while(lpSrc[(pn.y + dy[dir]) * w + (pn.x + dx[dir])]!=0){
				dir = (dir + 1 )%8;
			}
			//next pixel
			pn1.x = pn.x;
			pn1.y = pn.y;
			pn.x += dx[dir];
			pn.y += dy[dir];
			if(p1.x<0){
				p1.x = pn.x;
				p1.y = pn.y;
			}
		
	
			
			//compute derivative
			b = a;
			a = dir;
			int dd = (b-a+8)%8;


			//destination image
			if (jump > 3)
			{
				//lpDst[pn.y*w+pn.x]= 0;  //y - real part
				jump = 0;
				X[2*count+1] = pn.y; //real;
				X[2*count+2] = pn.x; //imaginary;
				count ++;
			}
			jump ++;
			toate++;
		}

		Nx = count;
		NFFT = (int)pow(2.0, ceil(log((double)Nx)/log(2.0)));

		/* pad the remainder of the array with zeros (0 + 0 j) */
		for(int i=Nx; i<NFFT; i++)
		{
			X[2*i+1] = 0.0;
			X[2*i+2] = 0.0;
		}
		four1(X, NFFT, 1);

		for(int i=20; i<NFFT; i++) 
		{
			X[2*i+1] = 0;
			X[2*i+2] = 0;
		}


		/* calculate IFFT */
		four1(X, NFFT, -1);

		/* normalize the IFFT */
		for(int i=0; i<NFFT; i++)
		{
			X[2*i+1] /= NFFT;
			X[2*i+2] /= NFFT;

			int xx = (int)X[2*i+1];
			int yy = (int)X[2*i+2];

			if (xx > 0 && xx < dwHeight && yy > 0 && yy < dwWidth)
			{
				lpDst[xx*w+yy] = 0; //real
			}
		}

		

		END_PROCESSING("CONTOUR");
	}


	
	void CDibView::OnProcessingTest()
	{
		
		BEGIN_PROCESSING();

		int XMIN = -5000;
		int XMAX = 5000;
		int ZMIN = 5000;
		int ZMAX = 30000;

		double projmat[3][4] = 
		//{{384.7287292, -0.5574115, 245.3588409, 595624.6250000},
		//{5.5824723, 384.1051941, 186.2327423, 974148.1875000},
		//{0.0113787, 0.0116350, 0.9998676, 2145.4038086}};

		{{384.7287292, -0.5574115, 245.3588409, 467563.9062500},
			{5.5824723, 384.1051941, 186.2327423, 974148.1875000},
			{0.0113787, 0.0113787, 0.9998676, 2145.4038086}};



		//for (int i=dwHeight - 1; i>=0; i--)
		//{
		//	for (int j=0; j<dwWidth-1; j++)
		//	{
		//		int x = XMIN + (XMAX-XMIN)*j/dwWidth;
		//		int z = ZMIN + (ZMAX - ZMIN)*(dwHeight - i - 1) / dwHeight;

		//		double uw = projmat[0][0] * x + projmat[0][2] * z + projmat[0][3]; 
		//		double vw = projmat[1][0] * x + projmat[1][2] * z + projmat[1][3]; 
		//		double ww = projmat[2][0] * x + projmat[2][2] * z + projmat[2][3]; 

		//		double u = uw/ww;
		//		double v = vw/ww;

		//		lpDst[i*w+j] = lpSrc[(int)((dwHeight -v -1)*w+u)];
		//	}
		//}

		double sigmaY = 2;
		double sigmaX = 2;
		int H = 25;
		int W = 5;

		int H2 = H/2;
		int W2 = W/2;

		double kernelY[25];
		double kernelX[21];

		for (int i = 0; i < H; i++)
		{
			kernelY[i] = exp(-((H2-i)*(H2-i))/(sigmaY*sigmaY*2));
		}

		for (int i = 0; i < W; i++ )
		{
			kernelX[i] = ( exp(-(W2-i)*(W2-i)/2*sigmaX*sigmaX) * ( 1 - ((W2-i)*(W2-i)/(sigmaX*sigmaX))) ) /(sigmaX*sigmaX);
			//kernelX[i] = (1- exp(-(W2-i)*(W2-i)/2*sigmaX*sigmaX)) ;// * ( 1 - ((W2-i)*(W2-i)/(sigmaX*sigmaX))) ) /(sigmaX*sigmaX);
		}

		for (int i = H2; i < dwHeight - H2; i++)   //2
		{
			for (int j = W2; j < dwWidth - W2; j++)
			{
				//parcurg imaginea destinatie   5   2  
				double sum = 0;
				for (int ii = 0; ii < H; ii++)  //2-2 = 0   - >    512-2+2 = 512  OK
				{
					sum = sum + lpSrc[(i + ii - H2)*w+j] * kernelY[ii];
				}
				for (int jj = 0; jj < W; jj++)
				{
					sum = sum + lpSrc[i*w + (j + jj - W2)] * kernelX[jj];
				}
				int tmp = sum/5;  //5H
				if (tmp < 0)
					tmp = 0;
				if (tmp > 255)
					tmp = 255;
				lpDst[i*w+j] = tmp;
			}
		}

		for (int i=0; i< dwHeight; i++)
		{
			for (int j=0; j<dwWidth-1; j++)
			{
				if ( lpDst[i*w+j] > 240 )
					lpDst[i*w+j] = 255;
				else
					lpDst[i*w+j] = 0;
			}
		}


		END_PROCESSING("Test");
	}