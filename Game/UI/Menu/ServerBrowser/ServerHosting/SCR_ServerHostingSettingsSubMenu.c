/*!
Wrapper sub menu clas for server hosting setting tab
*/
class SCR_ServerHostingSettingsSubMenu : SCR_SubMenuBase
{
	protected SCR_InputButtonComponent m_NavHost;
	protected SCR_InputButtonComponent m_NavSave;
	
	protected bool m_bIsListeningForCommStatus;

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);

		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Remove(OnCommStatusCheckFinished);
		m_bIsListeningForCommStatus = false;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuShow(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuShow(parentMenu);
		
		SCR_ServicesStatusHelper.RefreshPing();
		
		if (!m_bIsListeningForCommStatus)
			SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Insert(OnCommStatusCheckFinished);
		
		m_bIsListeningForCommStatus = true;
		UpdateHostButton();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuHide(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuHide(parentMenu);
		
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Remove(OnCommStatusCheckFinished);
		m_bIsListeningForCommStatus = false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateHostButton()
	{
		if (!m_NavHost)
			return;
		
		SCR_ServicesStatusHelper.SetConnectionButtonEnabled(m_NavHost, SCR_ServicesStatusHelper.SERVICE_BI_BACKEND_MULTIPLAYER);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCommStatusCheckFinished(SCR_ECommStatus status, float responseTime, float lastSuccessTime, float lastFailTime)
	{
		UpdateHostButton();
	}
	
	//------------------------------------------------------------------------------------------------
	void Init(notnull SCR_SuperMenuComponent superMenu)
	{
		SCR_SubMenuBase subMenu = SCR_SubMenuBase.Cast(GetRootWidget().FindHandler(SCR_SubMenuBase));
		if (!subMenu)
			return;
		
		subMenu.SetParentMenuComponent(superMenu);
		
		m_NavHost = CreateNavigationButton("MenuSelectHold", "#AR-ServerHosting_HostLocally", true);
		UpdateHostButton();
		
		// Tempory disabled saving
		if (!GetGame().IsPlatformGameConsole())
			m_NavSave = CreateNavigationButton("MenuSave", "#AR-PauseMenu_Save", true);
		
		// Hide 
		ShowNavigationButtons(m_bShown);
	}
	
	//-------------------------------------------------------------------------------------------
	// API 
	//-------------------------------------------------------------------------------------------
	
	//-------------------------------------------------------------------------------------------
	SCR_InputButtonComponent GetNavHostButton()
	{
		return m_NavHost;
	}
	
	//-------------------------------------------------------------------------------------------
	SCR_InputButtonComponent GetNavSaveButton()
	{
		return m_NavSave;
	}
}
