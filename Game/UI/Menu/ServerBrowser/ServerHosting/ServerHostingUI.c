//------------------------------------------------------------------------------------------------
class ServerHostingUI : SCR_TabDialog
{
	protected static string DIALOG_TAG_SAVE_CONFIRM = 	"save_confirm";
	protected static string DIALOG_TAG_SAVE_SUCCESS = 	"save_successful";
	protected static string DIALOG_TAG_SAVE_FAIL = 		"save_failed";
	protected static string DIALOG_TAG_SAVE_OVERRIDE = 	"save_override";
	protected static string DIALOG_TAG_NO_CONNECTION = 	"no_connection";
	
	protected static string JSON_POSTFIX = ".json";
	
	protected static string DIALOG_OVERRIDE =	"#AR-ServerHosting_OverrideWarning";
	protected static string DIALOG_SAVED = 		"#AR-ServerHosting_SaveSuccessful";
	
	protected const string COLOR_TAG = 		"<color rgba='%1'>";
	protected const string COLOR_TAG_END = 	"</color>";
	protected const Color SERVER_CONFIG_NAME_COLOR = UIColors.CONTRAST_COLOR;
	
	protected ref SCR_SuperMenuComponent m_SuperMenuComponent;
	
	protected ref SCR_DSConfig m_DSConfig = new SCR_DSConfig();
	protected static ref SCR_DSConfig m_TemporaryConfig;
	
	// Config widget wrapper components 
	protected ref SCR_ServerConfigListComponent m_ConfigList;
	protected ref SCR_ServerHostingSettingsSubMenu m_ConfigListSubMenu;
	
	protected ref SCR_ServerHostingModSubMenu m_ConfigMods;
	
	protected ref SCR_ServerConfigAdvancedComponent m_AdvancedSettings;
	protected ref SCR_ServerHostingSettingsSubMenu m_AdvancedSubMenu;
	
	// Values 
	protected string m_iUnifiedPort; // Unified port used acress sub menu to define port from basic in advanced settings and vice versa
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		m_SuperMenuComponent = SCR_SuperMenuComponent.Cast(GetRootWidget().FindHandler(SCR_SuperMenuComponent));
		
		// Server setting list 
		m_ConfigList = SCR_ServerConfigListComponent.Cast(
			m_SuperMenu.GetTabView().GetEntryContent(0).m_wTab.FindHandler(SCR_ServerConfigListComponent)
		);
		
		m_ConfigListSubMenu = SCR_ServerHostingSettingsSubMenu.Cast(
			m_SuperMenu.GetTabView().GetEntryContent(0).m_wTab.FindHandler(SCR_ServerHostingSettingsSubMenu)
		);
		
		m_ConfigListSubMenu.Init(m_SuperMenuComponent);
		
		// Mods 
		m_ConfigMods = SCR_ServerHostingModSubMenu.Cast(
			m_SuperMenu.GetTabView().GetEntryContent(1).m_wTab.FindHandler(SCR_ServerHostingModSubMenu)
		);
		
 		m_ConfigMods.GetEventOnWorkshopButtonActivate().Insert(OnWorkshopOpenActivate);
		
		
		// Advanced settings 
		m_AdvancedSettings =  SCR_ServerConfigAdvancedComponent.Cast(
			m_SuperMenu.GetTabView().GetEntryContent(2).m_wTab.FindHandler(SCR_ServerConfigAdvancedComponent)
		);
		
		m_AdvancedSubMenu = SCR_ServerHostingSettingsSubMenu.Cast(
			m_SuperMenu.GetTabView().GetEntryContent(2).m_wTab.FindHandler(SCR_ServerHostingSettingsSubMenu)
		);
		
		m_AdvancedSubMenu.Init(m_SuperMenuComponent);
		
		// Default IP and port
		GetGame().GetBackendApi().SetDefaultIpPort(m_DSConfig);
		m_AdvancedSettings.SetIPPort(m_DSConfig);
		
		// Setup buttons
		m_ConfigListSubMenu.GetNavHostButton().m_OnActivated.Insert(OnHostServerClick);
		
		if (m_ConfigListSubMenu.GetNavSaveButton())
			m_ConfigListSubMenu.GetNavSaveButton().m_OnActivated.Insert(OnSaveTemplateClick);
		
		m_AdvancedSubMenu.GetNavHostButton().m_OnActivated.Insert(OnHostServerClick);
		
		if (m_AdvancedSubMenu.GetNavSaveButton())
			m_AdvancedSubMenu.GetNavSaveButton().m_OnActivated.Insert(OnSaveTemplateClick);
		
		// Restore config 
		if (m_TemporaryConfig)
		{
			m_ConfigList.FillFromDSConfig(m_TemporaryConfig);
			m_AdvancedSettings.FillFromDSConfig(m_TemporaryConfig);
			m_ConfigMods.EnableModsFromDSConfig(m_TemporaryConfig);
			m_TemporaryConfig = null;
		}
		
		// Setup menu listeners 
		m_ConfigList.GetOnPortChanged().Insert(OnSubMenuChangePort);
		m_AdvancedSettings.GetOnPortChanged().Insert(OnSubMenuChangePort);
		
		// Call check fadein when fadein animation is expected to be done to verify dialog is fully visible
		GetGame().GetCallqueue().CallLater(FinishFadeIn, 1000/m_fAnimationRate*2);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);
		
		GetGame().GetInputManager().ActivateContext("InteractableDialogContext");
	}
	
	//------------------------------------------------------------------------------------------------
	//! Hotfix for when fadein is not finished 
	protected void FinishFadeIn()
	{
		GetRootWidget().SetOpacity(1);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Verify and store all info into config - return true if all config properties are valid
	protected bool VerifyAndStoreConfig()
	{
		if (!m_DSConfig || !m_ConfigList || !m_ConfigMods || !m_AdvancedSettings)
		{
			#ifdef SB_DEBUG 
			Print("Missing config references");
			#endif
			
			return false;
		}
		
		// Check all values validity 
		Widget invalidEntry = m_ConfigList.GetInvalidEntry();
		int invalidTab = 0;
		
		if (!invalidEntry)
		{
			invalidEntry = m_AdvancedSettings.GetInvalidEntry();
			invalidTab = 2;
		}
		
		if (invalidEntry)
		{
			// Show problematic tab
			if (m_SuperMenu.GetTabView().m_iSelectedTab != invalidTab)
				m_SuperMenu.GetTabView().ShowTab(invalidTab);
			
			// Move to invalid entry
			GetGame().GetWorkspace().SetFocusedWidget(invalidEntry);
			m_ConfigList.ScrollToTheEntry(invalidEntry);
			
			return false;
		}
		
		// Store
		array<ref SCR_WidgetListEntry> properties = m_ConfigList.GetInitialEntryList();
		array<ref SCR_WidgetListEntry> advanced = m_AdvancedSettings.GetInitialEntryList();
		for (int i = 0, count = advanced.Count(); i < count; i++)
		{
			properties.Insert(advanced[i]);
		}
		
		array<ref DSMod> mods = m_ConfigMods.SelectedModsList();
		WorkshopItem scenarioMod = m_ConfigList.GetScenarioOwnerMod();
		
		m_DSConfig.StoreFullJson(properties, mods, scenarioMod);
		return true;
	}
	
	//-------------------------------------------------------------------------------------------
	protected void OnOverrideConfirm(SCR_ConfigurableDialogUi dialog)
	{
		dialog.m_OnConfirm.Remove(OnOverrideConfirm);
		SaveConfig();
	}
	
	
	//-------------------------------------------------------------------------------------------
	// Callbacks 
	//-------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	protected void OnHostServerClick()
	{
 		if (!m_DSConfig || !m_ConfigList)
			return;
		
		// Check properties 
		if (!VerifyAndStoreConfig())
			return;
		
		// Check connection - prevent host 
		if (!GetGame().GetBackendApi().IsActive() || !SCR_ServicesStatusHelper.AreMultiplayerServicesAvailable())
		{
			SCR_ConfigurableDialogUi.CreateFromPreset(m_ConfigList.GetDialogs(), DIALOG_TAG_NO_CONNECTION);
			return;
		}
		
		// Host scenario 
		protected ref MissionWorkshopItem hostedScenario = m_ConfigList.GetSelectedScenario();
		
		if (hostedScenario)
			hostedScenario.Host(m_DSConfig);
		
		GameSessionStorage.s_Data["m_iRejoinAttempt"] = "0";
		
		// Save current menu 
		MenuBase lastMenu = GetGame().GetMenuManager().GetTopMenu();
		
		if (ServerBrowserMenuUI.Cast(lastMenu))
			SCR_MenuLoadingComponent.SaveLastMenu(ChimeraMenuPreset.ServerBrowserMenu);
		else if (SCR_ScenarioMenu.Cast(lastMenu))
			SCR_MenuLoadingComponent.SaveLastMenu(ChimeraMenuPreset.ScenarioMenu);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSaveTemplateClick()
	{
		if (!VerifyAndStoreConfig())
			return;
		
		DisplaySaveDialog();
	}
	
	protected string m_sFileName;
	
	//------------------------------------------------------------------------------------------------
	//! Show dialog with file name settings and confirmation
	protected void DisplaySaveDialog()
	{
		SCR_ServerConfigSaveDialog saveDialog = new SCR_ServerConfigSaveDialog();
		
		SCR_ConfigurableDialogUi dialog = SCR_ConfigurableDialogUi.CreateFromPreset(m_ConfigList.GetDialogs(), DIALOG_TAG_SAVE_CONFIRM, saveDialog);
		dialog.m_OnConfirm.Insert(OnSaveDialogConfirm);
		
		// Generate 
		string name = m_ConfigList.GenerateFileName(m_ConfigList.GetSelectedScenario());
		saveDialog.SetFileNameText(name);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSaveDialogConfirm(SCR_ConfigurableDialogUi dialog)
	{
		SCR_ServerConfigSaveDialog saveDialog = SCR_ServerConfigSaveDialog.Cast(dialog);
		m_sFileName = saveDialog.GetFileNameText();
		
		// Check if name is in config 
		array<string> configs = {};
		GetGame().GetBackendApi().GetAvailableConfigs(configs);
		string name = m_sFileName;
		
		// Go though locally saved configs 
		for (int i = 0, count = configs.Count(); i < count; i++)
		{
			if (name + JSON_POSTFIX  == configs[i])
			{
				// Display config exists
				SCR_ConfigurableDialogUi overrideDialog = SCR_ConfigurableDialogUi.CreateFromPreset(m_ConfigList.GetDialogs(), DIALOG_TAG_SAVE_OVERRIDE);
				
				string color = string.Format(COLOR_TAG, UIColors.SRGBAFloatToInt(SERVER_CONFIG_NAME_COLOR));
				string msg = WidgetManager.Translate(DIALOG_OVERRIDE, name, color, COLOR_TAG_END);
				overrideDialog.SetMessage(msg);
				overrideDialog.m_OnConfirm.Insert(OnOverrideConfirm);
				
				return;
			}
		}
		
		// Save
		SaveConfig();
	}
	
	//------------------------------------------------------------------------------------------------
	void SaveConfig()
	{
		string configName = m_sFileName + JSON_POSTFIX;
		bool saved = GetGame().GetBackendApi().SaveDSConfig(m_DSConfig, configName);
		
		if (saved)
		{
			// Show success dialog
			SCR_ConfigurableDialogUi dialog = SCR_ConfigurableDialogUi.CreateFromPreset(m_ConfigList.GetDialogs(), DIALOG_TAG_SAVE_SUCCESS);
			
			array<string> paths = {};
			GetGame().GetBackendApi().GetAvailableConfigPaths(paths);
			
			// Find right path and show message about file path
			array<string> configs = {};
			GetGame().GetBackendApi().GetAvailableConfigs(configs);
			string name = m_sFileName;
			
			for (int i = 0, count = configs.Count(); i < count; i++)
			{
				if (name + JSON_POSTFIX == configs[i])
				{
					string color = string.Format(COLOR_TAG, UIColors.SRGBAFloatToInt(SERVER_CONFIG_NAME_COLOR));
					string msg = WidgetManager.Translate(DIALOG_SAVED, paths[i], color, COLOR_TAG_END);
					
					if (dialog)
						dialog.SetMessage(msg);		
					
					break;
				}
			}
			
		}
		else
		{
			// Show fail dialog
			SCR_ConfigurableDialogUi dialog = SCR_ConfigurableDialogUi.CreateFromPreset(m_ConfigList.GetDialogs(), DIALOG_TAG_SAVE_FAIL);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnWorkshopOpenActivate()
	{
		SCR_MenuToolsLib.GetEventOnAllMenuClosed().Insert(AllMenuClosed);
		SCR_MenuToolsLib.CloseAllMenus( {MainMenuUI, ContentBrowserUI} );
	}
	
	//------------------------------------------------------------------------------------------------
	//! Callback for when all additional menu are closed and top menu is main menu
	protected void AllMenuClosed()
	{
		SCR_MenuToolsLib.GetEventOnAllMenuClosed().Remove(AllMenuClosed);
		
		// Store and close
		array<ref SCR_WidgetListEntry> properties = m_ConfigList.GetInitialEntryList();
		array<ref SCR_WidgetListEntry> advanced = m_AdvancedSettings.GetInitialEntryList();
		for (int i = 0, count = advanced.Count(); i < count; i++)
			properties.Insert(advanced[i]);
		
		array<ref DSMod> mods = m_ConfigMods.SelectedModsList();
		WorkshopItem scenarioMod = m_ConfigList.GetScenarioOwnerMod();
		
		m_DSConfig.StoreFullJson(properties, mods, scenarioMod); 
		m_TemporaryConfig = m_DSConfig;
		
		Close();
		
		// Open workshop
		ContentBrowserUI workshopUI = ContentBrowserUI.Cast(GetGame().GetMenuManager().GetTopMenu());
		if (!workshopUI)
			workshopUI = ContentBrowserUI.Cast(GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.ContentBrowser));
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call when sub menu change port setting to propagete unified port on multiple places
	protected void OnSubMenuChangePort(string port)
	{
		m_iUnifiedPort = port;
		
		m_ConfigList.SetPort(m_iUnifiedPort);
		m_AdvancedSettings.SetPorts(m_iUnifiedPort);
	}
	
	//------------------------------------------------------------------------------------------------
	// API
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Select scenario in scenario selection dropdown
	void SelectScenario(notnull MissionWorkshopItem scenario)
	{
		if (!m_ConfigList)
			return;
		
		m_ConfigList.SelectScenario(scenario);
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_DSConfig GetTemporaryConfig()
	{
		return m_TemporaryConfig;
	}
};
