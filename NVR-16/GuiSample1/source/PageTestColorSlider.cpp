#include "GUI/Pages/PageTestColorSlider.h"
#include "GUI/Pages/PageSearch.h"
#include "GUI/Pages/PageDesktop.h"

#include "biz.h"


CPageTestColorSlider::CPageTestColorSlider( VD_PCRECT pRect,VD_PCSTR psz /*= NULL*/,VD_BITMAP* icon /*= NULL*/,CPage * pParent /*= NULL*/, uint vstyle /*= 0*/ )
:CPageFloat(pRect, pParent,psz)
{
	CRect rtTemp;
	int i;
	
	rtTemp.left = TEXT_WIDTH *20;
	rtTemp.top = 8;
	rtTemp.right = rtTemp.left + TEXT_WIDTH*2;
	rtTemp.bottom = rtTemp.top + CTRL_HEIGHT;

	char buf[10];
	for (i = 0; i<4; ++i)
	{
		sprintf(buf, "%d", i+1);
		pBt[i] = CreateButton(rtTemp, this, buf, (CTRLPROC)&CPageTestColorSlider::OnClickBt, NULL, buttonNormalBmp);
		
		rtTemp.left = rtTemp.right + 8;
		rtTemp.right = rtTemp.left + TEXT_WIDTH*2;
	}

	rtTemp.left = 8;
	rtTemp.top = rtTemp.bottom + 8;
	rtTemp.right = m_Rect.Width() - 8;
	rtTemp.bottom = rtTemp.top + 12;
	//m_pSlider = new CSliderCtrlPartColor(rtTemp, this, 0, 24*60*60, 24, (CTRLPROC)&CPageTestColorSlider::OnSlider);
	m_pSlider = new CSliderCtrlPartColor(rtTemp, this, 0, 100, 10, (CTRLPROC)&CPageTestColorSlider::OnSlider);

	Range r;
	std::vector<Range> vr;
	for (i = 0; i < 10; ++i)
	{
		if (i%2)
		{
			r.start = i*10;
			r.end = r.start + 10;
			vr.push_back(r);
		}
	}

	m_pSlider->SetColorRange(vr);
}

CPageTestColorSlider::~CPageTestColorSlider()
{

}

void CPageTestColorSlider::SetSearchPage(CPageSearch* pPage)
{
	m_pPageSearch = pPage;
}

VD_BOOL CPageTestColorSlider::UpdateData( UDM mode )
{
	if(UDM_OPEN == mode)
	{
		printf("CPageTestColorSlider open!\n", __func__);
		
		//处理界面
		SetPlayBakStatus(1);
		
		m_pPageSearch->GetParentPage()->Show(FALSE);
		
		
		CPage** pPageList = GetPage();

		m_pDesktop = pPageList[EM_PAGE_DESKTOP];
		
		((CPageDesktop*)m_pDesktop)->HideAllOsdWithoutTime(1);		
		((CPageDesktop*)m_pDesktop)->ShowTimeTitle(0,FALSE);
		((CPageDesktop*)m_pDesktop)->SetModePlaying(); //yzw		

		for(int i = 0; i < 10; i++)
		{
			((CPageDesktop*)m_pDesktop)->ShowSplitLine(i, FALSE);
		}
	}
	else if (UDM_CLOSED == mode)
	{
		SetPlayBakStatus(0);
		
		SetSystemLockStatus(1);//cw_lock
		
		BizResumePreview(1);//yaogang modify 20140918
		
		((CPageDesktop*)m_pDesktop)->SetModePreviewing();//yzw
		
		if(GetTimeDisplayCheck())
		{
			((CPageDesktop*)m_pDesktop)->ShowTimeTitle(0,TRUE);
		}

		if(m_pPageSearch)
		{
			m_pPageSearch->GetParentPage()->Show(TRUE);//cw_test
			printf("playback over PageSearch Open cw^^^^^^^%s,%d\n",__func__,__LINE__);//cw_tes
			m_pPageSearch->Open();//cw_test UpdateData(UDM_OPENED)
			m_pPageSearch = NULL;
		}
		
		
		SetSystemLockStatus(0);
	}
	return TRUE;
}
VD_BOOL CPageTestColorSlider::MsgProc( uint msg, uint wpa, uint lpa )
{
	return CPage::MsgProc(msg, wpa, lpa);
}
	

void CPageTestColorSlider::OnClickBt()
{
	int i = 0;
	Range r;
	int SplitLineNum = 0;
	CButton *pFocusButton = (CButton *)GetFocusItem();

	for(i = 0; i < 4; i++)
	{
		if(pFocusButton == pBt[i])
		{
			printf("button %d target\n", i+1);
			break;
		}
	}

	switch (i)
	{
		case 0:
		{
			r.start = 0;
			r.end = 10;
			SplitLineNum = 2;
			m_pSlider->SetPos(5);
			m_pSlider->SetDisplayRange(r, SplitLineNum);
		} break;
		case 1:
		{
			r.start = 5;
			r.end = 85;
			SplitLineNum = 16;
			m_pSlider->SetPos(25);
			m_pSlider->SetDisplayRange(r, SplitLineNum);
		} break;
		case 2:
		{
			r.start = 11;
			r.end = 21;
			SplitLineNum = 10;
			m_pSlider->SetPos(16);
			m_pSlider->SetDisplayRange(r, SplitLineNum);
		} break;
		case 3:
		{
			r.start = 0;
			r.end = 100;
			SplitLineNum = 10;
			m_pSlider->SetDisplayRange(r, SplitLineNum);
		} break;
		
	}
}

void CPageTestColorSlider::OnSlider()
{
	int pos = m_pSlider->GetPos();
	printf("slider val: %d\n", pos);
}
