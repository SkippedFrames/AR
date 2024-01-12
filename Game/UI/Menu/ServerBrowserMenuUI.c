/*
Main class for server browser menu handling
Handles cooperation of varios components for server searching, joning, mods handling, etc.
*/
//------------------------------------------------------------------------------------------------
void ScriptInvokerRoomMethod(Room room);
typedef func ScriptInvokerRoomMethod;
typedef ScriptInvokerBase<ScriptInvokerRoomMethod> ScriptInvokerRoom;

//------------------------------------------------------------------------------------------------
class ServerBrowserMenuUI : MenuRootBase
{
	// Server list feedback strings
	const string TAG_DETAILS_FALLBACK_SEACHING = "Searching";
	const string TAG_DETAILS_FALLBACK_EMPTY = "Empty";
	const string TAG_MESSAGE_SEARCHING = "SEARCHING";
	const string TAG_MESSAGE_CONNECTING = "CONNECTING";

	const int SERVER_LIST_VIEW_SIZE = 32;
	const int ROOM_CONTENT_LOAD_DELAY = 500;
	const int ROOM_REFRESH_RATE = 10 * 1000;

	protected const int REJOIN_DELAY = 10;

	// This should be the same value as in High latency filter in {4F6F41C387ADC14E}Configs/ServerBrowser/ServerBrowserFilterSet.conf
	// TODO: unify with the value set in filters
	static const int HIGH_PING_SERVER_THRESHOLD = 200;
	
	protected bool m_bWaitForRefresh;
	protected bool m_bHostedServers;

	// Widget class reference
	protected ref ServerBrowserMenuWidgets m_Widgets = new ServerBrowserMenuWidgets();

	// Server list handling
	protected SCR_ServerBrowserEntryComponent m_ServerEntry;

	// Widget handlers
	protected SCR_TabViewComponent m_TabView;
	protected SCR_SortHeaderComponent m_SortBar;
	protected SCR_EditBoxComponent m_SearchEditBox;
	protected SCR_FilterPanelComponent m_FilterPanel;
	protected SCR_PooledServerListComponent m_ScrollableList;
	protected SCR_ServerScenarioDetailsPanelComponent m_ServerScenarioDetails;

	// States
	protected Widget m_wFirstServerEntry;
	protected SCR_EListMenuWidgetFocus m_iFocusedWidgetState = SCR_EListMenuWidgetFocus.LIST;

	protected static string m_sErrorMessage = "";
	protected static string m_sErrorMessageGroup = "";
	protected static string m_sErrorMessageDetail = "";

	// Skip the details dialog on double click
	protected bool m_bQuickJoin = true;

	// Lobby Rooms handling
	protected Room m_RoomToJoin;
	protected ref array<Room> m_aRooms = {};
	protected ref array<Room> m_aDirectFoundRooms = {};

	protected WorkshopApi m_WorkshopApi;
	protected ClientLobbyApi m_Lobby;
	protected ref ServerDetailsMenuUI m_DetailsDialog;
	protected SCR_ConfigurableDialogUi m_ModListFailDialog;

	// Privileges
	protected ref SCR_ScriptPlatformRequestCallback m_CallbackGetPrivilege;

	// Search callbacks
	protected ref ServerBrowserCallback m_CallbackLastSearch;
	protected ref SCR_BackendCallback m_CallbackScroll = new SCR_BackendCallback();
	protected ref SCR_RoomCallback m_CallbackFavorite = new SCR_RoomCallback();

	protected ref OnDirectJoinCallback m_CallbackSearchTarget = new OnDirectJoinCallback();

	// Room data refresh
	protected ref ServerBrowserCallback m_CallbackAutoRefresh = new ServerBrowserCallback();
	protected bool m_bFirstRoomLoad = true;
	protected bool m_bForceUnfilteredRequest;
	protected int m_iTotalNumberOfRooms;

	// Joining
	protected ref ServerBrowserCallback m_CallbackJoin = new ServerBrowserCallback();
	protected ref SCR_ServerBrowserDialogManager m_Dialogs = new SCR_ServerBrowserDialogManager();

	protected ref RoomJoinData m_JoinData = new RoomJoinData();
	protected ref SCR_RoomPasswordVerification m_PasswordVerification = new SCR_RoomPasswordVerification();

	// Filter parameters
	protected ref FilteredServerParams m_ParamsFilter = new FilteredServerParams();
	protected ref FilteredServerParams m_DirectJoinParams;

	// Server mod content
	protected ref SCR_RoomModsManager m_ModsManager = new SCR_RoomModsManager();
	protected ref array<ref SCR_WorkshopItem> m_aRequiredMods = {}; 
	protected ref array<ref SCR_WorkshopItemActionDownload> m_aUnrelatedDownloads = {};
	protected SCR_DownloadManager m_DownloadManager;

	// Dependencies arrays for pasing updated & outdated server mod dependecies
	protected MissionWorkshopItem m_RoomScenario;

	// Message components
	protected SCR_SimpleMessageComponent m_SimpleMessageWrap;
	protected SCR_SimpleMessageComponent m_SimpleMessageList;

	protected bool m_bIsWaitingForBackend = true;

	// Reconnecting to last played server
	protected ref ServerBrowserCallback m_CallbackSearchPreviousRoom = new ServerBrowserCallback();
	protected ref SCR_GetRoomsIds m_SearchIds = new SCR_GetRoomsIds();
	protected Room m_RejoinRoom;

	protected ref array<ref SCR_FilterEntry> m_aFiltersToSelect = {};
	protected bool m_bIsCheckingSpecificfilter;

	// Script invokers
	ref ScriptInvoker m_OnAutoRefresh = new ScriptInvoker();
	ref ScriptInvoker m_OnScenarioLoad = new ScriptInvoker();
	
	protected ref ScriptInvokerVoid m_OnFavoritesResponse = new ScriptInvokerVoid();
	
	protected static ref ScriptInvoker<> m_OnErrorMessageSet = new ScriptInvoker<>();

	// Entry Actions
	protected EInputDeviceType m_eLastInputType;
	protected bool m_bWasEntrySelected;
	protected SCR_ServerBrowserEntryComponent m_ClickedEntry; // Cache last clicked entry to trigger the correct dialog after the double click window
	
	//------------------------------------------------------------------------------------------------
	// OVERRIDES
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	//! Opening menu - do server browser setup - TODO@wernerjak - cleanup open
	override void OnMenuOpen()
	{
		// Workshop setup
		m_WorkshopApi = GetGame().GetBackendApi().GetWorkshop();
		m_Lobby = GetGame().GetBackendApi().GetClientLobby();

		// Items preloading
		if (m_WorkshopApi.NeedScan())
			m_WorkshopApi.ScanOfflineItems();

		// Find widgets
		m_Widgets.FindAllWidgets(GetRootWidget());

		// Accessing handlers
		SetupHandlers();
		SetupCallbacks();
		m_CallbackAutoRefresh.m_OnSuccess.Insert(OnRoomAutoRefresh);

		SetupParams(m_Lobby);

		// Setup list and message
		if (SCR_ServicesStatusHelper.IsBackendReady())
			Messages_ShowMessage(TAG_MESSAGE_SEARCHING);
		else
			Messages_ShowMessage(TAG_MESSAGE_CONNECTING, true);

		if (m_ScrollableList)
			m_ScrollableList.ShowEmptyRooms();

		// Setup Actions
		SCR_MenuActionsComponent actionsComp = SCR_MenuActionsComponent.FindComponent(GetRootWidget());
		if (actionsComp)
			actionsComp.GetOnAction().Insert(OnActionTriggered);

		UpdateNavigationButtons();

		SwitchFocus(SCR_EListMenuWidgetFocus.SORTING);
		
		super.OnMenuOpen();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpened()
	{
		super.OnMenuOpened();
		
		// Show last server
		if (!m_sErrorMessage.IsEmpty() && !GetGame().GetBackendApi().GetClientLobby().GetPreviousRoomId().IsEmpty())
			DisplayKick();
		
		m_OnErrorMessageSet.Insert(DisplayKick);
		
		Refresh();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusGained()
	{
		// Focus back on the host button
		if (m_bHostedServers)
			GetGame().GetCallqueue().CallLater(FocusWidget, 0, false, m_Widgets.m_wHostNewServerButton);
		else
			SwitchFocus(SCR_EListMenuWidgetFocus.LIST, true);
		
		super.OnMenuFocusGained();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusLost()
	{
		super.OnMenuFocusLost();
	}

	//------------------------------------------------------------------------------------------------
	//! Closing menu - clearing server browser data
	override void OnMenuClose()
	{
		// Clearing handlers
		if (m_TabView)
			m_TabView.m_OnChanged.Remove(OnTabViewSwitch);

		// Store filter pamarameters
		m_Lobby.StoreParams();

		// Remove callbacks
		m_Lobby.SetRefreshCallback(null);

		m_ScrollableList.GetOnSetPage().Remove(CallOnServerListSetPage);
		m_CallbackAutoRefresh.m_OnSuccess.Clear();

		ClearConnectionTimeoutWaiting();

		// Save filter
		m_FilterPanel.Save();
		
		super.OnMenuClose();
	}

	//------------------------------------------------------------------------------------------------
	//! Updating menu - continuous menu hanling
	override protected void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);

		// Run actions after backend initialization
		if (m_bIsWaitingForBackend)
			WaitingForRunningBackend();

		// Scroll list update
		if (m_ScrollableList)
			m_ScrollableList.UpdateScroll();

		//! Update Entry buttons
		EInputDeviceType inputDeviceType = GetGame().GetInputManager().GetLastUsedInputDevice();
		bool isEntrySelected = GetSelectedEntry();
		bool shouldUpdateButtons = inputDeviceType != m_eLastInputType || isEntrySelected != m_bWasEntrySelected;

		if (shouldUpdateButtons)
			UpdateNavigationButtons();

		m_eLastInputType = inputDeviceType;
		m_bWasEntrySelected = isEntrySelected;
	}

	//------------------------------------------------------------------------------------------------
	// INPUTS
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	protected void UpdateNavigationButtons()
	{
		if (!m_Widgets)
			return;

		SCR_ServerBrowserEntryComponent entry = GetSelectedEntry();
		bool versionMismatch, unjoinable;
		bool enabled = entry && entry.GetIsEnabled(versionMismatch, unjoinable);
		bool visible = entry && GetGame().GetInputManager().GetLastUsedInputDevice() != EInputDeviceType.MOUSE;

		if (m_Widgets.m_JoinButton)
			m_Widgets.m_JoinButton.SetVisible(visible && !unjoinable, false);
		
		if (m_Widgets.m_DetailsButton)
			m_Widgets.m_DetailsButton.SetVisible(visible && !unjoinable, false);
		
		if (m_Widgets.m_FavoritesButton)
			m_Widgets.m_FavoritesButton.SetVisible(visible && enabled, false);

		if (!enabled || !visible)
			return;

		Room room = entry.GetRoomInfo();
		if (!room)
			return;

		if (m_Widgets.m_FavoritesButton)
			m_Widgets.m_FavoritesButton.SetLabel(GetFavoriteLabel(room.IsFavorite()));
	}

	//------------------------------------------------------------------------------------------------
	protected void OnActionTriggered(string action, float multiplier)
	{
		//!TODO: Why are these being triggered twice?

		//! Proceed only if using mouse, the navigation buttons will take care of the interactions with keyboard or gamepad
		if (GetGame().GetInputManager().GetLastUsedInputDevice() != EInputDeviceType.MOUSE)
			return;

		switch (action)
		{
			case "MenuSelectDouble": OnServerEntryClickInteraction(multiplier); break;
			case "MenuServerDetails": OnActionDetails(); break;
			case "MenuFavourite": OnActionFavorite(); break;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Bind action for refreshing server list
	[MenuBindAttribute()]
	void OnActionRefresh()
	{
		m_bForceUnfilteredRequest = true;
		Refresh();
	}
	
	//------------------------------------------------------------------------------------------------
	void Refresh()
	{
		PrintDebug("m_RejoinRoom: " + m_RejoinRoom, "OnActionRefresh");

		// Try negotiate missing MP privilege
		if (!GetGame().GetPlatformService().GetPrivilege(UserPrivilege.MULTIPLAYER_GAMEPLAY))
		{
			Messages_ShowMessage("MISSING_PRIVILEGE_MP");
			//NegotiateMPPrivilegeAsync();
			NegotiatePrivilegeAsync(UserPrivilege.MULTIPLAYER_GAMEPLAY);
			return;
		}

		// Clear list
		//m_CallbackScroll.m_OnSuccess.Remove(OnScrollSuccess);
		m_CallbackScroll.GetEventOnResponse().Remove(OnScrollResponse);

		//  Setup list loading
		if (m_ScrollableList)
		{
			m_ScrollableList.MoveToTop();
			m_ScrollableList.ShowEmptyRooms();
			m_ScrollableList.ShowScrollbar(false);
		}
		
		CheckBackendState();
		
		// Remove callbacks
		m_Lobby.SetRefreshCallback(null);

		// Start loading
		if (m_bFirstRoomLoad)
			SearchRooms();
		else
			GetGame().GetCallqueue().CallLater(SearchRooms, ROOM_CONTENT_LOAD_DELAY, false);

		if (SCR_ServicesStatusHelper.IsBackendReady())
			Messages_ShowMessage(TAG_MESSAGE_SEARCHING);
		else
			Messages_ShowMessage(TAG_MESSAGE_CONNECTING, true);

		if (m_ServerScenarioDetails)
			m_ServerScenarioDetails.SetDefaultScenario(TAG_DETAILS_FALLBACK_SEACHING);
	}

	//------------------------------------------------------------------------------------------------
	//! Bind action for opening dialog with manual connect to ip
	[MenuBindAttribute()]
	void OnActionManualConnect()
	{
		MultiplayerDialogUI multiplayerDialog = m_Dialogs.CreateManualJoinDialog();

		if (!multiplayerDialog)
			return;

		// Ip handling
		multiplayerDialog.m_OnConfirm.Clear();
		multiplayerDialog.m_OnConfirm.Insert(JoinActions_DirectJoin);

		// Cancel
		multiplayerDialog.m_OnCancel.Clear();
		multiplayerDialog.m_OnCancel.Insert(OnDirectJoinCancel);
	}

	//------------------------------------------------------------------------------------------------
	//! Bind action for switching to filter component
	[MenuBindAttribute()]
	void OnActionFilter()
	{
		if (!m_FilterPanel)
			return;

		SCR_EListMenuWidgetFocus focus =  SCR_EListMenuWidgetFocus.FILTERING; 
		if (m_FilterPanel.GetFilterListBoxShown())
			focus =  SCR_EListMenuWidgetFocus.LIST; 

		// Set focus
		m_FilterPanel.ShowFilterListBox(focus == SCR_EListMenuWidgetFocus.FILTERING);
		SwitchFocus(focus);
	}

	//------------------------------------------------------------------------------------------------
	//! Bind action for leaving menu
	[MenuBindAttribute()]
	void OnActionBack()
	{
		// Remove specific filters
		if (m_ParamsFilter)
		{
			m_ParamsFilter.SetScenarioId("");
			m_ParamsFilter.SetHostedScenarioModId("");
		}

		// Close
		Close();
	}

	//------------------------------------------------------------------------------------------------
	//! Action for favoriting server
	void OnActionFavorite()
	{
		if (!m_ServerEntry)
		{
			Debug.Error("No entry selected.");
			return;
		}
		m_ServerEntry.SetFavorite(!m_ServerEntry.IsFavorite());
	}

	//------------------------------------------------------------------------------------------------
	//! Action for server details
	void OnActionDetails()
	{
		SCR_ServerBrowserEntryComponent entry = GetSelectedEntry();
		if (!entry)
			return;

		m_bQuickJoin = false;

		// Join
		Room room = entry.GetRoomInfo();
		if (room && room.Joinable())
			JoinActions_Join();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnServerEntryClickInteraction(float multiplier)
	{
		//! multiplier value in the action is used to differentiate between single and double click

		SCR_ServerBrowserEntryComponent entry = m_ClickedEntry;
		if (!entry || !GetEntryUnderCursor())
			return;

		switch (Math.Floor(multiplier))
		{
			case 1: OnServerEntryClick(entry); break;
			case 2: OnServerEntryDoubleClick(entry); break;
		}

		m_ClickedEntry = null;
	}

	//------------------------------------------------------------------------------------------------
	//! Join to the room on double click on server entry
	protected void OnServerEntryDoubleClick(SCR_ServerBrowserEntryComponent entry)
	{
		if (entry != m_ServerEntry)
			OnServerEntryFocusEnter(entry);
		
		m_bQuickJoin = true;
		
		// Join
		if (entry && entry.GetRoomInfo() && entry.GetRoomInfo().Joinable())
			JoinActions_Join();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEntryMouseButton(string tag)
	{
		OnServerEntryDoubleClick(GetSelectedEntry());
	}

	//------------------------------------------------------------------------------------------------
	protected void OnJoinButton()
	{
		OnServerEntryClick(GetSelectedEntry());
	}

	//------------------------------------------------------------------------------------------------
	//! Join to the room on double click on server entry
	protected void OnServerEntryClick(notnull SCR_ServerBrowserEntryComponent entry)
	{
		//! This means the player moved the mouse to another entry during the small window it takes the component to check for double click...
		//! ...so another room got focused, but we want the clicked one to be the focused one
		if (entry != m_ServerEntry)
			OnServerEntryFocusEnter(entry);
		
		m_RoomToJoin = entry.GetRoomInfo();		
		
		//! On MOUSE clicking opens the details dialog. On KEYBOARD and GAMEPAD single click is quick join, there's a separate button for details
		m_bQuickJoin = GetGame().GetInputManager().GetLastUsedInputDevice() != EInputDeviceType.MOUSE;

		// Join
		if (entry.GetRoomInfo() && entry.GetRoomInfo().Joinable())
			JoinActions_Join();
		
		//TODO: handle edge case in which we ask for the details while downloading another version of a required mod, make sure the shown download size is correct
	}

	//------------------------------------------------------------------------------------------------
	// Service check
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	//! Start looking for servers once backend is runnig
	protected void WaitingForRunningBackend()
	{
		if (!IsBackendReady())
			return;

		// Call room search or find last server
		string lastId = m_Lobby.GetPreviousRoomId();

		// Is there last server and reconnect is enabled
		bool reconnectEnabled = IsKickReconnectEnabled(m_Dialogs.GetCurrentKickDialog());

		if (!m_Lobby.IsPingAvailable())
			return;

		ClearConnectionTimeoutWaiting();
		Refresh();

		// Clear error msg
		ClearErrorMessage();

		// Stop waiting for backend
		m_bIsWaitingForBackend = false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsBackendReady()
	{
		auto ba = GetGame().GetBackendApi();
		return ba.IsActive() && ba.IsAuthenticated();
	}

	//------------------------------------------------------------------------------------------------	
	protected void CheckBackendState()
	{	
		if (!IsBackendReady())
		{
			m_bIsWaitingForBackend = true;
			auto callQueue = GetGame().GetCallqueue();
			callQueue.Remove(ConnectionTimeout);
			callQueue.CallLater(ConnectionTimeout, SCR_ServicesStatusHelper.CONNECTION_CHECK_EXPIRE_TIME);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ShowTimeoutDialog()
	{
		Messages_ShowMessage("NO_CONNECTION");
		if (m_Dialogs)
		{
			m_Dialogs.DisplayDialog(EJoinDialogState.BACKEND_TIMEOUT);
			m_Dialogs.GetOnConfirm().Clear();
			m_Dialogs.GetOnConfirm().Insert(OnConnectionTimeoutDialogConfirm);
		}
	}
	//------------------------------------------------------------------------------------------------
	//! Fail wating for backend if takes too long
	protected void ConnectionTimeout()
	{
		if (!IsBackendReady())
		{
			ShowTimeoutDialog();
		}
		ClearConnectionTimeoutWaiting();

		m_bIsWaitingForBackend = false;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnConnectionTimeoutDialogConfirm()
	{
		m_Dialogs.GetOnConfirm().Remove(OnConnectionTimeoutDialogConfirm);
		
		if (SCR_ServicesStatusHelper.IsBackendReady())
			Messages_ShowMessage(TAG_MESSAGE_SEARCHING);
		else
			Messages_ShowMessage(TAG_MESSAGE_CONNECTING, true);		
	}

	//------------------------------------------------------------------------------------------------
	protected void ClearConnectionTimeoutWaiting()
	{
		GetGame().GetCallqueue().Remove(ConnectionTimeout);
	}

	//------------------------------------------------------------------------------------------------
	//! Call this on receiving filtered rooms from search
	//! Setting up server list - TODO@wernerjak - can be partialy moved in server list comp
	protected void OnRoomsFound(array<Room> rooms = null)
	{		
		// Load rooms
		ClientLobbyApi lobby = GetGame().GetBackendApi().GetClientLobby();

		if (rooms)
		{
			// Direct join rooms
			lobby.Target(rooms);
		}
		else
		{
			// Room from search
			lobby.Rooms(m_aRooms);
			//TODO: [BUG] On trunk, switching to Community Tab returns an empty rooms array even if there are rooms?!

			if (m_aRooms.IsEmpty())
			{
				PrintDebug("No room found", "OnRoomsFound");
			}
		}
		
		// Setup room data refresh
		lobby.SetRefreshCallback(m_CallbackAutoRefresh);
		lobby.SetRefreshRate(ROOM_REFRESH_RATE);

		DisplayRooms(m_aRooms);

		// Move to top in list
		if (m_ScrollableList)
		{
			if (m_ScrollableList.IsListFocused() || m_bFirstRoomLoad)
			{
				m_ScrollableList.FocusFirstAvailableEntry();
			}
		}

		if (m_bFirstRoomLoad)
		{
			m_ScrollableList.SetIsListFocused(true);
			m_bFirstRoomLoad = false;
		}

		m_ScrollableList.ShowScrollbar(true);
		
		int currentRoomsCount = m_Lobby.TotalRoomCount();
		if (currentRoomsCount > m_iTotalNumberOfRooms)
			m_iTotalNumberOfRooms = currentRoomsCount;
		
		// Items Found Message
		if (m_FilterPanel)
			m_FilterPanel.SetItemsFoundMessage(m_Lobby.TotalRoomCount(), m_iTotalNumberOfRooms, m_Lobby.TotalRoomCount() != m_iTotalNumberOfRooms);
	}

	//------------------------------------------------------------------------------------------------
	protected void DisplayRooms(array<Room> rooms = null)
	{
		// Don't display if missing MP privilege
		if (!GetGame().GetPlatformService().GetPrivilege(UserPrivilege.MULTIPLAYER_GAMEPLAY))
		{
			return;
		}

		// Check list
		if (!m_ScrollableList || !m_ServerScenarioDetails)
		{
			PrintDebug("Missing important component references!", "DisplayRooms");
			return;
		}

		// Check addons
		SCR_AddonManager addonMgr = SCR_AddonManager.GetInstance();

		if (!addonMgr)
		{
			PrintDebug("Could not find addon mngr to verify ugc privilege", "DisplayRooms");
			return;
		}

		// Modds filter when UGC not enabled
		if (m_ParamsFilter.IsModdedFilterSelected() && !addonMgr.GetUgcPrivilege())
		{
			Messages_ShowMessage("MISSING_PRIVILEGE_UGC");

			// Negotatiate UGC privilege
			addonMgr.m_OnUgcPrivilegeResult.Insert(Platform_OnUgcPrivilegeResult);
			addonMgr.NegotiateUgcPrivilegeAsync();
		
			SwitchFocus(SCR_EListMenuWidgetFocus.LIST);
			if (m_FilterPanel)
				m_FilterPanel.ShowFilterListBox(false);
			
			return;
		}

		int roomCount = m_Lobby.TotalRoomCount();

		// Display no rooms found
		if (roomCount == 0)
		{
			// Setup list
			m_ScrollableList.SetRooms(rooms, 0, true);
			m_ScrollableList.ShowEmptyRooms();

			if (m_ServerScenarioDetails)
				m_ServerScenarioDetails.SetDefaultScenario(TAG_DETAILS_FALLBACK_EMPTY);

			// Show filters
			if (!m_FilterPanel.GetFilterListBoxShown())
				SwitchFocus(SCR_EListMenuWidgetFocus.FILTERING);
			if (m_FilterPanel)
				m_FilterPanel.ShowFilterListBox(true);
			
			// Show message
			Messages_ShowMessage("NO_FILTERED_SERVERS");

			return;
		}

		// Display rooms
		// Set rooms to fill server list
		m_ScrollableList.SetRooms(m_aRooms, m_Lobby.TotalRoomCount(), true);

		// Hide message
		Messages_Hide();
		
		if (m_FilterPanel && !m_FilterPanel.GetFilterListBoxShown())
			SwitchFocus(SCR_EListMenuWidgetFocus.LIST);
	}

	//------------------------------------------------------------------------------------------------
	// LOBBY ROOMS HANDLING
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	//! Call this once new room data are fetched
	protected void OnRoomAutoRefresh(ServerBrowserCallback callback)
	{
		m_Lobby.SetRefreshRate(ROOM_REFRESH_RATE);
		DisplayRooms(m_aRooms);

		m_bWaitForRefresh = false;

		m_OnAutoRefresh.Invoke();
	}

	// ---- SEARCH ROOMS ----
	// First search to get the total number of servers
	//------------------------------------------------------------------------------------------------
	protected void SearchRooms()
	{
		//TODO: query for filtered first, then independently query for total number, in order to reduce load time at start
		
		// Start loading and show loading feedback
		m_ServerEntry = null;
		m_ModsManager.Clear();
		
		// Remove callbacks
		m_Lobby.SetRefreshCallback(null);

		if (!m_bFirstRoomLoad && !m_bForceUnfilteredRequest)
		{
			SearchRoomsFiltered();
			return;
		}

		// Callback
		ref ServerBrowserCallback searchCallback = new ServerBrowserCallback;
		
		// Invoker actions
		searchCallback.m_OnSuccess.Insert(OnSearchAllRoomsSuccess);
		searchCallback.m_OnFail.Insert(OnSearchAllRoomsFail);
		searchCallback.m_OnTimeOut.Insert(OnSearchAllRoomsTimeOut);
		
		m_CallbackLastSearch = searchCallback;
		
		FilteredServerParams params = new FilteredServerParams();
		m_Lobby.SearchRooms(params, searchCallback);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSearchAllRoomsSuccess(ServerBrowserCallback callback)
	{
		OnSearchAllRooms();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSearchAllRoomsFail(ServerBrowserCallback callback, int code, int restCode, int apiCode)
	{
		OnSearchAllRooms();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSearchAllRoomsTimeOut()
	{
		OnSearchAllRooms();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSearchAllRooms()
	{
		m_CallbackLastSearch = null;
		
		// Total number of active servers
		m_iTotalNumberOfRooms = m_Lobby.TotalRoomCount();
		
		SearchRoomsFiltered();
	}
	
	// Searches to get the filtered servers to display	
	//------------------------------------------------------------------------------------------------
	protected void SearchRoomsFiltered()
	{
		if (m_ScrollableList)
			m_Lobby.SetViewSize(m_ScrollableList.GetPageEntriesCount() * 2);
		else
			m_Lobby.SetViewSize(SERVER_LIST_VIEW_SIZE);
		
		// Callback
		ref ServerBrowserCallback searchCallback = new ServerBrowserCallback;

		// Invoker actions
		searchCallback.m_OnSuccess.Insert(OnSearchRoomsSuccess);
		searchCallback.m_OnFail.Insert(OnSearchRoomsFail);
		searchCallback.m_OnTimeOut.Insert(OnSearchRoomsTimeOut);
		
		m_CallbackLastSearch = searchCallback;
		
		m_Lobby.SearchRooms(m_ParamsFilter, searchCallback);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSearchRoomsSuccess(ServerBrowserCallback callback)
	{
		PrintDebug(string.Format("Success - servers found: %1", m_Lobby.TotalRoomCount()), "OnSearchRoomsSuccess");

		// Display rooms
		OnRoomsFound();
		m_CallbackLastSearch = null;

		PrintDebug("AFTER - m_RejoinRoom: " + m_RejoinRoom, "OnSearchRoomsSuccess");
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSearchRoomsFail(ServerBrowserCallback callback, int code, int restCode, int apiCode)
	{
		PrintDebug(string.Format("Room search fail - code: %1 | restCode: %2 | apiCode: %3", code, restCode, apiCode), "OnSearchRoomsFail");
		m_CallbackLastSearch = null;

		// Run again if failed ping
		if (m_bFirstRoomLoad && m_ParamsFilter.GetSortOrder() == m_ParamsFilter.SORT_PING)
		{
			Refresh();
			return;
		}

		Messages_ShowMessage("BACKEND_SERVICE_FAIL");
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSearchRoomsTimeOut()
	{
		ShowTimeoutDialog();
		PrintDebug("time out!", "OnSearchRoomsTimeOut");
	}

	//------------------------------------------------------------------------------------------------
	//! Restore filtering parameters in UI
	protected void SetupParams(ClientLobbyApi lobby)
	{
		FilteredServerParams params = FilteredServerParams.Cast(lobby.GetParameters());
		if (!params)
		{
			// Default filter setup
			string strParams = lobby.GetStrParams();
			if (!strParams.IsEmpty())
			{
				m_ParamsFilter.ExpandFromRAW(strParams);
				lobby.ClearParams();
			}
		}
		else
		{
			// Restore previous filter setup
			m_ParamsFilter = params;
		}
		
		// Reset search
		m_ParamsFilter.SetSearch(string.Empty);
		
		SetupFilteringUI(m_ParamsFilter);
	}

	//------------------------------------------------------------------------------------------------
	//! Apply filter setup on each ui filtering widget - restoring filter UI states
	protected void SetupFilteringUI(FilteredServerParams filterParams)
	{
		// Tabview
		if (m_TabView)
		{
			if (m_ParamsFilter)
				m_TabView.ShowTab(m_ParamsFilter.GetSelectedTab());
		}
		
		if (m_FilterPanel)
		{
			bool show = m_FilterPanel.AnyFilterButtonsVisible();
			m_FilterPanel.ShowFilterListBox(show, show);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! General callbacks setup
	protected void SetupCallbacks()
	{
		// Loading
		SCR_MenuLoadingComponent.m_OnMenuOpening.Insert(OnOpeningByLoadComponent);

		// Server list
		m_ScrollableList.GetOnSetPage().Insert(CallOnServerListSetPage);
	}

	//------------------------------------------------------------------------------------------------
	protected void DisplayKick()
	{
		m_Dialogs.DisplayKickErrorDialog(m_sErrorMessage, m_sErrorMessageGroup, m_sErrorMessageDetail);

		// Find last
		string lastId = m_Lobby.GetPreviousRoomId();
		bool hasLastServer = !lastId.IsEmpty();
		bool reconnectEnabled = IsKickReconnectEnabled(m_Dialogs.GetCurrentKickDialog());

		if (hasLastServer && reconnectEnabled)
		{
			m_Dialogs.GetEventOnRejoinTimerOver().Insert(OnLastRoomReconnectConfirm);

			m_Dialogs.GetOnConfirm().Insert(OnLastRoomReconnectConfirm);
			m_Dialogs.GetOnCancel().Remove(OnRejoinCancel);
			m_Dialogs.GetOnCancel().Insert(OnRejoinCancel);

			m_Dialogs.DisplayReconnectDialog(m_RejoinRoom, REJOIN_DELAY, m_sErrorMessageDetail);
		}

		// Clear messages and backend check
		ClearErrorMessage();
	}

	//------------------------------------------------------------------------------------------------
	//! Return true if reconnect can be displayed after this kick dialog
	protected bool IsKickReconnectEnabled(SCR_ConfigurableDialogUi dialog)
	{
		// Check dialog
		if (!dialog)
			return false;

		// Get kick comp
		SCR_KickDialogUiPreset kickPreset = SCR_KickDialogUiPreset.Cast(dialog.GetDialogPreset());
		if (!kickPreset)
			return false;

		// Result
		return kickPreset.m_bCanBeReconnected;
	}

	//------------------------------------------------------------------------------------------------
	// ENTRIES
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	protected void OnServerEntryFocusEnter(SCR_ScriptedWidgetComponent entry)
	{
		SCR_ServerBrowserEntryComponent serverEntry = SCR_ServerBrowserEntryComponent.Cast(entry);
		
		if (!serverEntry || !m_ModsManager)
			return;

		Room room = serverEntry.GetRoomInfo();
		if (!room)
		{
			m_ServerEntry = null;
			return;
		}
		
		DisplayFavoriteAction(room.IsFavorite());
		UpdateNavigationButtons();
		serverEntry.m_OnClick.Insert(OnEntryMouseClick);
		
		if (m_ServerEntry == serverEntry)
			return;
		
		m_ServerEntry = serverEntry;
		m_RoomToJoin = room;

		m_ModsManager.Clear();
		m_ServerEntry.SetModsManager(m_ModsManager);

		ReceiveRoomContent(m_RoomToJoin, true);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnServerEntryFocusLeave(SCR_ScriptedWidgetComponent entry)
	{
		SCR_ServerBrowserEntryComponent serverEntry = SCR_ServerBrowserEntryComponent.Cast(entry);

		serverEntry.m_OnClick.Remove(OnEntryMouseClick);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEntryMouseClick(SCR_ScriptedWidgetComponent button)
	{
		m_ClickedEntry = SCR_ServerBrowserEntryComponent.Cast(button);
	}

	//------------------------------------------------------------------------------------------------
	protected void ReceiveRoomContent(notnull Room room, bool receiveMods)
	{
		// Call queue handling
		ScriptCallQueue callQueue = GetGame().GetCallqueue();

		// Clear previous call latter

		callQueue.Remove(ReceiveRoomContent_Mods);
		
		if (m_ServerScenarioDetails)
			m_ServerScenarioDetails.DisplayRoomData(room, receiveMods);

		// Allow check only if client is authorized to join server
		if (!room.IsAuthorized())
		{
			return;
		}

		// Check room mods count
		array<Dependency> roomMod = {};
		room.GetItems(roomMod);

		// Show mod count immidiatelly
		if (!roomMod.IsEmpty() && m_ServerScenarioDetails)
			m_ServerScenarioDetails.DisplayModsCount(roomMod.Count());

		// Receive mod data only if currently view is focused
		if (room.IsDownloadListLoaded())
		{

			ReceiveRoomContent_Mods(room);
			return;
		}
		else
		{
			// Load mods after short delay - to prevent spamming mods receive requiest with fast server selecting
			callQueue.CallLater(ReceiveRoomContent_Mods, ROOM_CONTENT_LOAD_DELAY, false, room);
		}		
	}

	//------------------------------------------------------------------------------------------------
	//! Separated receive scenario data for server
	//! Call this later if scenario is from mod to prevent too many request
	protected void ReceiveRoomContent_Scenario()
	{
		// Load scenario
		m_ModsManager.GetOnGetScenario().Insert(OnLoadingScenario);
		m_ModsManager.ReceiveRoomScenario(m_RoomToJoin);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Separeted receive mods data for server
	//! Call this later to prevent too many request
	protected void ReceiveRoomContent_Mods(Room room)
	{
		if (!m_ModsManager || !room || m_RoomToJoin != room)
			return;

		m_ModsManager.GetOnGetAllDependencies().Insert(OnLoadingDependencyList);
		m_ModsManager.ReceiveRoomMods(room);
	}

	//------------------------------------------------------------------------------------------------
	// SETUP
	//------------------------------------------------------------------------------------------------
	//! Getting reference for all server widget elements
	protected void SetupHandlers()
	{
		// Tab view
		m_TabView = SCR_TabViewComponent.Cast(m_Widgets.FindHandlerReference(null, m_Widgets.WIDGET_TAB_VIEW, SCR_TabViewComponent));
		if (m_TabView)
		{
			m_TabView.m_OnChanged.Insert(OnTabViewSwitch);
			OnTabViewSwitch(null, null, m_TabView.GetShownTab());

			if (!GetGame().IsPlatformGameConsole())
				m_TabView.AddTab("", "#AR-Workshop_ButtonHost");
		}

		// Filter panel
		m_FilterPanel = SCR_FilterPanelComponent.Cast(m_Widgets.FindHandlerReference(null, m_Widgets.WIDGET_FILTER, SCR_FilterPanelComponent));
		if (m_FilterPanel)
		{
			// Invoker
			m_FilterPanel.GetOnFilterPanelToggled().Insert(OnFilterPanelToggle);
			m_FilterPanel.GetOnFilterChanged().Insert(OnChangeFilter);

			// Attempt load previous filter setup
			m_FilterPanel.TryLoad();
			m_ParamsFilter.SetFilters(m_FilterPanel.GetFilter());
			
			FilterCrossplayCheck();
		}

		// Sorting header
		m_SortBar = SCR_SortHeaderComponent.Cast(m_Widgets.FindHandlerReference(null, m_Widgets.WIDGET_SERVER_HEADER, SCR_SortHeaderComponent));

		if (m_SortBar)
		{
			m_SortBar.m_OnChanged.Insert(OnChangeSort);

			// Initial sort
			// todo move default sorting values out of here, it can be set in layout file now
			m_SortBar.SetCurrentSortElement(3, ESortOrder.DESCENDING);
		}

		// Search edit box
		m_SearchEditBox = SCR_EditBoxSearchComponent.Cast(
			m_Widgets.FindHandlerReference(m_Widgets.m_wSearchEditBox, m_Widgets.WIDGET_SEARCH, SCR_EditBoxSearchComponent)
		);

		if (m_SearchEditBox)
			m_SearchEditBox.m_OnConfirm.Insert(OnSearchEditBoxConfirm);

		// Server list and scroll --------------------
		m_ScrollableList = SCR_PooledServerListComponent.Cast(m_Widgets.FindHandlerReference(
			null, m_Widgets.WIDGET_SCROLLABLE_LIST, SCR_PooledServerListComponent
		));

		// Add callbacks
		array<SCR_ServerBrowserEntryComponent> entries = m_ScrollableList.GetRoomEntries();

		foreach (SCR_ServerBrowserEntryComponent entry : entries)
		{
			// Set invoker actions
			entry.GetOnFocus().Insert(OnServerEntryFocusEnter);
			entry.GetOnFocusLost().Insert(OnServerEntryFocusLeave);
			entry.GetOnFavorite().Insert(OnRoomEntrySetFavorite);
			entry.GetOnMouseInteractionButtonClicked().Insert(OnEntryMouseButton);
		}

		// Detail screen
		m_ServerScenarioDetails = SCR_ServerScenarioDetailsPanelComponent.Cast(m_Widgets.FindHandlerReference(
			null, m_Widgets.WIDGET_SERVER_SCENARIO_DETAILS_PANEL, SCR_ServerScenarioDetailsPanelComponent
		));

		// Bacis setup and hide
		if (m_ServerScenarioDetails)
			m_ServerScenarioDetails.SetModsManager(m_ModsManager);

		// Messages
		m_SimpleMessageWrap = SCR_SimpleMessageComponent.Cast(m_Widgets.FindHandlerReference(null, m_Widgets.WIDGET_MESSAGE_WRAP, SCR_SimpleMessageComponent));
		m_SimpleMessageList = SCR_SimpleMessageComponent.Cast(m_Widgets.FindHandlerReference(null, m_Widgets.WIDGET_MESSAGE_LIST, SCR_SimpleMessageComponent));

		// Navigation buttons
		m_Widgets.m_JoinButton.m_OnActivated.Insert(OnJoinButton);
		m_Widgets.m_DetailsButton.m_OnActivated.Insert(OnActionDetails);
		m_Widgets.m_FavoritesButton.m_OnActivated.Insert(OnActionFavorite);
	}

	//------------------------------------------------------------------------------------------------
	//!	Init behavior when filter panel script is attached
	protected void FilterCrossplayCheck()
	{
		if (!m_FilterPanel)
			return;

		bool hasCrossplay = GetGame().GetPlatformService().GetPrivilege(UserPrivilege.CROSS_PLAY);
		//hasCrossplay = false;

		// Cross play privilege missing
		if (!hasCrossplay)
		{
			SCR_FilterSet filterSet = m_FilterPanel.GetFilter();
			if (filterSet)
			{
				SCR_FilterCategory crossPlay = filterSet.FindFilterCategory("Crossplay");

				if (crossPlay)
				{
					SCR_FilterEntry enabled = crossPlay.FindFilter("CrossplayEnabled");
					if (enabled)
					{
						enabled.SetSelected(false);
						m_FilterPanel.SelectFilter(enabled, false, false);
					}

					SCR_FilterEntry disabled = crossPlay.FindFilter("CrossPlayDisabled");
					if (disabled)
					{
						disabled.SetSelected(true);
						m_FilterPanel.SelectFilter(disabled, true, false);
					}
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Switch focus
	protected void SwitchFocus(SCR_EListMenuWidgetFocus focus, bool force = false)
	{
		if (m_iFocusedWidgetState == focus && !force)
			return; 

		Widget focusTarget;
		
		switch(focus)
		{
			case SCR_EListMenuWidgetFocus.LIST:
			{
				if (m_ServerEntry && m_ServerEntry.IsVisible())
					focusTarget = m_ServerEntry.GetRootWidget();
				else if (m_ScrollableList)
					focusTarget = m_ScrollableList.FirstAvailableEntry();

				break;
			}
		
			case SCR_EListMenuWidgetFocus.FILTERING:
			{
				focusTarget = m_FilterPanel.GetWidgets().m_FilterButton;
				break;
			}
			
			case SCR_EListMenuWidgetFocus.SORTING:
			{
				focusTarget = m_Widgets.m_wSortSessionFavorite;
				break;
			}
		}

		m_iFocusedWidgetState = focus;
		
		if (!focusTarget || !focusTarget.IsVisible())
		{
			// Fallback
			focusTarget = m_Widgets.m_wSortSessionFavorite;
			m_iFocusedWidgetState = SCR_EListMenuWidgetFocus.SORTING;
		}

		GetGame().GetWorkspace().SetFocusedWidget(focusTarget);
	}

	//------------------------------------------------------------------------------------------------
	//! Find filter by it's internal name define in ServerBrowserFilterSet.conf
	//! Set filter enabled
	void ActivateFilter(string filterName, bool enabled, bool processFilters = true)
	{
		// Setup filter
		SCR_FilterEntry filter = new SCR_FilterEntry;
		filter.m_sInternalName = filterName;
		filter.SetSelected(true);

		// Register filter
		m_aFiltersToSelect.Insert(filter);

		// Activate filter if possible
		if (processFilters)
			ActivateFiltersFromList();
	}

	//------------------------------------------------------------------------------------------------
	protected void ActivateFiltersFromList()
	{
		// Filters array check
		if (!m_aFiltersToSelect || m_aFiltersToSelect.IsEmpty())
			return;

		// Filter panel check
		if (!m_FilterPanel || !m_ParamsFilter)
			return;

		// Get filter set
		ref SCR_FilterSet filterSet = m_FilterPanel.GetFilter();
		if (!filterSet)
			return;

		// Check and enable each filter
		foreach (SCR_FilterEntry filter : m_aFiltersToSelect)
		{
			if (!filter)
				continue;

			// Find filter to select
			SCR_FilterEntry targetFilter = filterSet.FindFilter(filter.m_sInternalName);

			// Set select if is found
			if (targetFilter && targetFilter.GetSelected() != filter.GetSelected())
			{
				targetFilter.SetSelected(filter.GetSelected());
				m_FilterPanel.SelectFilter(targetFilter, filter.GetSelected());
			}
		}

		// Clear filters
		m_aFiltersToSelect.Clear();

		// Filter
		m_ParamsFilter.SetFilters(filterSet);
		Refresh();
	}

	//------------------------------------------------------------------------------------------------
	//! Separated focus function for later call
	protected void FocusWidget(Widget w)
	{
		GetGame().GetWorkspace().SetFocusedWidget(w);
	}

	//------------------------------------------------------------------------------------------------
	protected void CallOnServerListSetPage(int page)
	{
		GetGame().GetCallqueue().Remove(OnServerListSetPage);
		GetGame().GetCallqueue().CallLater(OnServerListSetPage, ROOM_CONTENT_LOAD_DELAY, false, page);
	}

	//------------------------------------------------------------------------------------------------
	//! Show message feedback by selected message component
	//! showWrap = true will hide server list and display message on full layout
	protected void Messages_ShowMessage(string messageTag, bool showWrap = false)
	{
		Widget wrapLayout = m_Widgets.m_wPanelEmpty;

		// Check message component references
		if (!wrapLayout || !m_SimpleMessageWrap || !m_SimpleMessageList)
		{
			string msg = string.Format("Missing ref - WrapLayout: %1 Wrap: %2, List: %3", wrapLayout, m_SimpleMessageWrap, m_SimpleMessageList);
			PrintDebug(msg, "Messages_ShowMessage");
			return;
		}

		// Show menu content
		if (m_Widgets.m_wContent)
			m_Widgets.m_wContent.SetVisible(!showWrap);

		// Display messages
		wrapLayout.SetVisible(showWrap);
		m_SimpleMessageWrap.GetRootWidget().SetVisible(showWrap);

		m_SimpleMessageList.GetRootWidget().SetVisible(!showWrap);

		// Wrapper
		if (showWrap)
		{
			m_SimpleMessageWrap.SetContentFromPreset(messageTag);
			//return;
		}

		// List
		m_SimpleMessageWrap.SetContentFromPreset(messageTag);
		m_SimpleMessageList.SetContentFromPreset(messageTag);
		
		// Hide footer buttons during full layout mode
		if (m_Widgets.m_RefreshButton)
			m_Widgets.m_RefreshButton.SetVisible(!showWrap, false);
		
		if (m_Widgets.m_DirectJoinButton)
			m_Widgets.m_DirectJoinButton.SetVisible(!showWrap, false);
		
		if (m_Widgets.m_FilterButton)
			m_Widgets.m_FilterButton.SetVisible(!showWrap, false);
			
	}

	//------------------------------------------------------------------------------------------------
	//! Hide both message widgets
	protected void Messages_Hide()
	{
		Widget wrapLayout = m_Widgets.m_wPanelEmpty;

		// Check message component references
		if (!wrapLayout || !m_SimpleMessageWrap || !m_SimpleMessageList)
		{
			string msg = string.Format("Missing ref - WrapLayout: %1 Wrap: %2, List: %3", wrapLayout, m_SimpleMessageWrap, m_SimpleMessageList);
			PrintDebug(msg, "Messages_Hide");
			return;
		}

		// Hide
		m_SimpleMessageWrap.GetRootWidget().SetVisible(false);
		m_SimpleMessageList.GetRootWidget().SetVisible(false);

		wrapLayout.SetVisible(false);

		if (m_Widgets.m_wContent)
			m_Widgets.m_wContent.SetVisible(true);
	}

	//------------------------------------------------------------------------------------------------
	protected void ClearErrorMessage()
	{
		m_sErrorMessage = "";
		m_sErrorMessageGroup = "";
		m_sErrorMessageDetail = "";
	}

	//------------------------------------------------------------------------------------------------
	//! Based on given boolean favorite nav button is displaying eather add or remove favorite
	protected void DisplayFavoriteAction(bool favorited)
	{
		string label = GetFavoriteLabel(favorited);

		if (m_Widgets && m_Widgets.m_FavoritesButton && m_ServerEntry)
			m_Widgets.m_FavoritesButton.SetLabel(label);
	}

	//------------------------------------------------------------------------------------------------
	// CALLBACKS
	//------------------------------------------------------------------------------------------------
	//! Call this when server browser is open by loading component
	//! Start waiting for backend
	protected void OnOpeningByLoadComponent(int menuPreset)
	{
		// Check menu opening
		if (menuPreset == ChimeraMenuPreset.ServerBrowserMenu)
			return;
	}

	//------------------------------------------------------------------------------------------------
	//! Set focus to last filtering element
	protected void OnChangeSort(SCR_SortHeaderComponent sortHeader)
	{
		if (!m_ParamsFilter)
			return;

		// Reload data
		bool sortAscending = sortHeader.GetSortOrderAscending();
		string sortElementName = sortHeader.GetSortElementName();
		m_ParamsFilter.SetSorting(sortElementName, sortAscending);

		if (!m_bFirstRoomLoad)
			Refresh();
	}

	//------------------------------------------------------------------------------------------------
	//! Set tab filters on switching tab view
	void OnTabViewSwitch(SCR_TabViewComponent tabView, Widget w, int id)
	{
		// Default setup
		m_ParamsFilter.SetFavoriteFilter(false);
		m_ParamsFilter.SetRecentlyPlayedFilter(false);
		m_ParamsFilter.SetOwnedOnly(false);
		m_bHostedServers = false;

		m_Widgets.m_wHostNewServerButton.SetVisible(false);

		switch (id)
		{
			// All
			case 0:
			{
				m_ParamsFilter.SetOfficialFilter(false, false);
				break;
			}

			// Community
			case 1:
			{
				m_ParamsFilter.SetOfficialFilter(true, false);
				break;
			}

			// Official
			case 2:
			{
				m_ParamsFilter.SetOfficialFilter(true, true);
				break;
			}

			// Favorite servers
			case 3:
			{
				m_ParamsFilter.SetFavoriteFilter(true);
				m_ParamsFilter.SetOfficialFilter(false, false);
				break;
			}

			// Recently played
			case 4:
			{
				m_ParamsFilter.SetRecentlyPlayedFilter(true);
				m_ParamsFilter.SetOfficialFilter(false, false);
				break;
			}

			// Hostest
			case 5:
			{
				// Widgets
				m_Widgets.m_wHostNewServerButton.SetVisible(true);
				// Call focus later to prevent override from auto widget focus
				GetGame().GetCallqueue().CallLater(FocusWidget, 0, false, m_Widgets.m_wHostNewServerButton);

				// Filters
				m_ParamsFilter.SetOfficialFilter(false, false);
				m_ParamsFilter.SetOwnedOnly(true);
				m_bHostedServers = true;

				break;
			}
		}

		// Set tab filter json
		if (m_ParamsFilter)
			m_ParamsFilter.SetSelectedTab(id);

		if (!m_bFirstRoomLoad)
			Refresh();
		
		Widget focus = GetGame().GetWorkspace().GetFocusedWidget();
		if (!focus || !focus.IsVisible())
			SwitchFocus(SCR_EListMenuWidgetFocus.LIST, true);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFilterPanelToggle(bool show)
	{
		if (!m_FilterPanel)
			return;
		
		// Focus back to server list
		if (show || m_Lobby.TotalRoomCount() == 0)
		{
			SwitchFocus(SCR_EListMenuWidgetFocus.FILTERING);
			return;
		}
			
		// Focus back to last entry
		SwitchFocus(SCR_EListMenuWidgetFocus.LIST);
	}

	//------------------------------------------------------------------------------------------------
	//! Call this when any of filter in filter panel is changed
	//! Get last updated filter and set it up for next search
	protected void OnChangeFilter(SCR_FilterEntry filter)
	{
		// Set filter and refresh
		m_ParamsFilter.SetFilters(m_FilterPanel.GetFilter());
		Refresh();

		// Prevent using cross play filter
		bool hasCrossplay = GetGame().GetPlatformService().GetPrivilege(UserPrivilege.CROSS_PLAY);
		//hasCrossplay = false;

		if (!hasCrossplay)
		{
			// Is crossplay filter
			if (filter.GetCategory().m_sInternalName == "Crossplay")

			if (!m_CallbackGetPrivilege)
				m_CallbackGetPrivilege = new SCR_ScriptPlatformRequestCallback();

			m_CallbackGetPrivilege.m_OnResult.Insert(OnCrossPlayPrivilegeResultFilter);
			NegotiatePrivilegeAsync(UserPrivilege.CROSS_PLAY);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCrossPlayPrivilegeResultFilter(UserPrivilege privilege, UserPrivilegeResult result)
	{
		if (privilege == UserPrivilege.CROSS_PLAY)
		{
			if (result != UserPrivilegeResult.ALLOWED)
			{
				FilterCrossplayCheck();
				Refresh();
			}
		}

		m_CallbackGetPrivilege.m_OnResult.Remove(OnCrossPlayPrivilegeResultFilter);
	}

	//------------------------------------------------------------------------------------------------
	//! Call this actions when server list page is changed
	protected void OnServerListSetPage(int page)
	{
		// Check list
		if (!m_ScrollableList)
			return;

		// Get entries count
		int entriesC = m_ScrollableList.GetPageEntriesCount();
		int pos = entriesC * page;

		// Setup scroll callback
		//m_CallbackScroll.m_OnSuccess.Insert(OnScrollSuccess);
		m_CallbackScroll.GetEventOnResponse().Insert(OnScrollResponse);

		array<Room> rooms = {};
		m_Lobby.Rooms(rooms);
		m_Lobby.Scroll(pos, m_CallbackScroll);
	}

	//------------------------------------------------------------------------------------------------
	//! Call this when scroll returns server response to handle actions
	protected void OnScrollResponse(SCR_BackendCallback callback)
	{
		switch (callback.GetResponseType())
		{
			case EBackendCallbackResponse.SUCCESS:
			{
				OnScrollSuccess(callback);
				break;
			}

			case EBackendCallbackResponse.ERROR:
			{
				PrintDebug("Scroll error!", "OnScrollResponse");
				break;
			}

			case EBackendCallbackResponse.TIMEOUT:
			{
				PrintDebug("Scroll timeout!", "OnScrollResponse");
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnScrollSuccess(SCR_BackendCallback callback)
	{
		// Check if loaded room should display details
		bool loadedRoomFocused = false;
		if (m_ServerEntry && !m_ScrollableList.IsRoomLoaded(m_ServerEntry))
			loadedRoomFocused = true;

		// Update rooms
		m_Lobby.Rooms(m_aRooms);

		if (m_aRooms.IsEmpty())
		{
			PrintDebug("No room found", "OnScrollSuccess");

			if (m_Lobby.TotalRoomCount() > 0)
			{
				if (m_ScrollableList)
					CallOnServerListSetPage(m_ScrollableList.GetCurrentPage());
				else
					Refresh();
			}
		}

		m_ScrollableList.UpdateLoadedPage();

		DisplayRooms(m_aRooms);

		// Focus on new room
		if (loadedRoomFocused)
			ReceiveRoomContent(m_ServerEntry.GetRoomInfo(), true);
	}

	//------------------------------------------------------------------------------------------------
	//! Fetch new servers on confirming search string
	protected void OnSearchEditBoxConfirm(SCR_EditBoxComponent editBox, string sInput)
	{
		m_ParamsFilter.SetSearch(sInput);
		Refresh();
	}

	//------------------------------------------------------------------------------------------------
	//! Favoriting server
	protected void OnRoomEntrySetFavorite(SCR_ListMenuEntryComponent entry, bool favorite)
	{
		SCR_ServerBrowserEntryComponent serverEntry = SCR_ServerBrowserEntryComponent.Cast(entry);
		if(!serverEntry)
			return;
			
		Room roomClicked = serverEntry.GetRoomInfo();
		if (!roomClicked)
			return;

		int roomId = m_aRooms.Find(roomClicked);
		if (roomId == -1)
			return;

		// Setup callback
		m_CallbackFavorite.SetRoom(roomClicked);
		m_CallbackFavorite.GetEventOnResponse().Insert(OnRoomSetFavoriteResponse);

		m_aRooms[roomId].SetFavorite(favorite, m_CallbackFavorite);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRoomSetFavoriteResponse(SCR_RoomCallback callback)
	{
		switch (callback.GetResponseType())
		{
			case EBackendCallbackResponse.SUCCESS:
			{
				if (callback.GetRoom())
				{
					DisplayFavoriteAction(callback.GetRoom().IsFavorite());
					m_OnFavoritesResponse.Invoke();
				}
				break;
			}

			case EBackendCallbackResponse.ERROR: PrintDebug("Error!", "OnRoomSetFavoriteResponse"); break;
			case EBackendCallbackResponse.TIMEOUT: PrintDebug("Timeout!", "OnRoomSetFavoriteResponse"); break;
		}

		// Clear
		m_CallbackFavorite.SetRoom(null);
		m_CallbackFavorite.GetEventOnResponse().Remove(OnRoomSetFavoriteResponse);
	}


	//------------------------------------------------------------------------------------------------
	//! Call this when mods list is received
	protected void OnLoadingDependencyList()
	{
		// Remove data receiving actions
		m_ModsManager.GetOnGetAllDependencies().Remove(OnLoadingDependencyList);

		array<ref SCR_WorkshopItem> updated = m_ModsManager.GetRoomItemsUpdated();
		array<ref SCR_WorkshopItem> outdated = m_ModsManager.GetRoomItemsToUpdate();

		// setup of server detail
		bool modsUpdated = outdated.IsEmpty();

		if (m_ServerScenarioDetails)
			m_ServerScenarioDetails.DisplayMods();
		
		ReceiveRoomContent_Scenario();
	}

	//------------------------------------------------------------------------------------------------
	//! Call this when mods loading got error
	//! Show mods loading fail message
	protected void OnModListFail(Room room)
	{
		if (m_ModListFailDialog)
			return;
		
		m_ModListFailDialog = SCR_CommonDialogs.CreateRequestErrorDialog();
		m_ModListFailDialog.m_OnClose.Insert(OnModListFailDialogClose);
	}
	
	//------------------------------------------------------------------------------------------------\
	protected void OnModListFailDialogClose(SCR_ConfigurableDialogUi dialog)
	{
		m_ModListFailDialog = null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Setting server details
	//! Call this when room receive scenrio data
	protected void OnLoadingScenario(Dependency scenario)
	{
		// Remove action
		m_ModsManager.GetOnGetScenario().Remove(OnLoadingScenario);

		// Set server details
		Room roomInfo = null;

		if (m_ServerEntry)
			roomInfo = m_ServerEntry.GetRoomInfo();

		if (!roomInfo)
			return;
		// Get scenario item
		MissionWorkshopItem scenarioItem = roomInfo.HostScenario();
		SCR_AddonManager mgr = SCR_AddonManager.GetInstance();

		// Hide scneario img if scenario if modded client can't see UGC
		Dependency scenarioMod = roomInfo.HostScenarioMod();
		bool hideScenario = scenarioMod && !mgr.GetUgcPrivilege();

		// Display room and scenario data
		if (m_ServerScenarioDetails)
		{
			// Hide scenario image
			m_ServerScenarioDetails.SetHideScenarioImg(hideScenario);

			if (scenarioItem)
			{
				m_ServerScenarioDetails.SetScenario(scenarioItem);
			}
			else
				m_ServerScenarioDetails.DisplayDefaultScenarioImage();
		}

		//! Update Details Dialog
		if (m_Dialogs)
			m_Dialogs.UpdateRoomDetailsScenarioImage(scenarioItem);

		m_RoomScenario = scenarioItem;

		m_OnScenarioLoad.Invoke(m_RoomScenario);
	}

	//------------------------------------------------------------------------------------------------
	//!  Call this on rejoin dialog cancel
	protected void OnRejoinCancel()
	{
		if (m_Dialogs.GetCurrentKickDialog())
		{
			m_Dialogs.GetOnConfirm().Remove(OnLastRoomReconnectConfirm);
			m_Dialogs.GetOnCancel().Remove(OnRejoinCancel);
		}

		GetGame().GetCallqueue().Remove(OnLastRoomReconnectConfirm);
	}

	//------------------------------------------------------------------------------------------------
	//! Display again old search if servers were overidden by direct join search
	protected void OnDirectJoinCancel()
	{
		// TODO@wernerjak - restore old search
	}

	//------------------------------------------------------------------------------------------------
	// HELPERS
	//------------------------------------------------------------------------------------------------
	protected MissionWorkshopItem MissionFromMod(Dependency scenario)
	{
		if (!scenario)
			return null;

		WorkshopItem item = scenario.GetCachedItem();
		if (!item)
			return null;

		Revision rev = item.GetActiveRevision();
		if (!rev)
			return null;

		array<MissionWorkshopItem> scenarios = {};
		rev.GetScenarios(scenarios);
		if (scenarios.IsEmpty())
			return null;

		for (int i = 0, count = scenarios.Count(); i < count; i++)
		{
			if (scenarios[i].GetOwner() == item)
				return scenarios[i];
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if is possible to attempt to join room
	//! Returns true if room version and platform match to client
	protected bool CanJoinRoom(Room room)
	{
		if (!room)
		{
			PrintDebug("No room found for can join room check", "CanJoinRoom");
			return false;
		}

		// Room to client version check
		bool versionMatch = ClientRoomVersionMatch(room);

		//TODO: check platform restriction

		return versionMatch && !m_ModsManager.HasBlockedMods() && room.Joinable();
	}

	//------------------------------------------------------------------------------------------------
	protected bool ClientRoomVersionMatch(Room room)
	{
		if (!room)
			return false;

		string clientV = GetGame().GetBuildVersion();
		string roomV = room.GameVersion();


		return (clientV == roomV);
	}

	//------------------------------------------------------------------------------------------------
	protected string GetFavoriteLabel(bool isFavorite)
	{
		if (isFavorite)
			return UIConstants.FAVORITE_LABEL_REMOVE;
		else
			return UIConstants.FAVORITE_LABEL_ADD;
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_ServerBrowserEntryComponent GetEntryUnderCursor()
	{
		Widget w = WidgetManager.GetWidgetUnderCursor();

		if (!w)
			return null;

		return SCR_ServerBrowserEntryComponent.Cast(w.FindHandler(SCR_ServerBrowserEntryComponent));
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_ServerBrowserEntryComponent GetSelectedEntry()
	{
		// We are not over a line, use currently focused line
		Widget wfocused = GetGame().GetWorkspace().GetFocusedWidget();
		SCR_ServerBrowserEntryComponent comp;
		if (wfocused)
			comp = SCR_ServerBrowserEntryComponent.Cast(wfocused.FindHandler(SCR_ServerBrowserEntryComponent));

		EInputDeviceType inputDevice = GetGame().GetInputManager().GetLastUsedInputDevice();
		bool isCursorOnInnerButton = m_ServerEntry && m_ServerEntry.IsInnerButtonInteraction();

		if (inputDevice == EInputDeviceType.MOUSE && (GetEntryUnderCursor() || isCursorOnInnerButton))
			return m_ServerEntry;

		return comp;
	}

	//------------------------------------------------------------------------------------------------
	// API
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	void FilterScenarioId(string scenarioId)
	{
		if (m_ParamsFilter)
			m_ParamsFilter.SetScenarioId(scenarioId);
	}

	//------------------------------------------------------------------------------------------------
	void FilterHostedScenarioModId(string scenarioModId)
	{
		if (m_ParamsFilter)
			m_ParamsFilter.SetHostedScenarioModId(scenarioModId);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set to find server by scenario id and scenario source mod id
	static void OpenWithScenarioFilter(string scenarioId, string scenarioModId = string.Empty)
	{
		ServerBrowserMenuUI sb = ServerBrowserMenuUI.Cast(GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.ServerBrowserMenu));
		if (sb)
		{
 			sb.FilterScenarioId(scenarioId);

			// Modded
			if (scenarioModId != string.Empty)
				sb.FilterHostedScenarioModId(scenarioModId);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Set to find server by mission workshop item
	static void OpenWithScenarioFilter(MissionWorkshopItem mission)
	{
		ServerBrowserMenuUI sb = ServerBrowserMenuUI.Cast(GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.ServerBrowserMenu));
		if (!sb)
			return;

		// Scenario id
		string scenarioId = mission.Id();
		sb.FilterScenarioId(scenarioId);

		// Modded
		string modId = "";
		WorkshopItem owner = mission.GetOwner();
		if (owner)
			modId = owner.Id();

		if (modId != string.Empty)
			sb.FilterHostedScenarioModId(modId);
	}

	//------------------------------------------------------------------------------------------------
	//! Set message for error dialog
	//! Error is display if message has text, message is cleared after displaying dialog
	static void SetErrorMessage(string msg, string group, string details = "")
	{
		m_sErrorMessage = msg;
		m_sErrorMessageGroup = group;
		m_sErrorMessageDetail = details;
		
		m_OnErrorMessageSet.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	static bool IsServerPingAboveThreshold(Room room)
	{
		//TODO: unify this check with the value in the filters
		return room && room.GetPing() >= HIGH_PING_SERVER_THRESHOLD;
	}
	
	//------------------------------------------------------------------------------------------------
	// ROOM JOINING PROCESS
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	//! Action for joining to selected server
	protected void JoinActions_Join()
	{
		// Prevent join if no entry selected
		if (!m_ServerEntry)
		{
			PrintDebug("Quick join is not possible because there is no selected server", "JoinActions_Join");
			return;
		}

		// Join
		Room roomToJoin = m_ServerEntry.GetRoomInfo();
		if (roomToJoin)
			JoinProcess_Init(roomToJoin);

		GameSessionStorage.s_Data["m_iRejoinAttempt"] = "0";
	}

	//------------------------------------------------------------------------------------------------
	// Action for finding server on direct join
	protected void JoinActions_DirectJoin(string params, EDirectJoinFormats format, bool publicNetwork)
	{
		JoinProcess_FindRoom(params, format, publicNetwork);
		GameSessionStorage.s_Data["m_iRejoinAttempt"] = "0";
	}

	//------------------------------------------------------------------------------------------------
	//! Confirm action to find new server
	protected void OnLastRoomReconnectConfirm()
	{
		// Clearup
		if (m_Dialogs.GetCurrentKickDialog())
		{
			m_Dialogs.GetOnConfirm().Remove(OnLastRoomReconnectConfirm);
			m_Dialogs.GetOnCancel().Remove(OnRejoinCancel);
		}

		GetGame().GetCallqueue().Remove(OnLastRoomReconnectConfirm);

		// Find
		string lastId = m_Lobby.GetPreviousRoomId();
		JoinProcess_FindRoomById(lastId, m_CallbackSearchPreviousRoom);
	}

	//------------------------------------------------------------------------------------------------
	//! Initialize joining process to specific room
	void JoinProcess_FindRoom(string params, EDirectJoinFormats format, bool publicNetwork)
	{
		m_DirectJoinParams = new FilteredServerParams();

		// Format
		switch (format)
		{
			// IP:PORT
			case EDirectJoinFormats.IP_PORT:
			{
				m_DirectJoinParams.SetHostAddress(params);
				break;
			}

			// Direct join code
			case EDirectJoinFormats.JOIN_CODE:
			{
				m_DirectJoinParams.SetJoinCode(params);
				break;
			}

			// Invalid format
			case EDirectJoinFormats.INVALID:
			{
				PrintDebug("Invalid format of direct join", "JoinProcess_FindRoom");
				return;
			}
		}

		// Set room search
		m_DirectJoinParams.SetUsePlayerLimit(false);
		m_Lobby.SearchTarget(m_DirectJoinParams, m_CallbackSearchTarget);

		// Setup join dialog
		SetupJoinDialogs();
		if (m_Dialogs)
			m_Dialogs.DisplayDialog(EJoinDialogState.SEARCHING_SERVER);

		// Set invokers
		m_CallbackSearchTarget.m_OnSuccess.Insert(JoinProcess_OnFindRoomSuccess);
		m_CallbackSearchTarget.m_OnFail.Insert(JoinProcess_OnFindRoomFail);
		m_CallbackSearchTarget.m_OnTimeOut.Insert(JoinProcess_OnFindRoomTimeout);
	}

	//------------------------------------------------------------------------------------------------
	//! Call this when room is found
	protected void JoinProcess_OnFindRoomSuccess(ServerBrowserCallback callback)
	{
		m_aDirectFoundRooms = m_CallbackSearchTarget.GetFoundRooms();
		if (m_aDirectFoundRooms.IsEmpty())
		{
			PrintDebug("No room found through direct join!", "JoinProcess_OnFindRoomSuccess");
			return;
		}

		// Check if room was found
		if (!m_aDirectFoundRooms || m_aDirectFoundRooms.Count() < 1)
		{
			if (m_Dialogs)
				m_Dialogs.DisplayDialog(EJoinDialogState.SERVER_NOT_FOUND);

			return;
		}

		if (m_aDirectFoundRooms.Count() == 1)
		{
			// Check content of specific server
			m_ModsManager.Clear();

			m_RoomToJoin = m_aDirectFoundRooms[0];
			JoinProcess_Init(m_RoomToJoin);
		}
		else
		{
			// Show multiple rooms in list - fallback logic, shouldn't happend in relased game
			OnRoomsFound(m_aDirectFoundRooms);
		}

		// Clear direct join callback
		JoinProcess_CleanFindCallback();
	}

	//------------------------------------------------------------------------------------------------
	//! Call this if room wasn't found because of error or time out
	protected void JoinProcess_OnFindRoomFail(ServerBrowserCallback callback, int code, int restCode, int apiCode)
	{
		if (m_Dialogs)
			m_Dialogs.DisplayDialog(EJoinDialogState.SERVER_NOT_FOUND);

		JoinProcess_CleanFindCallback();
	}

	//------------------------------------------------------------------------------------------------
	//! Call this if joining to room has timeout
	protected void JoinProcess_OnFindRoomTimeout(ServerBrowserCallback callback)
	{
		if (m_Dialogs)
			m_Dialogs.DisplayDialog(EJoinDialogState.SERVER_NOT_FOUND);

		JoinProcess_CleanFindCallback();
	}

	//------------------------------------------------------------------------------------------------
	protected void JoinProcess_CleanFindCallback()
	{
		m_CallbackSearchTarget.m_OnSuccess.Remove(JoinProcess_OnFindRoomSuccess);
		m_CallbackSearchTarget.m_OnFail.Remove(JoinProcess_OnFindRoomFail);
		m_CallbackSearchTarget.m_OnTimeOut.Remove(JoinProcess_OnFindRoomTimeout);
	}

	//------------------------------------------------------------------------------------------------
	//! Specific call for id search
	protected void JoinProcess_FindRoomById(string id, ServerBrowserCallback callback)
	{
		// Setup ID
		m_SearchIds.ClearIds();
		m_SearchIds.RegisterId(id);

		// Search
		m_Lobby.GetRoomsByIds(m_SearchIds, callback);
		callback.event_OnResponse.Insert(JoinProcess_OnFindRoomByIdResponse);

		// Setup join dialog
		SetupJoinDialogs();
		if (m_Dialogs)
			m_Dialogs.DisplayDialog(EJoinDialogState.SEARCHING_SERVER);
	}

	//------------------------------------------------------------------------------------------------
	//! Reaction for rooms found by id response
	protected void JoinProcess_OnFindRoomByIdResponse(ServerBrowserCallback callback)
	{
		switch (callback.GetResultType())
		{
			// Success
			case EServerBrowserRequestResult.SUCCESS:
			{
				m_Lobby.Rooms(m_aDirectFoundRooms);

				// No rooms received
				if (m_aDirectFoundRooms.IsEmpty())
				{
					JoinProcess_OnFindRoomFail(null, -1, -1, -1);
					break;
				}

				// Get first room
				m_RoomToJoin = m_aDirectFoundRooms[0];
				m_RejoinRoom = m_RoomToJoin;
				JoinProcess_Init(m_RoomToJoin);
				break;
			}

			// Error
			case EServerBrowserRequestResult.ERROR:
			{
				JoinProcess_OnFindRoomFail(null, -1, -1, -1);
				break;
			}

			// Timeout
			case EServerBrowserRequestResult.TIMEOUT:
			{
				JoinProcess_OnFindRoomFail(null, -1, -1, -1);
				break;
			}
		}

		// Clear
		callback.event_OnResponse.Remove(JoinProcess_OnFindRoomByIdResponse);
	}

	//------------------------------------------------------------------------------------------------
	//! Initialize joining process to specific room
	void JoinProcess_Init(Room roomToJoin)
	{
		if (!m_DownloadManager)
			m_DownloadManager = SCR_DownloadManager.GetInstance();
		
		// Set immidiate refresh to receive player count
		//m_Lobby.SetRefreshRate(1);
		m_bWaitForRefresh = true;

		// Setup join dialog
		SetupJoinDialogs();

		// Setup room
		m_RoomToJoin = roomToJoin;

		// Check cross play
		bool hasPrivilege = GetGame().GetPlatformService().GetPrivilege(UserPrivilege.CROSS_PLAY);

		if (roomToJoin.IsCrossPlatform() && !hasPrivilege)
		{
			//m_Dialogs.DisplayDialog(EJoinDialogState.MOD_UGC_PRIVILEGE_MISSING);

			// Negotatiate cross play privilege
			if (!m_CallbackGetPrivilege)
				m_CallbackGetPrivilege = new SCR_ScriptPlatformRequestCallback();

			m_CallbackGetPrivilege.m_OnResult.Insert(OnCrossPlayPrivilegeResultJoin);
			NegotiatePrivilegeAsync(UserPrivilege.CROSS_PLAY);
			return;
		}

		//Quick Join: Next step check version
		if (m_bQuickJoin)
			JoinProcess_CheckVersion(m_RoomToJoin);
		//Single click Join: Next step check password
		else
			JoinProcess_CheckRoomPasswordProtected(m_RoomToJoin);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCrossPlayPrivilegeResultJoin(UserPrivilege privilege, UserPrivilegeResult result)
	{
		if (privilege == UserPrivilege.CROSS_PLAY)
		{
			if (result == UserPrivilegeResult.ALLOWED)
				JoinProcess_CheckVersion(m_RoomToJoin);
		}

		m_CallbackGetPrivilege.m_OnResult.Remove(OnCrossPlayPrivilegeResultJoin);
	}

	//------------------------------------------------------------------------------------------------
	//! Check if client and room versions match
	//! Progress only if versions are matching, otherwise show error and stop progress
	protected void JoinProcess_CheckVersion(Room roomToJoin)
	{
		// Chekc match
		bool versionsMatch = ClientRoomVersionMatch(roomToJoin);

#ifdef SB_DEBUG
#else
		// Stop join process with error dialog with wrong version
		if (!versionsMatch)
		{
			m_Dialogs.SetJoinRoom(roomToJoin);
			m_Dialogs.DisplayDialog(EJoinDialogState.VERSION_MISMATCH);
			return;
		}
#endif

		// Check room password protection
		JoinProcess_CheckHighPing(roomToJoin);
	}

	//------------------------------------------------------------------------------------------------
	//! Display a warning dialog if the player chose to join a server with high ping
	protected void JoinProcess_CheckHighPing(Room roomToJoin)
	{
		if (IsServerPingAboveThreshold(roomToJoin))
		{
			m_Dialogs.SetJoinRoom(roomToJoin);
			m_Dialogs.DisplayDialog(EJoinDialogState.HIGH_PING_SERVER);
			m_Dialogs.GetOnConfirm().Insert(OnHighPingServerWarningDialogConfirm);
			
			return;
		}
		
		JoinProcess_CheckRoomPasswordProtected(roomToJoin);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnHighPingServerWarningDialogConfirm()
	{
		m_Dialogs.GetOnConfirm().Remove(OnHighPingServerWarningDialogConfirm);
		JoinProcess_CheckRoomPasswordProtected(m_Dialogs.GetJoinRoom());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if room requires password to join
	protected void JoinProcess_CheckRoomPasswordProtected(Room roomToJoin)
	{
		// Skip password if using direct join code or is invited
		bool skipPassword;

		if (m_Lobby.GetInviteRoom() == m_RoomToJoin || m_RejoinRoom)
			skipPassword = m_RoomToJoin.IsAuthorized();

		// Next step if no password protection
		if (!m_RoomToJoin.PasswordProtected() || skipPassword)
		{
			JoinProcess_ShowJoinDetailsDialog(roomToJoin);
			return;
		}

		// Open password dialog
		JoinProcess_PasswordDialogOpen();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRejoinAuthorizationFailed(string message)
	{
		m_PasswordVerification.GetOnFailVerification().Remove(OnRejoinAuthorizationFailed);
		
		m_Dialogs.CloseCurrentDialog();
		JoinProcess_PasswordClearInvokers();
		GetGame().GetCallqueue().CallLater(JoinProcess_PasswordDialogOpen);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void JoinProcess_PasswordDialogOpen()
	{
		m_Dialogs.DisplayDialog(EJoinDialogState.PASSWORD_REQUIRED);
		m_Dialogs.GetOnCancel().Insert(JoinProcess_PasswordClearInvokers);

		m_PasswordVerification.SetupDialog(m_Dialogs.GetCurrentDialog(), m_RoomToJoin);
		m_PasswordVerification.GetOnVerified().Insert(OnPasswordVerified);
		m_PasswordVerification.GetOnFailVerification().Insert(OnPasswordFailVerification);
	}

	//------------------------------------------------------------------------------------------------
	//! On successfull room password verification continue to cotent handling
	protected void OnPasswordVerified(Room room)
	{
		JoinProcess_PasswordClearInvokers();

		if (!room)
		{
			#ifdef WORKBENCH
				Print("ServerBrowserMenuUI - OnPasswordVerified() - NULL ROOM");
			#endif
			
			return;
		}
		
		m_RoomToJoin = room;
		JoinProcess_ShowJoinDetailsDialog(m_RoomToJoin);
		
		ReceiveRoomContent(m_RoomToJoin, true);
	}

	//------------------------------------------------------------------------------------------------
	//! On room password verication fail restart attemp
	protected void OnPasswordFailVerification(string message)
	{
		m_Dialogs.DisplayDialog(EJoinDialogState.PASSWORD_REQUIRED);
		m_PasswordVerification.SetupDialog(m_Dialogs.GetCurrentDialog(), m_RoomToJoin, "#AR-Password_FailMessage");
	}

	//------------------------------------------------------------------------------------------------
	protected void JoinProcess_PasswordClearInvokers()
	{
		m_PasswordVerification.GetOnVerified().Remove(OnPasswordVerified);
		m_PasswordVerification.GetOnFailVerification().Remove(OnPasswordFailVerification);
		m_Dialogs.GetOnCancel().Remove(JoinProcess_PasswordClearInvokers);
	}

	//------------------------------------------------------------------------------------------------
	protected void JoinProcess_ShowJoinDetailsDialog(Room roomToJoin)
	{
		// Skip the dialog on double click
		if (m_bQuickJoin)
		{
			JoinProcess_LoadModContent(roomToJoin);
			return;
		}

		CreateServerDetailsDialog(roomToJoin);
	}
	
	//------------------------------------------------------------------------------------------------
	void CreateServerDetailsDialog(Room roomToJoin)
	{
		if (!roomToJoin)
			return;

		// Create details dialog
		array<ref SCR_WorkshopItem> items = {};
		array<Dependency> dependencies = {};
		roomToJoin.GetItems(dependencies);

		if (m_ModsManager.GetRoomItemsScripted().Count() == dependencies.Count())
			items = m_ModsManager.GetRoomItemsScripted();

		SCR_ServerDetailsDialog serverDetails = m_Dialogs.CreateServerDetailsDialog(roomToJoin, items, m_OnFavoritesResponse);
		serverDetails.SetCanJoin(CanJoinRoom(roomToJoin));
		serverDetails.m_OnFavorites.Insert(OnActionFavorite);

		bool loaded = roomToJoin.IsDownloadListLoaded();

		if (!dependencies.IsEmpty())
		{
			//Fill with last loaded mod list
			if (!roomToJoin.IsDownloadListLoaded())
				m_ModsManager.GetOnGetAllDependencies().Insert(OnServerDetailModsLoaded);
			else
				OnServerDetailModsLoaded();
		}
		else
		{
			// Fill with emtpy data
			m_Dialogs.FillRoomDetailsMods({});
		}

		m_Dialogs.GetCurrentDialog().m_OnConfirm.Insert(JoinProcess_LoadModContent);
		m_Dialogs.GetCurrentDialog().m_OnClose.Insert(OnServerDetailsClosed);
	}

	//------------------------------------------------------------------------------------------------
	//! Call this to fill details mods list
	protected void OnServerDetailModsLoaded()
	{
		m_Dialogs.FillRoomDetailsMods(m_ModsManager.GetRoomItemsScripted(), m_ModsManager);
		m_ModsManager.GetOnGetAllDependencies().Remove(OnServerDetailModsLoaded);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnServerDetailsClosed()
	{
		m_ModsManager.GetOnGetAllDependencies().Remove(OnServerDetailModsLoaded);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Load data about room mods to check which data is client missing
	//! Show state if mods are already loaded or wait for receiving mods data
	protected void JoinProcess_LoadModContent(Room roomToJoin)
	{
		// Check references
		if (!m_Dialogs || !m_ModsManager)
			return;

		// Check mods use privilege - UGC privilege
		SCR_AddonManager mgr = SCR_AddonManager.GetInstance();
		array<Dependency> deps = {};

		m_RoomToJoin.GetItems(deps);

		if (!mgr.GetUgcPrivilege() && !deps.IsEmpty())
		{
			m_Dialogs.DisplayDialog(EJoinDialogState.MOD_UGC_PRIVILEGE_MISSING);

			// Negotatiate UGC privilege
			mgr.m_OnUgcPrivilegeResult.Insert(Platform_OnUgcPrivilegeResult);
			mgr.NegotiateUgcPrivilegeAsync();
			return;
		}

		// Show state of mods if all loaded
		JoinProcess_LoadModContentVisualize();
	}

	//------------------------------------------------------------------------------------------------
	protected void Platform_OnUgcPrivilegeResult(bool result)
	{
		SCR_AddonManager mgr = SCR_AddonManager.GetInstance();
		mgr.m_OnUgcPrivilegeResult.Remove(Platform_OnUgcPrivilegeResult);
		if (result)
			JoinProcess_LoadModContentVisualize();
	}

	//------------------------------------------------------------------------------------------------
	protected void JoinProcess_LoadModContentVisualize()
	{		
		// Show state of mods if all loaded
		if (m_RoomToJoin.IsDownloadListLoaded())
		{
			//SolveJoiningCases(); //TODO@wernerjak - solve joining cases function clearing
			JoinProcess_CheckModContent();
			return;
		}

		// Show mods loading dialog
		if (m_Dialogs)
			m_Dialogs.DisplayDialog(EJoinDialogState.CHECKING_CONTENT);

		// Set wait for loading
		m_ModsManager.GetOnGetAllDependencies().Insert(JoinProcess_CheckModContent);
		m_ModsManager.GetOnModsFail().Insert(JoinProcess_OnModCheckFailed);
		m_ModsManager.GetOnDependenciesLoadingPrevented().Insert(OnDependenciesLoadingPrevented);

		m_ModsManager.ReceiveRoomMods(m_RoomToJoin);
	}

	//------------------------------------------------------------------------------------------------
	protected void JoinProcess_OnModCheckFailed(Room room)
	{
		m_Dialogs.CloseCurrentDialog();
		OnModListFail(room);
		
		m_ModsManager.GetOnGetAllDependencies().Remove(JoinProcess_CheckModContent);
		m_ModsManager.GetOnModsFail().Remove(JoinProcess_OnModCheckFailed);
		m_ModsManager.GetOnDependenciesLoadingPrevented().Remove(OnDependenciesLoadingPrevented);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDependenciesLoadingPrevented(Room room, array<ref SCR_WorkshopItem> dependencies)
	{
		m_Dialogs.CloseCurrentDialog();
		
		if (room != m_RoomToJoin)
			return;
	
		JoinProcess_CheckModContent();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check state of room required mods on client
	//! Show missing mods data to download or move to next step
	//! Stop progess if room mods are restricted
	protected void JoinProcess_CheckModContent()
	{
		m_Dialogs.CloseCurrentDialog();
		m_Dialogs.SetJoinRoom(m_RoomToJoin);

		// Remove mods check actions
		m_ModsManager.GetOnGetAllDependencies().Remove(JoinProcess_CheckModContent);
		m_ModsManager.GetOnModsFail().Remove(JoinProcess_OnModCheckFailed);
		m_ModsManager.GetOnDependenciesLoadingPrevented().Remove(OnDependenciesLoadingPrevented);
		
		// Restricted content check
		array<ref SCR_WorkshopItem> items = m_ModsManager.GetRoomItemsScripted();

		array<ref SCR_WorkshopItem> restricedMods = SCR_AddonManager.SelectItemsOr(items, EWorkshopItemQuery.RESTRICTED);
		bool restricted = restricedMods.Count() > 0;

		// Stop join if there are restricted mods
		if (restricted)
		{
			SCR_ReportedAddonsDialog dialog = m_ModsManager.DisplayRestrictedAddonsList();
			dialog.GetOnAllReportsCanceled().Insert(JoinProcess_LoadModContentVisualize);
			return;
		}

		// Get necessary downloads
		m_aRequiredMods = SCR_AddonManager.SelectItemsBasic(items, EWorkshopItemQuery.NOT_LOCAL_VERSION_MATCH_DEPENDENCY);

		// If no downloads are required proceed with the joining process, otherwise stop unrelated downloads and get the required addons to join
		if (!m_aRequiredMods.IsEmpty())
		{
			m_aUnrelatedDownloads = m_DownloadManager.GetUnrelatedDownloads(items);
			
			if (!m_aUnrelatedDownloads.IsEmpty())
				JoinProcess_DisplayUnrelatedDownloadsWarning();
			else
				JoinProcess_DownloadRequiredMods();
			
			return;
		}
		
		JoinProcess_CheckRunningDownloads();
	}
	
	//------------------------------------------------------------------------------------------------
	// Display a dialog asking for unrelated download stop confirmation (this includes wrong versions of required mods)
	protected void JoinProcess_DisplayUnrelatedDownloadsWarning()
	{	
		m_Dialogs.DisplayJoinDownloadsWarning(m_aUnrelatedDownloads, SCR_EJoinDownloadsConfirmationDialogType.UNRELATED);
		m_Dialogs.GetOnConfirm().Insert(JoinProcess_StopDownloadingUnrelatedMods);
	}
	
	//------------------------------------------------------------------------------------------------
	// Pause existing downloads that are unrelated to the specific server we want to join
	protected void JoinProcess_StopDownloadingUnrelatedMods()
	{	
		m_Dialogs.GetOnConfirm().Remove(JoinProcess_StopDownloadingUnrelatedMods);
		
		if (!m_DownloadManager || m_DownloadManager.GetDownloadQueue().IsEmpty())
		{
			OnAllUnrelatedDownloadsStopped();
			return;
		}

		// Stop downloads		
		m_DownloadManager.GetOnAllDownloadsStopped().Insert(OnAllUnrelatedDownloadsStopped);

		foreach (SCR_WorkshopItemActionDownload download : m_aUnrelatedDownloads)
		{
			download.Cancel();
		}
		
		// Display filler dialog
		m_Dialogs.DisplayDialog(EJoinDialogState.UNRELATED_DOWNLOADS_CANCELING);
		
		//TODO: pause downloads instead of clearing them, and allow the player to resume them once out of multiplayer games
		//TODO: give the option to keep downloading while playing multiplayer?
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnAllUnrelatedDownloadsStopped()
	{
		if (m_DownloadManager)
			m_DownloadManager.GetOnAllDownloadsStopped().Remove(OnAllUnrelatedDownloadsStopped);
		
		// Must call the next frame or the download will instantly fail
		GetGame().GetCallqueue().CallLater(JoinProcess_CloseUnrelatedDownloadsCancelingDialog);
	}

	//------------------------------------------------------------------------------------------------
	protected void JoinProcess_CloseUnrelatedDownloadsCancelingDialog()
	{
		m_Dialogs.GetCurrentDialog().Close();
		JoinProcess_DownloadRequiredMods();
	}
	
	//------------------------------------------------------------------------------------------------
	// Start downloading necessary mods for the server
	protected void JoinProcess_DownloadRequiredMods()
	{
		if (m_DownloadManager)
			m_DownloadManager.DownloadItems(m_aRequiredMods);
		
		m_Dialogs.DisplayJoinDownloadsWarning(m_DownloadManager.GetDownloadQueue(), SCR_EJoinDownloadsConfirmationDialogType.REQUIRED);
		
		// Display Download Manager dialog
		m_Dialogs.GetOnDownloadComplete().Insert(JoinProcess_OnJoinRoomDemand);
		m_Dialogs.GetOnCancel().Insert(OnDownloadRequiredModsCancel);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDownloadRequiredModsCancel()
	{
		m_Dialogs.GetOnCancel().Remove(OnDownloadRequiredModsCancel);
		
		if (m_DownloadManager)
		{
			m_DownloadManager.EndAllDownloads();
			m_DownloadManager.ClearFailedDownloads();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void JoinProcess_OnJoinRoomDemand(Room roomToJoin)
	{
		m_Dialogs.GetOnDownloadComplete().Remove(JoinProcess_OnJoinRoomDemand);
		
		m_RoomToJoin = roomToJoin;
		JoinProcess_CheckRunningDownloads();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Final step: display a warning dialog if there are still downloads running, as these will be stopped
	protected void JoinProcess_CheckRunningDownloads()
	{
		int nCompleted, nTotal;
		SCR_DownloadManager mgr = SCR_DownloadManager.GetInstance();

		if (m_DownloadManager)
			m_DownloadManager.GetDownloadQueueState(nCompleted, nTotal);

		if (nTotal <= 0)
		{
			JoinProcess_Join();
			return;
		}
		
		m_Dialogs.DisplayJoinDownloadsWarning(m_DownloadManager.GetDownloadQueue(), SCR_EJoinDownloadsConfirmationDialogType.REQUIRED);
		m_Dialogs.GetOnConfirm().Insert(OnInterruptDownloadConfirm);

	}

	//------------------------------------------------------------------------------------------------
	protected void OnInterruptDownloadConfirm()
	{
		if (!m_RoomToJoin)
		{
			PrintDebug("Missing m_RoomToJoin!", "OnInteruptDownloadConfirm");
			return;
		}

		// TODO: pause and cache instead of canceling
		// Cancel downloading
		SCR_DownloadManager mgr = SCR_DownloadManager.GetInstance();
		if (mgr)
			mgr.EndAllDownloads();

		JoinProcess_Join();
	}
	
	//------------------------------------------------------------------------------------------------
	// Join ...Finally!
	protected void JoinProcess_Join()
	{
		// Add join callbacks
		if (!m_CallbackJoin)
			m_CallbackJoin = new ServerBrowserCallback;
		m_CallbackJoin.m_OnSuccess.Insert(JoinProcess_OnJoinSuccess);
		m_CallbackJoin.m_OnFail.Insert(JoinProcess_OnJoinFail);
		m_CallbackJoin.m_OnTimeOut.Insert(JoinProcess_OnJoinTimeout);
		
		// Join server
		m_RoomToJoin.Join(m_CallbackJoin, m_JoinData);
		
		if (m_Dialogs)
			m_Dialogs.DisplayDialog(EJoinDialogState.JOIN);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this if joining to room was successful
	//! Connect to the server
	protected void JoinProcess_OnJoinSuccess(ServerBrowserCallback callback)
	{
		// Connect - gathers mods needed and calls for a reload
		if (!GameStateTransitions.RequestConnectViaRoom(m_RoomToJoin))
			return; // Transition is not guaranteed

		// Save menu to reopen
		SCR_MenuLoadingComponent.SaveLastMenu(ChimeraMenuPreset.ServerBrowserMenu);

		// Cleanups - TODO@wernerjak - remove this cleaning to on function with other cleaning?
		m_RoomToJoin = null;
		GetGame().GetMenuManager().CloseAllMenus();
		GetGame().GetBackendApi().GetWorkshop().Cleanup();

		JoinProcess_Clear();
	}

	//------------------------------------------------------------------------------------------------
	protected void JoinProcess_OnJoinFail(ServerBrowserCallback callback, int code, int restCode, int apiCode)
	{
		JoinProcess_Clear();
		
		if (!m_Dialogs)
			return;
		
		// Server is full
		if (apiCode == EApiCode.EACODE_ERROR_MP_ROOM_IS_FULL)
		{
			m_Dialogs.SetJoinRoom(m_RoomToJoin);
			m_Dialogs.GetOnJoinRoomDemand().Insert(JoinProcess_OnJoinRoomDemand);
				
			m_Dialogs.DisplayDialog(EJoinDialogState.QUEUE_WAITING);
			GetGame().GetBackendApi().GetClientLobby().SetRefreshCallback(m_CallbackAutoRefresh);
			
			return;
		}
		
		// Banned
		if (!m_JoinData.scope.IsEmpty())
			m_Dialogs.DisplayJoinBan(m_JoinData);
		// Generic
		else
			m_Dialogs.DisplayJoinFail(apiCode);
	}

	//------------------------------------------------------------------------------------------------
	//! Call this if joining to room has timeout
	protected void JoinProcess_OnJoinTimeout(ServerBrowserCallback callback)
	{
		JoinProcess_Clear();
		SCR_CommonDialogs.CreateTimeoutOkDialog();
	}

	//------------------------------------------------------------------------------------------------
	//! Call this to kill joining process at any stage
	protected void JoinProcess_Clear()
	{
		m_CallbackJoin = null;
		
		ClientLobbyApi lobby = GetGame().GetBackendApi().GetClientLobby();
		lobby.ClearInviteRoom();

		// Clear mods manager callbacks
		m_ModsManager.GetOnGetAllDependencies().Clear();
		m_ModsManager.GetOnModsFail().Clear();
		m_ModsManager.GetOnDependenciesLoadingPrevented().Clear();
		m_ModsManager.GetOnGetScenario().Clear();

		// Clear dialog
		m_Dialogs.GetOnConfirm().Clear();
		m_Dialogs.GetOnCancel().Clear();
		m_Dialogs.GetOnDialogClose().Clear();
		m_Dialogs.GetOnDownloadComplete().Clear();
		m_Dialogs.GetOnDownloadCancelDialogClose().Clear();
		m_Dialogs.GetOnJoinRoomDemand().Clear();
		
		// TODO: remove specific subscriptions instead of clearing!
	}
	
	//------------------------------------------------------------------------------------------------
	//! Open joining dialog in default state
	protected void SetupJoinDialogs()
	{
		// Check dialog
		if (!m_Dialogs)
			return;

		// Setup basics
		m_Dialogs.SetServerBrowser(this);
		m_Dialogs.SetModManager(m_ModsManager);

		// Invokers
		m_Dialogs.GetOnDownloadComplete().Clear();
		m_Dialogs.GetOnCancel().Insert(JoinProcess_Clear);

		m_CallbackLastSearch = null;
	}

	//------------------------------------------------------------------------------------------------
	//! PRIVILEGES HANDLING
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	//! Attempt to allow MP privilege
	protected void NegotiateMPPrivilegeAsync()
	{
		if (!m_CallbackGetPrivilege)
		{
			m_CallbackGetPrivilege = new SCR_ScriptPlatformRequestCallback();
			m_CallbackGetPrivilege.m_OnResult.Insert(OnMPPrivilegeResult);
		}

		GetGame().GetPlatformService().GetPrivilegeAsync(UserPrivilege.MULTIPLAYER_GAMEPLAY, m_CallbackGetPrivilege);
	}

	//------------------------------------------------------------------------------------------------
	//! Attempt to enable selected privilege
	protected void NegotiatePrivilegeAsync(UserPrivilege privilege)
	{
		if (!m_CallbackGetPrivilege)
			m_CallbackGetPrivilege = new SCR_ScriptPlatformRequestCallback();

		switch (privilege)
		{
			case UserPrivilege.MULTIPLAYER_GAMEPLAY:
			{
				m_CallbackGetPrivilege.m_OnResult.Insert(OnMPPrivilegeResult);
				break;
			}

			case UserPrivilege.CROSS_PLAY:
			{
				m_CallbackGetPrivilege.m_OnResult.Insert(OnCrossPlayPrivilegeResult);
				break;
			}
		}

		GetGame().GetPlatformService().GetPrivilegeAsync(privilege, m_CallbackGetPrivilege);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMPPrivilegeResult(UserPrivilege privilege, UserPrivilegeResult result)
	{
		// Sucessful
		if (privilege == UserPrivilege.MULTIPLAYER_GAMEPLAY && result == UserPrivilegeResult.ALLOWED)
			Refresh();

		m_CallbackGetPrivilege.m_OnResult.Remove(OnMPPrivilegeResult);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCrossPlayPrivilegeResult(UserPrivilege privilege, UserPrivilegeResult result)
	{
		// Sucessful
		if (privilege == UserPrivilege.CROSS_PLAY && result == UserPrivilegeResult.ALLOWED)
			Refresh();

		m_CallbackGetPrivilege.m_OnResult.Remove(OnCrossPlayPrivilegeResult);
	}

	//------------------------------------------------------------------------------------------------
	// DEBUG
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	//! Custom debug print displayed with -scrDefine=SB_DEBUG argument
	protected void PrintDebug(string msg, string functionName = string.Empty)
	{
		#ifdef SB_DEBUG
		//Setup function format
		string fncStr = string.Empty;
		if (functionName != string.Empty)
			fncStr = string.Format(" Fnc: %1()", functionName);

		// Display message
		PrintFormat("[ServerBrowserMenuUI]%1 -- Msg: %2", fncStr, msg);
		#endif
	}
}

//------------------------------------------------------------------------------------------------
//! Overrided GetRoomsIds class to manipulation in script
class SCR_GetRoomsIds extends GetRoomsIds
{
	protected ref array<string> roomIds = {};

	//------------------------------------------------------------------------------------------------
	void RegisterId(string id)
	{
		roomIds.Insert(id);
		RegV("roomIds");
	}

	//------------------------------------------------------------------------------------------------
	void ClearIds()
	{
		roomIds.Clear();
	}
}
