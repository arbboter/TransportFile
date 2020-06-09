
// VPlayerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "VPlayer.h"
#include "VPlayerDlg.h"
#include "afxdialogex.h"
#include "resource.h"
#include <dshow.h>
#include <atlconv.h>

#pragma comment(lib, "strmiids.lib" )

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CVPlayerDlg dialog




CVPlayerDlg::CVPlayerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CVPlayerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_pPlayDlg = new CPlayList(); 
}

CVPlayerDlg::~CVPlayerDlg()
{
   delete m_pPlayDlg;
}

void CVPlayerDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_PROGRESS1, m_vTextProgress);
}

BEGIN_MESSAGE_MAP(CVPlayerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDOK, &CVPlayerDlg::OnBnClickedOk)
    ON_BN_CLICKED(IDCANCEL, &CVPlayerDlg::OnBnClickedCancel)
    ON_WM_CLOSE()
    ON_WM_SIZE()
    ON_WM_DROPFILES()
    ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnToolNotify)
    ON_COMMAND(ID_BTN_FULLSCREEN, &CVPlayerDlg::OnBtnFullscreen)
    ON_COMMAND(ID_BTN_LAST, &CVPlayerDlg::OnBtnLast)
    ON_COMMAND(ID_BTN_NEXT, &CVPlayerDlg::OnBtnNext)
    ON_COMMAND(ID_BTN_PAUSE, &CVPlayerDlg::OnBtnPause)
    ON_COMMAND(ID_BTN_PLAY, &CVPlayerDlg::OnBtnPlay)
    ON_COMMAND(ID_BTN_STOP, &CVPlayerDlg::OnBtnStop)
    ON_WM_TIMER()
    ON_WM_MOVE()
    ON_COMMAND(ID_FILE_OPEN, &CVPlayerDlg::OnFileOpen)
    ON_COMMAND(ID_WND_PLAYLIST, &CVPlayerDlg::OnWndPlaylist)
END_MESSAGE_MAP()


// CVPlayerDlg message handlers

void CVPlayerDlg::InitToolBar()
{
    // ����������
    if (!m_vToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_BOTTOM
        | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
        !m_vToolBar.LoadToolBar(IDR_TOOLBAR1))
    {
        MessageBox(L"����������ʧ��", L"��ʾ", MB_OK|MB_ICONINFORMATION);
        return;
    }

    // ����ͼ��
    int nSize = 72;
    m_imgToolBar.Create(nSize, nSize, ILC_COLOR32|ILC_MASK, 0, 0);
    
    // ����ͼƬ
    DWORD pIcons[] = {IDI_ICON_LAST, IDI_ICON_PLAY, IDI_ICON_PAUSE, IDI_ICON_STOP, 
                      IDI_ICON_NEXT, IDI_ICON_FULLSCREEN};
    int nNum = sizeof(pIcons)/sizeof(pIcons[0]);
    HICON hIco;
    for(INT i=0; i<nNum; ++i)
    {
        hIco = AfxGetApp()->LoadIcon(pIcons[i]);
        m_imgToolBar.Add(hIco);
    }
    
    // ������������ͼ��
    CToolBarCtrl& tbc = m_vToolBar.GetToolBarCtrl();
    tbc.SetImageList(&m_imgToolBar);

    // ��ʼ��DirectShow
    if(!InitDirectShow())
    {
        AfxMessageBox(L"Init DirectShow Env Failed.");
    }

    RepositionBars(AFX_IDW_CONTROLBAR_FIRST,AFX_IDW_CONTROLBAR_LAST,0);
}

BOOL CVPlayerDlg::InitDirectShow()
{
    // Init COM
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)){
        AfxMessageBox(L"Error - Can't init COM.");
        return FALSE;
    }

    // Create FilterGraph
    hr=CoCreateInstance(CLSID_FilterGraph, NULL,CLSCTX_INPROC_SERVER,IID_IGraphBuilder, (void **)&m_pGraph);
    if (FAILED(hr)){
        AfxMessageBox(L"Error - Can't create Filter Graph.");
        return FALSE;
    }
    //  Query Interface
    hr = m_pGraph->QueryInterface(IID_IMediaControl, (void **)&m_pControl);
    hr |= m_pGraph->QueryInterface(IID_IMediaEvent, (void **)&m_pEvent);
    hr |= m_pGraph->QueryInterface(IID_IMediaControl, (void **)&m_pControl);
    hr |= m_pGraph->QueryInterface(IID_IMediaEvent, (void **)&m_pEvent);
    hr |= m_pGraph->QueryInterface(IID_IBasicVideo, (void **)&m_pVideo);
    hr |= m_pGraph->QueryInterface(IID_IBasicAudio, (void **)&m_pAudio);
    hr |= m_pGraph->QueryInterface(IID_IVideoWindow, (void **)&m_pWindow);
    hr |= m_pGraph->QueryInterface(IID_IMediaSeeking, (void **)&m_pSeeking);
    if (FAILED(hr)){
        AfxMessageBox(L"Error - Can't Query Interface.");
        return FALSE;
    }

    return TRUE;
}

void CVPlayerDlg::UnitDirectShow()
{
    if(m_pVideo)
    {
        m_pVideo->Release();
    }
    if(m_pAudio)
    {
        m_pAudio->Release();
    }
    if(m_pWindow)
    {
        m_pWindow->Release();
    }
    if(m_pSeeking)
    {
        m_pSeeking->Release();
    }
    if(m_pControl)
    {
        m_pControl->Release();
    }
    if(m_pEvent)
    {
        m_pEvent->Release();
    }
    if(m_pGraph)
    {
        m_pGraph->Release();
    }

    CoUninitialize();
}

BOOL CVPlayerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

    // ���ñ�����
    SetWindowText(L"ϫ�²�����");

    // ���ز˵�
    m_vMenu.LoadMenu(IDR_MENU_MAIN);
    SetMenu(&m_vMenu);

    // ��ʼ�����ſ������
    InitToolBar();
    // ��ʼ��������
    m_vTextProgress.SetRange(0, 100);
    m_vTextProgress.SetText(L"00:00:00", L"23:59:59", 0);

    // ��ʾ�����б���
    m_pPlayDlg->Create(IDD_DLG_PLAYLIST); 
    m_pPlayDlg->ShowWindow(SW_SHOWNORMAL);
    m_pPlayDlg->m_pMainView = this;
    OnWndPlaylist();

    // ѭ������
    m_pPlayDlg->m_nPlayMode = PLAY_MODE_CYCLE;

    // ��С����Ӧ
    PostMessage(WM_SIZE, 0, 0);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CVPlayerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CVPlayerDlg::OnPaint()
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
HCURSOR CVPlayerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CVPlayerDlg::OnBnClickedOk()
{
    
}

void CVPlayerDlg::OnBnClickedCancel()
{
    m_pWindow->put_FullScreenMode(OAFALSE);
}

void CVPlayerDlg::OnClose()
{
    EndDialog(IDCLOSE);
}


void CVPlayerDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialogEx::OnSize(nType, cx, cy);
    if(!IsWindow(m_vToolBar.GetSafeHwnd()))
    {
        return;
    }

    // ������
    CRect rtMain;
    GetClientRect(&rtMain);

    // ������
    CRect rtCtrl;
    m_vToolBar.GetClientRect(&rtCtrl);
    int nSize = 72;
    DWORD dwWidth = nSize*(m_imgToolBar.GetImageCount() + 1);
    rtCtrl.top = rtMain.bottom - 72;
    rtCtrl.left = (rtMain.right-dwWidth)/2;
    rtCtrl.right = rtMain.right - rtCtrl.left;
    rtCtrl.bottom = rtMain.bottom;
    m_vToolBar.MoveWindow(rtCtrl);

    // ������
    CRect rtProgress;
    m_vTextProgress.GetClientRect(&rtProgress);
    DWORD dwHeight = rtProgress.Height();
    rtProgress = rtMain;
    rtProgress.bottom = rtCtrl.top;
    rtProgress.top = rtProgress.bottom  - dwHeight;
    m_vTextProgress.MoveWindow(rtProgress);

    // ��Ƶ��С�л�
    OnVideoWndSizeChange();

    // ����������
    SetPlayerBackground();
}

BOOL CVPlayerDlg::OnToolNotify(UINT id,NMHDR* pNMHDR,LRESULT* pResult)

{
    TOOLTIPTEXT* pT = (TOOLTIPTEXT*)pNMHDR;
    UINT nID = pNMHDR->idFrom;
    switch(nID)
    {
    case ID_BTN_LAST: 
        pT->lpszText= _T("�ϼ�");
        break;
    case ID_BTN_PLAY:
        pT->lpszText= _T("����");
        break;
    case ID_BTN_PAUSE:
        {
            OAFilterState nState;
            m_pControl->GetState(1000, &nState);
            if(nState == State_Paused)
            {
                pT->lpszText= _T("����");
            }
            else
            {
                pT->lpszText= _T("��ͣ");
            }
        }
        break;
    case ID_BTN_STOP:
        pT->lpszText= _T("ֹͣ");
        break;
    case ID_BTN_NEXT:
        pT->lpszText= _T("�¼�");
        break;
    case ID_BTN_FULLSCREEN:
        pT->lpszText= _T("ȫ��");
        break;
    default:
        break;
    }
    return FALSE;
}


void CVPlayerDlg::GetVRect(CRect& rtV)
{
    CRect rtMain, rtControl;
    GetClientRect(&rtMain);
    m_vTextProgress.GetClientRect(&rtControl);

    rtV = rtMain;
    rtV.bottom = rtV.top + rtMain.Height() - rtControl.Height();
    m_vToolBar.GetClientRect(&rtControl);
    rtV.bottom -= rtControl.Height();
}

void CVPlayerDlg::OnBtnFullscreen()
{
    m_pWindow->put_FullScreenMode(OATRUE);
}


void CVPlayerDlg::OnBtnLast()
{
    if(m_pPlayDlg->Last() < 0)
    {
        return;
    }
    OnBtnPlay();
}


void CVPlayerDlg::OnBtnNext()
{
    if(m_pPlayDlg->Next() < 0)
    {
        return;
    }
    OnBtnPlay();
}


void CVPlayerDlg::OnBtnPause()
{
    OAFilterState nState = State_Stopped;
    m_pControl->GetState(1000, &nState);
    // �ָ�����
    HICON hIco = NULL;
    if(nState == State_Paused)
    {
        hIco = AfxGetApp()->LoadIcon(IDI_ICON_PAUSE);
        m_pControl->Run();
    }
    // ��ͣ
    else if(nState == State_Running)
    {
        hIco = AfxGetApp()->LoadIcon(IDI_ICON_RESUME);
        m_pControl->Pause();
    }
    m_imgToolBar.Replace(3, hIco);
}

void CVPlayerDlg::OnVideoWndSizeChange()
{
    // ���ò��Ŵ���
    if(m_pWindow)
    {
        OAHWND hWnd = (OAHWND)this->GetSafeHwnd();
        CRect rt;
        GetVRect(rt);

        m_pWindow->put_Visible(OAFALSE);
        m_pWindow->put_Owner(hWnd);
        m_pWindow->put_Left(rt.left);
        m_pWindow->put_Top(rt.top);
        m_pWindow->put_Width(rt.Width());
        m_pWindow->put_Height(rt.Height());
        // �в�����ע��IVideoWindow��put_FullScreenMode()��Win7����������ġ�
        // ֻ�������ô�����ʽ�ĵ�ʱ������ʽ��ָ��WS_THICKFRAME��ſ�������ʹ��
        // 
        // ������ʵ������
        // Win10�µ�X64 VS2010���Բ���Ҫ����WS_THICKFRAME�����������˳�ȫ��ʱ������
        m_pWindow->put_WindowStyle(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS);
        m_pWindow->put_MessageDrain(hWnd);
        m_pWindow->put_Visible(OATRUE);
    }
}

void CVPlayerDlg::SetPlayerBackground()
{
    CRect rtMain;
    GetClientRect(&rtMain);
    CRect rtPlayer;
    GetVRect(rtPlayer);


    // ���ò���������
    InvalidateRect(rtMain);
    UpdateWindow();

    CDC * pDC = GetDC();
    
    // ����������ɫ
    pDC->FillSolidRect(rtPlayer, RGB(0, 0, 0));

    // ��д��������������
    CFont font;
    // ����ţ��߶ȵ�1/4
    int nFontSize = rtPlayer.Height()/5;
    font.CreatePointFont(nFontSize*10, TEXT("����"));
    pDC->SelectObject(&font); 
    
    // �ۺ�ɫ
    pDC->SetTextColor(RGB(255, 192, 203));
    pDC->SetBkMode(TRANSPARENT);
    rtPlayer.OffsetRect(0, nFontSize);
    pDC->DrawText(L"ϫ��", -1, &rtPlayer, DT_SINGLELINE | DT_CENTER) ;
    rtPlayer.OffsetRect(0, nFontSize*2);
    pDC->DrawText(L"������", -1, &rtPlayer, DT_SINGLELINE | DT_CENTER) ;
    Invalidate(FALSE);
}

void CVPlayerDlg::OnBtnPlay()
{
    // ��鲥���б�
    // m_arPlayList.Add(L"D:\\temp\\test.avi");
    CPlayItem oItem;
    if(m_pPlayDlg->GetCurSel(oItem) < 0)
    {
        AfxMessageBox(L"δѡ�в����������");
        return;
    }

    // ������ڲ��ţ�����ֹͣ
    OAFilterState nState = State_Stopped;
    m_pControl->GetState(1000, &nState);
    HICON hIco = NULL;
    if(nState != State_Stopped)
    {
        OnBtnStop();
    }

    // �˴���������ļ�ʧ�ܣ��ܿ�������Ϊ��������֧�֣��谲װ���ռ�����
    // �����MFC����ʱ���˴����³����쳣����������ΪMFC��֧�ָõ��ԣ��������м���
    HRESULT hr = m_pGraph->RenderFile(oItem.m_strPath, NULL);
    if (FAILED(hr)){
        AfxMessageBox(L"Error - Can't Render File.");
        return;
    }

    // ���ò��Ŵ���
    OnVideoWndSizeChange();

    // ���ò���
    SetTimer(1,1000,NULL);

    // ����
    m_pControl->Run();

    // ���ý�����
    LONGLONG duration;
    CString  strDura;
    m_pSeeking->GetDuration(&duration);
    if(duration != 0){
        int tns = int(duration/10000000);
        int thh  = tns / 3600;
        int tmm  = (tns % 3600) / 60;
        int tss  = (tns % 60);
        strDura.Format(_T("%02d:%02d:%02d"), thh, tmm, tss);
    }
    m_vTextProgress.SetText(L"00:00:00", strDura, 0);
}


void CVPlayerDlg::OnBtnStop()
{
    // ֹͣ
    KillTimer(1);
    long long position = 0;
    HRESULT hr;
    hr = m_pSeeking->SetPositions(&position, 
        AM_SEEKING_AbsolutePositioning | AM_SEEKING_SeekToKeyFrame, 
        0, AM_SEEKING_NoPositioning);
    hr = m_pControl->Stop();

    // Enumerate the filters And remove them
    IEnumFilters *pEnum = NULL;
    hr = m_pGraph->EnumFilters(&pEnum);
    if (SUCCEEDED(hr))
    {
        IBaseFilter *pFilter = NULL;
        while (S_OK == pEnum->Next(1, &pFilter, NULL))
        {
            // Remove the filter.
            m_pGraph->RemoveFilter(pFilter);
            // Reset the enumerator.
            pEnum->Reset();
            pFilter->Release();
        }
        m_pGraph->Release();
    }
}

void CVPlayerDlg::OnDropFiles(HDROP hDropInfo)
{
    //ȡ�ñ��϶��ļ�����Ŀ
    TCHAR szName[MAX_PATH] = {0};
    int nNum = DragQueryFile(hDropInfo,-1,NULL,0);
    CPlayItem oItem;
    for(int i=0;i< nNum; i++)
    {
        DragQueryFile(hDropInfo,i, szName, MAX_PATH);
        m_pPlayDlg->Add(szName);
    } 
    //�ϷŽ�����,�ͷ��ڴ�
    DragFinish(hDropInfo);

    // ���û��ѡ�У�Ĭ��ѡ��һ��
    if(m_pPlayDlg->GetCurSel() < 0)
    {
        m_pPlayDlg->SetCurSel(0);
    }
    OnBtnPlay();
}

void CVPlayerDlg::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent == 1)
    {
        CString strCurTime, strDura;
        long long curtime;
        long long duration;
        int  tns, thh, tmm, tss;
        int progress;
        // ��ǰ����ʱ�䣺ns
        m_pSeeking->GetCurrentPosition(&curtime);
        // change to second
        tns = int(curtime/10000000);
        thh  = tns / 3600;
        tmm  = (tns % 3600) / 60;
        tss  = (tns % 60);
        strCurTime.Format(_T("%02d:%02d:%02d"), thh, tmm, tss);
        // �ܲ���ʱ��
        m_pSeeking->GetDuration(&duration);
        progress = int(curtime*100/duration);
        m_vTextProgress.SetCurText(strCurTime, progress);
    }
    CDialogEx::OnTimer(nIDEvent);
}


void CVPlayerDlg::OnMove(int x, int y)
{
    CDialogEx::OnMove(x, y);
    if(!IsWindow(m_pPlayDlg->GetSafeHwnd()) || !m_pPlayDlg->IsWindowVisible())
    {
        return;
    }
    // ������
    CRect rtMain, rtCtrl;
    GetWindowRect(&rtMain);

    // �����б�
    m_pPlayDlg->GetWindowRect(&rtCtrl);
    DWORD dwWidth = rtCtrl.Width();
    rtCtrl.top = rtMain.top;
    rtCtrl.bottom = rtMain.bottom - 5;
    rtCtrl.left = rtMain.right - 8;
    rtCtrl.right = rtCtrl.left + dwWidth;
    m_pPlayDlg->MoveWindow(rtCtrl);
}


void CVPlayerDlg::OnFileOpen()
{
    // ����Ƶ�ļ�������
    CString strfilter;
    strfilter.Append(_T("Common media formats|*.avi;*.wmv;*.wmp;*.wm;*.asf;*.rm;*.ram;*.rmvb;*.ra;*.mpg;*.mpeg;*.mpe;*.m1v;*.m2v;*.mpv2;"));
    strfilter.Append(_T("*.mp2v;*.dat;*.mp4;*.m4v;*.m4p;*.vob;*.ac3;*.dts;*.mov;*.qt;*.mr;*.3gp;*.3gpp;*.3g2;*.3gp2;*.swf;*.ogg;*.wma;*.wav;"));
    strfilter.Append(_T("*.mid;*.midi;*.mpa;*.mp2;*.mp3;*.m1a;*.m2a;*.m4a;*.aac;*.mkv;*.ogm;*.m4b;*.tp;*.ts;*.tpr;*.pva;*.pss;*.wv;*.m2ts;*.evo;"));
    strfilter.Append(_T("*.rpm;*.realpix;*.rt;*.smi;*.smil;*.scm;*.aif;*.aiff;*.aifc;*.amr;*.amv;*.au;*.acc;*.dsa;*.dsm;*.dsv;*.dss;*.pmp;*.smk;*.flic|"));
    strfilter.Append(_T("Windows Media Video(*.avi;*wmv;*wmp;*wm;*asf)|*.avi;*.wmv;*.wmp;*.wm;*.asf|"));
    strfilter.Append(_T("Windows Media Audio(*.wma;*wav;*aif;*aifc;*aiff;*mid;*midi;*rmi)|*.wma;*.wav;*.aif;*.aifc;*.aiff;*.mid;*.midi;*.rmi|"));
    strfilter.Append(_T("Real(*.rm;*ram;*rmvb;*rpm;*ra;*rt;*rp;*smi;*smil;*.scm)|*.rm;*.ram;*.rmvb;*.rpm;*.ra;*.rt;*.rp;*.smi;*.smil;*.scm|"));
    strfilter.Append(_T("MPEG Video(*.mpg;*mpeg;*mpe;*m1v;*m2v;*mpv2;*mp2v;*dat;*mp4;*m4v;*m4p;*m4b;*ts;*tp;*tpr;*pva;*pss;*.wv;)|"));
    strfilter.Append(_T("*.mpg;*.mpeg;*.mpe;*.m1v;*.m2v;*.mpv2;*.mp2v;*.dat;*.mp4;*.m4v;*.m4p;*.m4b;*.ts;*.tp;*.tpr;*.pva;*.pss;*.wv;|"));
    strfilter.Append(_T("MPEG Audio(*.mpa;*mp2;*m1a;*m2a;*m4a;*aac;*.m2ts;*.evo)|*.mpa;*.mp2;*.m1a;*.m2a;*.m4a;*.aac;*.m2ts;*.evo|"));
    strfilter.Append(_T("DVD(*.vob;*ifo;*ac3;*dts)|*.vob;*.ifo;*.ac3;*.dts|MP3(*.mp3)|*.mp3|CD Tracks(*.cda)|*.cda|"));
    strfilter.Append(_T("Quicktime(*.mov;*qt;*mr;*3gp;*3gpp;*3g2;*3gp2)|*.mov;*.qt;*.mr;*.3gp;*.3gpp;*.3g2;*.3gp2|"));
    strfilter.Append(_T("Flash Files(*.flv;*swf;*.f4v)|*.flv;*.swf;*.f4v|Playlist(*.smpl;*.asx;*m3u;*pls;*wvx;*wax;*wmx;*mpcpl)|*.smpl;*.asx;*.m3u;*.pls;*.wvx;*.wax;*.wmx;*.mpcpl|"));
    strfilter.Append(_T("Others(*.ivf;*au;*snd;*ogm;*ogg;*fli;*flc;*flic;*d2v;*mkv;*pmp;*mka;*smk;*bik;*ratdvd;*roq;*drc;*dsm;*dsv;*dsa;*dss;*mpc;*divx;*vp6;*.ape;*.flac;*.tta;*.csf)"));
    strfilter.Append(_T("|*.ivf;*.au;*.snd;*.ogm;*.ogg;*.fli;*.flc;*.flic;*.d2v;*.mkv;*.pmp;*.mka;*.smk;*.bik;*.ratdvd;*.roq;*.drc;*.dsm;*.dsv;*.dsa;*.dss;*.mpc;*.divx;*.vp6;*.ape;*.amr;*.flac;*.tta;*.csf|"));
    strfilter.Append(_T("All Files(*.*)|*.*||"));

    // �ļ�����
    CFileDialog oFileDlg(TRUE,
        NULL,
        NULL,
        OFN_ALLOWMULTISELECT | OFN_ENABLESIZING | OFN_HIDEREADONLY,
        strfilter);
    const int nMaxFileNum = 128; 
    oFileDlg.m_ofn.lpstrFile = new TCHAR[_MAX_PATH * nMaxFileNum];
    memset(oFileDlg.m_ofn.lpstrFile, 0, _MAX_PATH * nMaxFileNum);
    oFileDlg.m_ofn.nMaxFile = _MAX_PATH * nMaxFileNum;
    CString strCurFile;
    CPlayItem oItem;
    if (IDOK == oFileDlg.DoModal())
    {
        POSITION pos = oFileDlg.GetStartPosition();
        while (NULL != pos)
        {
            strCurFile = oFileDlg.GetNextPathName(pos);
            m_pPlayDlg->Add(strCurFile);
        }
    }
    delete[] oFileDlg.m_ofn.lpstrFile; 

    // ���û��ѡ�У�Ĭ��ѡ��һ��
    if(m_pPlayDlg->GetCurSel() < 0)
    {
        m_pPlayDlg->SetCurSel(0);
    }
    OnBtnPlay();
}


void CVPlayerDlg::OnWndPlaylist()
{
    // ���ڲ˵�
    CMenu* pMenu = GetMenu()->GetSubMenu(1);
    if(pMenu == NULL)
    {
        return;
    }

    UINT nState = pMenu->GetMenuState(ID_WND_PLAYLIST, MF_BYCOMMAND);
    if(nState == MF_CHECKED)
    {
        pMenu->CheckMenuItem(ID_WND_PLAYLIST, MF_BYCOMMAND | MF_UNCHECKED);
        m_pPlayDlg->ShowWindow(SW_HIDE);
    }
    else
    {
        pMenu->CheckMenuItem(ID_WND_PLAYLIST, MF_BYCOMMAND | MF_CHECKED);
        m_pPlayDlg->ShowWindow(SW_SHOW);
    }
}
