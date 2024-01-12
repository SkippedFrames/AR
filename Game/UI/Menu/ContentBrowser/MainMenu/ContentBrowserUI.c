enum EWorkshopTabId
{
	ONLINE = 0,
	OFFLINE
};

//------------------------------------------------------------------------------------------------
//! The super menu class for WORKSHOP content browser
class ContentBrowserUI : SCR_SuperMenuBase
{
	static const string SERVICE_NAME_WS = "reforger-workshop-api";
	static const string SERVER_STATE_OK = "ok";
	
	protected Widget m_wRoot;
	
	protected SCR_InputButtonComponent m_NavBack;
	
	float m_fTimerLowFreqUpdate;
	
	protected SCR_ContentBrowser_AddonsSubMenu m_OnlineSubMenu;
	protected SCR_ContentBrowser_AddonsSubMenu m_OfflineSubMenu;
	
	
	//------------------------------------------------------------------------------------------------
	static ContentBrowserUI Create(EWorkshopTabId defaultTab = EWorkshopTabId.ONLINE)
	{
		MenuBase menuBase = GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.ContentBrowser);
		ContentBrowserUI browser = ContentBrowserUI.Cast(menuBase);
		
		if (defaultTab != EWorkshopTabId.ONLINE)
		{
			browser.m_TabViewComponent.ShowTab(defaultTab);
		}
		
		return browser;
	}
	
	
	//------------------------------------------------------------------------------------------------
	SCR_ContentBrowser_AddonsSubMenu GetOnlineSubMenu() { return m_OnlineSubMenu; }

	
	//------------------------------------------------------------------------------------------------
	SCR_ContentBrowser_AddonsSubMenu GetOfflineSubMenu() { return m_OfflineSubMenu; }
	
	
	//------------------------------------------------------------------------------------------------
	static void _print(string str, LogLevel logLevel = LogLevel.DEBUG)
	{
		Print(string.Format("[Content Browser] %1", str), logLevel);
	}
	
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		m_wRoot = GetRootWidget();
		
		// Setup the 'back' nav button
		m_NavBack = SCR_InputButtonComponent.GetInputButtonComponent("Back", m_wRoot);
		m_NavBack.m_OnActivated.Insert(OnNavButtonClose);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		super.OnMenuClose();
		
		// Restore server hosting 
		if (!GetGame().IsPlatformGameConsole())
		{		
			if (ServerHostingUI.GetTemporaryConfig())
			{
				GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.ServerBrowserMenu);
				GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.ServerHostingDialog);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);
		
		m_fTimerLowFreqUpdate += tDelta;
		if (m_fTimerLowFreqUpdate > 1)
		{
			LowFreqUpdate();
			m_fTimerLowFreqUpdate -= 1;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update non important UI elements
	protected void LowFreqUpdate()
	{
		SCR_AddonManager mgr = SCR_AddonManager.GetInstance();
		
		if(!mgr)
			return;
		
		// Warning sign
		// Show warning icon if there is any offline mod which has any issue
		array<ref SCR_WorkshopItem> allAddons = mgr.GetAllAddons();
		array<ref SCR_WorkshopItem> offlineAddons = SCR_AddonManager.SelectItemsBasic(allAddons, EWorkshopItemQuery.OFFLINE);
		
		bool anyIssue = false;
		
		foreach (SCR_WorkshopItem item : offlineAddons)
		{
			bool dependenciesMissing = item.GetAnyDependencyMissing();
			bool dependenciesOutdated = item.GetAnyDependencyUpdateAvailable();
			bool dependenciesDisabled = item.GetEnabledAndAnyDependencyDisabled();
			bool updateAvailable = item.GetUpdateAvailable();
			
			if (dependenciesMissing || dependenciesOutdated || dependenciesDisabled || updateAvailable)
			{
				anyIssue = true;
				break;
			}
		}
		
		m_TabViewComponent.ShowIcon(EWorkshopTabId.OFFLINE, anyIssue);		
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(SCR_TabViewComponent comp, Widget w)
	{
		super.OnTabCreate(comp, w);
		
		SCR_ContentBrowser_AddonsSubMenu subMenu = SCR_ContentBrowser_AddonsSubMenu.Cast(w.FindHandler(SCR_ContentBrowser_AddonsSubMenu));
		
		if (!subMenu)
			return;
		
		if (subMenu.m_eMode == EContentBrowserAddonsSubMenuMode.MODE_ONLINE)
		{
			m_OnlineSubMenu = subMenu;
			
			if (m_OnlineSubMenu)
				m_OnlineSubMenu.GetOnRequestOpenOfflinePage().Insert(OpenOfflineTab);
		}
		
		else if (subMenu.m_eMode == EContentBrowserAddonsSubMenuMode.MODE_OFFLINE)
		{
			m_OfflineSubMenu = subMenu;
		}
	}
	
		
	// Callbacks of common buttons
	
	//-----------------------------------------------------------------------------------------------------------------
	void OnNavButtonClose()
	{
		this.Close();
	}
	
	//-----------------------------------------------------------------------------------------------
	//! Downloads above this size should cause a confirmation popup.
	static float GetSmallDownloadThreshold()
	{
		return 50*1024*1024; // 50 Megabytes
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OpenOfflineTab()
	{
		if (m_TabViewComponent)
			m_TabViewComponent.ShowTab(EWorkshopTabId.OFFLINE);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool IsBackenRunning()
	{
		return GetWorkshopStatus() == SERVER_STATE_OK;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static string GetWorkshopStatus()
	{
		// Find WS status item
		ServiceStatusItem wsStatus = FindStatusItemByName(SERVICE_NAME_WS);

		if (!wsStatus)
			return string.Empty;
		
		return wsStatus.Status();
	}
	
	//------------------------------------------------------------------------------------------------
	// TODO: Move into generic backend script api
	protected static ServiceStatusItem FindStatusItemByName(string name)
	{
		BackendApi backend = GetGame().GetBackendApi();
		int statusesCount = backend.GetStatusCount();
		
		for (int i = 0; i < statusesCount; i++)
		{
			if (backend.GetStatusItem(i).Name() == name)
			{
				return backend.GetStatusItem(i);
			}
		}
		
		return null;
	}
	
};
