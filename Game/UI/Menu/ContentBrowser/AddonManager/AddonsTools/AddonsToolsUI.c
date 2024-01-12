/*!
Dialog containing all addons manager tools:
- Mod presets, Export JSON, Export CLI 

Might get extended with other mod related tools
*/

//------------------------------------------------------------------------------------------------
class AddonsToolsUI : SCR_TabDialog
{
	protected SCR_InputButtonComponent m_NavOpenWorkshop;

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		// Setup workshop button 
		Widget wsBtn = GetRootWidget().FindAnyWidget("NavOpenWorkshop");
		
		if (wsBtn)
			m_NavOpenWorkshop = SCR_InputButtonComponent.Cast(wsBtn.FindHandler(SCR_InputButtonComponent));
		
		if (m_NavOpenWorkshop)
		{
			m_NavOpenWorkshop.SetVisible(!IsAddonManagerOpened());
			m_NavOpenWorkshop.m_OnActivated.Insert(OnOpenWorkshopButton);
		}
		
		// Mod enabled callback
		EnableExportTabs();
		SCR_AddonManager.GetInstance().m_OnAddonsEnabledChanged.Insert(EnableExportTabs);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		super.OnMenuClose();
		
		SCR_AddonManager.GetInstance().m_OnAddonsEnabledChanged.Remove(EnableExportTabs);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);
		
		GetGame().GetInputManager().ActivateContext("InteractableDialogContext");
	}	
	
	//------------------------------------------------------------------------------------------------	
	//! Return true if workshop is opened on tab "Mod manager"
	protected bool IsAddonManagerOpened()
	{
		// Workshop opened
		ContentBrowserUI workshopUI = ContentBrowserUI.Cast(GetGame().GetMenuManager().GetTopMenu());
		
		if (!workshopUI)
			return false;
		
		return SCR_WorkshopListAddonsSubmenu.Cast(workshopUI.GetOpenedSubMenu());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnOpenWorkshopButton()
	{	
		SCR_MenuToolsLib.GetEventOnAllMenuClosed().Insert(AllMenuClosed);
		SCR_MenuToolsLib.CloseAllMenus( {MainMenuUI, ContentBrowserUI} );
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnCancel()
	{
		if (!GetGame().GetInputManager().IsContextActive("MenuTextEditContext"))
			super.OnCancel();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Callback for when all additional menu are closed and top menu is main menu
	protected void AllMenuClosed()
	{
		SCR_MenuToolsLib.GetEventOnAllMenuClosed().Remove(AllMenuClosed);
		Close();
		
		ContentBrowserUI workshopUI = ContentBrowserUI.Cast(GetGame().GetMenuManager().GetTopMenu());
		if (!workshopUI)
			workshopUI = ContentBrowserUI.Cast(GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.ContentBrowser));
		
		// Open tab
		if (workshopUI)
			workshopUI.OpenSubMenu(2);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Enable export tabs only if player has enabled mods
	protected void EnableExportTabs()
	{
		SCR_TabViewComponent tabView = m_SuperMenu.GetTabView();
		if (!tabView)
			return;
		
		bool enable = (SCR_AddonManager.GetInstance().CountOfEnabledAddons() > 0);
		
		tabView.EnableTab(1, enable);
		tabView.EnableTab(2, enable);
	}
};
