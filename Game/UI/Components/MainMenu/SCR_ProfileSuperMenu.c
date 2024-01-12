enum SCR_EProfileSuperMenuTabId
{
	NEWS = 0,
	COMMUNITY
};

//------------------------------------------------------------------------------------------------
class SCR_ProfileSuperMenu : SCR_SuperMenuBase
{
	protected static SCR_EProfileSuperMenuTabId m_eCurrentPage;

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		SCR_InputButtonComponent comp = SCR_InputButtonComponent.GetInputButtonComponent("Back", GetRootWidget());
		if (comp)
			comp.m_OnActivated.Insert(OnBack);

		SCR_InputButtonComponent tos = SCR_InputButtonComponent.GetInputButtonComponent("ToS", GetRootWidget());
		if (tos)
			tos.m_OnActivated.Insert(OnTos);

		Widget logoWidget = GetRootWidget().FindAnyWidget("LogoButton");
		if (logoWidget)
		{
			SCR_ModularButtonComponent logo = SCR_ModularButtonComponent.FindComponent(logoWidget);
			if (logo)
				logo.m_OnClicked.Insert(OnLogoClicked);
		}

		if (m_TabViewComponent)
			m_TabViewComponent.m_OnChanged.Insert(OnTabChanged);
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(SCR_TabViewComponent comp, Widget w)
	{
		super.OnTabCreate(comp, w);
		
		SCR_NewsSubMenu newsSubMenu = SCR_NewsSubMenu.Cast(w.FindHandler(SCR_NewsSubMenu));
		if (newsSubMenu)
			newsSubMenu.GetRequestCommunityPage().Insert(OnRequestCommunityPage);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBack()
	{
		Close();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTos()
	{
		GetGame().GetPlatformService().OpenBrowser(GetGame().GetBackendApi().GetLinkItem("Link_PrivacyPolicy"));
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLogoClicked()
	{
		GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.WelcomeDialog);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnRequestCommunityPage()
	{
		SetPage(SCR_EProfileSuperMenuTabId.COMMUNITY);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnTabChanged(SCR_TabViewComponent tabView, Widget widget, int index)
	{
		m_eCurrentPage = index;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPage(SCR_EProfileSuperMenuTabId page)
	{
		if (!m_TabViewComponent)
			return;

		m_TabViewComponent.ShowTab(page);
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_EProfileSuperMenuTabId GetCurrentPage()
	{
		return m_eCurrentPage;
	}
}
