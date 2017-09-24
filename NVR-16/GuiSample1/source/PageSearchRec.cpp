#include "GUI/Pages/PageSearchRec.h"

#include "biz.h"
#include "GUI/Pages/BizData.h"
#include "GUI/Pages/PageMessageBox.h"
#include "System/Locales.h"
#include "GUI/Pages/PageDesktop.h"//cw_panel
#include "GUI/Pages/PageMainFrameWork.h"

//full screen page
CPageSearchRec::CPageSearchRec(VD_PCRECT pRect,VD_PCSTR psz/* = NULL*/,VD_BITMAP* icon/* = NULL*/,CPage * pParent/* = NULL*/, uint vstyle/* = 0*/)
:CPageFloat(pRect, pParent,psz)
,m_ChnMax(0)
,todayOpenTime(0)
,todayBeginTime(0)
,m_startTime(0)
,m_endTime(0)
,m_MaskType(0xf)
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
	
	for(i = 0; i<5; ++i)
	{
		if (i == 4)
		{
			pChkRecType[i] = CreateCheckBox(CRect(rt.left + (i%2)*100, rt.top + (i>>1)*(TEXT_HEIGHT+4), rt.left+(i%2)*100+24, rt.bottom + (i>>1)*(TEXT_HEIGHT+4)), this, styleEditable, (CTRLPROC)&CPageSearchRec::OnEventAllType);
		}
		else
		{
			pChkRecType[i] = CreateCheckBox(CRect(rt.left + (i%2)*100, rt.top + (i>>1)*(TEXT_HEIGHT+4), rt.left+(i%2)*100+24, rt.bottom + (i>>1)*(TEXT_HEIGHT+4)), this, styleEditable, (CTRLPROC)&CPageSearchRec::OnEventType);
		}
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
		
		pComboBoxChn[i] = CreateComboBox(CRect(rt.left+(i%2*4+1)*TEXT_WIDTH, rt.top + (i>>1)*(TEXT_HEIGHT+20), rt.left+(i%2*4+3)*TEXT_WIDTH, rt.top + (i>>1)*(TEXT_HEIGHT+20)+TEXT_HEIGHT), this, NULL, NULL, (CTRLPROC)&CPageSearchRec::OnCombox, 0);
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
		m_pSlider[i] = new CSliderCtrlPartColor(CRect(rt.left, rt.top+i*12, rt.right, rt.top+(i+1)*12), 
			this, 0, 24*60*60-1, 24, NULL/*(CTRLPROC)&CPageSearchRec::OnSlider*/);
		
		m_pSlider[i]->SetTrackerEnable(FALSE);
	}

	memset(m_WindowChn, 0xff, sizeof(m_WindowChn));//0xff  无效通道
	//搜索
	memset(&sSearchResult0, 0, sizeof(sSearchResult0));
	sSearchResult0.psRecfileInfo = (SBizRecfileInfo*)calloc(sizeof(SBizRecfileInfo), 4000);
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
	static SYSTEM_TIME stPreTime;
	SYSTEM_TIME stSelTime;
	pCalendar->GetDate(&stSelTime);

	if (memcmp(&stPreTime, &stSelTime, sizeof(SYSTEM_TIME)) == 0)
	{
		return;
	}
	
	stPreTime = stSelTime;
	//将选中的填充
	pCalendar->SetMask(1<<(stSelTime.day-1));

	Search();
}

void CPageSearchRec::OnEventType()
{
	int mask_val = 0;
	int i;
	
	for(i=0; i<4; ++i)
	{
		if (pChkRecType[i]->GetValue())
		{
			mask_val |= i<<1;
		}
	}

	if (0xf == mask_val)
	{
		pChkRecType[i]->SetValue(1);
	}
	else
	{
		pChkRecType[i]->SetValue(0);
	}
	
	Search();
}

void CPageSearchRec::OnEventAllType()
{
	int value = pChkRecType[4]->GetValue();
	for(int i = 0; i<4; ++i)
	{
		pChkRecType[i]->SetValue(value);
	}

	Search();
}

void CPageSearchRec::OnCombox()
{
	Search();
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

void CPageSearchRec::Search()
{
	u32 startTime = 0;
	u32 endTime = 0;
	u8 MaskType = 0;
	u32 MaskChn = 0;
	int i = 0;

	//获得当前选中日期
	SYSTEM_TIME stSelTime;
	pCalendar->GetDate(&stSelTime);

	struct tm tM;
	memset(&tM, 0, sizeof(tM));
	
	tM.tm_year = stSelTime.year - 1900;
	tM.tm_mon = stSelTime.month - 1;
	tM.tm_mday = stSelTime.day;
	tM.tm_hour = 0;
	tM.tm_min = 0;
	tM.tm_sec = 0;

	int nTimeZone = GetTimeZone();
	startTime = mktime(&tM) - GetTimeZoneOffset(nTimeZone);

	if (startTime == todayBeginTime)//是今天
	{
		//printf("%s today\n", __func__);
		endTime = todayOpenTime;
	}
	else
	{
		tM.tm_hour = 23;
		tM.tm_min = 59;
		tM.tm_sec = 59;
		
		endTime = mktime(&tM) - GetTimeZoneOffset(nTimeZone);
	}
	
	//type
	if(pChkRecType[4]->GetValue())//all
	{
		MaskType = 0xf;
	}
	else
	{
		for (i = 0; i < 4; ++i)
		{
			if(pChkRecType[i]->GetValue())
			{
				MaskType |= 1<<i;
			}
		}
	}

	//chn
	int chn;	
	u8 windowChn[4];
	memset(windowChn, 0xff, sizeof(windowChn));//0xff  无效通道
	
	if (playChnNum == 1)
	{
		chn = pComboBoxChn[0]->GetCurSel();
		if (chn)
		{
			windowChn[0] = chn - 1;
			MaskChn |= 1<<windowChn[0];
		}
	}
	else
	{
		for (i=0; i<playChnNum; ++i)
		{
			chn = pComboBoxChn[i]->GetCurSel();
			if (chn)
			{
				windowChn[i] = chn - 1;
				MaskChn |= 1<<windowChn[i];
			}
		}
	}

	int b_search = 0;
	if (m_startTime != startTime)
	{
		m_startTime = startTime;
		m_endTime = endTime;
		b_search = 1;
	}

	if (m_MaskType != MaskType)
	{
		m_MaskType = MaskType;
		b_search = 1;
	}

	if (memcmp(m_WindowChn, windowChn, sizeof(windowChn)))
	{
		memcpy(m_WindowChn, windowChn, sizeof(windowChn));
		
		b_search = 1;
	}

	if (!b_search)
	{
		return ;
	}

	memset(&sSearchPara, 0, sizeof(sSearchPara));
	sSearchResult0.nFileNum = 0;
	memset(sSearchResult0.psRecfileInfo, 0, 4000*sizeof(SBizRecfileInfo));

	sSearchPara.nMaskType = m_MaskType;
	sSearchPara.nMaskChn = MaskChn;
	sSearchPara.nStartTime = m_startTime;
	sSearchPara.nEndTime = m_endTime;

	if(BizSysComplexDMSearch(EM_BIZFILE_REC, &sSearchPara, &sSearchResult0, 4000 ))
	{
		//failed
		return ;
	}

#if 0
	printf("search file result: %d\n", sSearchResult0.nFileNum);

	SBizRecfileInfo *psRecfileInfo = sSearchResult0.psRecfileInfo;
	for (i=0; i<sSearchResult0.nFileNum; ++i)
	{
		printf("%d, chn:%d, start: %u, end: %u\n", i, psRecfileInfo[i].nChn, psRecfileInfo[i].nStartTime, psRecfileInfo[i].nEndTime);
	}
#endif

	dealResult();
}

void CPageSearchRec::dealResult()
{
	int chn, i, j;
	std::vector<int> v_indexs[4];	//每通道 存储sSearchResult0.psRecfileInfo[] 下标
	std::vector<Range> v_ranges[4];	//每通道的录像区间

	//fill v_indexs
	SBizRecfileInfo *psRecfileInfo = sSearchResult0.psRecfileInfo;
	for (i=0; i<sSearchResult0.nFileNum; ++i)
	{
		chn = psRecfileInfo[i].nChn-1;
		
		for (j=0; j<playChnNum; ++j)
		{
			//比较窗口通道和搜索结果通道
			if ((0xff != m_WindowChn[j]) && (m_WindowChn[j] == chn))
			{
				v_indexs[j].push_back(i);
				//printf("window index:%d, chn: %d, file index: %d\n", j, chn, i);
			}
		}
	}

	//fill v_ranges
	Range r;	
	//因为搜索结果是按时间递减排列的，所以反向遍历
	std::vector<int>::const_reverse_iterator c_rev_iter;
	for (j=0; j<playChnNum; ++j)
	{
		r.start = r.end = 0;

		if (v_indexs[j].empty())
		{
			continue;
		}

		//接下来要把时间连续的文件组合到一个Range
		u32 file_start = 0;
		u32 file_end = 0;
		for (c_rev_iter = v_indexs[j].rbegin(); c_rev_iter != v_indexs[j].rend(); ++c_rev_iter)
		{
			file_start = psRecfileInfo[*c_rev_iter].nStartTime;
			file_end = psRecfileInfo[*c_rev_iter].nEndTime;

			if (file_end < file_start)
			{
				printf("Error: %s: file_end(%u) < file_start(%u)\n", __func__, file_end, file_start);
				continue;
			}

			if (r.end > file_start)
			{
				printf("Error: %s: range end(%u) > file_start(%u)\n", __func__, r.end, file_start);
				continue;
			}
				
			if (!r.start) //先确定start
			{
				r.start = file_start;
				r.end = file_end;

				//printf("start: %u\n", file_start);
				continue;
			}

			//然后组合时间连续的文件
			//间隔< 5秒，认为连续
			if (file_start - r.end < 5)
			{
				r.end = file_end;
			}
			else //不连续
			{
				//printf("end: %u\n", r.end);

				v_ranges[j].push_back(r);

				r.start = r.end = 0;

				--c_rev_iter;//为了在下一个循环重新处理这个文件
			}
		}
		//保存最后一个区间
		if (r.selfCheck())
		{
			//printf("end: %u\n", r.end);
			v_ranges[j].push_back(r);
		}
		
	}

	//对录像最早和最晚的时间进行修正
	//因为最早的文件可能跨越了零点，就是开始时间在前一天
	#if 0
		char str_start[32] = {0};
		char str_end[32] = {0};
		int cnt = 0;
	#endif
	std::vector<Range>::iterator iter;
	for (j=0; j<playChnNum; ++j)
	{
	#if 0
		cnt = 0;
	#endif
		if (!v_ranges[j].empty())
		{
			iter = v_ranges[j].begin();
			if (iter->start < m_startTime)
			{
				//printf("adjust start window:%d\n", j);
				iter->start = m_startTime;
			}

			iter = v_ranges[j].end()-1;
			if (iter->end > m_endTime)
			{
				//printf("adjust end window:%d\n", j);
				iter->end = m_endTime;
			}

			for (iter = v_ranges[j].begin(); iter != v_ranges[j].end(); ++iter)
			{
			#if 0
				GetTimeForBackup2(iter->start, str_start);
				GetTimeForBackup2(iter->end, str_end);
				printf("window: %d, chn: %d, cnt: %d range[%u, %u][%s, %s]\n",
					j, m_WindowChn[j], cnt, iter->start, iter->end, str_start, str_end);
				cnt++;
			#endif
				//记录相对于零点的偏移
				iter->start -= m_startTime;
				iter->end -= m_startTime;
			}
		
		}

		//空也设置，即不进行着色
		m_pSlider[j]->SetColorRange(v_ranges[j]);
	}
}

void CPageSearchRec::UpdateCalendar(SYSTEM_TIME* pTime)
{
	pCalendar->SetDate(pTime);
	pCalendar->SetMask(1<<(pTime->day-1));
}

VD_BOOL CPageSearchRec::UpdateData( UDM mode )
{
	int i;
	time_t long_time = time(NULL);
	
	//csp modify 20131213
	int nTimeZone = GetTimeZone();
	long_time += GetTimeZoneOffset(nTimeZone);

	struct tm tm0;
	struct tm *tmptime = &tm0;
	localtime_r((time_t*)&long_time, tmptime);//当前北京时间
	
	if(UDM_OPEN == mode)
	{
		SYSTEM_TIME stTime1;
		stTime1.year = tmptime->tm_year+1900;
		stTime1.month = tmptime->tm_mon+1;
		stTime1.day = tmptime->tm_mday;
		stTime1.hour = 0;
		stTime1.minute = 0;
		stTime1.second = 0;
		
		UpdateCalendar(&stTime1);
		AdjustChn();
	}
	else if (UDM_CLOSED == mode)
	{
		
	}
	else if (UDM_EMPTY == mode)
	{
		playChnNum = 1;
		//time range
		todayOpenTime = long_time-GetTimeZoneOffset(nTimeZone);
		
		tm0.tm_hour = 0;
		tm0.tm_min = 0;
		tm0.tm_sec = 0;
		tm0.tm_isdst = 0;
		tm0.tm_wday = 0;
		tm0.tm_yday = 0;
		todayBeginTime = mktime(&tm0)-GetTimeZoneOffset(nTimeZone);
		
		m_startTime = 0;
		m_endTime = 0;
		m_MaskType = 0xf;
		memset(m_WindowChn, 0xff, sizeof(m_WindowChn));//0xff  无效通道
		memset(&sSearchPara, 0, sizeof(sSearchPara));
		sSearchResult0.nFileNum = 0;
		memset(sSearchResult0.psRecfileInfo, 0, 4000*sizeof(SBizRecfileInfo));

		for(i = 0; i<5; ++i)
		{
			pChkRecType[i]->SetValue(1);		
		}

		std::vector<Range> vr;
		for(i=0; i<4; ++i)
		{
			pComboBoxChn[i]->SetCurSel(0);
			pComboBoxChn[i]->Show(FALSE);

			m_pSlider[i]->SetColorRange(vr);//empty
			m_pSlider[i]->Show(FALSE);
		}
		
		pComboBoxChn[0]->Show(TRUE);
		m_pSlider[0]->Show(TRUE);

		pBtnSingle->Enable(FALSE);
		pBtnFour->Enable(TRUE);
		
	}
	
	return TRUE;
}

VD_BOOL CPageSearchRec::MsgProc( uint msg, uint wpa, uint lpa )
{
	return CPage::MsgProc(msg, wpa, lpa);
}



