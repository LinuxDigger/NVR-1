#include "GUI/Pages/PageSearchRec.h"

#include "biz.h"
#include "GUI/Pages/BizData.h"
#include "GUI/Pages/PageMessageBox.h"
#include "System/Locales.h"
#include "GUI/Pages/PageDesktop.h"//cw_panel
#include "GUI/Pages/PageMainFrameWork.h"



#define RIGHT_PAGE_WIDTH	(204)
#define BOTTOM_PAGE_HEIGHT	(10/*上边沿*/+24/*刻度值*/+12*4/*进度条*/+24/*下边沿*/)	//106

//full screen page
CPageSearchRec::CPageSearchRec(VD_PCRECT pRect,VD_PCSTR psz/* = NULL*/,VD_BITMAP* icon/* = NULL*/,CPage * pParent/* = NULL*/, uint vstyle/* = 0*/)
:CPageFloat(pRect, pParent,psz)
{
	int i, j;

	printf("%s l:%d, r:%d, t:%d, b:%d\n", __func__, m_Rect.left, m_Rect.right, m_Rect.top, m_Rect.bottom);
	int right_page_l = m_Rect.Width()-RIGHT_PAGE_WIDTH;
	int right_page_b = m_Rect.Height()-BOTTOM_PAGE_HEIGHT;
	//frame lines
	//竖
	pFrameLines[0] = CreateStatic(CRect(0, 0, 2, m_Rect.Height()), this, "");
	pFrameLines[1] = CreateStatic(CRect(right_page_l-2, 0, right_page_l, right_page_b), this, "");
	pFrameLines[2] = CreateStatic(CRect(m_Rect.Width()-2, 0, m_Rect.Width(), m_Rect.Height()), this, "");
	//横
	pFrameLines[3] = CreateStatic(CRect(0, 0, m_Rect.Width(), 2), this, "");
	pFrameLines[4] = CreateStatic(CRect(0, right_page_b-2, m_Rect.Width(), right_page_b), this, "");
	pFrameLines[5] = CreateStatic(CRect(0, m_Rect.Height()-2, m_Rect.Width(), m_Rect.Height()), this, "");

	for (i=0; i<6; ++i)
	{
		pFrameLines[i]->SetBkColor(VD_RGB(232,232,232));
	}

	CRect rt;
	rt.left = right_page_l+10;
	rt.right = rt.left+100;
	rt.top = 10;	
	rt.bottom = rt.top + TEXT_HEIGHT;
	//title
	pTitle = CreateStatic(rt,this, psz);

	
	//日历
	rt.left = right_page_l+3;
	rt.right = m_Rect.Width()-3;
	rt.top = rt.bottom + 4;	
	rt.bottom = rt.top+TEXT_HEIGHT*8+10;
	
	pCalendar = CreateCalendar(rt, this, NULL, (CTRLPROC)&CPageSearchRec::OnDaySelected);
	//printf("Calendar rect:(%d,%d,%d,%d)\n",rt.left,rt.right,rt.top,rt.bottom);

	//录像类型
	rt.left = right_page_l+10;
	rt.right = rt.left+24;
	rt.top = rt.bottom +10;
	rt.bottom = rt.top + TEXT_HEIGHT;
	
	char* szChkText[5] = {
	"&CfgPtn.Timer", 
	"&CfgPtn.Moving",
	"&CfgPtn.Sensoring", 
	"&CfgPtn.Manual", 
	"&CfgPtn.All"};
	
	for(i = 0; i<5; i++)
	{
		pChkRecType[i] = CreateCheckBox(CRect(rt.left + (i%2)*100, rt.top + (i>>1)*(TEXT_HEIGHT+4), rt.left+(i%2)*100+24, rt.bottom + (i>>1)*(TEXT_HEIGHT+4)), this, styleEditable, (CTRLPROC)&CPageSearchRec::OnEventType);
		pTextRecType[i] = CreateStatic(CRect(rt.left+(i%2)*100+24, rt.top + (i>>1)*(TEXT_HEIGHT+4), rt.left+(i%2)*100+80, rt.bottom + (i>>1)*(TEXT_HEIGHT+4)), this, szChkText[i]);
		pTextRecType[i]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT));
		pTextRecType[i]->SetTextAlign(VD_TA_LEFT);
		pChkRecType[i]->SetValue(1);		
	}
	
	rt.bottom += (i>>1)*(TEXT_HEIGHT+4);

	//单画面、4画面按键
	playChnNum = 1;//默认单画面回放
	VD_BITMAP* pBmpButtonNormal = VD_LoadBitmap(DATA_DIR"/temp/tool_1x1.bmp");
	VD_BITMAP* pBmpButtonSelect = VD_LoadBitmap(DATA_DIR"/temp/tool_1x1_f.bmp");
	
	rt.left = right_page_l+10;
	rt.right = rt.left + pBmpButtonNormal->width;
	rt.top = rt.bottom +10;
	rt.bottom = rt.top + pBmpButtonNormal->height;

	pBtnSingle = CreateButton(rt, this, NULL, (CTRLPROC)&CPageSearchRec::OnClkBtn, NULL, buttonNormalBmp, 1);		
	pBtnSingle->SetBitmap(pBmpButtonNormal, pBmpButtonSelect, pBmpButtonSelect, pBmpButtonSelect);

	pBmpButtonNormal = VD_LoadBitmap(DATA_DIR"/temp/tool_2x2.bmp");
	pBmpButtonSelect = VD_LoadBitmap(DATA_DIR"/temp/tool_2x2_f.bmp");
	rt.left = rt.right;
	rt.right = rt.left + pBmpButtonNormal->width;	
	pBtnFour = CreateButton(rt, this, NULL, (CTRLPROC)&CPageSearchRec::OnClkBtn, NULL, buttonNormalBmp, 1);		
	pBtnFour->SetBitmap(pBmpButtonNormal, pBmpButtonSelect, pBmpButtonSelect, pBmpButtonSelect);

	//单画面、4画面边框
	rt.left = right_page_l+3;
	rt.right =	m_Rect.Width()-3;
	rt.top = rt.bottom;
	rt.bottom = rt.top+TEXT_HEIGHT*4;
	CTableBox* pTabFrame = CreateTableBox(rt, this, 1, 1);
	pTabFrame->SetFrameColor(VD_RGB(232,232,232));

	int btnPlay_top = rt.bottom;
	
	//单画面、4画面边框内的回放通道选择
	m_ChnMax = GetVideoMainNum();	
	rt.left = right_page_l+12;
	//rt.right = rt.left+24;
	rt.top += 12;
	char str_num[10];
	char str_chn[10];
	for(i=0; i<4; ++i)
	{
		sprintf(str_num, "%d", i+1);
		pTextChn[i] = CreateStatic(CRect(rt.left + (i%2*4)*TEXT_WIDTH, rt.top + (i>>1)*(TEXT_HEIGHT+20), rt.left+(i%2*4+1)*TEXT_WIDTH, rt.top + (i>>1)*(TEXT_HEIGHT+20)+TEXT_HEIGHT), this, str_num);
		pTextChn[i]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT));
		pTextChn[i]->SetTextAlign(VD_TA_LEFT);
		
		pComboBoxChn[i] = CreateComboBox(CRect(rt.left+(i%2*4+1)*TEXT_WIDTH, rt.top + (i>>1)*(TEXT_HEIGHT+20), rt.left+(i%2*4+3)*TEXT_WIDTH, rt.top + (i>>1)*(TEXT_HEIGHT+20)+TEXT_HEIGHT), this, NULL, NULL, NULL, 0);
		pComboBoxChn[i]->SetBkColor(VD_RGB(67,77,87));

		pComboBoxChn[i]->AddString("&CfgPtn.NONE");
		for (j=0; j<m_ChnMax; ++j)
		{
			sprintf(str_chn, "%d", j+1);
			pComboBoxChn[i]->AddString(str_chn);
		}
		pComboBoxChn[i]->SetCurSel(0);
	}
	
	//播放按键
	rt.left = right_page_l+10;
	rt.right = rt.left + 4*TEXT_WIDTH;
	rt.top = btnPlay_top +20;
	rt.bottom = rt.top + CTRL_HEIGHT;
	pBtnPlay= CreateButton(rt, this, "&CfgPtn.StartPlay", (CTRLPROC)&CPageSearchRec::OnClkBtn, NULL, buttonNormalBmp);	
	pBtnPlay->SetBitmap(VD_LoadBitmap(DATA_DIR"/temp/btn.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"));


	//进度条BOTTOM_PAGE_HEIGHT (10/*上边沿*/+24/*刻度值*/+12*4/*进度条*/+24/*下边沿*/)
	rt.left = 10;
	rt.right = m_Rect.Width() - 10;
	rt.top = right_page_b + 10;
	rt.bottom = rt.top + TEXT_HEIGHT;

	double interval = rt.Width()/(double)24;
	
	for (i=0; i<25; ++i)//24 hour
	{
		sprintf(str_num, "%d", i);
		
		if (i<10) //一位数
		{
			pTextHour[i] = CreateStatic(CRect(rt.left+i*interval-TEXT_HEIGHT/4, rt.top, rt.left+i*interval+TEXT_HEIGHT/4, rt.top+TEXT_HEIGHT), this, str_num);
		}
		else if (i<24)//两位数
		{
			pTextHour[i] = CreateStatic(CRect(rt.left+i*interval-TEXT_HEIGHT/2, rt.top, rt.left+i*interval+TEXT_HEIGHT/2, rt.top+TEXT_HEIGHT), this, str_num);
		}
		else //最后一个偏左一点
		{
			pTextHour[i] = CreateStatic(CRect(rt.left+i*interval-TEXT_HEIGHT/2-4, rt.top, rt.left+i*interval+TEXT_HEIGHT/2-4, rt.top+TEXT_HEIGHT), this, str_num);
		}
		
		pTextHour[i]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT));
		pTextHour[i]->SetTextAlign(VD_TA_XCENTER);
	}

	rt.left = 10;
	rt.right = m_Rect.Width() - 10;
	rt.top = right_page_b + 10 + TEXT_HEIGHT;
	//rt.bottom = rt.top + TEXT_HEIGHT;
	for (i=0; i<4; ++i)
	{
		m_pSlider[i] = new CSliderCtrlPartColor(CRect(rt.left, rt.top+i*12, rt.right, rt.top+(i+1)*12), this, 0, 24*60*60, 24, (CTRLPROC)&CPageSearchRec::OnSlider);
	}

}

CPageSearchRec::~CPageSearchRec()
{
	
}

void CPageSearchRec::OnClkBtn()
{
	int i = 0;
	CButton *pFocusButton = static_cast<CButton *>(GetFocusItem());
	
	if (pFocusButton == pBtnSingle)
	{
		playChnNum = 1;
		AdjustChn();
	}
	else if (pFocusButton == pBtnFour)
	{
		playChnNum = 4;
		AdjustChn();
	}
	else if (pFocusButton == pBtnPlay)
	{
		printf("CPageSearchRec::OnClkBtn btn Play\n");
	}
	else
	{
		printf("CPageSearchRec::OnClkBtn btn invalid\n");
	}
}
void CPageSearchRec::OnDaySelected()
{
	//获得当前选中日期
	SYSTEM_TIME stSelTime;
	pCalendar->GetDate(&stSelTime);
	
	//将选中的填充
	pCalendar->SetMask(1<<(stSelTime.day-1));
}

void CPageSearchRec::OnEventType()
{
	
}

void CPageSearchRec::OnSlider()
{
	
}

void CPageSearchRec::AdjustChn()
{
	int i;
	
	if (playChnNum == 1)
	{
		pBtnSingle->Enable(FALSE);
		//pBtnSingle->SetFocus(TRUE);
		pBtnFour->Enable(TRUE);

		for (i=1; i<4; ++i)
		{
			pTextChn[i]->Show(FALSE);		
			pComboBoxChn[i]->Show(FALSE);
			m_pSlider[i]->Show(FALSE);
		}
	}
	else if (playChnNum == 4)
	{
		pBtnSingle->Enable(TRUE);
		pBtnFour->Enable(FALSE);
		//pBtnFour->SetFocus(TRUE);

		for (i=1; i<4; ++i)
		{
			pTextChn[i]->Show(TRUE);		
			pComboBoxChn[i]->Show(TRUE);
			m_pSlider[i]->Show(TRUE);
		}
	}
	else
	{
		printf("CPageSearchRec::AdjustChn playChnNum: %d invalid\n", playChnNum);
	}
}

void CPageSearchRec::UpdateCalendar(SYSTEM_TIME* pTime)
{
	pCalendar->SetDate(pTime);
	pCalendar->SetMask(1<<(pTime->day-1));
}

VD_BOOL CPageSearchRec::UpdateData( UDM mode )
{
	time_t long_time = time(NULL);
	
	//csp modify 20131213
	int nTimeZone = GetTimeZone();
	long_time += GetTimeZoneOffset(nTimeZone);

	struct tm tm0;
	struct tm *tmptime = &tm0;
	localtime_r((time_t*)&long_time, tmptime);
	
	SYSTEM_TIME stTime1, stTime2;
	stTime1.year = tmptime->tm_year+1900;
	stTime1.month = tmptime->tm_mon+1;
	stTime1.day = tmptime->tm_mday;
	stTime1.hour = tmptime->tm_hour;
	stTime1.minute = tmptime->tm_min;
	stTime1.second = tmptime->tm_sec;
	
	if(UDM_OPEN == mode)
	{
		//printf("CPageSearchRec open\n");
		UpdateCalendar(&stTime1);
		AdjustChn();
	}
	else if (UDM_CLOSED == mode)
	{
		
	}
	return TRUE;
}

VD_BOOL CPageSearchRec::MsgProc( uint msg, uint wpa, uint lpa )
{
	return CPage::MsgProc(msg, wpa, lpa);
}



