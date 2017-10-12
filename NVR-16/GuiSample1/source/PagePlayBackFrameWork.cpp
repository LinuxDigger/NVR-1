#include <time.h>

#include "GUI/Pages/PagePlayBackFrameWork.h"
#include "GUI/Pages/PageChnSelect.h"
#include "GUI/Pages/PageAudioSelect.h"
#include "GUI/Pages/PageColorSetup.h"
#include "GUI/Pages/PagePlayBackVoColorSetup.h"
#include "GUI/Pages/PagePlayrateSelect.h"
#include "GUI/Pages/PageDesktop.h"
#include "GUI/Pages/PageSearch.h"
#include "GUI/Pages/BizData.h"
#include "biz.h"

//#include "GUI/Pages/PageCfgRecParam.h"

/*
char* shortcut[MAX_MAIN_ITEM] = {
	"Record",
	"Ptz",	
	"Alarm",
	"Opration",
	"Param Config",
	"System Info",
};

char *shortcutBmpName[MAX_MAIN_ITEM][2] = {    
	{DATA_DIR"/config_record_1.bmp",	DATA_DIR"/config_record_2.bmp"},
	{DATA_DIR"/config_ptz_1.bmp",		DATA_DIR"/config_ptz_2.bmp"},	
	{DATA_DIR"/config_alarm_1.bmp",		DATA_DIR"/config_alarm_2.bmp"},
	{DATA_DIR"/config_opration_1.bmp",	DATA_DIR"/config_opration_2.bmp"},
	{DATA_DIR"/config_setting_1.bmp",	DATA_DIR"/config_setting_2.bmp"},
	{DATA_DIR"/config_sysinfo_1.bmp",	DATA_DIR"/config_sysinfo_2.bmp"},
};
*/

#define BOTTOM_PAGE_HEIGHT  (10/*上边沿*/+ 32/*按键+间隔*/ +24/*刻度值*/+12*4/*进度条*/+24/*下边沿*/)

//m_SliderRangeMode
enum{
	EM_SLIDER_MODE_24HR,
	EM_SLIDER_MODE_2HR,
	EM_SLIDER_MODE_1HR,
	EM_SLIDER_MODE_30MIN
};
//进度条均分多少份，依赖EM_SLIDER_MODE_XX
const int split_line_num[] = {24, 12, 12, 10};


enum{//与下面的数组一一对应
	PB_BUTTON_PAUSE,
	PB_BUTTON_STOP,
	PB_BUTTON_STEP,
	PB_BUTTON_FASTFORWARD,	
	PB_BUTTON_TRIANGLE1,
	PB_BUTTON_BACKWARD,
	PB_BUTTON_TRIANGLE2,
	PB_BUTTON_1x1,
	PB_BUTTON_TRIANGLE3,
	PB_BUTTON_2x2,
	PB_BUTTON_ZOOM,
	PB_BUTTON_SETCOLOR,
	PB_BUTTON_AUDIO,
	PB_BUTTON_HIDE,	
	PB_BUTTON_EXIT,
	PB_BUTTON_PRESECT,
	PB_BUTTON_NEXTSECT,
	PB_BUTTON_24HR,
	PB_BUTTON_2HR,
	PB_BUTTON_1HR,
	PB_BUTTON_30M,	
};

#define PB_BMP_NUM  17
char *pbBmpName[PB_BMP_NUM][3] = {    
	{DATA_DIR"/temp/pause.bmp",	DATA_DIR"/temp/pause_f.bmp",	DATA_DIR"/temp/pause_n.bmp"},
	{DATA_DIR"/temp/stop.bmp",	DATA_DIR"/temp/stop_f.bmp",	DATA_DIR"/temp/stop_n.bmp"},	
	{DATA_DIR"/temp/next_frame.bmp",	DATA_DIR"/temp/next_frame_f.bmp",	DATA_DIR"/temp/next_frame_n.bmp"},
	{DATA_DIR"/temp/fast_forward.bmp",	DATA_DIR"/temp/fast_forward_f.bmp",	DATA_DIR"/temp/fast_forward_n.bmp"},
	{DATA_DIR"/temp/tool_triangle.bmp",	DATA_DIR"/temp/tool_triangle_f.bmp",0},
	{DATA_DIR"/temp/reward.bmp",	DATA_DIR"/temp/reward_f.bmp",	DATA_DIR"/temp/reward_n.bmp"},
	{DATA_DIR"/temp/tool_triangle.bmp",	DATA_DIR"/temp/tool_triangle_f.bmp",0},
	{DATA_DIR"/temp/play_1x1.bmp",	DATA_DIR"/temp/play_1x1_f.bmp",	DATA_DIR"/temp/play_1x1_n.bmp"},
	{DATA_DIR"/temp/tool_triangle.bmp",	DATA_DIR"/temp/tool_triangle_f.bmp",0},
	{DATA_DIR"/temp/play_2x2.bmp",	DATA_DIR"/temp/play_2x2_f.bmp",	DATA_DIR"/temp/play_2x2_n.bmp"},
	{DATA_DIR"/temp/play_zoom.bmp",	DATA_DIR"/temp/play_zoom_f.bmp",0},
	{DATA_DIR"/temp/set_color.bmp",	DATA_DIR"/temp/set_color_f.bmp",0},
	{DATA_DIR"/temp/play_audio.bmp",	DATA_DIR"/temp/play_audio_f.bmp",0},
	{DATA_DIR"/temp/player_hide.bmp",	DATA_DIR"/temp/player_hide_f.bmp",0},
	{DATA_DIR"/temp/exit_pb.bmp",	DATA_DIR"/temp/exit_pb_f.bmp",0},
	{DATA_DIR"/temp/prev_section.bmp",	DATA_DIR"/temp/prev_section_f.bmp",0},
	{DATA_DIR"/temp/next_section.bmp",	DATA_DIR"/temp/next_section_f.bmp",0},
};

static VD_BITMAP* pBmpButtonNormal[PB_BMP_NUM];
static VD_BITMAP* pBmpButtonSelect[PB_BMP_NUM];
static VD_BITMAP* pBmpButtonDisable[PB_BMP_NUM];
static VD_BITMAP* pBmpPlay;
static VD_BITMAP* pBmpPlay_f;

static VD_BITMAP* pBmpAudioMute;
static VD_BITMAP* pBmpAudioMute_f;

static int GetMaxChnNum()
{
	return GetVideoMainNum();
}

CPagePlayBackFrameWork::CPagePlayBackFrameWork( VD_PCRECT pRect,VD_PCSTR psz /*= NULL*/,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/, uint vstyle /*= 0*/ )
:CPageFloat(pRect, pParent)//,m_Mutex(MUTEX_RECURSIVE)//csp modify 20121118
,m_Mutex(MUTEX_FAST)
{
	m_nIsFinished = 1;//csp modify 20121118
	
	m_bPlayBackPage = TRUE;
	//m_bPlayBackHide = TRUE;
	m_pDesktop = NULL;
	
	m_pbRect.left = pRect->left;
	m_pbRect.top = pRect->top;
	m_pbRect.right = pRect->right;
	m_pbRect.bottom = pRect->bottom;
	
	#if 1
	isStop = 0;
	isPause = 0;
	playRate = 0;
	playRate_back = 0;
	IsPbZoom = 0;
	exitstatue = 0;
	
	int MaxChn = GetMaxChnNum();
	
	//int PB_FRAME_TOP = 500;
	
	//SetRect(CRect(0, 0, 506, 400), FALSE);
	
	int rd = 10;
	int space = 20;
	CRect rtTmp;
	rtTmp.left = 10;//41;
	rtTmp.top = 7;
	
	SetMargin(0,0,0,0);
	
	pBmpPlay = VD_LoadBitmap(DATA_DIR"/temp/play.bmp");
	pBmpPlay_f = VD_LoadBitmap(DATA_DIR"/temp/play_f.bmp");
	
	pBmpAudioMute = VD_LoadBitmap(DATA_DIR"/temp/play_audio_m.bmp");
	pBmpAudioMute_f = VD_LoadBitmap(DATA_DIR"/temp/play_audio_m_f.bmp");
	
	int i;
	//播放、停止、帧进、快进
	for (i = 0; i<4; i++)
	{
		pBmpButtonNormal[i] = VD_LoadBitmap(pbBmpName[i][0]);
		pBmpButtonSelect[i] = VD_LoadBitmap(pbBmpName[i][1]);
		
		pButton[i] = CreateButton(CRect(rtTmp.left, rtTmp.top , 
						rtTmp.left+pBmpButtonNormal[i]->width, rtTmp.top+pBmpButtonNormal[i]->height), 
						this, NULL, (CTRLPROC)&CPagePlayBackFrameWork::OnClkPbCtl, NULL, buttonNormalBmp);

		if(pbBmpName[i][2])
		{
			pBmpButtonDisable[i] = VD_LoadBitmap(pbBmpName[i][2]);
			pButton[i]->SetBitmap(pBmpButtonNormal[i], pBmpButtonSelect[i], pBmpButtonSelect[i],pBmpButtonDisable[i] );
		}
		else
		{
			pButton[i]->SetBitmap(pBmpButtonNormal[i], pBmpButtonSelect[i], pBmpButtonSelect[i]);
		}
		
		rtTmp.left +=10+space;
	}

	//快进速度设置按键(三角)
	pBmpButtonNormal[i] = VD_LoadBitmap(pbBmpName[i][0]);
	pBmpButtonSelect[i] = VD_LoadBitmap(pbBmpName[i][1]);
    int FastforwarLeft = rtTmp.left;
	pButton[i] = CreateButton(CRect(rtTmp.left, rtTmp.top +4, 
					rtTmp.left+pBmpButtonNormal[i]->width, rtTmp.top+4+pBmpButtonNormal[i]->height), 
					this, NULL, (CTRLPROC)&CPagePlayBackFrameWork::OnClkPbCtl, NULL, buttonNormalBmp);
	
	if(pbBmpName[i][2])
	{
		pBmpButtonDisable[i] = VD_LoadBitmap(pbBmpName[i][2]);
		pButton[i]->SetBitmap(pBmpButtonNormal[i], pBmpButtonSelect[i], pBmpButtonSelect[i],pBmpButtonDisable[i]);
	}
	else
	{
		pButton[i]->SetBitmap(pBmpButtonNormal[i], pBmpButtonSelect[i], pBmpButtonSelect[i]);
	}
	
	rtTmp.left += space-3;
	i++;

	//快退
	pBmpButtonNormal[i] = VD_LoadBitmap(pbBmpName[i][0]);
	pBmpButtonSelect[i] = VD_LoadBitmap(pbBmpName[i][1]);
	pButton[i] = CreateButton(CRect(rtTmp.left, rtTmp.top , 
					rtTmp.left+pBmpButtonNormal[i]->width, rtTmp.top+pBmpButtonNormal[i]->height), 
					this, NULL, (CTRLPROC)&CPagePlayBackFrameWork::OnClkPbCtl, NULL, buttonNormalBmp);

	if(pbBmpName[i][2])
	{
		pBmpButtonDisable[i] = VD_LoadBitmap(pbBmpName[i][2]);
		pButton[i]->SetBitmap(pBmpButtonNormal[i], pBmpButtonSelect[i], pBmpButtonSelect[i],pBmpButtonDisable[i] );
	}
	else
	{
		pButton[i]->SetBitmap(pBmpButtonNormal[i], pBmpButtonSelect[i], pBmpButtonSelect[i]);
	}
	
	rtTmp.left += 10+space;
	i++;
	
	//快退速度设置按键(三角)
	pBmpButtonNormal[i] = VD_LoadBitmap(pbBmpName[i][0]);
	pBmpButtonSelect[i] = VD_LoadBitmap(pbBmpName[i][1]);
	pButton[i] = CreateButton(CRect(rtTmp.left, rtTmp.top +4, 
					rtTmp.left+pBmpButtonNormal[i]->width, rtTmp.top+4+pBmpButtonNormal[i]->height), 
					this, NULL, (CTRLPROC)&CPagePlayBackFrameWork::OnClkPbCtl, NULL, buttonNormalBmp);
    int BackwarLeft = rtTmp.left;
	
	if(pbBmpName[i][2])
	{
		pBmpButtonDisable[i] = VD_LoadBitmap(pbBmpName[i][2]);
		pButton[i]->SetBitmap(pBmpButtonNormal[i], pBmpButtonSelect[i], pBmpButtonSelect[i],pBmpButtonDisable[i] );
	}
	else
	{
		pButton[i]->SetBitmap(pBmpButtonNormal[i], pBmpButtonSelect[i], pBmpButtonSelect[i]);
	}

	//yaogang  显示当前播放速度
	rtTmp.left += 10+space;//space-3;
	//rtTmp.left = ((pRect->right - pRect->left) >> 1)-15;

	m_pStatic[1] = CreateStatic(CRect(rtTmp.left, rtTmp.top , 
								rtTmp.left+TEXT_WIDTH*3, rtTmp.top+22), this, ">>1X");
	m_pStatic[1]->SetTextAlign(VD_TA_LEFT);

	//回放画面数选择按键组(单画面、单画面选择三角、4画面)
	rtTmp.left += TEXT_WIDTH*3;
	i++;

	int Triangle2Right;
	for (; i<10; i++)
	{
		if(i==8)
		{
			pBmpButtonNormal[i] = VD_LoadBitmap(pbBmpName[i][0]);
			pBmpButtonSelect[i] = VD_LoadBitmap(pbBmpName[i][1]);
            Triangle2Right = rtTmp.left;
			pButton[i] = CreateButton(CRect(rtTmp.left, rtTmp.top+4 , 
							rtTmp.left+pBmpButtonNormal[i]->width, rtTmp.top+4+pBmpButtonNormal[i]->height), 
							this, NULL, (CTRLPROC)&CPagePlayBackFrameWork::OnClkPbCtl, NULL, buttonNormalBmp);

			
			if(pbBmpName[i][2])
			{
				pBmpButtonDisable[i] = VD_LoadBitmap(pbBmpName[i][2]);
				pButton[i]->SetBitmap(pBmpButtonNormal[i], pBmpButtonSelect[i], pBmpButtonSelect[i],pBmpButtonDisable[i] );
			}
			else
			{
				pButton[i]->SetBitmap(pBmpButtonNormal[i], pBmpButtonSelect[i], pBmpButtonSelect[i]);
			}
			
			rtTmp.left += 16;
		}
		else
		{
			pBmpButtonNormal[i] = VD_LoadBitmap(pbBmpName[i][0]);
			pBmpButtonSelect[i] = VD_LoadBitmap(pbBmpName[i][1]);
			pButton[i] = CreateButton(CRect(rtTmp.left, rtTmp.top , 
							rtTmp.left+pBmpButtonNormal[i]->width, rtTmp.top+pBmpButtonNormal[i]->height), 
							this, NULL, (CTRLPROC)&CPagePlayBackFrameWork::OnClkPbCtl, NULL, buttonNormalBmp);

			
			if(pbBmpName[i][2])
			{
				pBmpButtonDisable[i] = VD_LoadBitmap(pbBmpName[i][2]);
				pButton[i]->SetBitmap(pBmpButtonNormal[i], pBmpButtonSelect[i], pBmpButtonSelect[i],pBmpButtonDisable[i] );
			}
			else
			{
				pButton[i]->SetBitmap(pBmpButtonNormal[i], pBmpButtonSelect[i], pBmpButtonSelect[i]);
			}
			
			rtTmp.left +=10+space;
		}
	}

	//rtTmp.left += 10+space;
	//zoom and color setup not support	
	for (; i<12; i++)
	{
		pBmpButtonNormal[i] = VD_LoadBitmap(pbBmpName[i][0]);
		pBmpButtonSelect[i] = VD_LoadBitmap(pbBmpName[i][1]);
		pButton[i] = CreateButton(CRect(rtTmp.left, rtTmp.top, 
						rtTmp.left+pBmpButtonNormal[i]->width, rtTmp.top+pBmpButtonNormal[i]->height), 
						this, NULL, (CTRLPROC)&CPagePlayBackFrameWork::OnClkPbCtl, NULL, buttonNormalBmp);

		
		if(pbBmpName[i][2])
		{
			pBmpButtonDisable[i] = VD_LoadBitmap(pbBmpName[i][2]);
			pButton[i]->SetBitmap(pBmpButtonNormal[i], pBmpButtonSelect[i], pBmpButtonSelect[i],pBmpButtonDisable[i] );
		}
		else
		{
			pButton[i]->SetBitmap(pBmpButtonNormal[i], pBmpButtonSelect[i], pBmpButtonSelect[i]);
		}
		
		rtTmp.left +=10+space;
	}
	
	//播放时间
	m_pStatic[3] = CreateStatic(CRect(rtTmp.left, rtTmp.top , 
								rtTmp.left+TEXT_WIDTH*3, rtTmp.top+22), this, ""/*"15:29:59"*/);
	m_pStatic[3]->SetTextAlign(VD_TA_RIGHT);
	
	//声音、最小化、关闭
	rtTmp.left = pRect->right - pRect->left - 10/*41*/ - 3 * 34; //zlbfix20110828 //rtTmp.left +=110;
	int slider_range_btn_left = rtTmp.left-10-4*TEXT_WIDTH*3/2;
	for (; i<15; i++)
	{
		pBmpButtonNormal[i] = VD_LoadBitmap(pbBmpName[i][0]);
		pBmpButtonSelect[i] = VD_LoadBitmap(pbBmpName[i][1]);
		pButton[i] = CreateButton(CRect(rtTmp.left, rtTmp.top, 
						rtTmp.left+pBmpButtonNormal[i]->width, rtTmp.top+pBmpButtonNormal[i]->height), 
						this, NULL, (CTRLPROC)&CPagePlayBackFrameWork::OnClkPbCtl, NULL, buttonNormalBmp);

		
		if(pbBmpName[i][2])
		{
			pBmpButtonDisable[i] = VD_LoadBitmap(pbBmpName[i][2]);
			pButton[i]->SetBitmap(pBmpButtonNormal[i], pBmpButtonSelect[i], pBmpButtonSelect[i],pBmpButtonDisable[i] );
		}
		else
		{
			pButton[i]->SetBitmap(pBmpButtonNormal[i], pBmpButtonSelect[i], pBmpButtonSelect[i]);
		}
		
		rtTmp.left +=10+24;
	}

#if 0
	//前一段视频按键i = 15
	rtTmp.left = 41;
	rtTmp.top += 32;
	pBmpButtonNormal[i] = VD_LoadBitmap(pbBmpName[i][0]);
	pBmpButtonSelect[i] = VD_LoadBitmap(pbBmpName[i][1]);
	pButton[i] = CreateButton(CRect(rtTmp.left, rtTmp.top, 
					rtTmp.left+pBmpButtonNormal[i]->width, rtTmp.top+pBmpButtonNormal[i]->height), 
					this, NULL, (CTRLPROC)&CPagePlayBackFrameWork::OnClkPbCtl, NULL, buttonNormalBmp);

	if(pbBmpName[i][2])
	{
		pBmpButtonDisable[i] = VD_LoadBitmap(pbBmpName[i][2]);
		pButton[i]->SetBitmap(pBmpButtonNormal[i], pBmpButtonSelect[i], pBmpButtonSelect[i],pBmpButtonDisable[i] );
	}
	else
	{
		pButton[i]->SetBitmap(pBmpButtonNormal[i], pBmpButtonSelect[i], pBmpButtonSelect[i]);
	}
	pButton[i]->Show(FALSE);
#else
	pButton[15] = NULL;
#endif
#if 0
	//printf("%s 1\n", __func__);
	rtTmp.left += 10+24;
	i++;
	rtTmp.right = rtTmp.left+ pRect->right - pRect->left - 82 - 2 * 34;
	m_pSlider = CreateSliderCtrl(CRect(rtTmp.left, rtTmp.top+5, 
					rtTmp.right, rtTmp.top+17), this, 0, 100, (CTRLPROC)&CPagePlayBackFrameWork::OnSlider, sliderNoNum);
	//m_pSlider = new CSliderCtrlPartColor(CRect(rtTmp.left, rtTmp.top+5, 
	//				rtTmp.right, rtTmp.top+17), this, 0, 24*60*60, 24, (CTRLPROC)&CPagePlayBackFrameWork::OnSlider);
#else
	//进度条
	rtTmp.left = 10;
	rtTmp.right = pRect->right - 10;
	//rtTmp.top = right_page_b + 10;
	rtTmp.top += 32;
	rtTmp.bottom = rtTmp.top + TEXT_HEIGHT;

	double interval = rtTmp.Width()/(double)24;
	char str_num[10];
	
	for (i=0; i<25; ++i)//24 hour
	{
		sprintf(str_num, "%02d", i);
		
		if (i==0) //一位数
		{
			pTextSlider[i] = CreateStatic(CRect(rtTmp.left-7, rtTmp.top, rtTmp.left-7+TEXT_HEIGHT, rtTmp.top+TEXT_HEIGHT), this, str_num);
		}
		else if (i<24)//两位数
		{
			pTextSlider[i] = CreateStatic(CRect(rtTmp.left+i*interval-TEXT_HEIGHT/2, rtTmp.top, rtTmp.left+i*interval+TEXT_HEIGHT/2, rtTmp.top+TEXT_HEIGHT), this, str_num);
		}
		else //最后一个偏左一点
		{
			pTextSlider[i] = CreateStatic(CRect(rtTmp.left+i*interval-TEXT_HEIGHT/2-4, rtTmp.top, rtTmp.left+i*interval+TEXT_HEIGHT/2-4, rtTmp.top+TEXT_HEIGHT), this, str_num);
		}
		
		pTextSlider[i]->SetTextColor(VD_GetSysColor(COLOR_CTRLTEXT));
		pTextSlider[i]->SetTextAlign(VD_TA_XCENTER);
	}

	rtTmp.left = 10;
	rtTmp.right = pRect->right - 10;
	rtTmp.top = rtTmp.bottom;
	
	//rt.bottom = rt.top + TEXT_HEIGHT;
	for (i=0; i<4; ++i)
	{
		m_pSlider[i] = new CSliderCtrlPartColor(CRect(rtTmp.left, rtTmp.top+i*12, rtTmp.right, rtTmp.top+(i+1)*12), 
			this, 0, 24*60*60-1, 24, (CTRLPROC)&CPagePlayBackFrameWork::OnSlider);
		
		//m_pSlider[i]->SetTrackerEnable(FALSE);
	}

	m_SliderRangeMode = EM_SLIDER_MODE_24HR;

	
#endif	
	pButton[16] = NULL;
#if 0
	//后一段视频按键i = 16
	rtTmp.left = rtTmp.right + 7;
	pBmpButtonNormal[i] = VD_LoadBitmap(pbBmpName[i][0]);
	pBmpButtonSelect[i] = VD_LoadBitmap(pbBmpName[i][1]);
	pButton[i] = CreateButton(CRect(rtTmp.left, rtTmp.top, 
					rtTmp.left+pBmpButtonNormal[i]->width, rtTmp.top+pBmpButtonNormal[i]->height), 
					this, NULL, (CTRLPROC)&CPagePlayBackFrameWork::OnClkPbCtl, NULL, buttonNormalBmp);

	
	if(pbBmpName[i][2])
	{
		pBmpButtonDisable[i] = VD_LoadBitmap(pbBmpName[i][2]);
		pButton[i]->SetBitmap(pBmpButtonNormal[i], pBmpButtonSelect[i], pBmpButtonSelect[i],pBmpButtonDisable[i] );
	}
	else
	{
		pButton[i]->SetBitmap(pBmpButtonNormal[i], pBmpButtonSelect[i], pBmpButtonSelect[i]);
	}
	pButton[i]->Show(FALSE);
#else

#endif	
	
	//Slider Range Btn
	char *pslider_btn_text[] = {"24H", "2H", "1H", "30M"};
	
	rtTmp.top = 7;
	rtTmp.left = slider_range_btn_left;
	for (i=0; i<4; ++i)
	{
		pButton[PB_BUTTON_24HR+i] = CreateButton(CRect(rtTmp.left+i*TEXT_WIDTH*3/2, rtTmp.top, 
			rtTmp.left+(i+1)*TEXT_WIDTH*3/2, rtTmp.top+CTRL_HEIGHT),
			this, pslider_btn_text[i], (CTRLPROC)&CPagePlayBackFrameWork::OnClkBtnSlider, NULL, buttonNormalBmp);	
		pButton[PB_BUTTON_24HR+i]->SetBitmap(VD_LoadBitmap(DATA_DIR"/temp/btn.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"), VD_LoadBitmap(DATA_DIR"/temp/btn_f.bmp"));
	}
#if 0	
	rtTmp.left = 41;
	rtTmp.top += 32+32;

	//本地时间
	i = 0;
	m_pStatic[i] = CreateStatic(CRect(rtTmp.left, rtTmp.top , 
								rtTmp.left+170, rtTmp.top+22), this, "04/28/2011 15:00:00");
	m_pStatic[i]->SetTextAlign(VD_TA_LEFT);
	m_pStatic[i]->Show(FALSE);
	i++;

	
	//播放速率
	//rtTmp.left += ((pRect->right - pRect->left) >> 1);
	rtTmp.left = ((pRect->right - pRect->left) >> 1)-15;

	m_pStatic[i] = CreateStatic(CRect(rtTmp.left, rtTmp.top , 
								rtTmp.left+100, rtTmp.top+22), this, ">>1X");
	m_pStatic[i]->SetTextAlign(VD_TA_LEFT);
	
	i++;

	rtTmp.left += 100;
	
	/*
	m_pChnName = CreateCheckBox(CRect(rtTmp.left, 
		rtTmp.top+2,
		rtTmp.left+20, 
		rtTmp.top+22), this);
		*/

	rtTmp.left += 5;

	/*
	m_pStatic[i] = CreateStatic(CRect(rtTmp.left, rtTmp.top , 
								rtTmp.left+105, rtTmp.top+28), this, "Chn Name");
	m_pStatic[i]->SetTextAlign(VD_TA_CENTER);
	*/
	
	i++;
	
	rtTmp.right = pRect->right - pRect->left - 41;
	rtTmp.left = rtTmp.right - 190;
	
	m_pStatic[i] = CreateStatic(CRect(rtTmp.left, rtTmp.top , 
								rtTmp.right, rtTmp.top+22), this, ""/*"15:29:59"*/);
	m_pStatic[i]->SetTextAlign(VD_TA_RIGHT);
	//m_pStatic[i]->SetBkColor(VD_RGB(67,77,87));
	#endif

	CRect rtFloat;
	rtFloat.left = FastforwarLeft + m_pbRect.left;
	rtFloat.right = rtFloat.left + 75;
	rtFloat.top = m_Rect.bottom - BOTTOM_PAGE_HEIGHT/*100*/ - 173;
	rtFloat.bottom = rtFloat.top + 173;
	m_pPagePlayrate0 = new CPagePlayrateSelect(rtFloat,NULL,NULL,this,0,1);
	
	rtFloat.left = BackwarLeft + m_pbRect.left;
	rtFloat.right = rtFloat.left + 75;
	rtFloat.top = m_Rect.bottom - BOTTOM_PAGE_HEIGHT/*100*/ - 173;
	rtFloat.bottom = rtFloat.top + 173;
	m_pPagePlayrate1 = new CPagePlayrateSelect(rtFloat,NULL,NULL,this,0,0);
	
	rtFloat.left = Triangle2Right+ m_pbRect.left;
	rtFloat.right = rtFloat.left + 190;

	char tmpchar[20] = {0};
	GetProductNumber(tmpchar);
	if((0 == strcasecmp(tmpchar, "R9624T"))
		|| (0 == strcasecmp(tmpchar, "R9632S"))
		 || (0 == strcasecmp(tmpchar, "R9624SL"))
		  || (0 == strcasecmp(tmpchar, "R9616S"))
		  || (0 == strcasecmp(tmpchar, "R9608S")))
	{
		rtFloat.top = m_Rect.bottom - 100 - 60;
		rtFloat.bottom = rtFloat.top + 60;
		m_pPageChnSel = new CPageChnSelect(rtFloat,NULL,NULL,this,0,4);
	}
	else
	{
		rtFloat.top = m_Rect.bottom - 100 - 60;//cw_9508S 30*(MaxChn/4+1)
		rtFloat.bottom = rtFloat.top + 60;
		m_pPageChnSel = new CPageChnSelect(rtFloat,NULL,NULL,this,0,4);//cw_9508S add",0,4"
	}
	//printf("parent of start : %x\n", m_pParent);
	rtFloat.left = 500;
	rtFloat.right = rtFloat.left + 280;
	rtFloat.top = m_Rect.bottom - 100 - 84;
	rtFloat.bottom = rtFloat.top + 84;
	m_pPageAudioSel = new CPageAudioSelect(rtFloat, NULL,NULL,this);
	
	GetVgaResolution(&nScreenWidth, &nScreenHeight);
	
	int middle_y = nScreenHeight / 2;
	int middle_x = nScreenWidth * 7 / 10;
	
	rtFloat.left = middle_x;
	rtFloat.right = middle_x + 170;
	rtFloat.top = middle_y - 146;
	rtFloat.bottom = middle_y + 146;
	m_pPagePlayBackVoColorSetup = new CPagePlayBackVoColorSetup(rtFloat, NULL,NULL,this);
	#endif
	
	m_pPageSearch = NULL;
	
	BizPlayBackRegistFunCB(0, StopElecZoom);
}

CPagePlayBackFrameWork::~CPagePlayBackFrameWork()
{
	
}

VD_PCSTR CPagePlayBackFrameWork::GetDefualtTitleCenter()
{
	//printf("GetDefualtTitleCenter %d\n", curMainItemSel);
	return "Playback";
}

void CPagePlayBackFrameWork::InitSlider()
{
	int chn, i, j;
	std::vector<Range> v_ranges[4];	//每通道的录像区间
	u32 file_start = 0;
	u32 file_end = 0;

	//fill v_ranges
	Range r(0, 24*60*60-1);
	Range range_date(m_CurPlayDate, m_CurPlayDate+24*60*60-1);
	
	for (i=0; i<4; ++i)
	{
		//m_pSlider[i]->SetDisplayRange(r, 24);
		//m_pSlider[i]->SetPos(0);

		pButton[PB_BUTTON_24HR+i]->Enable(TRUE);
	}
	pButton[PB_BUTTON_24HR]->Enable(FALSE);
	
	#if 0
	int cnt = 0;
	char str_start[32] = {0};
	char str_end[32] = {0};
	GetTimeForBackup2(range_date.start, str_start);
	GetTimeForBackup2(range_date.end, str_end);
	printf("range date[%u, %u][%s, %s]\n", range_date.start, range_date.end, str_start, str_end);
	#endif
	
	SBizRecfileInfo *psRecfileInfo = NULL;
	s32 nRealFileNums = 0;

	for (j=0; j<nPlayChnNum; ++j)
	{
		r.start = r.end = 0;
		psRecfileInfo = NULL;
		nRealFileNums = 0;
		
		if (0 == BizPlaybackGetChnFileInfo(j, &psRecfileInfo, &nRealFileNums))
		{
			printf("playno: %d, nRealFileNums: %d\n", j, nRealFileNums);
			if (nRealFileNums && psRecfileInfo)
			{
				for (i = nRealFileNums-1; i>-1; --i)
				{
					file_start = psRecfileInfo[i].nStartTime;
					file_end = psRecfileInfo[i].nEndTime;

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
						if (r.end < range_date.start || r.start > range_date.end)
						{
							printf("error: %s 1:r[%d, %d] not in range date[%d, %d]\n",
								__func__, r.start, r.end, range_date.start, range_date.end);
						}
						else
						{
							v_ranges[j].push_back(r);

							r.start = r.end = 0;

							++i;//为了在下一个循环重新处理这个文件
						}
					}
				}
				//保存最后一个区间
				if (r.selfCheck())
				{
					if (r.end < range_date.start || r.start > range_date.end)
					{
						printf("error: %s 2:r[%d, %d] not in range date[%d, %d]\n",
							__func__, r.start, r.end, range_date.start, range_date.end);
					}
					else
					{
						v_ranges[j].push_back(r);
					}
				}
			}
		}	
	}
	
	//对录像最早和最晚的时间进行修正
	//因为最早的文件可能跨越了零点，就是开始时间在前一天	
	std::vector<Range>::iterator iter;
	for (j=0; j<nPlayChnNum; ++j)
	{
	#if 0
		cnt = 0;
	#endif
		if (!v_ranges[j].empty())
		{
			iter = v_ranges[j].begin();
			if (iter->start < range_date.start)
			{
				//printf("adjust start window:%d\n", j);
				iter->start = range_date.start;
			}

			iter = v_ranges[j].end()-1;
			if (iter->end > range_date.end)
			{
				//printf("adjust end window:%d\n", j);
				iter->end = range_date.end;
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
				iter->start -= range_date.start;
				iter->end -= range_date.start;
			}
		
		}

		//空也设置，即不进行着色
		m_pSlider[j]->SetColorRange(v_ranges[j]);
	}
	
	adjustSlider(0, EM_SLIDER_MODE_24HR);
	
}

void CPagePlayBackFrameWork::OnSlider()
{
#if 0
	int pos = m_pSlider->GetPos();
	int curTime = m_total*pos/100 + m_startTime;
	//printf("m_total = %d, m_startTime = %d, curTime = %d\n",m_total, m_startTime, curTime);
	
	BizPlaybackControl(EM_BIZCTL_SEEK,curTime);
	
	if(isPause)
	{
		BizPlaybackControl(EM_BIZCTL_PAUSE, 0);
	}
#else
	int i = 0; 
	int cur_pos = ~0;

	CSliderCtrlPartColor *pslider = (CSliderCtrlPartColor *)GetFocusItem();

	//CGuard guard(m_Mutex);//死锁
	
	for (i=0; i<4; ++i)
	{
		if (pslider == m_pSlider[i])
		{
			cur_pos = pslider->GetPos();
			printf("slider%d pos: %d\n", i, cur_pos);

			BizPlaybackControl(EM_BIZCTL_SEEK, cur_pos + m_CurPlayDate);
	
			if(isPause)
			{
				BizPlaybackControl(EM_BIZCTL_PAUSE, 0);
			}

			break;
		}
	}

	//同步移动
	for (i=0; i<4; ++i)
	{
		if (pslider != m_pSlider[i] && cur_pos != ~0)
		{
			m_pSlider[i]->SetPos(cur_pos);
		}
	}
#endif
}

void CPagePlayBackFrameWork::SetPbTotalTime(int totalTime)
{
	m_total = totalTime;

}

void CPagePlayBackFrameWork::SetPbStartTime(int startTime)
{
	m_startTime = startTime;
}

void CPagePlayBackFrameWork::SetPbInfo(SBizSearchPara sp)
{
	//m_hPbMgr = handle;
	sBizSearchParam = sp;
}

void CPagePlayBackFrameWork::SetPbProg(int nProg)
{
#if 0
	m_pSlider->SetPos(nProg);
#else
	int i;

	for (i=0; i<4; ++i)
	{
		m_pSlider[i]->SetPos(nProg);	
	}
#endif
}

void CPagePlayBackFrameWork::SetPbCurTime(char* curTime)
{
	//m_pStatic[0]->SetText(curTime);
	m_AllowOperateMenue = 1;
}

void CPagePlayBackFrameWork::SetPbCurTime(u32 curTime)
{
	m_AllowOperateMenue = 1;
	
	//CGuard guard(m_Mutex);
	char str_time[32];	
	Range r;
	Range range_date(m_CurPlayDate, m_CurPlayDate+24*60*60-1);
	
	if (!isInRange(curTime, range_date))
	{
		printf("warning: %s curTime: %d not in range date[%d, %d]\n",
			__func__, curTime, range_date.start, range_date.end);

		return;
	}

	GetTimeForBackup2(curTime, str_time);
	m_pStatic[3]->SetText(str_time);

	//根据模式调整下一区域
	adjustSlider(curTime - m_CurPlayDate, m_SliderRangeMode);
	
}

//依赖EM_SLIDER_MODE_XX 得到进度条的宽度
s32 CPagePlayBackFrameWork::getSliderWidth(int mode)
{
	int width = 0;
	
	switch (mode)
	{
		case EM_SLIDER_MODE_24HR:
		{
			width = 24*60*60;
		} break;
		
		case EM_SLIDER_MODE_2HR:
		{
			width = 2*60*60;
		} break;
		
		case EM_SLIDER_MODE_1HR:
		{
			width = 1*60*60;
		} break;
		
		case EM_SLIDER_MODE_30MIN:
		{
			width = 30*60;
		} break;
		
		default:
			printf("warning: %s mode: %d not support\n", __func__, mode);
	}

	return width;
}

//依赖EM_SLIDER_MODE_XX 得到进度条刻度一格的跨度
s32 CPagePlayBackFrameWork::getSliderSpan(int mode)
{
	int span = 0;
	
	switch (mode)
	{
		case EM_SLIDER_MODE_24HR:
		{
			span = 24*60*60/split_line_num[mode];
		} break;
		
		case EM_SLIDER_MODE_2HR:
		{
			span = 2*60*60/split_line_num[mode];
		} break;
		
		case EM_SLIDER_MODE_1HR:
		{
			span = 60*60/split_line_num[mode];
		} break;
		
		case EM_SLIDER_MODE_30MIN:
		{
			span = 30*60/split_line_num[mode];
		} break;
		
		default:
			printf("warning: %s mode: %d not support\n", __func__, mode);
	}

	return span;
}
//函数外必须保证cur_pos 在[0, 24*60*60] 中
void CPagePlayBackFrameWork::adjustSlider(u32 cur_pos, int SliderRangeMode)
{
	//return;
	
	int i;
	int b_adjust = 0;
	Range r;
	s32 width;
	m_pSlider[0]->GetDisplayRange(r);

	//调整DisplayRange 的条件
	if (!isInRange(cur_pos, r))
	{
		printf("%s cur_posr: %d not in range[%d, %d]\n", 
			__func__, cur_pos, r.start, r.end);
		
		b_adjust = 1;
	}

	if (m_SliderRangeMode != SliderRangeMode)
	{
		printf("%s m_SliderRangeMode: %d, new SliderRangeMode: %d\n", 
			__func__, m_SliderRangeMode, SliderRangeMode);
		
		m_SliderRangeMode = SliderRangeMode;
		b_adjust = 1;
	}

	if (b_adjust)
	{
		width = getSliderWidth(m_SliderRangeMode);
		if (0 == width)
		{
			return ;
		}

		r.start = cur_pos/width*width;
		r.end = (cur_pos/width+1)*width-1;
		printf("%s: adjust display range[%d, %d]\n", __func__, r.start, r.end);

		adjustSliderText(r, split_line_num[m_SliderRangeMode]);
		
		for (i=0; i<4; ++i)
		{
			m_pSlider[i]->SetDisplayRange(r, split_line_num[m_SliderRangeMode]);			
		}
	}

	for (i=0; i<4; ++i)
	{
		m_pSlider[i]->SetPos(cur_pos);			
	}
}

void CPagePlayBackFrameWork::adjustSliderText(const Range &r, const int split_line_num)
{
	if (split_line_num == 0)
	{
		printf("%s split_line_num == 0\n", __func__, split_line_num);
		return ;
	}

	int i;
	u32 start_time = r.start + m_CurPlayDate;
	u32 time_interval = (r.end+1-r.start)/split_line_num;
	int text_width;
	CRect rtTmp;

	rtTmp.left = m_Rect.left + 10;
	rtTmp.right = m_Rect.right - 10;
	rtTmp.top = m_Rect.top + 7+32;
	rtTmp.bottom = rtTmp.top + TEXT_HEIGHT;

	double interval = rtTmp.Width()/(double)split_line_num;
	char str_num[10];

	for (i=0; i<25; ++i)//24 hour
	{
		pTextSlider[i]->Show(FALSE);
	}
#if 0 //test
	sprintf(str_num, "%02d", i);
	VD_RECT rect;
	pTextSlider[0]->GetRect(&rect);
	printf("l: %d, r: %d, t: %d, b: %d\n", rect.left, rect.right, rect.top, rect.bottom);
	rect.left += 100;
	rect.right += 100;
	pTextSlider[0]->SetRect(&rect);
	pTextSlider[0]->SetText(str_num);
	pTextSlider[0]->Show(TRUE);

	//GetTimeForBackup3(m_CurPlayDate + r.start + time_interval*1, str_num);
	//printf("str_num: %s\n", str_num);

	//pTextSlider[1]->SetRect(CRect(10, 10, 10+text_width, 10+TEXT_HEIGHT));
	//pTextSlider[1]->SetText(str_num);
	//pTextSlider[1]->Show(TRUE);
	
#else
	for (i=0; i<split_line_num+1; ++i)
	{
		if (EM_SLIDER_MODE_24HR == m_SliderRangeMode) //只显示小时 00 01 02..... 24
		{
			text_width = TEXT_WIDTH;
			sprintf(str_num, "%02d", i);
		}
		else	//显示小时: 分钟eg:  xx:xx  
		{
			text_width = 2*TEXT_WIDTH + TEXT_WIDTH/2;// 2.5*TEXT_WIDTH
			GetTimeForBackup3(m_CurPlayDate + r.start + time_interval*i, str_num);
		}

		if (0 == i) //第一个偏右一点
		{
			pTextSlider[i]->SetRect(CRect(rtTmp.left-7, rtTmp.top, rtTmp.left-7+text_width, rtTmp.top+TEXT_HEIGHT));
		}
		else if(split_line_num == i)//最后一个偏左一点
		{
			pTextSlider[i]->SetRect(CRect(rtTmp.right+7-text_width, rtTmp.top, rtTmp.right+7, rtTmp.top+TEXT_HEIGHT));
		}
		else //中间的两位数
		{
			pTextSlider[i]->SetRect(CRect(rtTmp.left+i*interval-text_width/2, rtTmp.top, rtTmp.left+i*interval+text_width/2, rtTmp.top+TEXT_HEIGHT));
		}
		
		pTextSlider[i]->SetText(str_num);
		pTextSlider[i]->Show(TRUE);
	}
	
#endif

}

void CPagePlayBackFrameWork::SetPbTotalTime(char* totalTime)
{
	m_pStatic[3]->SetText(totalTime);
}

void CPagePlayBackFrameWork::SetPreviewMode(EMBIZPREVIEWMODE previewMode)
{
	this->previewMode = previewMode;
}

void CPagePlayBackFrameWork::StopPb()
{
	BizPlaybackControl(EM_BIZCTL_STOP, 0);
}

void CPagePlayBackFrameWork::SetMute(BOOL bMute)
{
	if(bMute)
	{
		pButton[PB_BUTTON_AUDIO]->SetBitmap(pBmpAudioMute, pBmpAudioMute_f, pBmpAudioMute_f);
	}
	else
	{
		pButton[PB_BUTTON_AUDIO]->SetBitmap(pBmpButtonNormal[PB_BUTTON_AUDIO], pBmpButtonSelect[PB_BUTTON_AUDIO], pBmpButtonSelect[PB_BUTTON_AUDIO]);
	}
}

void CPagePlayBackFrameWork::SetPlayrate(int rate, int type)
{
	BizSetPlayType(type);
	
	if(1 == type)
	{
		switch(rate)
		{
			case -3:
				m_pStatic[1]->SetText(">>1/8X");
				BizPlaybackControl(EM_BIZCTL_SET_SPEED, -8);
				break;
			case -2:
				m_pStatic[1]->SetText(">>1/4X");
				BizPlaybackControl(EM_BIZCTL_SET_SPEED, -4);
				break;
			case -1:
				m_pStatic[1]->SetText(">>1/2X");
				BizPlaybackControl(EM_BIZCTL_SET_SPEED, -2);
				break;
			case 0:
				m_pStatic[1]->SetText(">>1X");
				BizPlaybackControl(EM_BIZCTL_SET_SPEED, 1);
				break;
			case 1:
				m_pStatic[1]->SetText(">>2X");
				BizPlaybackControl(EM_BIZCTL_SET_SPEED, 2);
				break;
			case 2:
				m_pStatic[1]->SetText(">>4X");
				BizPlaybackControl(EM_BIZCTL_SET_SPEED, 4);
				break;
			case 3:
				m_pStatic[1]->SetText(">>8X");
				BizPlaybackControl(EM_BIZCTL_SET_SPEED, 8);
				break;
			//csp modify 20130429
			case 4:
				m_pStatic[1]->SetText(">>16X");
				BizPlaybackControl(EM_BIZCTL_SET_SPEED, 16);
				break;
			case 5:
				m_pStatic[1]->SetText(">>32X");
				BizPlaybackControl(EM_BIZCTL_SET_SPEED, 32);
				break;
			case 6:
				m_pStatic[1]->SetText(">>64X");
				BizPlaybackControl(EM_BIZCTL_SET_SPEED, 64);
				break;
			default:
				break;
		}
		
		playRate = rate;
		
		if(!m_nForward)
		{
			m_nForward = 1;
			BizPlaybackControl(EM_BIZCTL_FORWARD, 0);
			BizGUiWriteLog(BIZ_LOG_MASTER_PLAYBACK, BIZ_LOG_SLAVE_PB_FORWARD);
		}
	}
	else
	{
		switch(rate)
		{
			case -3:
				m_pStatic[1]->SetText("<<1/8X");
				BizPlaybackControl(EM_BIZCTL_SET_SPEED, -8);
				break;
			case -2:
				m_pStatic[1]->SetText("<<1/4X");
				BizPlaybackControl(EM_BIZCTL_SET_SPEED, -4);
				break;
			case -1:
				m_pStatic[1]->SetText("<<1/2X");
				BizPlaybackControl(EM_BIZCTL_SET_SPEED, -2);
				break;
			case 0:
				m_pStatic[1]->SetText("<<1X");
				BizPlaybackControl(EM_BIZCTL_SET_SPEED, 1);
				break;
			case 1:
				m_pStatic[1]->SetText("<<2X");
				BizPlaybackControl(EM_BIZCTL_SET_SPEED, 2);
				break;
			case 2:
				m_pStatic[1]->SetText("<<4X");
				BizPlaybackControl(EM_BIZCTL_SET_SPEED, 4);
				break;
			case 3:
				m_pStatic[1]->SetText("<<8X");
				BizPlaybackControl(EM_BIZCTL_SET_SPEED, 8);
				break;
			default:
				break;
		}
		
		playRate_back = rate;
		
		if(m_nForward)
		{
			m_nForward = 0;
			BizPlaybackControl(EM_BIZCTL_BACKWARD, 0);
			BizGUiWriteLog(BIZ_LOG_MASTER_PLAYBACK, BIZ_LOG_SLAVE_PB_BACKWARD);
		}
	}
	
    //add by Lirl on Nov/03/2011
    isStop = 0;
    isPause = 0;
    pButton[0]->SetBitmap(pBmpButtonNormal[0], pBmpButtonSelect[0], pBmpButtonSelect[0], pBmpButtonDisable[0]);
}

void CPagePlayBackFrameWork::OnClkBtnSlider()
{
	CButton *pFocusButton = (CButton *)GetFocusItem();
	int SliderRangeMode = EM_SLIDER_MODE_24HR;
	int i;
	
	for (i=0; i<4; ++i)
	{
		pButton[PB_BUTTON_24HR+i]->Enable(TRUE);
	}
	pFocusButton->Enable(FALSE);
	
	if (pFocusButton == pButton[PB_BUTTON_24HR])
	{
		printf("%s PB_BUTTON_24HR\n", __func__);
		SliderRangeMode = EM_SLIDER_MODE_24HR;
	}
	else if (pFocusButton == pButton[PB_BUTTON_2HR])
	{
		printf("%s PB_BUTTON_2HR\n", __func__);
		SliderRangeMode = EM_SLIDER_MODE_2HR;
	}
	else if (pFocusButton == pButton[PB_BUTTON_1HR])
	{
		printf("%s PB_BUTTON_1HR\n", __func__);
		SliderRangeMode = EM_SLIDER_MODE_1HR;
	}
	else if (pFocusButton == pButton[PB_BUTTON_30M])
	{
		printf("%s PB_BUTTON_30M\n", __func__);
		SliderRangeMode = EM_SLIDER_MODE_30MIN;
	}

	adjustSlider(m_pSlider[0]->GetPos(), SliderRangeMode);
	
}

void CPagePlayBackFrameWork::OnClkPbCtl()
{
	static int bForward = 1;
	static int IsTextCopyLast = 0;
	//static int j = 0;
	int i = 0;
	BOOL bFind = FALSE;
	CButton *pFocusButton = (CButton *)GetFocusItem();
	for(i = 0; i < PB_BMP_NUM; i++)
	{
		if(pFocusButton == pButton[i])
		{
			bFind = TRUE;
			break;
		}
	}
	
	if(bFind)
	{
		bForward = m_nForward;
		switch(i)
		{
			case PB_BUTTON_PAUSE:
			{
				if(isStop)
				{
					BizStartPlayback(EM_BIZPLAY_TYPE_TIME,&sBizSearchParam);

					for(int i=1; i<10 ;i++)
					{
						if(i==7 || i==8 || i==9)
						{
							continue;
						}
						
						pButton[i]->Enable(TRUE);
					}

					pButton[0]->SetBitmap(pBmpButtonNormal[0], pBmpButtonSelect[0], pBmpButtonSelect[0],pBmpButtonDisable[0] );

					isStop = 0;
					isPause = 0;
					bForward = 1;

					m_pStatic[1]->SetText("");
				}
				else
				{
					isPause = !isPause;
					if(isPause)
					{
						printf("PB_BUTTON_PAUSE\n");
						
						IsTextCopyLast = 1;
						BizPlaybackControl(EM_BIZCTL_PAUSE, 0);
						pButton[0]->SetBitmap(pBmpPlay,pBmpPlay_f, pBmpPlay_f);
						strcpy(m_nPlayText, m_pStatic[1]->GetText());
						m_pStatic[1]->SetText(""/*"| |"*/);
						
						BizGUiWriteLog(BIZ_LOG_MASTER_PLAYBACK, BIZ_LOG_SLAVE_PB_PAUSE);
					}
					else
					{
						printf("PB_BUTTON_RESUME\n");
						//ModPlayBackControl(m_hPbMgr,EM_BIZCTL_RESUME,0);
						
						IsTextCopyLast = 0;
						BizPlaybackControl(EM_BIZCTL_RESUME, 0);
						pButton[0]->SetBitmap(pBmpButtonNormal[0], pBmpButtonSelect[0], pBmpButtonSelect[0],pBmpButtonDisable[0] );
						m_pStatic[1]->SetText((VD_PCSTR)m_nPlayText);
						//strcpy(m_nPlayText, ">>1X");
						BizGUiWriteLog(BIZ_LOG_MASTER_PLAYBACK, BIZ_LOG_SLAVE_PB_RESUME);
					}
				}
			}break;
			case PB_BUTTON_STOP:
			{
				printf("PB_BUTTON_STOP\n");
				
				//isStop = 1;
				
				//BizPlaybackControl(EM_BIZCTL_STOP, 0); //造成死锁
				
				//printf("PB_BUTTON_STOP 1\n");
				/*
				for(int i=1; i<10 ;i++)
				{
					//if(i==4 || i==6 || i==8)
					//{
					//	continue;
					//}
					
					pButton[i]->Enable(FALSE);
				}*/
				
				//printf("PB_BUTTON_STOP 2\n");
				//printf("stop %x, %x\n", pBmpPlay,pBmpPlay_f);
				//pButton[0]->SetBitmap(pBmpPlay,pBmpPlay_f, pBmpPlay_f);
				//m_pStatic[1]->SetText("");	
				
				int curTime = m_startTime;
				
				BizPlaybackControl(EM_BIZCTL_SEEK,curTime);	
				
				isPause = 1;
				
				BizPlaybackControl(EM_BIZCTL_PAUSE, 0);
				
				pButton[0]->SetBitmap(pBmpPlay, pBmpPlay_f, pBmpPlay_f);
				
				if(!IsTextCopyLast)
				{
					strcpy(m_nPlayText, m_pStatic[1]->GetText());
					IsTextCopyLast = 1;
				}
				
				m_pStatic[1]->SetText(""/*"| |"*/);
				
				BizGUiWriteLog(BIZ_LOG_MASTER_PLAYBACK, BIZ_LOG_SLAVE_PB_STOP);
				
				SetPbProg(0);//csp modify 20130407
			}break;
			case PB_BUTTON_STEP:
			{
				printf("PB_BUTTON_STEP\n");
				
				IsTextCopyLast = 0;
				
				m_pStatic[1]->SetText(">>1X");
				BizSetPlayType(1);
				
				strcpy(m_nPlayText, m_pStatic[1]->GetText());
				
				if(!isPause)
				{
					BizPlaybackControl(EM_BIZCTL_SET_SPEED, 1);
					
					playRate = 0;
					
					BizPlaybackControl(EM_BIZCTL_FORWARD, 0);
					
					bForward = 1;
				}
				
				isPause = 1;
				
				BizPlaybackControl(EM_BIZCTL_STEP, 0);
				
				pButton[0]->SetBitmap(pBmpPlay, pBmpPlay_f, pBmpPlay_f);
			}break;
			case PB_BUTTON_FASTFORWARD:
			{
				printf("%s PB_BUTTON_FASTFORWARD\n", __func__);
				//ModPlayBackControl(m_hPbMgr,EM_CTL_SPEED_UP,0);
				
				IsTextCopyLast = 0;
				
				BizSetPlayType(1);
				
				if(bForward)
				{
					//BizPlaybackControl(EM_BIZCTL_SPEED_UP, 0);
					if(isPause)
					{
						pButton[0]->SetBitmap(pBmpButtonNormal[0], pBmpButtonSelect[0], pBmpButtonSelect[0],pBmpButtonDisable[0] );
						isPause = 0;
					}
					
					playRate++;
					
					#if 1//csp modify 20130429
					if(playRate > 3)
					{
						playRate = -3;
					}
					#else
					if(playRate > 3)
					{
						playRate = -3;
					}
					#endif
					
					switch(playRate)
					{
						case -3:
							m_pStatic[1]->SetText(">>1/8X");
							BizPlaybackControl(EM_BIZCTL_SET_SPEED, -8);
							break;
						case -2:
							m_pStatic[1]->SetText(">>1/4X");
							BizPlaybackControl(EM_BIZCTL_SET_SPEED, -4);
							break;
						case -1:
							m_pStatic[1]->SetText(">>1/2X");
							BizPlaybackControl(EM_BIZCTL_SET_SPEED, -2);
							break;
						case 0:
							m_pStatic[1]->SetText(">>1X");
							BizPlaybackControl(EM_BIZCTL_SET_SPEED, 1);
							break;
						case 1:
							m_pStatic[1]->SetText(">>2X");
							BizPlaybackControl(EM_BIZCTL_SET_SPEED, 2);
							break;
						case 2:
							m_pStatic[1]->SetText(">>4X");
							BizPlaybackControl(EM_BIZCTL_SET_SPEED, 4);
							break;
						case 3:
							m_pStatic[1]->SetText(">>8X");
							BizPlaybackControl(EM_BIZCTL_SET_SPEED, 8);
							break;
						//csp modify 20130429
						case 4:
							m_pStatic[1]->SetText(">>16X");
							BizPlaybackControl(EM_BIZCTL_SET_SPEED, 16);
							break;
						case 5:
							m_pStatic[1]->SetText(">>32X");
							BizPlaybackControl(EM_BIZCTL_SET_SPEED, 32);
							break;
						case 6:
							m_pStatic[1]->SetText(">>64X");
							BizPlaybackControl(EM_BIZCTL_SET_SPEED, 64);
							break;
						default:
							break;
					}
				}
				else
				{
					playRate = 0;
					
					m_pStatic[1]->SetText(">>1X");
					
					BizPlaybackControl(EM_BIZCTL_SET_SPEED, 1);
					BizPlaybackControl(EM_BIZCTL_FORWARD, 0);
					
					bForward = 1;

					BizGUiWriteLog(BIZ_LOG_MASTER_PLAYBACK, BIZ_LOG_SLAVE_PB_FORWARD);
				}
			}break;
			case PB_BUTTON_TRIANGLE1:
			{
				printf("PB_BUTTON_TRIANGLE1\n");
				
				m_pPagePlayrate0->Open();
			}break;
			case PB_BUTTON_BACKWARD:
			{
				printf("%s PB_BUTTON_BACKWARD\n", __func__);
				//ModPlayBackControl(m_hPbMgr,EM_CTL_SPEED_DOWN,0);
				
				IsTextCopyLast = 0;
				
				BizSetPlayType(0);
				
				if(bForward)
				{
					playRate_back = 0;
					m_pStatic[1]->SetText("<<1X");
					BizPlaybackControl(EM_BIZCTL_SET_SPEED, 1);
					BizPlaybackControl(EM_BIZCTL_BACKWARD, 0);
					
					bForward = 0;
					
					BizGUiWriteLog(BIZ_LOG_MASTER_PLAYBACK, BIZ_LOG_SLAVE_PB_BACKWARD);
				}
				else
				{
					if(isPause)
					{
						pButton[0]->SetBitmap(pBmpButtonNormal[0], pBmpButtonSelect[0], pBmpButtonSelect[0],pBmpButtonDisable[0] );
						isPause = 0;
					}
					
					playRate_back++;
					
					if(playRate_back > 3)
					{
						playRate_back = -3;
					}
					
					switch(playRate_back)
					{
						case -3:
							m_pStatic[1]->SetText("<<1/8X");
							BizPlaybackControl(EM_BIZCTL_SET_SPEED, -8);
							break;
						case -2:
							m_pStatic[1]->SetText("<<1/4X");
							BizPlaybackControl(EM_BIZCTL_SET_SPEED, -4);
							break;
						case -1:
							m_pStatic[1]->SetText("<<1/2X");
							BizPlaybackControl(EM_BIZCTL_SET_SPEED, -2);
							break;
						case 0:
							m_pStatic[1]->SetText("<<1X");
							BizPlaybackControl(EM_BIZCTL_SET_SPEED, 1);
							break;
						case 1:
							m_pStatic[1]->SetText("<<2X");
							BizPlaybackControl(EM_BIZCTL_SET_SPEED, 2);
							break;
						case 2:
							m_pStatic[1]->SetText("<<4X");
							BizPlaybackControl(EM_BIZCTL_SET_SPEED, 4);
							break;
						case 3:
							m_pStatic[1]->SetText("<<8X");
							BizPlaybackControl(EM_BIZCTL_SET_SPEED, 8);
							break;
						default:
							break;
					}
					break;
				}
			}break;
			case PB_BUTTON_TRIANGLE2:
			{
				printf("PB_BUTTON_TRIANGLE2\n");

				m_pPagePlayrate1->Open();
				
			}break;
			case PB_BUTTON_1x1:
			{
				printf("PB_BUTTON_1x1\n");
				
				for(i=0;i<10;i++)
				{
					((CPageDesktop*)m_pDesktop)->ShowSplitLine(i, FALSE);
				}
				
				BizPlayBackZoom(-1);
				
				m_nIsZoomed = 1;
			}break;
			case PB_BUTTON_TRIANGLE3:
			{
				printf("PB_BUTTON_TRIANGLE3\n");
				
				CRect rect;
				pFocusButton->GetRect(&rect);
				
				rect.right = rect.left + 175;
				rect.bottom = rect.bottom - 18;
				rect.top = rect.bottom - 62;
				
				m_pPageChnSel->SetDesktop(m_pDesktop);
				m_pPageChnSel->SetCurStatusPlaying(1);
				
				m_pPageChnSel->SetRect(&rect, TRUE);
				m_pPageChnSel->Open();
				
				
			}break;
			case PB_BUTTON_2x2:
			{
				printf("PB_BUTTON_2x2\n");
				for(i=0;i<10;i++)
					((CPageDesktop*)m_pDesktop)->ShowSplitLine(i, FALSE);
				int nXScale = 2, nYScale = 2;				
				int nChnWidth = nScreenWidth / nXScale;
				int nChnHeight = nScreenHeight / nYScale;
				for (i = 0; i < nXScale - 1; i++)
				{
					((CPageDesktop*)m_pDesktop)->SetSplitLineRect(i, nChnWidth * (i + 1), 0,  nScreenHeight, EM_SPLIT_V);
					((CPageDesktop*)m_pDesktop)->ShowSplitLine(i, TRUE);
				}

				for (i = 0; i < nYScale - 1; i++)
				{
					((CPageDesktop*)m_pDesktop)->SetSplitLineRect(i + nXScale - 1, 0, nChnHeight * (i + 1),  nScreenWidth, EM_SPLIT_H);
					((CPageDesktop*)m_pDesktop)->ShowSplitLine(i + nXScale - 1, TRUE);
				}
				
				BizPlayBackZoom(4);
				m_nIsZoomed = 0;
			}break;
			case PB_BUTTON_ZOOM:
			{
				printf("PB_BUTTON_ZOOM\n");
				
				if(1 == nPlayChnNum)
				{
					#ifndef CHIP_HISI3531//csp modify 20130509
					if(GetPbPauseStatue())
					{
						MessageBox("&CfgPtn.PlayBackFilePause", "&CfgPtn.WARNING", MB_OK|MB_ICONWARNING);
						break;
					}
					#endif
					
					SetZoomStatue(1);
					
					Close();
					
					SetIsZoomed(0);
					
					int format = BizPlayBackGetVideoFormat(0);
					
					if(format < 0)
					{
						printf("The format of Real playback file is error!!!\n");
					}
					else
					{
						BizSetPlayBackFileFormat(format);
					}
					
					((CPageDesktop*)m_pDesktop)->SetElecZoomChn(0);
					((CPageDesktop*)m_pDesktop)->SetPbElecZoomstatus(1);
					((CPageDesktop*)m_pDesktop)->ShowElecZoomTile(0,TRUE);
					((CPageDesktop*)m_pDesktop)->SetCurVideoSize();
				}
				else
				{
					CRect rect;
					pFocusButton->GetRect(&rect);
					
					rect.right = rect.left + 175;
					rect.bottom = rect.bottom - 32;
					rect.top = rect.bottom - 62;
					
					m_pPageChnSel->SetDesktop(m_pDesktop);
					m_pPageChnSel->SetPbCurStatusElecZoom(1);
					m_pPageChnSel->SetCurStatusPlaying(1);
					m_pPageChnSel->SetRect(&rect, TRUE);
					m_pPageChnSel->Open();
				}
			}break;
			case PB_BUTTON_SETCOLOR:
			{
				printf("PB_BUTTON_SETCOLOR\n");

				m_pPagePlayBackVoColorSetup->Open();
				
			}break;
			case PB_BUTTON_AUDIO:
			{
				printf("PB_BUTTON_AUDIO\n");
				
				m_nMute = (!m_nMute);
				SetMute((BOOL)m_nMute);
				BizPlaybackControl(EM_BIZCTL_MUTE,(u32)m_nMute);
			}break;
			case PB_BUTTON_HIDE:
			{
				printf("PB_BUTTON_HIDE\n");
				
				m_bPlayBackHide = TRUE;

				CRect rect(-10,-10,-10,-10);
				this->SetRect(&rect,TRUE);
				
			}break;
			case PB_BUTTON_EXIT:
			{
				printf("PB_BUTTON_EXIT\n");
				IsPbZoom = 0;
				StopPb();
			}break;
			case PB_BUTTON_PRESECT:
			{
				BizPlaybackControl(EM_BIZCTL_PRE_SECT, 0);
				if(isPause)
				{
					BizPlaybackControl(EM_BIZCTL_PAUSE, 0);
				}
				
			}break;
			case PB_BUTTON_NEXTSECT:
			{
				BizPlaybackControl(EM_BIZCTL_NXT_SECT, 0);
				if(isPause)
				{
					BizPlaybackControl(EM_BIZCTL_PAUSE, 0);
				}
			}break;
			default:
			break;
		}
		m_nForward = bForward;
	}
}

//csp modify 20121118
u8 CPagePlayBackFrameWork::GetPBFinished()
{
	return m_nIsFinished;
}

//csp modify 20121118
u8 CPagePlayBackFrameWork::SetPBFinished(u8 state)
{
	m_nIsFinished = state;
	return m_nIsFinished;
}

VD_BOOL CPagePlayBackFrameWork::UpdateData( UDM mode )
{
	if(UDM_CLOSED == mode)
	{
		printf("Pageplayback closed cw^^^^^^^%s,%d\n",__func__,__LINE__);//cw_test
		
		SetPlayBakStatus(0);
		
		if(IsPbZoom)
		{
			return 0;
		}
		
		SetCurPlayRate(1);
		BizSetPlayType(0);
		
		m_pPagePlayrate0->Close();
		m_pPagePlayrate1->Close();
		
		m_nIsZoomed = 0;
		isStop = 0;
		isPause = 0;
		m_AllowOperateMenue = 0;
		
		for(int i=1; i<10; i++)
		{
			pButton[i]->Enable(TRUE);
		}
		
		pButton[0]->SetBitmap(pBmpButtonNormal[0], pBmpButtonSelect[0], pBmpButtonSelect[0],pBmpButtonDisable[0] );
		
		SetSystemLockStatus(1);//cw_lock
		
		((CPagePlayBackVoColorSetup*)m_pPagePlayBackVoColorSetup)->SetDefaultVoImageParam();
		
		((CPageChnSelect*)m_pPageChnSel)->Close();
		
		((CPageAudioSelect*)m_pPageAudioSel)->Close();
		
		((CPagePlayBackVoColorSetup*)m_pPagePlayBackVoColorSetup)->Close();
		
		((CPagePlayrateSelect*)m_pPagePlayrate0)->Close();
		
		((CPagePlayrateSelect*)m_pPagePlayrate1)->Close();

		//printf("Pageplayback closed 1\n",__func__);
		BizResumePreview(1);//yaogang modify 20140918
		//printf("Pageplayback closed 2\n",__func__);
		((CPageDesktop*)m_pDesktop)->SetModePreviewing();//yzw
		
		if(GetTimeDisplayCheck())
		{
			((CPageDesktop*)m_pDesktop)->ShowTimeTitle(0,TRUE);
		}
		
		if(m_pPageSearch)
		{
			m_pPageSearch->GetParentPage()->Show(TRUE);//cw_test
			//printf("playback over PageSearch Open cw^^^^^^^%s,%d\n",__func__,__LINE__);//cw_tes
			m_pPageSearch->Open();//cw_test UpdateData(UDM_OPENED)
			m_pPageSearch = NULL;
		}
		
		SetSystemLockStatus(0);
	}
	else if(UDM_EMPTY == mode)
	{
		printf("Pageplayback empty cw^^^^^^^%s,%d\n",__func__,__LINE__);//cw_test
		
		isStop = 0;
		isPause = 0;
		m_AllowOperateMenue = 0;
		
		for(int i=1; i<10; i++)
		{
			if(0)
			{
				continue;
			}
			
			pButton[i]->Enable(TRUE);
		}
		
		SetSystemLockStatus(1);//cw_lock
		
		SetCurPlayRate(1);
		BizSetPlayType(0);
		
		pButton[0]->SetBitmap(pBmpButtonNormal[0], pBmpButtonSelect[0], pBmpButtonSelect[0],pBmpButtonDisable[0] );
		
		((CPageDesktop*)m_pDesktop)->SetAllElecZoomStatueEmpty();
		
		((CPagePlayBackVoColorSetup*)m_pPagePlayBackVoColorSetup)->SetDefaultVoImageParam();
		
		BizResumePreview(1);
		
		((CPageDesktop*)m_pDesktop)->SetModePreviewing();//yzw
		
		if(GetTimeDisplayCheck())
		{
			((CPageDesktop*)m_pDesktop)->ShowTimeTitle(0,TRUE);
		}
		
		if(m_pPageSearch)
		{
			m_pPageSearch->GetParentPage()->Show(TRUE);//cw_test
			m_pPageSearch->Open();//cw_test UpdateData(UDM_OPENED)
			m_pPageSearch = NULL;
		}
		
		SetSystemLockStatus(0);//cw_lock
	}
	else if(UDM_OPEN == mode)
	{
		m_nIsFinished = 0;//csp modify 20121118
		
		if(m_pPageSearch)//cw_test
		{
			//printf("1Playback open from search cw^^^^^^^%s,%d\n",__func__,__LINE__);//cw_test
			m_pPageSearch->GetParentPage()->Show(FALSE);
		}
		//printf(" 2SetPlayBakStatus cw^^^^^^^%s,%d\n",__func__,__LINE__);//cw_test
		SetPlayBakStatus(1);
		
		int i = 0;
		
		BizGUiWriteLog(BIZ_LOG_MASTER_PLAYBACK, BIZ_LOG_SLAVE_PB_START);  //cw_log
//		printf("hide osd cw^^^^^^^%s,%d\n",__func__,__LINE__);//cw_test
		((CPageDesktop*)m_pDesktop)->HideAllOsdWithoutTime(nPlayChnNum);
//		printf("hide time cw^^^^^^^%s,%d\n",__func__,__LINE__);//cw_test
		((CPageDesktop*)m_pDesktop)->ShowTimeTitle(0,FALSE);
		//((CPageDesktop*)m_pDesktop)->HideCoverRect();
//		printf("set mode playing cw^^^^^^^%s,%d\n",__func__,__LINE__);//cw_test
		((CPageDesktop*)m_pDesktop)->SetModePlaying(); //yzw
//		printf("set line cw^^^^^^^%s,%d\n",__func__,__LINE__);//cw_test
		if(nPlayChnNum == 1)
		{
			for(int i = 0; i < 10; i++)
			{
				((CPageDesktop*)m_pDesktop)->ShowSplitLine(i, FALSE);
			}
			
			pButton[7]->Show(FALSE);
			pButton[8]->Show(FALSE);
			pButton[9]->Show(FALSE);
		}
		else if(nPlayChnNum == 4)
		{
			for(i = 0; i < 10; i++)//9624
			{
				((CPageDesktop*)m_pDesktop)->ShowSplitLine(i, FALSE);
			}
			
			if(!m_nIsZoomed)
			{
				int nXScale = 2, nYScale = 2;				
				int nChnWidth = nScreenWidth / nXScale;
				int nChnHeight = nScreenHeight / nYScale;
				
				for (i = 0; i < nXScale - 1; i++)
				{
					((CPageDesktop*)m_pDesktop)->SetSplitLineRect(i, nChnWidth * (i + 1), 0,  nScreenHeight, EM_SPLIT_V);
					((CPageDesktop*)m_pDesktop)->ShowSplitLine(i, TRUE);
				}

				for (i = 0; i < nYScale - 1; i++)
				{
					((CPageDesktop*)m_pDesktop)->SetSplitLineRect(i + nXScale - 1, 0, nChnHeight * (i + 1),  nScreenWidth, EM_SPLIT_H);
					((CPageDesktop*)m_pDesktop)->ShowSplitLine(i + nXScale - 1, TRUE);
				}
			}
			
			pButton[7]->Show(TRUE);
			pButton[8]->Show(TRUE);
			pButton[9]->Show(TRUE);
		}
		else
		{
			for(i = 0; i < 10; i++)
			{
				((CPageDesktop*)m_pDesktop)->ShowSplitLine(i, FALSE);
			}			
		}
		
		//printf("set pb color cw^^^^^^^%s,%d\n",__func__,__LINE__);//cw_test
		
		((CPagePlayBackVoColorSetup*)m_pPagePlayBackVoColorSetup)->SetPlayBackColor();
		m_pStatic[1]->SetText(">>1X");
		
		playRate = 0;
		playRate_back = 0;
		m_nForward = 1;
		
		//printf("m_nForward-1=%d\n",m_nForward);
		
		if(exitstatue)
		{
			int currate = GetCurPlayRate();
			
			switch(currate)
			{
				case 92:
					m_pStatic[1]->SetText("<<1/8X");
					playRate_back = -3;
					break;
				case 96:
					m_pStatic[1]->SetText("<<1/4X");
					playRate_back = -2;
					break;
				case 98:
					m_pStatic[1]->SetText("<<1/2X");
					playRate_back = -1;
					break;
				case 101:
					m_pStatic[1]->SetText("<<1X");
					playRate_back = 0;
					break;
				case 102:
					m_pStatic[1]->SetText("<<2X");
					playRate_back = 1;
					break;
				case 104:
					m_pStatic[1]->SetText("<<4X");
					playRate_back = 2;
					break;
				case 108:
					m_pStatic[1]->SetText("<<8X");
					playRate_back = 3;
					break;
				default:
					break;
			}
			
			switch(currate)
			{
				case -8:
					m_pStatic[1]->SetText(">>1/8X");
					playRate = -3;
					break;
				case -4:
					m_pStatic[1]->SetText(">>1/4X");
					playRate = -2;
					break;
				case -2:
					m_pStatic[1]->SetText(">>1/2X");
					playRate = -1;
					break;
				case 1:
					m_pStatic[1]->SetText(">>1X");
					playRate = 0;
					break;
				case 2:
					m_pStatic[1]->SetText(">>2X");
					playRate = 1;
					break;
				case 4:
					m_pStatic[1]->SetText(">>4X");
					playRate = 2;
					break;
				case 8:
					m_pStatic[1]->SetText(">>8X");
					playRate = 3;
					break;
				//csp modify 20130429
				case 16:
					m_pStatic[1]->SetText(">>16X");
					playRate = 4;
					break;
				case 32:
					m_pStatic[1]->SetText(">>32X");
					playRate = 5;
					break;
				case 64:
					m_pStatic[1]->SetText(">>64X");
					playRate = 6;
					break;
				default:
					break;
			}
			
			exitstatue = 0;
			
			if(currate > 90)
			{
				m_nForward = 0;
			}
			else
			{
				m_nForward = 1;
			}
		}
		
		//printf("m_nForward-2=%d\n",m_nForward);
		
		m_nMute = 0;
		SetMute((BOOL)m_nMute);
		//printf("3 playback control cw^^^^^^^%s,%d\n",__func__,__LINE__);//cw_test
		BizPlaybackControl(EM_BIZCTL_MUTE,(u32)m_nMute);
		//printf("4 playback control cw^^^^^^^%s,%d\n",__func__,__LINE__);//cw_test
		SetPbTotalTime("00:00:00/00:00:00");

		//yaogang
		InitSlider();
	}
	
	return TRUE;
}

void CPagePlayBackFrameWork::SetDesktop(CPage* pDesktop)
{
	m_pDesktop = pDesktop;
}

VD_BOOL CPagePlayBackFrameWork::MsgProc(uint msg, uint wpa, uint lpa)
{
	if(m_AllowOperateMenue == 0)
	{
		return TRUE;
	}
	
	int i = 0;
	int px;
	int py;
	
	static int bForward = 1;
	
	u8 lock_flag = 0;//cw_shutdown
	GetSystemLockStatus(&lock_flag);
	if(lock_flag)
	{
		return FALSE;
	}
	
	//printf("msg = %d, file:%s, func:%s\n", msg, __FILE__, __FUNCTION__);
	if(msg == XM_KEYDOWN)
	{
		uchar key = wpa;
		
		#if 0//csp modify 20130326
		if(key>=KEY_0 && key<=KEY_9)
		{
			BoardGetReUsedKey(wpa, &key);
			printf("key reused: %d\n",key);
			wpa = key;
		}
		#endif
		
		bForward = m_nForward;//csp modify 20121104
		
		switch(key)
		{
			case KEY_RET://cw_panel
				{
					printf("KEY_RET\n");
					
					CItem* pItem = GetFocusItem();
					
					if(pItem == NULL)
					{
						return 0;
					}
					
					break;
				}
			case KEY_ESC:
				printf("KEY_ESC\n");
				
				if(m_bPlayBackHide)
				{
					SetRect(&m_pbRect,TRUE);
					m_bPlayBackHide = FALSE;
					
					return 0;
				}
				
				IsPbZoom = 0;
				
				StopPb();
				
				break;
			case KEY_STOP:
				printf("PB_BUTTON_STOP\n");
				
				isStop = 1;
				
				BizPlaybackControl(EM_BIZCTL_STOP, 0);
				
				BizGUiWriteLog(BIZ_LOG_MASTER_PLAYBACK, BIZ_LOG_SLAVE_PB_STOP);
				
				break;
			case KEY_PLAY:
				printf("KEY_PLAY\n");
			case KEY_PAUSE:
				if(isStop)
				{
					BizStartPlayback(EM_BIZPLAY_TYPE_TIME,&sBizSearchParam);
					
					for(int i=1; i<10 ;i++)
					{
						if(i==4 || i==6 || i==8)
						{
							continue;
						}
						
						pButton[i]->Enable(TRUE);
					}
					
					pButton[0]->SetBitmap(pBmpButtonNormal[0], pBmpButtonSelect[0], pBmpButtonSelect[0],pBmpButtonDisable[0] );
					
					isStop = 0;
					isPause = 0;
					
					m_pStatic[1]->SetText("");
				}
				else
				{
					isPause = !isPause;
					
					if(isPause)
					{
						printf("PB_BUTTON_PAUSE\n");
						
						BizPlaybackControl(EM_BIZCTL_PAUSE, 0);
						
						pButton[0]->SetBitmap(pBmpPlay, pBmpPlay_f, pBmpPlay_f);
						strcpy(m_nPlayText, m_pStatic[1]->GetText());
						
						m_pStatic[1]->SetText(""/*"| |"*/);
						
						BizGUiWriteLog(BIZ_LOG_MASTER_PLAYBACK, BIZ_LOG_SLAVE_PB_PAUSE);
					}
					else
					{
						printf("PB_BUTTON_RESUME\n");
						
						BizPlaybackControl(EM_BIZCTL_RESUME, 0);
						
						pButton[0]->SetBitmap(pBmpButtonNormal[0], pBmpButtonSelect[0], pBmpButtonSelect[0],pBmpButtonDisable[0] );
						m_pStatic[1]->SetText((VD_PCSTR)m_nPlayText);
						
						BizGUiWriteLog(BIZ_LOG_MASTER_PLAYBACK, BIZ_LOG_SLAVE_PB_RESUME);
					}
				}
				break;

			case KEY_6://csp modify 20130326
			case KEY_STEPF:
				printf("PB_BUTTON_STEP\n");
				isPause = 1;
				BizPlaybackControl(EM_BIZCTL_STEP, 0);
				pButton[0]->SetBitmap(pBmpPlay, pBmpPlay_f, pBmpPlay_f);
				break;

			case KEY_1://csp modify 20130326
			case KEY_PREV:
				printf("PB_BUTTON_PRESECT\n");
				
				if(isPause)
				{
					pButton[0]->SetBitmap(pBmpButtonNormal[0], pBmpButtonSelect[0], pBmpButtonSelect[0],pBmpButtonDisable[0] );
					isPause = 0;
				}
				
				BizPlaybackControl(EM_BIZCTL_PRE_SECT, 0);
				break;

			case KEY_2://csp modify 20130326
			case KEY_NEXT:
				printf("PB_BUTTON_NEXTSECT\n");
				
				if(isPause)
				{
					pButton[0]->SetBitmap(pBmpButtonNormal[0], pBmpButtonSelect[0], pBmpButtonSelect[0],pBmpButtonDisable[0] );
					isPause = 0;
				}
				
				BizPlaybackControl(EM_BIZCTL_NXT_SECT, 0);
				break;

			case KEY_4://csp modify 20130326
			case KEY_FAST:
				printf("%s PB_BUTTON_FASTFORWARD\n", __func__);
				
				BizSetPlayType(1);
				
				if(bForward)
				{
					BizPlaybackControl(EM_BIZCTL_SPEED_UP, 0);
					
					if(isPause)
					{
						pButton[0]->SetBitmap(pBmpButtonNormal[0], pBmpButtonSelect[0], pBmpButtonSelect[0],pBmpButtonDisable[0] );
						isPause = 0;
					}
					
					playRate++;
					
					#if 1//csp modify 20130429
					if(playRate > 6)
					{
						playRate = 6;
					}
					#else
					if(playRate > 3)
					{
						playRate = 3;
					}
					#endif
					
					switch(playRate)
					{
						case -3://csp modify 20130429
							m_pStatic[1]->SetText(">>1/8X");
							SetCurPlayRate(-8);//csp modify 20130429
							break;
						case -2:
							m_pStatic[1]->SetText(">>1/4X");
							SetCurPlayRate(-4);//csp modify 20130429
							break;
						case -1:
							m_pStatic[1]->SetText(">>1/2X");
							SetCurPlayRate(-2);//csp modify 20130429
							break;
						case 0:
							m_pStatic[1]->SetText(">>1X");
							SetCurPlayRate(1);//csp modify 20130429
							break;
						case 1:
							m_pStatic[1]->SetText(">>2X");
							SetCurPlayRate(2);//csp modify 20130429
							break;
						case 2:
							m_pStatic[1]->SetText(">>4X");
							SetCurPlayRate(4);//csp modify 20130429
							break;
						case 3:
							m_pStatic[1]->SetText(">>8X");
							SetCurPlayRate(8);//csp modify 20130429
							break;
						//csp modify 20130429
						case 4:
							m_pStatic[1]->SetText(">>16X");
							SetCurPlayRate(16);//csp modify 20130429
							break;
						case 5:
							m_pStatic[1]->SetText(">>32X");
							SetCurPlayRate(32);//csp modify 20130429
							break;
						case 6:
							m_pStatic[1]->SetText(">>64X");
							SetCurPlayRate(64);//csp modify 20130429
							break;
						default:
							break;
					}
				}
				else
				{
					playRate = 0;
					
					m_pStatic[1]->SetText(">>1X");
					BizPlaybackControl(EM_BIZCTL_SET_SPEED, 1);
					BizPlaybackControl(EM_BIZCTL_FORWARD, 0);
					
					bForward = 1;
				}
				break;

			case KEY_3://csp modify 20130326
			case KEY_SLOW:
				printf("PB_BUTTON_SLOWFORWARD\n");
				
				BizSetPlayType(1);
				
				if(bForward)
				{
					BizPlaybackControl(EM_BIZCTL_SPEED_DOWN, 0);
					
					if(isPause)
					{
						pButton[0]->SetBitmap(pBmpButtonNormal[0], pBmpButtonSelect[0], pBmpButtonSelect[0],pBmpButtonDisable[0] );
						isPause = 0;
					}
					
					playRate--;
					
					if(playRate < -3)
					{
						playRate = -3;
					}
					
					switch(playRate)
					{
						case -3:
							m_pStatic[1]->SetText(">>1/8X");
							SetCurPlayRate(-8);//csp modify 20130429
							break;
						case -2:
							m_pStatic[1]->SetText(">>1/4X");
							SetCurPlayRate(-4);//csp modify 20130429
							break;
						case -1:
							m_pStatic[1]->SetText(">>1/2X");
							SetCurPlayRate(-2);//csp modify 20130429
							break;
						case 0:
							m_pStatic[1]->SetText(">>1X");
							SetCurPlayRate(1);//csp modify 20130429
							break;
						case 1:
							m_pStatic[1]->SetText(">>2X");
							SetCurPlayRate(2);//csp modify 20130429
							break;
						case 2:
							m_pStatic[1]->SetText(">>4X");
							SetCurPlayRate(4);//csp modify 20130429
							break;
						case 3:
							m_pStatic[1]->SetText(">>8X");
							SetCurPlayRate(8);//csp modify 20130429
							break;
						//csp modify 20130429
						case 4:
							m_pStatic[1]->SetText(">>16X");
							SetCurPlayRate(16);//csp modify 20130429
							break;
						case 5:
							m_pStatic[1]->SetText(">>32X");
							SetCurPlayRate(32);//csp modify 20130429
							break;
						case 6:
							m_pStatic[1]->SetText(">>64X");
							SetCurPlayRate(64);//csp modify 20130429
							break;
						default:
							break;
					}
				}
				else
				{
					playRate = 0;
					
					m_pStatic[1]->SetText(">>1X");
					BizPlaybackControl(EM_BIZCTL_SET_SPEED, 1);
					BizPlaybackControl(EM_BIZCTL_FORWARD, 0);
					
					bForward = 1;
				}
				break;

			case KEY_5://csp modify 20130326
			case KEY_BACK:
				{
					printf("PB_BUTTON_BACKWARD-key,bForward=%d\n",bForward);
					
					BizSetPlayType(0);
					
					if(bForward)
					{
						playRate_back = 0;
						
						m_pStatic[1]->SetText("<<1X");
						BizPlaybackControl(EM_BIZCTL_SET_SPEED, 1);
						BizPlaybackControl(EM_BIZCTL_BACKWARD, 0);
						
						bForward = 0;
						
						BizGUiWriteLog(BIZ_LOG_MASTER_PLAYBACK, BIZ_LOG_SLAVE_PB_BACKWARD);
					}
					else
					{
						if(isPause)
						{
							pButton[0]->SetBitmap(pBmpButtonNormal[0], pBmpButtonSelect[0], pBmpButtonSelect[0],pBmpButtonDisable[0] );
							isPause = 0;
						}
						
						playRate_back++;
						
						if(playRate_back > 3)
						{
							playRate_back = -3;
						}
						
						switch(playRate_back)
						{
							case -3:
								m_pStatic[1]->SetText("<<1/8X");
								BizPlaybackControl(EM_BIZCTL_SET_SPEED, -8);
								break;
							case -2:
								m_pStatic[1]->SetText("<<1/4X");
								BizPlaybackControl(EM_BIZCTL_SET_SPEED, -4);
								break;
							case -1:
								m_pStatic[1]->SetText("<<1/2X");
								BizPlaybackControl(EM_BIZCTL_SET_SPEED, -2);
								break;
							case 0:
								m_pStatic[1]->SetText("<<1X");
								BizPlaybackControl(EM_BIZCTL_SET_SPEED, 1);
								break;
							case 1:
								m_pStatic[1]->SetText("<<2X");
								BizPlaybackControl(EM_BIZCTL_SET_SPEED, 2);
								break;
							case 2:
								m_pStatic[1]->SetText("<<4X");
								BizPlaybackControl(EM_BIZCTL_SET_SPEED, 4);
								break;
							case 3:
								m_pStatic[1]->SetText("<<8X");
								BizPlaybackControl(EM_BIZCTL_SET_SPEED, 8);
								break;
							default:
								break;
						}
						break;
					}
				}
				break;
			default:
				break;
		}
		
		m_nForward = bForward;//csp modify 20121104
	}	
	else if(msg == XM_LBUTTONDBLCLK)
	{
		if(m_bPlayBackHide && (nPlayChnNum > 1))
		{
			int px = VD_HIWORD(lpa);
        	int py = VD_LOWORD(lpa);
			//printf("px:%d, py:%d\n", px, py);
			
			if(nPlayChnNum <= 4)
			{
				if(m_nIsZoomed)
				{
					for(i=0;i<10;i++)
						((CPageDesktop*)m_pDesktop)->ShowSplitLine(i, FALSE);
					int nXScale = 2, nYScale = 2;				
					int nChnWidth = nScreenWidth / nXScale;
					int nChnHeight = nScreenHeight / nYScale;
					for (i = 0; i < nXScale - 1; i++)
					{
						((CPageDesktop*)m_pDesktop)->SetSplitLineRect(i, nChnWidth * (i + 1), 0,  nScreenHeight, EM_SPLIT_V);
						((CPageDesktop*)m_pDesktop)->ShowSplitLine(i, TRUE);
					}

					for (i = 0; i < nYScale - 1; i++)
					{
						((CPageDesktop*)m_pDesktop)->SetSplitLineRect(i + nXScale - 1, 0, nChnHeight * (i + 1),  nScreenWidth, EM_SPLIT_H);
						((CPageDesktop*)m_pDesktop)->ShowSplitLine(i + nXScale - 1, TRUE);
					}

					BizPlayBackZoom(4);
					m_nIsZoomed = 0;
				}
				else
				{
					u8 chnTmp = GetMouseInWhichChn(px, py, 4);
					u64 tmp = 0;
					BizPlayBackGetRealPlayChn(&tmp);
					if(!(tmp & (1 << chnTmp)))
					{
						//MessageBox("&CfgPtn.NoPlayBackFile", "&CfgPtn.WARNING" ,
			       		// MB_OK|MB_ICONWARNING);
						return TRUE;
					}
					
					if(0 == BizPlayBackZoom((s32)chnTmp))
					{
						for(i=0;i<10;i++)
							((CPageDesktop*)m_pDesktop)->ShowSplitLine(i, FALSE);

						m_nIsZoomed = 1;
					}
				}
			}
		}
		return TRUE;
	}

	switch(msg)
	{
		case XM_LBUTTONDOWN:
			break;
		case XM_MOUSEMOVE:
			break;
		case XM_LBUTTONUP:
			break;
		case XM_LBUTTONDBLCLK:
			break;
		case XM_RBUTTONDOWN:
		case XM_RBUTTONDBLCLK:
			if(m_bPlayBackHide)
			{
				SetRect(&m_pbRect,TRUE);
				m_bPlayBackHide = FALSE;
				return 0;
			}
			else
			{
				CRect rect(-10,-10,-10,-10);
				SetRect(&rect,TRUE);
				m_bPlayBackHide = TRUE;				
				return 0;
			}
			break;
		case XM_RBUTTONUP:
			break;
		default:
			break;
	}       

	return CPageFloat::MsgProc(msg, wpa, lpa);
}

void CPagePlayBackFrameWork::SetIsZoomed(u8 zoom)
{
	m_nIsZoomed = zoom;
}

u8 CPagePlayBackFrameWork::GetMouseInWhichChn(int x, int y, int chnNum)
{
	static int width = -1;
	static int height = -1;
	
	if(width == -1)
		GetVgaResolution(&width, &height);
	
	int i = 0;
	int j = 0;
	int rows = 1;
	int lines = 1;	
	
	if((chnNum <=4) && (chnNum > 1))
	{
		rows = 2;
		lines = 2;
	}
	else if(chnNum <=9)
	{
		rows = 3;
		lines = 3;
	}
	else
	{
		rows = 4;
		lines = 4;
	}

	for(i = 0; i < rows; i++)
	{
		for(j = 0; j < lines; j++)
		{
			if((x >= j*width/lines) && (x <= (j+1)*width/lines) 
				&& (y >= i*height/rows) && (y <= (i+1)*height/rows))
				break;
		}
		if(j<lines)
			break;
	}
	
	//printf("zoom chn%d\n", (i*lines+j));
	
	return (u8)(i*lines+j);
}

void CPagePlayBackFrameWork::SetPlayChnNum( int nChnNum )
{
	int i;
	nPlayChnNum = nChnNum;

	if (1 == nPlayChnNum)
	{
		for (i=1; i<4; ++i)
		{
			m_pSlider[i]->Show(FALSE);
		}
	}
	else if (4 == nPlayChnNum)
	{
		for (i=1; i<4; ++i)
		{
			m_pSlider[i]->Show(TRUE);
		}
	}
}

void CPagePlayBackFrameWork::SetPlayDate(u32 startTime)
{
	m_CurPlayDate = startTime;
}

void CPagePlayBackFrameWork::SetSearchPage(CPage* pPage)
{
	m_pPageSearch = pPage;
}

void CPagePlayBackFrameWork::GetCurPlayMute(u8* mute)
{
	*mute = m_nMute;
}

void CPagePlayBackFrameWork::SetCurPlayMute(u8 mute)
{
	m_nMute = mute;
	SetMute((BOOL)m_nMute);
	BizPlaybackControl(EM_BIZCTL_MUTE,(u32)m_nMute);
}

void CPagePlayBackFrameWork::SetZoomStatue(u8 flag)
{
	IsPbZoom = flag;
}

u8 CPagePlayBackFrameWork::GetZoomStatue()
{
	return IsPbZoom;
}

int CPagePlayBackFrameWork::GetPbNum()
{
	return nPlayChnNum;
}

int CPagePlayBackFrameWork::GetPbPauseStatue()
{
	return isPause;
}

void CPagePlayBackFrameWork::SetExitStatue(u8 flag)
{
	exitstatue = flag;
}

