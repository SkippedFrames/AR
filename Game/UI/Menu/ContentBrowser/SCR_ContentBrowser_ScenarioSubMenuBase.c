/* Sub menu base for handlign scenario lines.
Child classes:
- SCR_ContentBrowser_ScenarioSubMenu
- SCR_ContentBrowserDetails_OverviewSubMenu
*/

//------------------------------------------------------------------------------------------------
class SCR_ContentBrowser_ScenarioSubMenuBase : SCR_SubMenuBase
{
	protected ref array<SCR_ContentBrowser_ScenarioLineComponent> m_aScenarioLines = {};

	protected SCR_ContentBrowser_ScenarioLineComponent m_SelectedLine;
	protected SCR_ContentBrowser_ScenarioLineComponent m_LastSelectedLine;

	protected MissionWorkshopItem m_SelectedScenario;
	protected ref SCR_MissionHeader m_Header;

	// Line Actions
	protected EInputDeviceType m_eLastInputType;
	protected bool m_bWasLineSelected;
	protected SCR_ContentBrowser_ScenarioLineComponent m_ClickedLine; // Cache last clicked line to trigger the correct dialog after the double click window

	// Nav buttons
	protected SCR_InputButtonComponent m_NavPlay;
	protected SCR_InputButtonComponent m_NavContinue;
	protected SCR_InputButtonComponent m_NavRestart;
	protected SCR_InputButtonComponent m_NavHost;
	protected SCR_InputButtonComponent m_NavFindServers;
	protected SCR_InputButtonComponent m_NavFavorite;

	// Invokers
	protected ref ScriptInvokerBool m_OnLineFavorite;

	protected bool m_bIsListeningForCommStatus;
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuOpen(parentMenu);

		// Listen for Actions
		SCR_MenuActionsComponent actionsComp = SCR_MenuActionsComponent.FindComponent(GetRootWidget());
		if (actionsComp)
			actionsComp.GetOnAction().Insert(OnActionTriggered);

		// Right footer buttons
		// These are visible when using keyboard / gamepad and focusing a line
		SCR_DynamicFooterComponent footer = parentMenu.GetFooterComponent();
		footer.GetOnButtonActivated().Insert(OnInteractionButtonPressed);

		m_NavPlay = footer.FindButton("Play");
		m_NavContinue = footer.FindButton("Continue");
		m_NavRestart = footer.FindButton("Restart");
		m_NavHost = footer.FindButton("Host");
		m_NavFindServers = footer.FindButton("FindServers");
		m_NavFavorite = footer.FindButton("Favorite");

		UpdateNavigationButtons();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(SCR_SuperMenuBase parentMenu, float tDelta)
	{
		super.OnMenuUpdate(parentMenu, tDelta);

		//! Update selected scenario
		MissionWorkshopItem selectedMission = GetSelectedScenario();
		if (selectedMission && m_SelectedScenario != selectedMission)
		{
			m_SelectedScenario = selectedMission;
			m_Header = SCR_MissionHeader.Cast(MissionHeader.ReadMissionHeader(selectedMission.Id()));
		}

		//! Update buttons
		EInputDeviceType inputDeviceType = GetGame().GetInputManager().GetLastUsedInputDevice();
		bool isLineSelected = GetSelectedLine();
		bool shouldUpdateButtons = inputDeviceType != m_eLastInputType || isLineSelected != m_bWasLineSelected;

		if (shouldUpdateButtons)
			UpdateNavigationButtons();

		m_eLastInputType = inputDeviceType;
		m_bWasLineSelected = isLineSelected;
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuShow(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuShow(parentMenu);

		SCR_ServicesStatusHelper.RefreshPing();
		
		if (!m_bIsListeningForCommStatus)
			SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Insert(OnCommStatusCheckFinished);
		
		m_bIsListeningForCommStatus = true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuHide(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuHide(parentMenu);

		UpdateNavigationButtons(false);
		
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Remove(OnCommStatusCheckFinished);
		m_bIsListeningForCommStatus = false;
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusGained()
	{
		super.OnMenuFocusGained();

		SCR_ServicesStatusHelper.RefreshPing();
		
		UpdateNavigationButtons();
	}

	// ---- EVENTS ----
	//------------------------------------------------------------------------------------------------
	protected void OnCommStatusCheckFinished(SCR_ECommStatus status, float responseTime, float lastSuccessTime, float lastFailTime)
	{
		UpdateNavigationButtons();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnLineFocus(SCR_ScriptedWidgetComponent entry)
	{
		SCR_ContentBrowser_ScenarioLineComponent lineComp = SCR_ContentBrowser_ScenarioLineComponent.Cast(entry);

		m_SelectedLine = lineComp;
		m_SelectedLine.m_OnClick.Insert(OnLineMouseClick);

		UpdateNavigationButtons();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLineFocusLost(SCR_ScriptedWidgetComponent entry)
	{
		m_LastSelectedLine = m_SelectedLine;
		m_SelectedLine.m_OnClick.Remove(OnLineMouseClick);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLineMouseClick(SCR_ScriptedWidgetComponent button)
	{
		m_ClickedLine = SCR_ContentBrowser_ScenarioLineComponent.Cast(button);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLineFavorite(SCR_ListMenuEntryComponent entry, bool favorite)
	{
		OnScenarioStateChanged(SCR_ContentBrowser_ScenarioLineComponent.Cast(entry));
	}

	//------------------------------------------------------------------------------------------------
	//! Called from scenario line component when scenario state changes
	protected void OnScenarioStateChanged(SCR_ContentBrowser_ScenarioLineComponent comp)
	{
	}

	// ---- INPUTS ----
	// SWITCHES
	//------------------------------------------------------------------------------------------------
	protected void OnActionTriggered(string action, float multiplier)
	{
		//! TODO: set which input modes should trigger the actions in the component itself
		if (GetGame().GetInputManager().GetLastUsedInputDevice() != EInputDeviceType.MOUSE)
			return;

		switch (action)
		{
			case "MenuSelectDouble": OnLineClickInteraction(multiplier); break;
			case "MenuRestart": OnRestartButton(); break;
			case "MenuJoin": OnJoinButton(); break;
			case "MenuFavourite": OnFavouriteButton(); break;
			case "MenuHost":
			{
				if (!GetGame().IsPlatformGameConsole())
					OnHostButton();
				break;
			}
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
			case "FindServers": OnJoinButton(); break;
			case "Favorite": OnFavouriteButton(); break;
			case "Host": OnHostButton(); break;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnConfirmationDialogButtonPressed(SCR_ScenarioConfirmationDialogUi dialog, string tag)
	{
		if (!dialog)
			return;

		MissionWorkshopItem scenario = dialog.GetScenario();

		switch (tag)
		{
			case "confirm": Play(scenario); break;
			case "load":	Continue(scenario); break;
			case "restart": Restart(scenario); break;
			case "join": 	Join(scenario); break;
			case "host": 	Host(scenario); break;
		}
	}

	// CLICKS
	//------------------------------------------------------------------------------------------------
	protected void OnLineClickInteraction(float multiplier)
	{
		//! multiplier value in the action is used to differentiate between single and double click

		SCR_ContentBrowser_ScenarioLineComponent lineComp = m_ClickedLine;
		if (!lineComp || !GetLineUnderCursor())
			return;

		switch (Math.Floor(multiplier))
		{
			case 1: OnLineClick(lineComp); break;
			case 2: OnLineDoubleClick(lineComp); break;
		}

		m_ClickedLine = null;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLineClick(SCR_ContentBrowser_ScenarioLineComponent lineComp)
	{
		if (!lineComp)
			return;

		//! CONFIRMATION DIALOG
		MissionWorkshopItem scenario = lineComp.GetScenario();
		if (!scenario)
			return;

		//! If using Mouse single click opens confirmation dialog, double click goes straight to the play interaction
		if (GetGame().GetInputManager().GetLastUsedInputDevice() == EInputDeviceType.MOUSE)
		{
			SCR_ScenarioConfirmationDialogUi scenarioConfirmationDialog = SCR_ScenarioDialogs.CreateScenarioConfirmationDialog(scenario, GetOnLineFavorite());
			if (!scenarioConfirmationDialog)
			{
				OnPlayInteraction(scenario);
				return;
			}

			//! Bind dialog delegates
			scenarioConfirmationDialog.m_OnButtonPressed.Insert(OnConfirmationDialogButtonPressed);
			scenarioConfirmationDialog.GetOnFavorite().Insert(SetFavorite);
		}
		//! If using Gamepad or Keyboard there's no confirmation dialog and single click starts the play interaction
		else
		{
			OnPlayInteraction(scenario);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLineDoubleClick(SCR_ContentBrowser_ScenarioLineComponent lineComp)
	{
		if (!lineComp)
			return;

		OnPlayInteraction(lineComp.GetScenario());
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
	protected void OnFavouriteButton()
	{
		SetFavorite(GetSelectedScenario());
		if (m_NavFavorite && GetSelectedLine())
			m_NavFavorite.SetLabel(GetFavoriteLabel(GetSelectedScenario().IsFavorite()));
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

	// INTERACTIONS
	//------------------------------------------------------------------------------------------------
	protected void OnPlayInteraction(MissionWorkshopItem scenario)
	{
		if (!scenario)
			return;

		bool canBeLoaded = m_Header && GetGame().GetSaveManager().HasLatestSave(m_Header);

		if (canBeLoaded)
			Continue(scenario);
		else
			Play(scenario);
	}

	//------------------------------------------------------------------------------------------------
	protected void Play(MissionWorkshopItem scenario)
	{
		if (!scenario)
			return;

		SCR_WorkshopUiCommon.TryPlayScenario(scenario);
	}

	//------------------------------------------------------------------------------------------------
	protected void Continue(MissionWorkshopItem scenario)
	{
		if (!scenario)
			return;

		if (m_Header && !m_Header.GetSaveFileName().IsEmpty())
			GetGame().GetSaveManager().SetFileNameToLoad(m_Header);
		else
			GetGame().GetSaveManager().ResetFileNameToLoad();

		SCR_WorkshopUiCommon.TryPlayScenario(scenario);
	}

	//------------------------------------------------------------------------------------------------
	protected void Restart(MissionWorkshopItem scenario)
	{
		if (!scenario)
			return;

		m_SelectedScenario = scenario;
		m_Header = SCR_MissionHeader.Cast(MissionHeader.ReadMissionHeader(scenario.Id()));
		bool canBeLoaded = m_Header && GetGame().GetSaveManager().HasLatestSave(m_Header);

		if (!canBeLoaded)
			return;

		SCR_ConfigurableDialogUi dialog = SCR_CommonDialogs.CreateDialog("scenario_restart");
		dialog.m_OnConfirm.Insert(OnRestartConfirmed);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRestartConfirmed()
	{
		GetGame().GetSaveManager().ResetFileNameToLoad();
		SCR_WorkshopUiCommon.TryPlayScenario(m_SelectedScenario);
	}

	//------------------------------------------------------------------------------------------------
	protected void Join(MissionWorkshopItem scenario)
	{
		if (!scenario)
			return;

		if (scenario.GetPlayerCount() > 1)
			ServerBrowserMenuUI.OpenWithScenarioFilter(scenario);
	}

	//------------------------------------------------------------------------------------------------
	protected void Host(MissionWorkshopItem scenario)
	{
		if (!scenario)
			return;

		bool mp = scenario.GetPlayerCount() > 1;
		if (!mp)
			return;

		// Open server hosting dialog
		ServerHostingUI dialog = ServerHostingUI.Cast(GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.ServerHostingDialog));

		dialog.SelectScenario(scenario);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetFavorite(MissionWorkshopItem scenario)
	{
		if (!scenario || !m_SelectedLine || m_SelectedLine.GetScenario() != scenario)
			return;

		bool isFavorite = !scenario.IsFavorite();

		//Update the scenario
		scenario.SetFavorite(isFavorite);

		//Update the widgets
		m_SelectedLine.NotifyScenarioUpdate();

		//Delegate to update the dialog
		if (m_OnLineFavorite)
			m_OnLineFavorite.Invoke(isFavorite);
	}

	// ---- HELPERS ----
	// Create lines for scenarios
	//------------------------------------------------------------------------------------------------
	protected bool CreateLines(array<MissionWorkshopItem> scenarios, Widget parent)
	{
		foreach (MissionWorkshopItem scenario : scenarios)
		{
			Widget w = GetGame().GetWorkspace().CreateWidgets(SCR_ContentBrowser_ScenarioLineWidgets.s_sLayout, parent);
			if (!w)
				return false;

			SCR_ContentBrowser_ScenarioLineComponent comp = SCR_ContentBrowser_ScenarioLineComponent.FindComponent(w);
			if (!comp)
				return false;

			comp.SetScenario(scenario);
			m_aScenarioLines.Insert(comp);

			comp.GetOnFavorite().Insert(OnLineFavorite);
			comp.GetOnMouseInteractionButtonClicked().Insert(OnInteractionButtonPressed);
			comp.GetOnFocus().Insert(OnLineFocus);
			comp.GetOnFocusLost().Insert(OnLineFocusLost);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateNavigationButtons(bool visible = true)
	{
		bool mp;
		bool canBeLoaded;

		if (visible)
		{
			MissionWorkshopItem scenario = GetSelectedScenario();
			mp = scenario.GetPlayerCount() > 1;
			SCR_MissionHeader header = SCR_MissionHeader.Cast(MissionHeader.ReadMissionHeader(scenario.Id()));
			canBeLoaded = header && GetGame().GetSaveManager().HasLatestSave(header);
		}

		if (m_NavPlay)
			m_NavPlay.SetVisible(visible && !canBeLoaded, false);

		if (m_NavContinue)
			m_NavContinue.SetVisible(visible && canBeLoaded, false);

		if (m_NavRestart)
			m_NavRestart.SetVisible(visible && canBeLoaded, false);

		if (m_NavHost)
		{
			m_NavHost.SetVisible(visible && mp && !GetGame().IsPlatformGameConsole() /*&& SCR_ContentBrowser_ScenarioSubMenu.GetHostingAllowed()*/, false);
			SCR_ServicesStatusHelper.SetConnectionButtonEnabled(m_NavHost, SCR_ServicesStatusHelper.SERVICE_BI_BACKEND_MULTIPLAYER); //TODO: update on event instead of tick
		}

		if (m_NavFindServers)
		{
			m_NavFindServers.SetVisible(visible && mp, false);
			SCR_ServicesStatusHelper.SetConnectionButtonEnabled(m_NavFindServers, SCR_ServicesStatusHelper.SERVICE_BI_BACKEND_MULTIPLAYER); //TODO: update on event instead of tick
		}

		if (m_NavFavorite)
		{
			m_NavFavorite.SetVisible(visible, false);
			if (visible)
				m_NavFavorite.SetLabel(GetFavoriteLabel(GetSelectedScenario().IsFavorite()));
		}
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
	protected SCR_ContentBrowser_ScenarioLineComponent GetSelectedLine()
	{
		// We are not over a line, use currently focused line
		Widget wfocused = GetGame().GetWorkspace().GetFocusedWidget();
		SCR_ContentBrowser_ScenarioLineComponent comp;
		if (wfocused)
			comp = SCR_ContentBrowser_ScenarioLineComponent.Cast(wfocused.FindHandler(SCR_ContentBrowser_ScenarioLineComponent));

		EInputDeviceType inputDevice = GetGame().GetInputManager().GetLastUsedInputDevice();
		bool isCursorOnInnerButton = m_SelectedLine && m_SelectedLine.IsInnerButtonInteraction();

		if (inputDevice == EInputDeviceType.MOUSE && (GetLineUnderCursor() || isCursorOnInnerButton))
			return m_SelectedLine;
		else
			return comp;
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_ContentBrowser_ScenarioLineComponent GetLineUnderCursor()
	{
		// Note that this returns null if the cursor is on a button nested inside the line main button

		Widget w = WidgetManager.GetWidgetUnderCursor();

		if (!w)
			return null;

		return SCR_ContentBrowser_ScenarioLineComponent.Cast(w.FindHandler(SCR_ContentBrowser_ScenarioLineComponent));
	}

	//------------------------------------------------------------------------------------------------
	protected MissionWorkshopItem GetSelectedScenario()
	{
		SCR_ContentBrowser_ScenarioLineComponent comp = GetSelectedLine();

		if (!comp)
			return null;

		return comp.GetScenario();
	}

	// ---- PUBLIC ----
	//------------------------------------------------------------------------------------------------
	ScriptInvokerBool GetOnLineFavorite()
	{
		if (!m_OnLineFavorite)
			m_OnLineFavorite = new ScriptInvokerBool();

		return m_OnLineFavorite;
	}
}
