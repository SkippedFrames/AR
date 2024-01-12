class MainMenuUI : ChimeraMenuBase
{
	protected SCR_MenuTileComponent m_FocusedTile;
	protected DialogUI m_BannedDetectionDialog;

	protected ref array<string> m_aBannedItems = {};
	protected static ref array<ref SCR_NewsEntry> m_aNews = {};
	protected static ref array<ref SCR_NewsEntry> m_aNotifications = {};
	protected ref array<ref Widget> m_aTiles = {};

	protected static bool s_bDidCheckNetwork;
	protected static const int SERVICES_STATUS_CHECK_DELAY = 2000; // needed for backend API to prepare
	protected bool m_bFirstLoad;


	
	//------------------------------------------------------------------------------------------------
	protected override void OnMenuOpen()
	{
		super.OnMenuOpen();

		// Init common Workshop UI
		SCR_WorkshopUiCommon.OnGameStart();

		Widget w = GetRootWidget();
		Widget content = w.FindAnyWidget("Grid");
		Widget descriptionRoot = w.FindAnyWidget("Description");
		Widget footer = w.FindAnyWidget("Footer");
		if (!content || !descriptionRoot || !footer)
			return;

		// Listen to buttons in grid
		Widget child = content.GetChildren();
		while (child)
		{
			SCR_MenuTileComponent tile = SCR_MenuTileComponent.Cast(child.FindHandler(SCR_MenuTileComponent));
			if (!tile)
				break;

			if (!m_FocusedTile)
				m_FocusedTile = tile;

			tile.m_OnClicked.Insert(OnTileClick);
			tile.m_OnFocused.Insert(OnTileFocus);
			child = child.GetSibling();
		}

		// Subscribe to buttons
		SCR_InputButtonComponent back = SCR_InputButtonComponent.GetInputButtonComponent("Back", footer);
		if (back)
			back.m_OnActivated.Insert(OnBack);

		// Services Status button
		SCR_InputButtonComponent servicesStatus = SCR_InputButtonComponent.GetInputButtonComponent("ServicesStatus", footer);
		if (servicesStatus)
			servicesStatus.m_OnActivated.Insert(OnServicesStatus);

		SCR_InputButtonComponent feedback = SCR_InputButtonComponent.GetInputButtonComponent("Feedback", footer);
		if (feedback)
			feedback.m_OnActivated.Insert(OnFeedback);

		SCR_InputButtonComponent credits = SCR_InputButtonComponent.GetInputButtonComponent("Credits", footer);
		if (credits)
			credits.m_OnActivated.Insert(OnCredits);

		#ifdef WORKBENCH
		GetGame().GetInputManager().AddActionListener("MenuBackWB", EActionTrigger.DOWN, OnBack);
		#endif

		// Get the entries for the first time
		GetNewsEntries(null);
		GetNotificationEntries(null);

		// Check Service Statuses
		if (!s_bDidCheckNetwork)
		{
			s_bDidCheckNetwork = true;
			GetGame().GetCallqueue().CallLater(CheckServicesStatusIfFocused, SERVICES_STATUS_CHECK_DELAY);
		}

		Widget logoWidget = w.FindAnyWidget("LogoButton");
		if (logoWidget)
		{
			SCR_ModularButtonComponent logo = SCR_ModularButtonComponent.FindComponent(logoWidget);
		if (logo)
			logo.m_OnClicked.Insert(OnLogoClicked);
		}
		
		// Check ping sites 
		GetGame().GetBackendApi().GetClientLobby().MeasureLatency(null);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnMenuOpened()
	{
		super.OnMenuOpened();
		
		// Opening Last menu
		SCR_MenuLoadingComponent.LoadLastMenu();
		SCR_MenuLoadingComponent.ClearLastMenu();

		BaseContainer cont = GetGame().GetGameUserSettings().GetModule("SCR_RecentGames");
		if (cont)
		{
			cont.Get("m_bFirstTimePlay", m_bFirstLoad);
			cont.Set("m_bFirstTimePlay", false);	
		}
		
		//PrintFormat("[OnMenuOpened] m_bFirstLoad: %1", m_bFirstLoad);

		// Save changes to the settings
		GetGame().UserSettingsChanged();
		GetGame().SaveUserSettings();		
				
		// Check first start of session
		bool firstLoadInSession = GameSessionStorage.s_Data["m_bMenuFirstOpening"].IsEmpty();
		GameSessionStorage.s_Data["m_bMenuFirstOpening"] = "false";
		
		//PrintFormat("[OnMenuOpened] firstLoadInSession: %1", firstLoadInSession);
		
		if (!firstLoadInSession)
			return;
		
		if (!m_bFirstLoad)
			return;		
				
		GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.WelcomeDialog);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnMenuFocusLost()
	{
		GetRootWidget().SetEnabled(false);

		super.OnMenuFocusLost();
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnMenuFocusGained()
	{
		GetRootWidget().SetEnabled(true);

		if (m_FocusedTile)
			GetGame().GetWorkspace().SetFocusedWidget(m_FocusedTile.GetRootWidget(), true);

		super.OnMenuFocusGained();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBack()
	{
		if (!IsFocused())
			return;

		TryExitGame();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCredits()
	{
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.CreditsMenu);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTileClick(notnull SCR_MenuTileComponent comp)
	{
		if (!IsFocused())
			return;

		ChimeraMenuPreset preset = comp.m_eMenuPreset;
		if (preset == ChimeraMenuPreset.FeedbackDialog)
			GetGame().GetMenuManager().OpenDialog(preset);
		else if (preset == ChimeraMenuPreset.ContentBrowser)
			SCR_WorkshopUiCommon.TryOpenWorkshop();
		else
			GetGame().GetMenuManager().OpenMenu(preset);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTileFocus(SCR_MenuTileComponent comp)
	{
		if (!IsFocused())
			return;

		m_FocusedTile = comp;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnButtonSelect()
	{
		if (!IsFocused())
			return;

		if (m_FocusedTile)
			OnTileClick(m_FocusedTile);
	}

	//------------------------------------------------------------------------------------------------
	void OnServicesStatus()
	{
		if (!IsFocused())
			return;

		SCR_CommonDialogs.CreateServicesStatusDialog();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFeedback()
	{
		if (!IsFocused())
			return;

		GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.FeedbackDialog);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLogoClicked()
	{
		GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.WelcomeDialog);
	}

	//------------------------------------------------------------------------------------------------
	protected void RestoreFocus()
	{
		if (m_FocusedTile)
			GetGame().GetWorkspace().SetFocusedWidget(m_FocusedTile.GetRootWidget());
	}

	//------------------------------------------------------------------------------------------------
	static int GetNewsEntries(out array<ref SCR_NewsEntry> entries = null)
	{
		if (entries)
			entries.Clear();

		m_aNews.Clear();

		// Print( "NEWS: Count - " + GetGame().GetBackendApi().GetNewsCount() );
		for (int i, cnt = GetGame().GetBackendApi().GetNewsCount(); i < cnt; i++)
		{
			NewsFeedItem item = GetGame().GetBackendApi().GetNewsItem(i);
			if (item)
				m_aNews.Insert(new SCR_NewsEntry(item));
		}

		if (entries)
			entries = m_aNews;

		return m_aNews.Count();
	}

	//------------------------------------------------------------------------------------------------
	static int GetNotificationEntries(out array<ref SCR_NewsEntry> entries = null)
	{
		if (entries)
			entries.Clear();

		m_aNotifications.Clear();

		//Print( "NOTIFICATIONS: Count - " + GetGame().GetBackendApi().GetNotifyCount() );
		for (int i, cnt = GetGame().GetBackendApi().GetNotifyCount(); i < cnt; i++)
		{
			NewsFeedItem item = GetGame().GetBackendApi().GetNotifyItem(i);
			if (item)
				m_aNews.Insert(new SCR_NewsEntry(item));
		}

		if (entries)
			entries = m_aNotifications;

		return m_aNotifications.Count();
	}

	//------------------------------------------------------------------------------------------------
	static int GetUnreadNewsCount()
	{
		int count;
		foreach (SCR_NewsEntry entry : m_aNews)
		{
			if (!entry.m_bRead)
				count++;
		}
		return count;
	}

	//------------------------------------------------------------------------------------------------
	static int GetUnreadNotificationCount()
	{
		int count;
		foreach (SCR_NewsEntry entry : m_aNotifications)
		{
			if (!entry.m_bRead)
				count++;
		}
		return count;
	}

	//------------------------------------------------------------------------------------------------
	//! Shows exit game dialog, or some other dialog if something should
	//! prevent user from exiting the game through the main menu
	protected static void TryExitGame()
	{
		int nCompleted, nTotal;
		SCR_DownloadManager mgr = SCR_DownloadManager.GetInstance();
		if (mgr)
			mgr.GetDownloadQueueState(nCompleted, nTotal);

		if (nTotal > 0)
			new SCR_ExitGameWhileDownloadingDialog();
		else
			new SCR_ExitGameDialog();
	}

	//------------------------------------------------------------------------------------------------
	//! Check Services Status only if the main menu is still focused (given the check is delayed by SERVICES_STATUS_CHECK_DELAY)
	protected void CheckServicesStatusIfFocused()
	{
		if (!IsFocused())
			return;

		SCR_ServicesStatusDialogUI.OpenIfServicesAreNotOK();
	}
}
