/*!
Class for handling server browser dialogs.
Server browser use multiple dialogs in joining process for checking servers, mod content and restrictinos.
*/

//------------------------------------------------------------------------------------------------
class SCR_ServerBrowserDialogManager
{
	// Resources
	const ResourceName CONFIG_DIALOGS = "{471EFCF445C3E9C6}Configs/ServerBrowser/JoiningDialogs.conf";
	const ResourceName CONFIG_DIALOGS_ERROR = "{D3BFEE28E7D5B6A1}Configs/ServerBrowser/KickDialogs.conf";
	protected const string OFFICIAL_SERVER_SCOPE = "officialServer";

	// Dialog tags references
	protected const string TAG_SEARCHING_SERVER 				= "SEARCHING_SERVER";
	protected const string TAG_JOIN 							= "JOIN";
	protected const string TAG_REJOIN 							= "REJOIN";
	protected const string TAG_SERVER_NOT_FOUND 				= "SERVER_NOT_FOUND";
	protected const string TAG_VERSION_MISMATCH 				= "VERSION_MISMATCH";
	protected const string TAG_CHECKING_CONTENT 				= "CHECKING_CONTENT";
	protected const string TAG_MOD_UGC_PRIVILEGE_MISSING 		= "MOD_UGC_PRIVILEGE_MISSING";
	protected const string TAG_MODS_DOWNLOADING 				= "MODS_DOWNLOADING";
	protected const string TAG_QUEUE_WAITING 					= "QUEUE_WAITING";
	protected const string TAG_PASSWORD_REQUIRED 				= "PASSWORD_REQUIRED";
	protected const string TAG_BACKEND_TIMEOUT					= "BACKEND_TIMEOUT";
	protected const string TAG_KICK_DEFAULT 					= "DEFAULT_ERROR";
	protected const string TAG_BANNED							= "JOIN_FAILED_BAN";
	protected const string TAG_HIGH_PING_SERVER					= "HIGH_PING_SERVER";
	protected const string TAG_UNRELATED_DOWNLOADS_CANCELING	= "UNRELATED_DOWNLOADS_CANCELING";

	protected const int MAX_AUTO_REJOINS = 3;
	protected const string STR_LIGHT_BAN = "#AR-LightBan";
	protected const string STR_HEAVY_BAN = "#AR-HeavyBan";

	// References
	protected Room m_JoinRoom;
	protected SCR_RoomModsManager m_ModManager;
	protected ServerBrowserMenuUI m_ServerBrowser;

	// States
	protected bool m_bIsOpen = false;
	protected int m_iOpenDialogCount = 0;
	protected EJoinDialogState m_iDisplayState;
	protected SCR_ConfigurableDialogUi m_CurrentDialog;
	protected SCR_ConfigurableDialogUi m_CurrentKickDialog;

	// Join dialog widget handling
	const string WIDGET_IMAGE_ICON = "ImgIcon";
	const string WIDGET_TXT_SUBMSG = "TxtSubMsg";
	const string WIDGET_LOADING = "LoadingCircle0";

	const string WIDGET_PROGRESS = "VProgress";
	const string WIDGET_TXT_PROGRESS = "TxtProgress";
	const string WIDGET_PROGRESS_BAR = "ProgressBar";

	const string WIDGET_BUTTON_ADDITIONAL = "Additional";
	const string ERROR_TAG_DEFAULT = "JOIN_FAILED";

	protected int m_iDownloadedCount = 0;
	protected ref array<ref SCR_WorkshopItemAction> m_JoinDownloadActions = {};

	// Invokers
	protected ref ScriptInvokerVoid m_OnConfirm;
	protected ref ScriptInvokerVoid m_OnCancel;
	protected ref ScriptInvokerVoid m_OnDialogClose;

	protected ref ScriptInvokerRoom m_OnDownloadComplete;
	protected ref ScriptInvokerRoom m_OnJoinRoomDemand;
	protected ref ScriptInvokerVoid Event_OnRejoinTimerOver;
	protected ref ScriptInvokerVoid Event_OnCloseAll;
	protected ref ScriptInvokerVoid m_OnDownloadCancelDialogClose;

	//------------------------------------------------------------------------------------------------
	// Public functions
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! Display dialog by state and setup current state
	void DisplayDialog(EJoinDialogState state)
	{
		// Set
		m_iDisplayState = state;

		//m_bIsOpen = true;
		if (m_CurrentDialog)
			m_CurrentDialog.Close();

		// Visual set
		switch (state)
		{
			// Searching server
			case EJoinDialogState.SEARCHING_SERVER:
				SetDialogByTag(TAG_SEARCHING_SERVER);
				break;

			// Joining
			case EJoinDialogState.JOIN:
				SetDialogByTag(TAG_JOIN);
				break;

			// Joining
			case EJoinDialogState.REJOIN:
			{
				SetDialogByTag(TAG_REJOIN);
				break;
			}

			// Server by given parameters wasn't found
			case EJoinDialogState.SERVER_NOT_FOUND:
				SetDialogByTag(TAG_SERVER_NOT_FOUND);
				break;

			// Wrong version restriction
			case EJoinDialogState.VERSION_MISMATCH:
			{
				SetDialogByTag(TAG_VERSION_MISMATCH);

				// Display version difference text
				string clientV = GetGame().GetBuildVersion();
				string RoomV = m_JoinRoom.GameVersion();

				string msg = string.Format("%1: %2 \n%3: %4", "#AR-ServerBrowser_Server", RoomV, "#AR-Editor_AttributeCategory_GameSettings_Name", clientV);
				m_CurrentDialog.SetMessage(msg);
				break;
			}

			// Checking mod content
			case EJoinDialogState.CHECKING_CONTENT:
				SetDialogByTag(TAG_CHECKING_CONTENT);
				break;

			// Mod to update
			case EJoinDialogState.MODS_TO_UPDATE:
				//DisplayModsToUpdate();
				break;

			// Cleint can't access user generated content
			case EJoinDialogState.MOD_UGC_PRIVILEGE_MISSING:
				SetDialogByTag(TAG_MOD_UGC_PRIVILEGE_MISSING);
				break;

			// Queue waiting
			case EJoinDialogState.QUEUE_WAITING:
				DisplayWaitingQueue();
				break;

			// Password input dialog
			case EJoinDialogState.PASSWORD_REQUIRED:
			{
				DisplayPasswordRequired();
				break;
			}

			// Password input dialog
			case EJoinDialogState.BACKEND_TIMEOUT:
			{
				SetDialogByTag(TAG_BACKEND_TIMEOUT);
				break;
			}
			
			// High ping
			case EJoinDialogState.HIGH_PING_SERVER:
			{
				SetDialogByTag(TAG_HIGH_PING_SERVER);
				break;
			}
			
			// Unrelated Downloads canceling filler dialog
			case EJoinDialogState.UNRELATED_DOWNLOADS_CANCELING:
			{
				SetDialogByTag(TAG_UNRELATED_DOWNLOADS_CANCELING);
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Create and setup error dialog when ending multiplayer game
	void DisplayKickErrorDialog(string tag, string group, string strDetail)
	{
		SCR_ConfigurableDialogUi dialogUi = SCR_ConfigurableDialogUi.CreateFromPreset(CONFIG_DIALOGS_ERROR, tag);

		// Use group as fallback if no dialog found
		if (!dialogUi)
		{
			dialogUi = SCR_ConfigurableDialogUi.CreateFromPreset(CONFIG_DIALOGS_ERROR, group);
		}

		// Show default error if tag is not found
		if (!dialogUi)
		{
			dialogUi = SCR_ConfigurableDialogUi.CreateFromPreset(CONFIG_DIALOGS_ERROR, TAG_KICK_DEFAULT);
		}

		m_CurrentKickDialog = dialogUi;

		if (dialogUi)
		{
			// Set error details
			SCR_ErrorDialog errorDialog = SCR_ErrorDialog.Cast(dialogUi.GetRootWidget().FindHandler(SCR_ErrorDialog));
			if (errorDialog)
			{
				errorDialog.SetErrorDetail(strDetail);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void DisplayReconnectDialog(Room roomToJoin, int time, string strDetail = "")
	{
		// Check dialog
		if (!m_CurrentKickDialog)
		{
			//PrintDebug("No kick dialog", "DisplayReconnectDialog");
			return;
		}

		// Check kick preset
		SCR_KickDialogUiPreset kickPreset = SCR_KickDialogUiPreset.Cast(m_CurrentKickDialog.GetDialogPreset());
		if (!kickPreset)
		{
			//PrintDebug("Missing dialog kick preset", "DisplayReconnectDialog");
			return;
		}

		m_CurrentKickDialog.Close();

		// Set reconnect
		if (!kickPreset.m_ReconnectPreset)
		{
			//PrintDebug("Missing dialog reconnect preset", "DisplayReconnectDialog");
			return;
		}

		SCR_ConfigurableDialogUi dialog = SCR_ConfigurableDialogUi.CreateByPreset(
			kickPreset.m_ReconnectPreset);

		m_CurrentKickDialog = dialog;

		// Set Messages
		dialog.SetMessage(kickPreset.m_sMessage);

		SCR_RejoinDialog rejoinDialog = SCR_RejoinDialog.Cast(m_CurrentKickDialog.GetRootWidget().FindHandler(SCR_RejoinDialog));
		if (rejoinDialog)
		{
			rejoinDialog.SetErrorDetail(strDetail);

			string errorStr = kickPreset.m_sMessage;
			rejoinDialog.SetErrorMessage(errorStr);

			// Check rejoin attempt
			string strAttempt = GameSessionStorage.s_Data["m_iRejoinAttempt"];
			int attempt = strAttempt.ToInt();

			if (attempt <= MAX_AUTO_REJOINS)
			{
				errorStr += "\n" + "#AR-ServerBrowser_JoinMessageDefault";
				rejoinDialog.SetErrorMessage(errorStr);

				dialog.SetMessage(errorStr + " " + rejoinDialog.GetTimer());

				// Setup timer
				rejoinDialog.GetEventOnTimerChanged().Insert(OnDialogTimerChange);

				rejoinDialog.ShowLoading(true);
				rejoinDialog.SetTimer(time);
				rejoinDialog.RunTimer(true);
			}
			else
			{
				dialog.SetMessage(errorStr);

				// Block rejoin
				rejoinDialog.ShowLoading(false);
				m_CurrentKickDialog.FindButton("confirm").SetEnabled(false);
			}
		}

		// Setup dialog
		if (dialog)
		{
			// Title
			dialog.SetTitle(kickPreset.m_sTitle);

			// Reconnect button
			dialog.m_OnConfirm.Insert(OnDialogConfirm);
			dialog.m_OnCancel.Insert(OnDialogCancel);
			dialog.m_OnClose.Insert(OnDialogClose);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void InvokeEventOnRejoinTimerOver()
	{
		if (Event_OnRejoinTimerOver)
			Event_OnRejoinTimerOver.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetEventOnRejoinTimerOver()
	{
		if (!Event_OnRejoinTimerOver)
			Event_OnRejoinTimerOver = new ScriptInvokerVoid();

		return Event_OnRejoinTimerOver;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDialogTimerChange(SCR_RejoinDialog dialog, int time)
	{
		m_CurrentKickDialog.SetMessage(dialog.GetErrorMessage() + " " + time);

		if (time == 0)
		{
			InvokeEventOnRejoinTimerOver();
			m_CurrentKickDialog.Close();
		}
	}

	//------------------------------------------------------------------------------------------------
	void SetDialogMessage(string msg)
	{
		if (!m_CurrentDialog)
			return;

		m_CurrentDialog.SetMessage(msg);
	}

	//------------------------------------------------------------------------------------------------
	void CloseCurrentDialog()
	{
		if (m_CurrentDialog)
			m_CurrentDialog.Close();
	}

	//------------------------------------------------------------------------------------------------
	//! Quick short code to open dialog by tag and cache it
	protected void SetDialogByTag(string tag, SCR_ConfigurableDialogUi dialog = null)
	{
		// Check dialog resource
		if (CONFIG_DIALOGS.IsEmpty())
			return;

		// Remove invokers actions from old dialog
		if (m_CurrentDialog)
			m_CurrentDialog.m_OnConfirm.Remove(OnDialogConfirm);

		// Create dialog
		m_CurrentDialog = SCR_ConfigurableDialogUi.CreateFromPreset(CONFIG_DIALOGS, tag, dialog);
		m_iOpenDialogCount++;
		m_bIsOpen = true;

		// Add invoker actions to current dialog
		m_CurrentDialog.m_OnConfirm.Insert(OnDialogConfirm);
		m_CurrentDialog.m_OnCancel.Insert(OnDialogCancel);
		m_CurrentDialog.m_OnClose.Insert(OnDialogClose);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDialogConfirm()
	{
		if (m_OnConfirm)
			m_OnConfirm.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDialogCancel()
	{
		if (m_OnCancel)
			m_OnCancel.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	protected void InvokeOnCloseAll()
	{
		if (!Event_OnCloseAll)
			Event_OnCloseAll = new ScriptInvokerVoid();

		Event_OnCloseAll.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDialogClose(SCR_ConfigurableDialogUi dialog)
	{
		if (m_OnDialogClose)
			m_OnDialogClose.Invoke();

		if (m_CurrentDialog == dialog)
			InvokeOnCloseAll();
	}

	//------------------------------------------------------------------------------------------------
	// Spefic dialog handling
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	//! Show join fail message in dialog
	void DisplayJoinFail(EApiCode apiError)
	{
		CloseCurrentDialog();
		
		string errorTag = ERROR_TAG_DEFAULT;

		// TODO@wernerjak - setup dialogs

		switch (apiError)
		{
			case EApiCode.EACODE_ERROR_P2P_USER_JOIN_BAN: errorTag = TAG_BANNED; break;
			case EApiCode.EACODE_ERROR_DS_USER_JOIN_BAN: errorTag = TAG_BANNED; break;
			case EApiCode.EACODE_ERROR_USER_IS_BANNED_FROM_SHARED_GAME: errorTag = TAG_BANNED; break;
			case EApiCode.EACODE_ERROR_PLAYER_IS_BANNED: errorTag = TAG_BANNED; break;
			//case EApiCode.EACODE_ERROR_MAINTENANCE_IN_PROGRESS: errorTag = "JOIN_FAILED_MAITANANCE"; break;
			//case EApiCode.EACODE_ERROR_MP_ROOM_IS_NOT_JOINABLE: errorTag = "JOIN_FAILED_NOT_JOINABLE"; break;
		}

		SetDialogByTag(errorTag);		
		
		// Show additional message
		if (m_CurrentDialog)
		{
			SCR_ErrorDialog errorDialog = SCR_ErrorDialog.Cast(m_CurrentDialog.GetRootWidget().FindHandler(SCR_ErrorDialog));
			if (errorDialog)
			{
				string strApiError = typename.EnumToString(EApiCode, apiError);
				errorDialog.SetErrorDetail("#AR-Workshop_Error: " + strApiError);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void DisplayJoinBan(RoomJoinData data)
	{	
		CloseCurrentDialog();
		
		SetDialogByTag(TAG_BANNED);
		
		if (!m_CurrentDialog)
			return;
		
		if (data.expiresAt < 0)
			return;
		
		int time = data.expiresAt - System.GetUnixTime();
		
		// Show message with time 	
		if (time <= 0)
			return;
		
		string message = STR_LIGHT_BAN;
		
		// Is ban for official servers
		if (data.scope.Contains(OFFICIAL_SERVER_SCOPE))
			message = STR_HEAVY_BAN;
		
		int timeMinutes = time / 60; // show minutes
		m_CurrentDialog.SetMessage(WidgetManager.Translate(message, timeMinutes.ToString()));
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this when all reports from dialog are cancled to clear invoker actions and display download dialog
	protected void OnAllReportsCanceled(SCR_ReportedAddonsDialog dialog)
	{
		dialog.GetOnAllReportsCanceled().Remove(OnAllReportsCanceled);
	}

	//------------------------------------------------------------------------------------------------
	// Display a dialog asking for downloads cancel confirmation
	void DisplayJoinDownloadsWarning(array<ref SCR_WorkshopItemActionDownload> downloads, SCR_EJoinDownloadsConfirmationDialogType type)
	{
		// Remove invokers actions from old dialog
		if (m_CurrentDialog)
			m_CurrentDialog.m_OnConfirm.Remove(OnDialogConfirm);
		
		m_CurrentDialog = SCR_ServerJoinDownloadsConfirmationDialog.Create(downloads, type);

		// Add invoker actions to current dialog
		m_CurrentDialog.m_OnConfirm.Insert(OnDialogConfirm);
		m_CurrentDialog.m_OnCancel.Insert(OnDialogCancel);
		m_CurrentDialog.m_OnClose.Insert(OnDialogClose);
		
		if (type == SCR_EJoinDownloadsConfirmationDialogType.REQUIRED)
		{
			SCR_DownloadManager.GetInstance().GetOnDownloadQueueCompleted().Insert(OnDownloadingDone);
			SCR_DownloadManager.GetInstance().GetEventOnDownloadFail().Insert(OnDownloadActionFailed);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDownloadingDone()
	{
		// Check if any of required addon is not failed
		array<ref SCR_WorkshopItem> required = m_ModManager.GetRoomItemsScripted();

		for (int i = 0, count = required.Count(); i < count; i++)
		{
			// Is offline
			if (!required[i].GetOffline())
				return;
		}

		if (m_OnDownloadComplete)
			m_OnDownloadComplete.Invoke(m_JoinRoom);
		
		SCR_DownloadManager.GetInstance().GetOnDownloadQueueCompleted().Remove(OnDownloadingDone);
		SCR_DownloadManager.GetInstance().GetEventOnDownloadFail().Remove(OnDownloadActionFailed);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDownloadActionFailed(notnull SCR_WorkshopItemAction action)
	{
		CloseCurrentDialog();

		SCR_DownloadManager.GetInstance().GetOnDownloadQueueCompleted().Remove(OnDownloadingDone);
		SCR_DownloadManager.GetInstance().GetEventOnDownloadFail().Remove(OnDownloadActionFailed);
	}

	//------------------------------------------------------------------------------------------------
	//! Display waiting queue when server is full
	//! Show current state of players
	protected void DisplayWaitingQueue()
	{
		SetDialogByTag(TAG_QUEUE_WAITING);
		UpdateWaitingQueue();

		// Set state text
		TextWidget txtStateTitle = TextWidget.Cast(m_CurrentDialog.GetRootWidget().FindAnyWidget("TxtStateTitle"));
		if (txtStateTitle)
			txtStateTitle.SetText("#AR-PlayerList_Header");

		// Set action
		m_CurrentDialog.m_OnConfirm.Insert(OnWaitingQueueConfirm);

		#ifdef SB_DEBUG
		Print("[SCR_ServerBrowserDialogManager] displaying queue waiting dialog");
		#endif
	}


	//------------------------------------------------------------------------------------------------
	//! Dispaly dialog for password insert whenever oassword is required
	protected void DisplayPasswordRequired()
	{
		SCR_EditboxDialogUi editboxDialog = new SCR_EditboxDialogUi();
		SetDialogByTag(TAG_PASSWORD_REQUIRED, editboxDialog);

		editboxDialog.m_OnWriteModeLeave.Insert(OnPasswordEditboxChanged);
		editboxDialog.FindButton("confirm").SetEnabled(false);
	}

	//------------------------------------------------------------------------------------------------
	//! Call this whenever password dialog editbox is being edited
	protected void OnPasswordEditboxChanged(string text)
	{
		if (!m_CurrentDialog || !SCR_EditboxDialogUi.Cast(m_CurrentDialog))
			return;

		SCR_InputButtonComponent navButton = m_CurrentDialog.FindButton("confirm");
		if (navButton)
			navButton.SetEnabled(!text.IsEmpty());

		navButton = m_CurrentDialog.FindButton("cancel");
		if (navButton)
			navButton.SetEnabled(true);
	}

	//------------------------------------------------------------------------------------------------
	//! Call this on server browser resfresh
	protected void OnServerBrowserAutoRefresh()
	{
		if (m_iDisplayState == EJoinDialogState.QUEUE_WAITING)
			UpdateWaitingQueue();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnWaitingQueueConfirm(SCR_ConfigurableDialogUi dialog)
	{
		if (m_ServerBrowser && m_OnJoinRoomDemand)
			m_OnJoinRoomDemand.Invoke(m_JoinRoom);

		m_CurrentDialog.m_OnConfirm.Remove(OnWaitingQueueConfirm);
	}

	//------------------------------------------------------------------------------------------------
	//! Update wating queue with acutal player count
	protected void UpdateWaitingQueue()
	{
		// Check room
		if (!m_JoinRoom)
			return;

		// Gte player state
		int count = m_JoinRoom.PlayerCount();
		int limit = m_JoinRoom.PlayerLimit();

		if (!m_CurrentDialog)
			return;

		// Set text
		TextWidget txtState = TextWidget.Cast(m_CurrentDialog.GetRootWidget().FindAnyWidget("TxtState"));
		if (txtState)
			txtState.SetText(count.ToString() + "/" + limit.ToString());

		// Setup button
		SCR_InputButtonComponent btnConfirm = m_CurrentDialog.FindButton("confirm");
		if (btnConfirm)
		{
			bool enable = limit - count > 0;
			btnConfirm.SetEnabled(enable);
		}
	}

	//! DECOUPLED DIALOGS
	//------------------------------------------------------------------------------------------------
	MultiplayerDialogUI CreateManualJoinDialog()
	{
		MultiplayerDialogUI multiplayerDialogUI = new MultiplayerDialogUI();
		MultiplayerDialogUI dialog = MultiplayerDialogUI.Cast(SCR_ConfigurableDialogUi.CreateFromPreset(CONFIG_DIALOGS, "MANUAL_CONNECT", multiplayerDialogUI));
		m_CurrentDialog = dialog;

		return dialog;
	}

	//------------------------------------------------------------------------------------------------
	SCR_ServerDetailsDialog CreateServerDetailsDialog(Room room, array<ref SCR_WorkshopItem> items, ScriptInvokerVoid onFavoritesResponse = null)
	{
		SCR_ServerDetailsDialog dialog = SCR_ServerDetailsDialog.CreateServerDetailsDialog(room, items, "SERVER_DETAILS", CONFIG_DIALOGS, onFavoritesResponse);
		m_CurrentDialog = dialog;

		return dialog;
	}

	//------------------------------------------------------------------------------------------------
	void FillRoomDetailsMods(array<ref SCR_WorkshopItem> items, SCR_RoomModsManager modsManager = null)
	{
		SCR_ServerDetailsDialog dialog = SCR_ServerDetailsDialog.Cast(m_CurrentDialog);
		dialog.FillModList(items, modsManager);
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateRoomDetailsScenarioImage(MissionWorkshopItem scenario)
	{
		SCR_ServerDetailsDialog dialog = SCR_ServerDetailsDialog.Cast(m_CurrentDialog);
		if(dialog)
			dialog.SetScenarioImage(scenario);
	}

	
	//------------------------------------------------------------------------------------------------
	// Get & Set API
	//------------------------------------------------------------------------------------------------
	
	// Invokers
	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnConfirm()
	{
		if (!m_OnConfirm)
			m_OnConfirm = new ScriptInvokerVoid();

		return m_OnConfirm;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnCancel()
	{
		if (!m_OnCancel)
			m_OnCancel = new ScriptInvokerVoid();

		return m_OnCancel;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnDialogClose()
	{
		if (!m_OnDialogClose)
			m_OnDialogClose = new ScriptInvokerVoid();

		return m_OnDialogClose;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnCloseAll()
	{
		if (!Event_OnCloseAll)
			Event_OnCloseAll = new ScriptInvokerVoid();

		return Event_OnCloseAll;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerRoom GetOnDownloadComplete()
	{
		if (!m_OnDownloadComplete)
			m_OnDownloadComplete = new ScriptInvokerRoom();

		return m_OnDownloadComplete;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnDownloadCancelDialogClose()
	{
		if (!m_OnDownloadCancelDialogClose)
			m_OnDownloadCancelDialogClose = new ScriptInvokerVoid();

		return m_OnDownloadCancelDialogClose;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerRoom GetOnJoinRoomDemand()
	{
		if (!m_OnJoinRoomDemand)
			m_OnJoinRoomDemand = new ScriptInvokerRoom();

		return m_OnJoinRoomDemand;
	}
	
	// Helpers
	//------------------------------------------------------------------------------------------------
	Room GetJoinRoom()
	{
		return m_JoinRoom;
	}

	//------------------------------------------------------------------------------------------------
	EJoinDialogState GetDisplayState()
	{
		return m_iDisplayState;
	}

	//------------------------------------------------------------------------------------------------
	SCR_ConfigurableDialogUi GetCurrentDialog()
	{
		return m_CurrentDialog;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsDialogOpen()
	{
		return GetCurrentDialog();
	}

	//------------------------------------------------------------------------------------------------
	SCR_ConfigurableDialogUi GetCurrentKickDialog()
	{
		return m_CurrentKickDialog;
	}

	//------------------------------------------------------------------------------------------------
	bool IsOpen()
	{
		return m_bIsOpen;
	}

	//------------------------------------------------------------------------------------------------
	void SetJoinRoom(Room room)
	{
		m_JoinRoom = room;
	}

	//------------------------------------------------------------------------------------------------
	void SetServerBrowser(ServerBrowserMenuUI serverBrowser)
	{
		m_ServerBrowser = serverBrowser;

		if (m_ServerBrowser)
		{
			m_ServerBrowser.m_OnAutoRefresh.Remove(OnServerBrowserAutoRefresh);
			m_ServerBrowser.m_OnAutoRefresh.Insert(OnServerBrowserAutoRefresh);
		}
	}

	//------------------------------------------------------------------------------------------------
	void SetModManager(SCR_RoomModsManager modManager)
	{
		m_ModManager = modManager;
	}

};

//------------------------------------------------------------------------------------------------
//! Enum for tracking current state of joining process
enum EJoinDialogState
{
	// Basic states
	SEARCHING_SERVER, // Basic msg dialog with loading
	JOIN, // Basic msg dialog with loading
	REJOIN, // Rejoin dialog if client is kicked

	// Basic issues states
	SERVER_NOT_FOUND, // Basic error msg dialog
	VERSION_MISMATCH, // Basic error msg dialog
	JOIN_FAILED,

	// Content states
	CHECKING_CONTENT, // Basic msg dialog with loading
	MODS_TO_UPDATE, // Download confirm dialog
	MODS_DOWNLOADING, // Progress bar dialog
	MOD_RESTRICTED, // Basic error msg dialog
	MOD_UGC_PRIVILEGE_MISSING, // Client can't use user generated content

	// Additional actions states
	QUEUE_WAITING, // Dialog with player count
	PASSWORD_REQUIRED, // Editbox dialog

	// Issues
	SERVICE_DOWN, // Backend service is not available
	SERVER_DOWN, // Joined server is not running
	BACKEND_TIMEOUT,
	
	// Warnings
	HIGH_PING_SERVER,
	
	// Fillers
	UNRELATED_DOWNLOADS_CANCELING // Waiting for all downloads to cancel
};

//------------------------------------------------------------------------------------------------
//! Enum for confirmation dialogs that guide the player through the download processes required to join the servers
enum SCR_EJoinDownloadsConfirmationDialogType
{
	ALL,
	UNRELATED,
	REQUIRED
};
