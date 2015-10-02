
// Gesture_RecognitionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Gesture_Recognition.h"
#include "Gesture_RecognitionDlg.h"
#include "afxdialogex.h"
#include "cvheader.h"
#include "resource.h"

#include <conio.h>
#include <stdio.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

cv::CascadeClassifier face_cascade;
cv::CascadeClassifier fist_cascade;
CvCapture* capture;
CRect rect;
IplImage* mybackground=NULL;
CDC *pDC;
HDC hDC;
CWnd *pwnd;
int gamestartcounting = 0;
int computergesture = 0;
int usergesture = -1;//-1-undetected
// CGesture_RecognitionDlg dialog
#ifndef HISTORYQUEUE
#define HISTORYQUEUE
class historyqueue //To avoid error
{
public:
	historyqueue()
	{
		int i;
		for (i = 0; i < 5; i++) this->p[i] = -1;
		this->current = 0;
	}
	void push(int x)
	{
		this->p[this->current] = x;
		this->current++;
		this->current = this->current % 5;
	}
	int query()
	{
		int j;
		int q[3];
		for (j = 0; j < 3; j++) q[j] = 0;
		for (j = 0; j < 5; j++) if (this->p[j] != -1) q[this->p[j]]++;
		for (j = 0; j < 3; j++) if (q[j] >= 3) return j;
		return -1;
	}
private:
	int p[5];
	int current;
};
#endif
historyqueue* history = new historyqueue();
CGesture_RecognitionDlg::CGesture_RecognitionDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CGesture_RecognitionDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CGesture_RecognitionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CGesture_RecognitionDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON1, &CGesture_RecognitionDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CGesture_RecognitionDlg::OnBnClickedButton2)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_BUTTON3, &CGesture_RecognitionDlg::OnBnClickedButton3)
END_MESSAGE_MAP()


// CGesture_RecognitionDlg message handlers

BOOL CGesture_RecognitionDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	pwnd = GetDlgItem(IDC_STATIC);
	//pwnd->MoveWindow(35,30,352,288);  
	pDC = pwnd->GetDC();
	//pDC =GetDC();  
	hDC = pDC->GetSafeHdc();
	pwnd->GetClientRect(&rect);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

//Let's put the xml file to the resource.
//extract resource to the temp file
bool extract_rc(LPCTSTR dist, LPCTSTR type, LPCTSTR name)
{

	HANDLE tmpfile = ::CreateFile(dist, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);
	if (tmpfile == INVALID_HANDLE_VALUE)
		return false;


	HRSRC    res = ::FindResource(NULL, name, type);
	HGLOBAL    mem = ::LoadResource(NULL, res);
	DWORD    dw_size = ::SizeofResource(NULL, res);


	DWORD dw_write = 0;
	::WriteFile(tmpfile, mem, dw_size, &dw_write, NULL);
	::CloseHandle(tmpfile);

	return true;
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CGesture_RecognitionDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CGesture_RecognitionDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}




void CGesture_RecognitionDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if (nIDEvent == 1){
		IplImage* myframe;
		myframe = cvQueryFrame(capture);
		cv::Mat m = myframe;
		CvvImage m_CvvImage;
		m_CvvImage.CopyOf(myframe, 1);
		m_CvvImage.DrawToHDC(hDC, &rect);
		m_CvvImage.Destroy();
		usergesture = mygesturedetect(m);
		history->push(usergesture);
		switch (history->query())
		{
		case -1: GetDlgItem(IDC_STATICUSER)->SetWindowTextW(_T("You : Gesture Not Detected")); break;
		case 0: GetDlgItem(IDC_STATICUSER)->SetWindowTextW(_T("You : Scissor")); break;
		case 1: GetDlgItem(IDC_STATICUSER)->SetWindowTextW(_T("You : Rock")); break;
		case 2: GetDlgItem(IDC_STATICUSER)->SetWindowTextW(_T("You : Paper")); break;
		}
		
	}
	if (nIDEvent == 2){
		TCHAR buf[50];
		_itow_s(gamestartcounting--, buf,10);
		GetDlgItem(IDC_STATICCOUNT)->SetWindowTextW(buf);
		if (gamestartcounting < 0)
		{
			int u = history->query();
			switch (computergesture)
			{
			case 0: GetDlgItem(IDC_STATICCOUNT)->SetWindowTextW(_T("Computer: Scissor")); break;
			case 1: GetDlgItem(IDC_STATICCOUNT)->SetWindowTextW(_T("Computer: Rock")); break;
			case 2: GetDlgItem(IDC_STATICCOUNT)->SetWindowTextW(_T("Computer: Paper")); break;
			}
			if (u == computergesture)
			{

				GetDlgItem(IDC_STATICRESULT)->SetWindowTextW(_T("Tie"));
			}
			else if (u - computergesture == 1 || (u == 0 && computergesture == 2))//user win
			{
				CString tmp;
				GetDlgItem(IDC_EDIT1)->GetWindowText(tmp);
				_itow_s(_tstoi(tmp) + 1, buf, 30);
				GetDlgItem(IDC_EDIT1)->SetWindowTextW(buf);

				GetDlgItem(IDC_STATICRESULT)->SetWindowTextW(_T("You Win"));
			}
			else
			{
				CString tmp;
				GetDlgItem(IDC_EDIT2)->GetWindowText(tmp);
				_itow_s(_tstoi(tmp) + 1, buf, 30);
				GetDlgItem(IDC_EDIT2)->SetWindowTextW(buf);
				GetDlgItem(IDC_STATICRESULT)->SetWindowTextW(_T("You Lose"));
			}
			CString tmp,tmp2;
			switch (u)
			{
			case -1:tmp = "Undetected"; break;
			case 0:tmp = "Scissor"; break;
			case 1:tmp = "Rock"; break;
			case 2:tmp = "Paper"; break;
			}
			GetDlgItem(IDC_STATICRESULT)->GetWindowText(tmp2);
			GetDlgItem(IDC_STATICRESULT)->SetWindowText(tmp2+"  (You:"+tmp+")");
			KillTimer(2);
		}
	}
	CDialogEx::OnTimer(nIDEvent);
}


void CGesture_RecognitionDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	KillTimer(2);
	gamestartcounting = 3;
	srand((unsigned)time(NULL));
	computergesture = rand() % 3; //0-scissor, 1-rock, 2-paper
	SetTimer(2, 1000, NULL);
	char buf[30];
	_itoa_s(gamestartcounting--, buf, 2,10);
	GetDlgItem(IDC_STATICCOUNT)->SetWindowText(CString(buf));

}


void CGesture_RecognitionDlg::OnBnClickedButton2()
{
	// TODO: Add your control notification handler code here
	GetDlgItem(IDC_EDIT1)->SetWindowTextW(_T("0"));
	GetDlgItem(IDC_EDIT2)->SetWindowTextW(_T("0"));
	
}


int CGesture_RecognitionDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	TCHAR tmpxml[_MAX_PATH];
	::GetTempPath(_MAX_PATH, tmpxml);
	_tcscat_s(tmpxml, _T("face.xml"));
	
	if (!extract_rc(tmpxml, _T("XML"), MAKEINTRESOURCE(IDR_XML1)))
	{
		MessageBox(_T("FAIL TO EXTRACT FILE TO TMP"));
		return 0;
	}

	if (CDialogEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	
	if (!capture)
	{
		capture = cvCaptureFromCAM(0);		  
	}

	if (!capture)
	{
		MessageBox(_T("FAIL TO START CAMERA"));
		return 0;
	}
	
	std::wstring arr_w( tmpxml );
	std::string arr_s( arr_w.begin(), arr_w.end() );
	
	if (!face_cascade.load(arr_s)){ MessageBox(_T("Error loading haarcascade_frontalface_alt.xml\n")); };
	IplImage* m_Frame;
	m_Frame = cvQueryFrame(capture);
	CvvImage m_CvvImage;
	m_CvvImage.CopyOf(m_Frame, 1);
	if (true)
	{
		m_CvvImage.DrawToHDC(hDC, &rect);
		//cvWaitKey(10);  
	}

	DeleteFile(tmpxml);

	SetTimer(1, 11, NULL);
	
	return 0;
}


void CGesture_RecognitionDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: Add your message handler code here
	cvReleaseCapture(&capture);
}


void CGesture_RecognitionDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	// TODO: Add your message handler code here
	GetDlgItem(IDC_EDIT1)->SetWindowTextW(_T("0"));
	GetDlgItem(IDC_EDIT2)->SetWindowTextW(_T("0"));
	CFont *m_Font1 = new CFont;
	m_Font1->CreatePointFont(160, _T("Arial Bold"));
	CStatic * m_Label = (CStatic *)GetDlgItem(IDC_STATICCOUNT);
	m_Label->SetFont(m_Font1);
}


void CGesture_RecognitionDlg::OnBnClickedButton3()
{
	// TODO: Add your control notification handler code here
	if (mybackground) cvReleaseImage(&mybackground);
	mybackground = cvCloneImage(cvQueryFrame(capture));

}
