// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"	
#include "../uieditor/Global.h"

CMainDlg::CMainDlg(LPCTSTR pszLayoutId) : SHostWnd(pszLayoutId),m_pHover(NULL),m_bVirtualRoot(FALSE)
{
}

CMainDlg::~CMainDlg()
{
}

//TODO:消息映射
void CMainDlg::OnClose()
{
	SNativeWnd::DestroyWindow();
	PostQuitMessage(0);
}

void CMainDlg::OnMaximize()
{
	SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
}
void CMainDlg::OnRestore()
{
	SendMessage(WM_SYSCOMMAND, SC_RESTORE);
}
void CMainDlg::OnMinimize()
{
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
}

LRESULT CMainDlg::OnMouseEvent(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(GetKeyState(VK_MENU)&0x80)
	{
		SetMsgHandled(FALSE);
		return TRUE;
	}else
	{
		CPoint pt(GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam));
		if(uMsg == WM_LBUTTONDOWN)
		{//tell editor the clicked window id.
			m_pHover->SetVisible(FALSE);
			SWND swnd = SwndFromPoint(pt,true);
			m_pHover->SetVisible(TRUE);
			if(swnd)
			{
				SWindow *p = SWindowMgr::GetWindow(swnd);
				SList<int> lstIndex;
				GetSwndIndex(p,lstIndex);
				if(m_bVirtualRoot)
				{
					lstIndex.RemoveHead();
				}
				HWND hEditor = FindWindowEx(NULL,NULL,ksz_editor_cls,ksz_editor_wnd);
				if(hEditor)
				{
					COPYDATASTRUCT cds;
					cds.dwData = kcds_id;
					cds.cbData = sizeof(int)*lstIndex.GetCount();
					int *pData = new int[lstIndex.GetCount()];
					int i=0;
					SPOSITION pos = lstIndex.GetHeadPosition();
					while(pos)
					{
						pData[i++]=lstIndex.GetNext(pos);
					}
					cds.lpData = pData;
					::SendMessage(hEditor,WM_COPYDATA,(WPARAM)m_hWnd,(LPARAM)&cds);
					delete []pData;
				}
			}
		}else if(uMsg == WM_MOUSEMOVE)
		{//highlight the hovered window
			m_pHover->SetVisible(FALSE);
			SWND swnd = SwndFromPoint(pt,true);
			m_pHover->SetVisible(TRUE);
			if(swnd)
			{
				SWindow *p = SWindowMgr::GetWindow(swnd);
				CRect rc = p->GetWindowRect();
				m_pHover->Move(rc);
			}else
			{
				m_pHover->Move(0,0,0,0);
			}
		}

		return TRUE;
	}
}

void CMainDlg::GetSwndIndex(SWindow *pWnd,SList<int> &lstIndex)
{
	int idx = 0;
	SWindow *pPrev = pWnd->GetWindow(GSW_PREVSIBLING);
	while(pPrev)
	{
		idx++;
		pPrev = pPrev->GetWindow(GSW_PREVSIBLING);
	}
	lstIndex.AddHead(idx);
	SWindow *pParent = pWnd->GetParent();
	if(pParent)
	{
		GetSwndIndex(pParent,lstIndex);
	}
}

BOOL CMainDlg::OnLoadLayoutFromResourceID(const SStringT & resId)
{
	if(resId.IsEmpty())
		return FALSE;
	pugi::xml_document xmlDoc;
	BOOL bLoaded=FALSE;
	if(!m_utf8Buffer.IsEmpty())
	{
		bLoaded = xmlDoc.load_buffer(m_utf8Buffer.c_str(),m_utf8Buffer.GetLength(),pugi::parse_default,pugi::encoding_utf8);
	}else
	{
		bLoaded =LOADXML(xmlDoc,m_strXmlLayout);
	}
	if(bLoaded)
	{
		pugi::xml_node xmlSoui = xmlDoc.child(L"SOUI");
		if(xmlSoui)
		{
			return InitFromXml(xmlDoc.child(L"SOUI"));
		}else
		{//include element.
			const WCHAR ksz_include_host[] = L"<soui  margin=\"2,2,2,2\" resizable=\"1\">"\
				L"<root width=\"600\" height=\"400\" colorBkgnd=\"#888888\"/>"\
				L"</soui>";
			pugi::xml_document xmlDoc2;
			xmlDoc2.load_buffer(ksz_include_host,sizeof(ksz_include_host),pugi::parse_default,pugi::encoding_wchar);
			pugi::xml_node xmlRoot2=xmlDoc2.first_child().first_child();
			xmlRoot2.append_copy(xmlDoc.first_child());
			m_bVirtualRoot = TRUE;
			return InitFromXml(xmlDoc2.first_child());
		}
	}else
	{
		SASSERT_FMTA(FALSE,"Load layout [%s] Failed",S_CT2A(m_strXmlLayout));
		return FALSE;
	}
}

void CMainDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if(nChar == VK_ESCAPE|| nChar==VK_RETURN)
	{
		OnClose();
	}
}

BOOL CMainDlg::OnCopyData(HWND wnd, PCOPYDATASTRUCT pCopyDataStruct)
{
	if(pCopyDataStruct->dwData == kcds_id)
	{
		m_utf8Buffer=SStringA((const char*)pCopyDataStruct->lpData,pCopyDataStruct->cbData);
		OnDestroy();
		CREATESTRUCT cs;
		cs.cx = 0;
		cs.cy = 0;
		OnCreate(&cs);
		//todo: keep tab visible pages same as before.
	}
	return TRUE;
}

int CMainDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	int nRet = SHostWnd::OnCreate(lpCreateStruct);
	if(nRet==0)
	{
		const WCHAR *pszXml=L"<window float=\"1\" msgTransparent=\"1\" margin=\"1,1,1,1\" colorBorder=\"#ff0000\"/>";
		m_pHover = CreateChildren(pszXml);
	}
	return nRet;
}
