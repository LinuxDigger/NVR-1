#ifndef __PAGE_SEARCH_REC_H__
#define __PAGE_SEARCH_REC_H__


#include "PageFloat.h"
#include "biz.h"

#define RIGHT_PAGE_WIDTH	(204)
#define BOTTOM_PAGE_HEIGHT	(10/*上边沿*/+24/*刻度值*/+12*4/*进度条*/+24/*下边沿*/)	//106

#if 0
typedef struct 
{
	std::vector<int> v_indexs; //存储sSearchResult0.psRecfileInfo[] 下标
	std::vector<Range> v_ranges;
}
#endif

class CPageSearchRec:public CPageFloat
{
public:
	CPageSearchRec(VD_PCRECT pRect,VD_PCSTR psz = NULL,VD_BITMAP* icon = NULL,CPage * pParent = NULL, uint vstyle = 0);
	~CPageSearchRec();
	VD_BOOL UpdateData( UDM mode );
	VD_BOOL MsgProc( uint msg, uint wpa, uint lpa );

private:
	void OnDaySelected();
	void OnEventType();
	void OnEventAllType();
	void OnCombox();
	void OnClkBtn();
	void OnSlider();
	void UpdateCalendar(SYSTEM_TIME* pTime);
	void AdjustChn();
	void Search();
	void dealResult();//搜索后处理结果并对进度条着色
	
private:
	CStatic* pTitle;
	CStatic* pFrameLines[6];
	CCalendar* pCalendar;

	//录像类型
	CCheckBox* pChkRecType[5];
	CStatic* pTextRecType[5];

	//单画面、四画面
	//std::vector<CItem*>	items[2];
	int playChnNum;
	CButton* pBtnSingle;
	CButton* pBtnFour;
	CComboBox *pComboBoxChn[4];
	CStatic* pTextChn[4];

	CButton* pBtnPlay;

	//进度条
	CStatic* pTextHour[25];//小时刻度
	CSliderCtrlPartColor *m_pSlider[4];	

	int m_ChnMax;

	//搜索
	u32 todayOpenTime;
	u32 todayBeginTime;
	u32 m_startTime;
	u32 m_endTime;
	u8 m_MaskType;
	u8 m_WindowChn[4];

	SBizSearchPara sSearchPara;
	SBizSearchResult sSearchResult0;
	
	
};

#endif


