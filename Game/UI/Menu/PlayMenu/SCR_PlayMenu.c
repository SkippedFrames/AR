enum EPlayMenuContentType
{
	FEATURED,
	RECOMMENDED,
	RECENT
}

class SCR_PlayMenuEntry : Managed
{
	EPlayMenuContentType m_eContentType;
	ref MissionWorkshopItem m_Item;
	SCR_PlayMenuTileComponent m_Tile;
	Widget m_wRoot;

	void SCR_PlayMenuEntry(MissionWorkshopItem item, EPlayMenuContentType type)
	{
		m_Item = item;
		m_eContentType = type;
	}
}

//------------------------------------------------------------------------------------------------
class SCR_PlayMenu : MenuRootBase
{
	protected ResourceName m_sConfig = "{6409EA8EA4BFF7E6}Configs/PlayMenu/PlayMenuEntries.conf";
	protected ref Resource m_Config;
	protected BaseContainer m_ConfigEntries;

	const string TYPE_FEATURED = "m_aFeaturedScenarios";
	const string TYPE_RECOMMENDED = "m_aRecommendedScenarios";

	protected SCR_PlayMenuComponent m_Featured;
	protected SCR_PlayMenuComponent m_Recommended;
	protected SCR_PlayMenuComponent m_Recent;

	protected ref array<MissionWorkshopItem> m_aScenariosFeatured = {};
	protected ref array<MissionWorkshopItem> m_aScenariosRecommended = {};
	protected ref array<MissionWorkshopItem> m_aScenariosRecent = {};

	protected ref array<ref SCR_PlayMenuEntry> m_aEntriesFeatured = {};
	protected ref array<ref SCR_PlayMenuEntry> m_aEntriesRecommended = {};
	protected ref array<ref SCR_PlayMenuEntry> m_aEntriesRecent = {};

	protected ref MissionWorkshopItem m_ItemTutorial;
	protected WorkshopApi m_WorkshopAPI;
	protected SCR_PlayMenuTileComponent m_CurrentTile;
	protected Widget m_wRoot;
	protected bool m_bTutorialPlayed;
	protected bool m_bShowPlayTutorialDialog;
	protected int m_iPlayTutorialShowCount;
	protected int m_iPlayTutorialShowMax;

	const int THRESHOLD_RECENTLY_PLAYED = 3600 * 24 * 30;

	protected Widget m_wFooter;

	protected SCR_InputButtonComponent m_Scenarios;
	protected SCR_InputButtonComponent m_Play;
	protected SCR_InputButtonComponent m_Continue;
	protected SCR_InputButtonComponent m_Restart;
	protected SCR_InputButtonComponent m_Host;
	protected SCR_InputButtonComponent m_FindServer;

	protected SCR_PlayMenuTileComponent m_ClickedTile; // Cache last clicked line to trigger the correct dialog after the double click window

	protected MissionWorkshopItem m_SelectedScenario;
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		m_wRoot = GetRootWidget();
		m_Recent = SCR_PlayMenuComponent.GetComponent("Recent", m_wRoot);
		m_Recommended = SCR_PlayMenuComponent.GetComponent("Recommended", m_wRoot);
		m_Featured = SCR_PlayMenuComponent.GetComponent("Featured", m_wRoot);
		m_WorkshopAPI = GetGame().GetBackendApi().GetWorkshop();

		m_wFooter = m_wRoot.FindWidget("MenuBase1.SizeBase.VerticalLayout0.SizeFooter.Footer");

		//! Inputs
		//! Buttons
		SCR_DynamicFooterComponent footer = GetFooterComponent();
		footer.GetOnButtonActivated().Insert(OnInteractionButtonPressed);
		
		SCR_InputButtonComponent back = footer.FindButton("Back");
		m_Play = footer.FindButton("Play");
		m_Continue = footer.FindButton("Continue");
		m_Restart = footer.FindButton("Restart");
		m_Host = footer.FindButton("Host");
		m_FindServer = footer.FindButton("FindServer");
		m_Scenarios = footer.FindButton("Scenarios");
		
		//! Listeners
		SCR_MenuActionsComponent actionsComp = SCR_MenuActionsComponent.FindComponent(GetRootWidget());
		if (actionsComp)
			actionsComp.GetOnAction().Insert(OnActionTriggered);

		SCR_ServicesStatusHelper.RefreshPing();
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Insert(OnCommStatusCheckFinished);
		
		// Read the PlayMenu config
		m_Config = BaseContainerTools.LoadContainer(m_sConfig);
		if (!m_Config)
			return;

		m_ConfigEntries = m_Config.GetResource().ToBaseContainer();
		if (!m_ConfigEntries)
			return;

		// Get tutorial
		ResourceName tutorial;
		m_ConfigEntries.Get("m_TutorialScenario", tutorial);
		m_ItemTutorial = m_WorkshopAPI.GetInGameScenario(tutorial);

		if (m_ItemTutorial)
			m_bTutorialPlayed = m_ItemTutorial.GetTimeSinceLastPlay() > -1;

		// Check how many times "Play tutorial" dialog was shown
		BaseContainer settings = GetGame().GetGameUserSettings().GetModule("SCR_RecentGames");

		if (settings)
		{
			settings.Get("m_iPlayTutorialShowCount", m_iPlayTutorialShowCount);
			settings.Get("m_iPlayTutorialShowMax", m_iPlayTutorialShowMax);
		}

		m_bShowPlayTutorialDialog = !m_bTutorialPlayed && m_iPlayTutorialShowCount < m_iPlayTutorialShowMax;

		//PrintFormat("[OnMenuOpen] m_bShowPlayTutorialDialog: %1 | m_bTutorialPlayed: %2 | m_iPlayTutorialShowCount: %3", m_bShowPlayTutorialDialog, m_bTutorialPlayed, m_iPlayTutorialShowCount);

		// Get scenarios
		GetScenarios(m_aScenariosFeatured, EPlayMenuContentType.FEATURED);
		GetScenarios(m_aScenariosRecommended, EPlayMenuContentType.RECOMMENDED);
		GetScenarios(m_aScenariosRecent, EPlayMenuContentType.RECENT);

		// Get menu entries
		CreateMenuEntries(m_aEntriesFeatured, m_aScenariosFeatured, EPlayMenuContentType.FEATURED);
		CreateMenuEntries(m_aEntriesRecommended, m_aScenariosRecommended, EPlayMenuContentType.RECOMMENDED);
		CreateMenuEntries(m_aEntriesRecent, m_aScenariosRecent, EPlayMenuContentType.RECENT);

		SetupSectionTiles(m_Featured, m_aEntriesFeatured);
		SetupSectionTiles(m_Recommended, m_aEntriesRecommended);
		SetupSectionTiles(m_Recent, m_aEntriesRecent);

		// Set starting focused tile
		if (m_aEntriesRecommended.Count() > 0)
			m_Recommended.SetFocusedItem(0);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		super.OnMenuClose();
		
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Remove(OnCommStatusCheckFinished);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetupSectionTiles(SCR_PlayMenuComponent section, array<ref SCR_PlayMenuEntry> entries)
	{
		ref array<Widget> widgets = section.GetWidgets();

		// Remove entries that do not fit into the layout
		if (entries.Count() > widgets.Count())
			entries.Resize(widgets.Count());

		// Setup grid tiles
		foreach (int i, SCR_PlayMenuEntry entry : entries)
		{
			Widget w = widgets.Get(i);
			if (!w)
				continue;

			SCR_PlayMenuTileComponent tile = SCR_PlayMenuTileComponent.Cast(w.FindHandler(SCR_PlayMenuTileComponent));
			if (!tile)
				continue;

			entry.m_Tile = tile;
			entry.m_wRoot = tile.m_wRoot;

			tile.m_OnPlay.Insert(OnPlayButton);
			tile.m_OnContinue.Insert(OnContinueButton);
			tile.m_OnRestart.Insert(OnRestartButton);
			tile.m_OnHost.Insert(OnHostButton);
			tile.m_OnFindServer.Insert(OnJoinButton);

			tile.m_OnFocused.Insert(OnTileFocused);
			tile.m_OnFocusLost.Insert(OnTileFocusLost);

			tile.Setup(entry.m_Item, entry.m_eContentType);
		}
	}

	//------------------------------------------------------------------------------------------------
	void CreateMenuEntries(out array<ref SCR_PlayMenuEntry> entries, array<MissionWorkshopItem> scenarios, EPlayMenuContentType type)
	{
		foreach (MissionWorkshopItem scenario : scenarios)
		{
			entries.Insert(new SCR_PlayMenuEntry(scenario, type));
		}
	}

	//------------------------------------------------------------------------------------------------
	void GetScenarios(out array<MissionWorkshopItem> scenarios, EPlayMenuContentType type)
	{
		if (type == EPlayMenuContentType.RECENT)
		{
			GetRecentScenarios(scenarios);
			return;
		}

		array<ResourceName> aResources = {};

		if (type == EPlayMenuContentType.FEATURED)
			m_ConfigEntries.Get(TYPE_FEATURED, aResources);
		else
			m_ConfigEntries.Get(TYPE_RECOMMENDED, aResources);

		foreach (ResourceName sResource : aResources)
		{
			MissionWorkshopItem scenario = m_WorkshopAPI.GetInGameScenario(sResource);

			if (scenario && !IsMissingDependency(scenario))
				scenarios.Insert(scenario);
		}
	}

	//------------------------------------------------------------------------------------------------
	void GetRecentScenarios(out array<MissionWorkshopItem> scenarios)
	{
		// Get missions from Workshop API
		m_WorkshopAPI.GetPageScenarios(scenarios, 0, 10000);

		int count = scenarios.Count();
		int elapsed;
		MissionWorkshopItem scenario;

		// Remove scenarios from disabled addons
		for (int i = count - 1; i >= 0; i--)
		{
			scenario = scenarios[i];
			elapsed = scenario.GetTimeSinceLastPlay();

			if (elapsed == -1 || elapsed > THRESHOLD_RECENTLY_PLAYED || IsMissingDependency(scenario))
			{
				//PrintFormat("[GetRecentScenarios] removed: %1 | last played: %2 | missing dependency: %3", scenario.Name(), elapsed, IsMissingDependency(scenario));
				scenarios.Remove(i);
				continue;
			}
		};

		SCR_Sorting<MissionWorkshopItem, SCR_CompareMissionTimeSinceLastPlay>.HeapSort(scenarios, false);
		/*
		// DEBUG: Print sorted recent scenarios
		count = scenarios.Count();
		for (int i = 0; i < count; i++)
		{
			scenario = scenarios[i];
			PrintFormat("[GetRecentScenarios] found: %1 | last played: %2", scenario.Name(), scenario.GetTimeSinceLastPlay());
		}
		*/
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsMissingDependency(MissionWorkshopItem scenario)
	{
		WorkshopItem addon = scenario.GetOwner();

		// There are no dependencies if scenario is not from an addon
		if (!addon)
			return false;

		array<Dependency> dependencies = {};
		addon.GetActiveRevision().GetDependencies(dependencies);

		// Check if any dependency is missing
		foreach (Dependency dependency : dependencies)
		{
			if (!dependency.IsOffline())
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsUnique(MissionWorkshopItem item, array<ref SCR_PlayMenuEntry> entries)
	{
		if (!item)
			return false;

		foreach (SCR_PlayMenuEntry entry : entries)
		{
			if (!entry || !entry.m_Item)
				continue;

			if (item == entry.m_Item)
				return false;
		}

		return true;
	}

	//! Input Events
	//------------------------------------------------------------------------------------------------
	protected void OnConfirmationDialogButtonPressed(SCR_ScenarioConfirmationDialogUi dialog, string tag)
	{
		if (!dialog)
			return;

		MissionWorkshopItem scenario = dialog.GetScenario();
		
		switch (tag)
		{
			case "confirm":
				Play(scenario);
				break;

			case "load":
				Continue(scenario);
				break;

			case "restart":
				Restart(scenario);
				break;

			case "join":
				Join(scenario);
				break;

			case "host":
				Host(scenario);
				break;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnActionTriggered(string action, float multiplier)
	{
		//! TODO: set which input modes should trigger the actions in the component itself
		if (GetGame().GetInputManager().GetLastUsedInputDevice() != EInputDeviceType.MOUSE)
			return;

		switch (action)
		{
			case "MenuSelectDouble":
				OnTileClickInteraction(multiplier);
				break;

			case "MenuRestart":
				OnRestartButton();
				break;

			case "MenuJoin":
				OnJoinButton();
				break;

			case "MenuHost":
				OnHostButton();
				break;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnInteractionButtonPressed(string action)
	{
		switch (action)
		{
			case "Play": OnPlayButton(); break;
			case "Continue": OnContinueButton(); break;
			case "Restart": OnRestartButton(); break;
			case "FindServer": OnJoinButton(); break;
			case "Scenarios": OnScenarios(); break;
			case "Host": OnHostButton(); break;
			case "Back": OnBack(); break;
		}
	}
	
	// CLICKS
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	protected void OnTileClickInteraction(float multiplier)
	{
		//! multiplier value in the action is used to differentiate between single and double click

		SCR_PlayMenuTileComponent tile = m_ClickedTile;
		if (!tile || !GetTileUnderCursor())
			return;

		MissionWorkshopItem scenario = tile.m_Item;
		if (!scenario)
			return;

		switch (Math.Floor(multiplier))
		{
			case 1: OnClickInteraction(scenario); break;
			case 2: OnDoubleClickInteraction(scenario); break;
		}

		m_ClickedTile = null;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnClickInteraction(MissionWorkshopItem scenario)
	{
		//! Confirmation Dialog
		SCR_ScenarioConfirmationDialogUi scenarioConfirmationDialog = SCR_ScenarioDialogs.CreateScenarioConfirmationDialog(scenario);
		if (!scenarioConfirmationDialog)
		{
			OnPlayInteraction(scenario);
			return;
		}
	
		//! Bind dialog delegates
		scenarioConfirmationDialog.m_OnButtonPressed.Insert(OnConfirmationDialogButtonPressed);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDoubleClickInteraction(MissionWorkshopItem scenario)
	{
		if (!scenario)
			return;

		OnPlayInteraction(scenario);
	}
	
	// BUTTONS
	//------------------------------------------------------------------------------------------------
	protected void OnRestartButton()
	{
		Restart(GetSelectedScenario());
	}

	//------------------------------------------------------------------------------------------------
	protected void OnJoinButton()
	{
		Join(GetSelectedScenario());
	}

	//------------------------------------------------------------------------------------------------
	protected void OnHostButton()
	{
		Host(GetSelectedScenario());
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayButton()
	{
		Play(GetSelectedScenario());
	}

	//------------------------------------------------------------------------------------------------
	protected void OnContinueButton()
	{
		Continue(GetSelectedScenario());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnTileMouseClick(SCR_TileBaseComponent tile)
	{
		m_ClickedTile = SCR_PlayMenuTileComponent.Cast(tile);
	}

	// INTERACTIONS
	//------------------------------------------------------------------------------------------------
	protected void OnPlayInteraction(MissionWorkshopItem scenario)
	{
		if (!scenario)
			return;

		SCR_MissionHeader header = SCR_MissionHeader.Cast(MissionHeader.ReadMissionHeader(scenario.Id()));

		if (!header)
			return;

		bool canBeLoaded = header && GetGame().GetSaveManager().HasLatestSave(header);
		if (canBeLoaded)
			Continue(scenario);
		else
			Play(scenario);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnBack()
	{
		Close();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnScenarios()
	{
		SCR_MenuLoadingComponent.SaveLastMenu(ChimeraMenuPreset.PlayMenu);

		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.ScenarioMenu);
	}

	//------------------------------------------------------------------------------------------------
	protected void Play(MissionWorkshopItem scenario)
	{
		if (!scenario)
			return;
		
		m_SelectedScenario = scenario;
		
		if (m_bShowPlayTutorialDialog && m_ItemTutorial && m_ItemTutorial != m_CurrentTile.m_Item)
		{
			BaseContainer settings = GetGame().GetGameUserSettings().GetModule("SCR_RecentGames");

			if (settings)
			{
				m_iPlayTutorialShowCount++;

				settings.Set("m_iPlayTutorialShowCount", m_iPlayTutorialShowCount);
				GetGame().UserSettingsChanged();
			}

			DialogUI dialog = ShowTutorialDialog();
			if (dialog)
				dialog.m_OnCancel.Insert(PlayCurrentScenario);
		}
		else
		{
			PlayCurrentScenario();
		}

		SCR_MenuLoadingComponent.SaveLastMenu(ChimeraMenuPreset.PlayMenu);
	}

	//------------------------------------------------------------------------------------------------
	protected void Continue(MissionWorkshopItem scenario)
	{
		if (!scenario)
			return;
		
		m_SelectedScenario = scenario;
		
		SCR_MissionHeader header = m_CurrentTile.m_Header;
		if (header && !header.GetSaveFileName().IsEmpty())
			GetGame().GetSaveManager().SetFileNameToLoad(header);
		else
			GetGame().GetSaveManager().ResetFileNameToLoad();

		PlayCurrentScenario();

		SCR_MenuLoadingComponent.SaveLastMenu(ChimeraMenuPreset.PlayMenu);
	}

	//------------------------------------------------------------------------------------------------
	protected void Restart(MissionWorkshopItem scenario)
	{
		if (!scenario)
			return;
		
		m_SelectedScenario = scenario;

		SCR_ConfigurableDialogUi dialog = SCR_CommonDialogs.CreateDialog("scenario_restart");
		dialog.m_OnConfirm.Insert(OnRestartConfirmed);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRestartConfirmed()
	{
		GetGame().GetSaveManager().ResetFileNameToLoad();
		PlayCurrentScenario();

		SCR_MenuLoadingComponent.SaveLastMenu(ChimeraMenuPreset.PlayMenu);
	}

	//------------------------------------------------------------------------------------------------
	protected void Host(MissionWorkshopItem scenario)
	{
		if (!scenario)
			return;
		
		m_SelectedScenario = scenario;
		
		HostCurrentScenario();

		SCR_MenuLoadingComponent.SaveLastMenu(ChimeraMenuPreset.PlayMenu);
	}

	//------------------------------------------------------------------------------------------------
	protected void Join(MissionWorkshopItem scenario)
	{
		if (!scenario)
			return;
		
		m_SelectedScenario = scenario;
		
		FindCurrentScenarioServers();

		SCR_MenuLoadingComponent.SaveLastMenu(ChimeraMenuPreset.PlayMenu);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTileFocused(SCR_PlayMenuTileComponent tile)
	{
		m_CurrentTile = tile;

		UpdateNavigationButtons();
		
		tile.m_OnClicked.Insert(OnTileMouseClick);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTileFocusLost(SCR_PlayMenuTileComponent tile)
	{
		UpdateNavigationButtons(false);
		
		tile.m_OnClicked.Remove(OnTileMouseClick);
	}

	//------------------------------------------------------------------------------------------------
	protected DialogUI ShowTutorialDialog()
	{
		DialogUI dialog = DialogUI.Cast(GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.TutorialDialog));
		dialog.m_OnConfirm.Insert(OnPlayTutorial);
		return dialog;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateNavigationButtons(bool show = true)
	{
		if (!m_CurrentTile)
			return;
		
		show = show && GetSelectedTile();
		
		SCR_MissionHeader header = m_CurrentTile.m_Header;
		MissionWorkshopItem item = m_CurrentTile.m_Item;

		bool canContinue = header && GetGame().GetSaveManager().HasLatestSave(header);
		m_Play.SetVisible(show && !m_CurrentTile.m_bIsMouseInteraction && !canContinue, false);
		m_Continue.SetVisible(show && !m_CurrentTile.m_bIsMouseInteraction && canContinue, false);

		m_Restart.SetVisible(show && !m_CurrentTile.m_bIsMouseInteraction && canContinue, false);
		m_Host.SetVisible(show && !m_CurrentTile.m_bIsMouseInteraction && !GetGame().IsPlatformGameConsole() && item.GetPlayerCount() > 1, false);
		SCR_ServicesStatusHelper.SetConnectionButtonEnabled(m_Host, SCR_ServicesStatusHelper.SERVICE_BI_BACKEND_MULTIPLAYER);
		
		m_FindServer.SetVisible(show && !m_CurrentTile.m_bIsMouseInteraction && item.GetPlayerCount() > 1, false);
		SCR_ServicesStatusHelper.SetConnectionButtonEnabled(m_FindServer, SCR_ServicesStatusHelper.SERVICE_BI_BACKEND_MULTIPLAYER);
	}
	
	//------------------------------------------------------------------------------------------------
	void TryPlayScenario(MissionWorkshopItem scenario)
	{
		if (!scenario)
			return;

		SCR_WorkshopUiCommon.TryPlayScenario(scenario);
	}

	//------------------------------------------------------------------------------------------------
	protected void PlayCurrentScenario()
	{
		if (m_SelectedScenario)
			TryPlayScenario(m_SelectedScenario);
	}

	//------------------------------------------------------------------------------------------------
	protected void FindCurrentScenarioServers()
	{
		if (m_SelectedScenario)
			ServerBrowserMenuUI.OpenWithScenarioFilter(m_SelectedScenario);
	}

	//------------------------------------------------------------------------------------------------
	protected void HostCurrentScenario()
	{
		if (!m_SelectedScenario)
			return;
		
		ServerHostingUI dialog = ServerHostingUI.Cast(GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.ServerHostingDialog));

		if (!dialog)
			return;

		dialog.SelectScenario(m_SelectedScenario);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayTutorial()
	{
		TryPlayScenario(m_ItemTutorial);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusGained()
	{
		super.OnMenuFocusGained();

		SCR_ServicesStatusHelper.RefreshPing();
		
		// Restore focus to the last accessed tile
		if (m_CurrentTile)
		{
			GetGame().GetWorkspace().SetFocusedWidget(m_CurrentTile.m_wRoot);
			return;
		}

		// Fallback to the 1st item in the *recent items* list
		if (m_Recent)
			m_Recent.SetFocusedItem(0);
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_PlayMenuTileComponent GetSelectedTile()
	{
		// We are not over a line, use currently focused line
		Widget wfocused = GetGame().GetWorkspace().GetFocusedWidget();
		SCR_PlayMenuTileComponent comp;
		if (wfocused)
			comp = SCR_PlayMenuTileComponent.Cast(wfocused.FindHandler(SCR_PlayMenuTileComponent));

		EInputDeviceType inputDevice = GetGame().GetInputManager().GetLastUsedInputDevice();
		bool isCursorOnInnerButton = m_CurrentTile && m_CurrentTile.m_bIsMouseInteraction;
		
		if (inputDevice == EInputDeviceType.MOUSE && (GetTileUnderCursor() || isCursorOnInnerButton))
			return m_CurrentTile;
		else
			return comp;
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_PlayMenuTileComponent GetTileUnderCursor()
	{
		Widget w = WidgetManager.GetWidgetUnderCursor();

		if (!w)
			return null;

		return SCR_PlayMenuTileComponent.Cast(w.FindHandler(SCR_PlayMenuTileComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	protected MissionWorkshopItem GetSelectedScenario()
	{
		SCR_PlayMenuTileComponent comp = GetSelectedTile();

		if (!comp)
			return null;

		return comp.m_Item;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCommStatusCheckFinished(SCR_ECommStatus status, float responseTime, float lastSuccessTime, float lastFailTime)
	{
		UpdateNavigationButtons();
	}
}
