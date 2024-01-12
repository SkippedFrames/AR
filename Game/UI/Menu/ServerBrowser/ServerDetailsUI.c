//! Class for server details menu screen 
//! Server details should contain more complex data to server setup, description and mods 
//! Server details should also provide more hadnling option for server and its mods 

//-----------------------------------------------------------------------------------------------
enum EServerConnectingState
{
	LOADING_INFO = 0,
	MISSING_MODS = 1,
	DOWNLOADING_MODS = 2,
	CAN_JOIN = 3,
	
	CONNECT = 10,
	ACTIVATE_MODS = 11,
};

//-----------------------------------------------------------------------------------------------
class ServerDetailsMenuUI: ChimeraMenuBase
{	
	// Widget names 
	const string WIDGET_TEXT_SERVER_NAME = "Title";
	
	const string WIDGET_TAB_VIEW = "TabViewRoot0";
	const string WIDGET_BUTTON_BACK = "Back";
	const string WIDGET_BUTTON_MODS_HANDLING = "BtnModsHandling";
	const string WIDGET_BUTTON_JOIN = "BtnJoin";
	const string WIDGET_BUTTON_FAVORITE = "BtnFavorite";
	const string WIDGET_BUTTON_MOD_DETAILS = "BtnModDetails";
	const string ACTION_BACK = "MenuBack"; 
	
	const string LOADING_MODS = "LoadingMods";
	
	const string WIDGET_MODS_ISSUE = "vModsIssue";
	const string WIDGET_MODS_UPDATED = "vModsUpdated";
	
	const string WIDGET_DESCRIPTION = "vDescription";
	const string WIDGET_IMAGE_SCENARIO = "ImgScenario";
	const string WIDGET_TEXT_SERVER_DESC = "TxtServerDescription";
	
	const string WIDGET_DETAILS_PANEL_MODS = "DetailsPanelMods";
	
	const string ENTRY_NAME = "txtName";
	const string ENTRY_STATE = "imgState";
	
	const string IMAGE_SET_ICONS = "{2EFEA2AF1F38E7F0}UI/Textures/Icons/icons_wrapperUI-64.imageset";
	//const string LAYOUT_MOD_ENTRY = "{EB6E87758B1E2FFC}UI/layouts/Menus/ServerBrowser/ServerModEntry.layout"; 
	const string LAYOUT_MOD_ENTRY = "{D077825E029C5889}UI/layouts/Menus/ServerBrowser/ContentEntry.layout";
	
	const string IMAGE_NAME_AVAILABLE = "check";
	const string IMAGE_NAME_UPDATE = "report";
	
	// Strings 
	const string STR_JOIN = "#AR-Workshop_ButtonJoin";
	const string STR_DOWNLOAD = "#AR-Workshop_ButtonDownload";
	
	const string ACTION_JOIN = "#AR-Workshop_ButtonJoin"; 
	const string ACTION_DONWLOAD = "#AR-Workshop_ButtonDownload";
	
	// Details widgets names // const string DETAIL_ = "";
	const string DETAIL_SCENARIO = "detailScenario";
	const string DETAIL_PLAYERS = "detailPlayers";
	const string DETAIL_PLATFORM = "detailPlatform";
	const string DETAIL_OPEN = "detailPassword";
	const string DETAIL_PING = "detailPing";
	
	// State 
	protected EServerConnectingState m_iHandlingState = 0;
	
	// api
	protected ClientLobbyApi m_Lobby;
	protected WorkshopApi m_WorkshopApi;
	
	// Server borwser & server reference
	protected ServerBrowserMenuUI m_ServerBrowserMenu;
	protected SCR_RoomModsManager m_ModsManager;
	protected Room m_Room;
	
	// Mods info
	protected MissionWorkshopItem m_Scenario;
	protected ref array<Widget> m_aModsWidgets = new  ref array<Widget>;
	protected ref array<SCR_ModEntryComponent> m_aModEntries = new ref array<SCR_ModEntryComponent>;

	// Focused mod
	protected SCR_ModEntryComponent m_FocusedModEntry;
	
	// Widgets 
	protected TextWidget m_wTxtServerName;
	protected Widget m_wModsListIssues;
	protected Widget m_wModsListUpdated;
	
	protected Widget m_wDescription;
	protected Widget m_wDescriptionMods;
	protected ImageWidget m_wImgScenario;
	protected TextWidget m_wTxtServerDescription;
	
	protected SCR_TabViewComponent m_TabView;
	
	// Details panels 
	//protected SCR_DetailsPanelContentComponent m_DetailPanelMods;
	
	// Buttons 
	protected SCR_InputButtonComponent m_ButtonBack;
	protected SCR_InputButtonComponent m_NavHandleButton;
	protected SCR_InputButtonComponent m_NavJoin;
	protected SCR_InputButtonComponent m_NavFavorite;
	protected SCR_InputButtonComponent m_NavModDetails;
	protected SCR_MultipleStatesButtonComponent m_BtnModsHandling;
	
	// Details widgets 
	protected SCR_ServerDetailEntryComponent m_DetailScenario;
	protected SCR_ServerDetailEntryComponent m_DetailPlayers;
	protected SCR_ServerDetailEntryComponent m_DetailPlatforms;
	protected SCR_ServerDetailEntryComponent m_DetailOpen;
	protected SCR_ServerDetailEntryComponent m_DetailPing;
	
	protected Widget m_wLoadingMods;
	
	// Invokers 
	ref ScriptInvoker m_OnOpen = new ref ScriptInvoker();
	ref ScriptInvoker m_OnModEntryFocus = new ref ScriptInvoker();
	
	// Mod content states   
	protected bool m_bIsModed = false;
	protected bool m_bMenuOpenChecked = false;
	protected bool m_bIsContentUpdated = false;
	protected int m_iDownloadedModsCout = 0;
	
	//------------------------------------------------------------------------------------------------
	override protected void OnMenuOpen()
	{
		// Base references 
		m_WorkshopApi = GetGame().GetBackendApi().GetWorkshop(); 
		m_Lobby = GetGame().GetBackendApi().GetClientLobby();
		
		// Texts setups 
		m_wTxtServerName = TextWidget.Cast(GetRootWidget().FindAnyWidget(WIDGET_TEXT_SERVER_NAME));
		
		m_wModsListIssues = GetRootWidget().FindAnyWidget(WIDGET_MODS_ISSUE);
		m_wModsListUpdated = GetRootWidget().FindAnyWidget(WIDGET_MODS_UPDATED);
		
		m_wDescription = GetRootWidget().FindAnyWidget(WIDGET_DESCRIPTION);
		m_wImgScenario = ImageWidget.Cast(GetRootWidget().FindAnyWidget(WIDGET_IMAGE_SCENARIO));
		m_wTxtServerDescription = TextWidget.Cast(GetRootWidget().FindAnyWidget(WIDGET_TEXT_SERVER_DESC));
		
		// Details panels 
		Widget detailsPanelMods = GetRootWidget().FindAnyWidget(WIDGET_DETAILS_PANEL_MODS);
		/*if (detailsPanelMods)
			m_DetailPanelMods = SCR_DetailsPanelContentComponent.Cast(detailsPanelMods.FindHandler(SCR_DetailsPanelContentComponent));*/
		
		//m_wDescriptionMods = m_DetailPanelMods.m_wRoot.FindAnyWidget(WIDGET_DESCRIPTION);
		
		m_wLoadingMods = GetRootWidget().FindAnyWidget(LOADING_MODS);
		if (m_wLoadingMods)
			m_wLoadingMods.SetVisible(true);
		
		SetupButtons();
		SetupDetailsEntries();
		
		//m_OnOpen.Invoke(this, true, m_Room);
		m_bMenuOpenChecked = false;
		
		if (m_wDescription)
			m_wDescription.SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnMenuClose()
	{
		if (m_OnOpen && m_Room)
		{
			m_OnOpen.Invoke(this, false, m_Room);
		}
		
		if (m_ModsManager)
			m_ModsManager.GetOnGetAllDependencies().Remove(SetupRequiredMods);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnMenuUpdate(float tDelta)
	{
		// 
		if (!m_bMenuOpenChecked && m_OnOpen && m_Room)
		{
			m_OnOpen.Invoke(this, true, m_Room);
			m_bMenuOpenChecked = true;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Setup data 
	void SetServerDetail(ServerBrowserMenuUI serverBrowserMenu, SCR_RoomModsManager manager, Room room)
	{
		m_ServerBrowserMenu = serverBrowserMenu;
		m_ModsManager = manager;
		m_Room = room;
		
		if(!m_ServerBrowserMenu)
			return; 
		
		// Invokers 
		m_ServerBrowserMenu.m_OnScenarioLoad.Insert(OnScenario);
		
		//m_ModsManager.m_OnModDownload.Insert(OnModDonwload);
		
		// Tab view 
		Widget wTabView = GetRootWidget().FindAnyWidget(WIDGET_TAB_VIEW);
		if (wTabView)
			m_TabView = SCR_TabViewComponent.Cast(wTabView.FindHandler(SCR_TabViewComponent)); 
		
		// Show room na in header
		if (m_wTxtServerName)	
			m_wTxtServerName.SetText(m_Room.Name());
		
		// Details 
		UpdateDetailsEntries();
		
		// Load mods 
		if (m_Room.IsDownloadListLoaded())
			SetupRequiredMods();
		else
			m_ModsManager.GetOnGetAllDependencies().Insert(SetupRequiredMods); 
			
	}
	
	//------------------------------------------------------------------------------------------------
	//! Setup of all handling and navigation buttons of server details
	protected void SetupButtons()
	{
		InputManager inputManager = GetGame().GetInputManager();
		
		// Donwload handling button
		Widget wHandlerButton = GetRootWidget().FindAnyWidget(WIDGET_BUTTON_MODS_HANDLING);
		if (wHandlerButton)
		{
			//Find mods hadnling button
			m_BtnModsHandling = SCR_MultipleStatesButtonComponent.Cast(wHandlerButton.FindHandler(SCR_MultipleStatesButtonComponent));
			
			if (m_BtnModsHandling)
			{
				//  Handling button setup invoker
				m_BtnModsHandling.m_OnStateChange.Insert(OnStateChange);
				m_BtnModsHandling.m_OnClicked.Insert(ModsHandlingClick);
				
				// State 
				m_BtnModsHandling.ChangeState(EServerConnectingState.LOADING_INFO);
				ChangeState(EServerConnectingState.LOADING_INFO);
			}
		}
		
		// Navigation buttons 
		Widget wNavJoin = GetRootWidget().FindAnyWidget(WIDGET_BUTTON_JOIN);
		if (wNavJoin)
			m_NavJoin = SCR_InputButtonComponent.Cast(wNavJoin.FindHandler(SCR_InputButtonComponent));

		// SEtup buttons 
		NavButtonByName(m_NavFavorite, WIDGET_BUTTON_FAVORITE).m_OnActivated.Insert(OnActionfavorite);
		NavButtonByName(m_NavModDetails, WIDGET_BUTTON_MOD_DETAILS).m_OnActivated.Insert(OnActionModDetails);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Find nav button based by given name
	//! Save nav button in reference button
	protected SCR_InputButtonComponent NavButtonByName(out SCR_InputButtonComponent navButton, string widgetName)
	{		
		Widget wButton = GetRootWidget().FindAnyWidget(widgetName);
		if (!wButton)
			return null;
		
		navButton = SCR_InputButtonComponent.Cast(wButton.FindHandler(SCR_InputButtonComponent));
		
		return navButton;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Find details entries widgets 
	protected void SetupDetailsEntries()
	{
		m_DetailScenario = FindDetailsEntry(DETAIL_SCENARIO);
		
		m_DetailPlayers = FindDetailsEntry(DETAIL_PLAYERS);
		
		m_DetailPlatforms = FindDetailsEntry(DETAIL_PLATFORM);
		
		m_DetailOpen = FindDetailsEntry(DETAIL_OPEN);
		
		m_DetailPing = FindDetailsEntry(DETAIL_PING);
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_ServerDetailEntryComponent FindDetailsEntry(string widgetName)
	{
		SCR_ServerDetailEntryComponent entry = null;
		
		Widget root = GetRootWidget().FindAnyWidget(widgetName);
		
		if (!root)
			return entry;
		
		entry = SCR_ServerDetailEntryComponent.Cast(root.FindHandler(SCR_ServerDetailEntryComponent));
		return entry;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update information in details entries 
	protected void UpdateDetailsEntries()
	{
		if (!m_Room)
			return;
		
		// Dynamic data ----------------------
		
		// Players 
		if (m_DetailPlayers)
		{
			string playersCurrent = m_Room.PlayerCount().ToString();
			string playersMax =  m_Room.PlayerLimit().ToString();
			
			m_DetailPlayers.SetDataText(playersCurrent + "/" + playersMax);
		}
		
		// Ping 
		if (m_DetailPing)
		{
			m_DetailPing.SetDataText("Unknown");
		}
		
		// Static data ----------------------
		
		// Scenario 
		if (m_DetailScenario)
		{
			m_DetailScenario.SetDataText(m_Room.ScenarioName());
		}
		
		// Platform 
		if (m_DetailPlatforms)
		{
			string platform = "PC";
			string platformImg = "platform-windows";
			
			if (m_Room.IsCrossPlatform())
			{
				platform = "Cross-play";
				platformImg = "platform-crossplay";
			}
			else
			{
				// Pick plaform 
			}
			
			m_DetailPlatforms.SetDataText(platform);
			m_DetailPlatforms.SetIconFromImageSet(platformImg);
		}
		
		// Is open 
		if (m_DetailOpen)
		{
			string lockedImg = "server-unlocked";
			if (m_Room.PasswordProtected())
				lockedImg = "server-locked";
			
			m_DetailOpen.SetIconFromImageSet(lockedImg);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SwitchTab(int index)
	{
		if (!m_TabView)
			return;
		
		m_TabView.ShowTab(index);
	}
	
	//------------------------------------------------------------------------------------------------
	[MenuBindAttribute()]
	void OnActionBack()
	{
		Close();
	}
	
	//------------------------------------------------------------------------------------------------
	//! This action servers for getting player to server join 
	//! It also includes actions that leads to fixing issues to join like mods synchronization 
	void OnActionJoin()
	{		
		// Join 
		//m_ServerBrowserMenu.JoinRoom(m_Room);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnActionDownload(SCR_InputButtonComponent navComponent, string actionName)
	{
		// Mods not selected -> go to mods page  
		int modsTabId = 1;
		if (m_TabView.GetShownTab() != modsTabId)
		{
			m_TabView.ShowTab(modsTabId);
			return;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnActionfavorite()
	{
		bool isFavorite = m_Room.IsFavorite();
		m_Room.SetFavorite(!isFavorite, null);
		//m_ServerBrowserMenu.RoomToFavorite(m_Room, !isFavorite);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Open Content browser details page with info about selected mod 
	protected void OnActionModDetails()
	{
		if (!m_FocusedModEntry)
			return;
		
		Dependency dep = m_FocusedModEntry.GetDependency();

		if (!dep)
			return;
		
		SCR_WorkshopItem item = SCR_AddonManager.GetInstance().Register(dep);
		
		ContentBrowserDetailsMenu.OpenForWorkshopItem(item);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetupRequiredMods()
	{
		//m_bItemsChecked = true;
		array<ref SCR_WorkshopItem> updated = m_ModsManager.GetRoomItemsUpdated();
		array<ref SCR_WorkshopItem> outdated = m_ModsManager.GetRoomItemsToUpdate();
		
		if (m_wLoadingMods)
			m_wLoadingMods.SetVisible(false);
		
		// Skip if no modes are required 
		if (updated.IsEmpty() && outdated.IsEmpty())
		{
			// Set Mods 
			m_bIsModed = false;
			m_bIsContentUpdated = true;
			
			// Move to mods tab 
			if (m_TabView)
				m_TabView.EnableTab(1, m_bIsModed);

			// State 
			m_BtnModsHandling.ChangeState(EServerConnectingState.CAN_JOIN);
			ChangeState(EServerConnectingState.CAN_JOIN);
			
			return;
		}
		
		// Set Mods 
		m_bIsModed = true;
		if (m_TabView)
			m_TabView.EnableTab(1, m_bIsModed);
		
		// Check mods enabled if all are updated 
		if (!updated.IsEmpty() && outdated.IsEmpty())
		{
			CreateModsList();
			m_BtnModsHandling.ChangeState(EServerConnectingState.CAN_JOIN);
			ChangeState(EServerConnectingState.CAN_JOIN);
			
			m_bIsContentUpdated = true;
			
			return;
		}
				
		// Create list & finish
		m_BtnModsHandling.ChangeState(EServerConnectingState.MISSING_MODS);
		ChangeState(EServerConnectingState.MISSING_MODS);
		CreateModsList();
		
		m_bIsContentUpdated = false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Show every mod that needs to be downloaded or updated
	protected void CreateModsList()
	{
		// Create mods with issues 
		array<ref SCR_WorkshopItem> updated = m_ModsManager.GetRoomItemsUpdated();
		
		foreach (SCR_WorkshopItem item : updated)
		{
			Dependency dependency = item.GetDependency(); 
			CreateModEntry(dependency, "", false);
		}
		
		// Create available mods 
		array<ref SCR_WorkshopItem> outdated = m_ModsManager.GetRoomItemsToUpdate();
		
		foreach (SCR_WorkshopItem item : outdated)
		{
			Dependency dependency = item.GetDependency(); 
			CreateModEntry(dependency, "", true);
		}
		
		// Focus first 
		if (m_aModEntries.Count() > 0)
			GetGame().GetWorkspace().SetFocusedWidget(m_aModEntries[0].GetRootWidget());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateModEntry(Dependency dependency, string name, bool state)
	{	
		if (!dependency)
			return;
		
		if (!m_wModsListIssues || !m_wModsListUpdated)
			return;
		
		// Set spawn parent
		Widget parent = m_wModsListIssues;
		if (state)
			parent = m_wModsListUpdated;
		
		// Create
		Widget itemEntry = GetGame().GetWorkspace().CreateWidgets(LAYOUT_MOD_ENTRY, parent);
		if (!itemEntry)
			return;
		
		// Register 
		SCR_ModEntryComponent modEntry = SCR_ModEntryComponent.Cast(itemEntry.FindHandler(SCR_ModEntryComponent));
		if (modEntry)
		{
			string targetVersion = dependency.GetVersion();
			
			SCR_WorkshopItem item = SCR_AddonManager.GetInstance().Register(dependency);
			
			modEntry.SetModContent(item, targetVersion);
			
			int size = dependency.TotalFileSize();
			string unit = SCR_ByteFormat.ReadableSizeFromBytes(size);
			modEntry.SetModDataSize(size.ToString() + unit);
			
			if (state)
				modEntry.MarkAsUpdated();
			
			modEntry.m_OnFocus.Insert(OnModEntryFocus);
			modEntry.m_OnModDonwloaded.Insert(OnModDownloaded);
			m_aModEntries.Insert(modEntry);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnScenario(MissionWorkshopItem scenario)
	{
		// Show no info if there is no data 
		if (!scenario)
		{
			if (m_wTxtServerDescription)
				m_wTxtServerDescription.SetVisible(false);
			
			return;
		}
		
		// Display scenario data 
		if (m_wDescription)
			m_wDescription.SetVisible(true);
		
		if (m_wTxtServerDescription)
				m_wTxtServerDescription.SetVisible(true);

		// Image
		ImageScale img = scenario.Thumbnail().GetLocalScale(0);
		if (m_wImgScenario && img)
			m_wImgScenario.LoadImageTexture(0, img.Path());
		
		// Description 
		string sDescription = scenario.Description();
		if (m_wTxtServerDescription)
			m_wTxtServerDescription.SetText(sDescription);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this when dialog handling phase is switched 
	//! Change content of navigation button for handling - download, join, to fit current handling action 
	protected void OnStateChange(int state)
	{
		// Additional changes by state 
		switch (state)
		{
			// Loading info about server mods
			case EServerConnectingState.LOADING_INFO:
				m_BtnModsHandling.SetProgressText("");
			break;
			// Download
			case EServerConnectingState.MISSING_MODS:
			break; 
			
			// Downloading
			case EServerConnectingState.DOWNLOADING_MODS:
				m_BtnModsHandling.SetProgressText("");
			break; 
			
			// Connect
			case EServerConnectingState.CAN_JOIN:
				m_BtnModsHandling.SetProgressText("");
			break; 
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Switch between click behaviors of process button by process state 
	protected void ModsHandlingClick()
	{
		int state = m_BtnModsHandling.GetSelectedItem();
		
		switch (state)
		{
			// Download
			case EServerConnectingState.MISSING_MODS:
				//m_iDownloadedModsCout = 0;

				if (m_BtnModsHandling)
					m_BtnModsHandling.ChangeState(EServerConnectingState.DOWNLOADING_MODS);
			break; 
			
			// Connect
			case EServerConnectingState.CAN_JOIN:
				/*if (m_ServerBrowserMenu)
					m_ServerBrowserMenu.JoinRoom(m_Room);*/
			break; 
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this when downloading of mod is finished  
	protected void OnModDonwload(SCR_WorkshopItem item, int modsCount, int downloadedCount)
	{
		if (downloadedCount == modsCount)
			ChangeState(EServerConnectingState.CAN_JOIN);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set State of detilais panel 
	//! Panel can be - Fetching content data, Ready for download content, Ready to play 
	protected void ChangeState(EServerConnectingState state)
	{
		m_iHandlingState = state;
		
		switch (m_iHandlingState)
		{
			// Fetching content data 
			case EServerConnectingState.LOADING_INFO:
			StateFetchingData();
			break;
			
			//  Ready for download content: Mods fetching done, mods not matching -> can download mods 
			case EServerConnectingState.MISSING_MODS:
			StateMissingMods();
			break;
			
			// Ready to play: Mods matching -> can join
			case EServerConnectingState.CAN_JOIN:
			StateCanJoin();
			break;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Setup for state when server details is still waiting for mods data check 
	protected void StateFetchingData()
	{
		// Handling button 
		if (m_NavJoin)
		{
			m_NavJoin.m_OnActivated.Clear();
			m_NavJoin.m_OnActivated.Insert(OnActionDownload);
			m_NavJoin.SetLabel(STR_DOWNLOAD);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Mods data are checked but not matching 
	protected void StateMissingMods()
	{
		// Handling button 
		if (m_NavJoin)
		{
			m_NavJoin.m_OnActivated.Clear();
			m_NavJoin.m_OnActivated.Insert(OnActionDownload);
			m_NavJoin.SetLabel(STR_DOWNLOAD);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Mods data are loaded and mods are matching, game can be run 
	protected void StateCanJoin()
	{
		// Handling button 
		if (m_NavJoin)
		{
			m_NavJoin.m_OnActivated.Clear();
			m_NavJoin.m_OnActivated.Insert(OnActionJoin);
			m_NavJoin.SetLabel(STR_JOIN);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Call this whenever is mod entry focused - unifying invoke  
	protected void OnModEntryFocus(Widget entry, Dependency dependency)
	{
		//m_OnModEntryFocus.Invoke(dependency);
		if (!entry || !dependency)
			return;
		
		if (m_wDescriptionMods)
			m_wDescriptionMods.SetVisible(true);
		
		//m_DetailPanelMods.DisplayContentMod(dependency);
		
		// Set focus 
		foreach (SCR_ModEntryComponent modEntry : m_aModEntries)
		{
			if (modEntry.GetRootWidget() == entry)
				m_FocusedModEntry = modEntry;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnModDownloaded(SCR_ModEntryComponent modEntry)
	{
		m_iDownloadedModsCout++;

		// Download is complete = all items downloaded
		if (m_ModsManager.GetRoomItemsToUpdate().Count() == m_iDownloadedModsCout)
		{
			// Set handling button to connect on last download
			ChangeState(EServerConnectingState.CAN_JOIN);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void ServerDetailsMenuUI()
	{
		
	}
	
	//------------------------------------------------------------------------------------------------
	void ~ServerDetailsMenuUI()
	{
		
	}
};
