/*
The overview tab of the details view
*/

class SCR_ContentBrowserDetails_OverviewSubMenu : SCR_ContentBrowser_ScenarioSubMenuBase
{
	protected ContentBrowserDetailsMenu m_DetailsMenu;
	protected ref SCR_WorkshopItem m_WorkshopItem;

	protected ref SCR_ContentBrowserDetails_OverviewSubMenuWidgets m_Widgets;

	protected ref SCR_WorkshopDownloadSequence m_DownloadRequest;

	protected SCR_LoadingOverlayDialog m_LoadingOverlayDlg;

	protected bool m_bGalleryUpdated = false; // Set true when we first time show images in gallery.

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuOpen(parentMenu);

		m_DetailsMenu = ContentBrowserDetailsMenu.Cast(parentMenu);
		m_WorkshopItem = m_DetailsMenu.m_WorkshopItem;

		m_Widgets = new SCR_ContentBrowserDetails_OverviewSubMenuWidgets;
		m_Widgets.Init(m_wRoot);

		// Hide verrsion selection in release
		#ifndef WORKSHOP_DEBUG
			m_Widgets.m_VersionComboBoxComponent0.SetVisible(false, false);
		#endif

		// Hide scenario and gallery section
		m_Widgets.m_Gallery.SetVisible(false);
		m_Widgets.m_ScenarioSection.SetVisible(false);

		// Init details panel
		m_Widgets.m_AddonDetailsPanelComponent.SetForceShowVersion(true); // Show version regardless of addon being downloaded or not

		if (m_WorkshopItem)
		{
			if (SCR_WorkshopUiCommon.GetConnectionState())
			{
				m_WorkshopItem.m_OnGetAsset.Insert(Callback_OnGetAsset);
				m_WorkshopItem.m_OnChanged.Insert(Callback_OnItemChanged);
				m_WorkshopItem.m_OnScenariosLoaded.Insert(Callback_OnDownloadScenarios);
				m_WorkshopItem.m_OnTimeout.Insert(Callback_OnTimeout);
				m_WorkshopItem.LoadDetails();
			}
			else
			{
				Callback_OnDownloadScenarios();
			}
		}

		// Init event handlers last of all.
		// We don't want event handlers to get called while we are still initializing UI.
		m_Widgets.m_VoteUpButtonComponent.m_OnToggled.Insert(OnVoteUpButton);						// Toggleable!
		m_Widgets.m_VoteDownButtonComponent.m_OnToggled.Insert(OnVoteDownButton);					// Toggleable!
		m_Widgets.m_EnableButtonComponent.m_OnToggled.Insert(OnEnableButtonToggled);				// Toggleable!
		m_Widgets.m_FavoriteButtonComponent.m_OnToggled.Insert(OnAddonFavouriteButtonToggled);		// Toggleable!
		m_Widgets.m_ReportButtonComponent.m_OnClicked.Insert(OnReportButton);
		
		m_Widgets.m_DownloadButtonComponent.m_OnClicked.Insert(OnDownloadButton);
		m_Widgets.m_DeleteButtonComponent.m_OnClicked.Insert(OnDeleteButton);
		m_Widgets.m_DownloadingButtonComponent.m_OnClicked.Insert(OnDownloadingButton);
		
		m_Widgets.m_SolveIssuesButtonComponent.m_OnClicked.Insert(OnSolveIssuesButton);

		// Init the addon details panel
		m_Widgets.m_AddonDetailsPanelComponent.SetWorkshopItem(m_WorkshopItem);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuShow(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuShow(parentMenu);

		UpdateAllWidgets();

		// Set the default focused widget to one we're sure is always present
		Widget defaultFocus = m_Widgets.m_FavoriteButton;

		GetGame().GetCallqueue().CallLater(GetGame().GetWorkspace().SetFocusedWidget, 0, false, defaultFocus, false);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuOpen(parentMenu);

		if (m_WorkshopItem)
		{
			m_WorkshopItem.m_OnGetAsset.Remove(Callback_OnGetAsset);
			m_WorkshopItem.m_OnChanged.Remove(Callback_OnItemChanged);
			m_WorkshopItem.m_OnScenariosLoaded.Remove(Callback_OnDownloadScenarios);
			m_WorkshopItem.m_OnTimeout.Remove(Callback_OnTimeout);
		}

		UpdateNavigationButtons(false);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusGained()
	{
		//Bring back focus to the last selected line after closing a pop-up dialog
		if (!SCR_MenuHelper.IsAnyDialogOpen())
		{
			Widget target;
			
			if (m_LastSelectedLine)
				target = m_LastSelectedLine.GetRootWidget();
			else if (m_Widgets && m_Widgets.m_ScenariosList)
				target = m_Widgets.m_ScenariosList.GetChildren();
			
			if (!target)
				target = m_Widgets.m_FavoriteButton;
				
			GetGame().GetWorkspace().SetFocusedWidget(target);
		}
		
		super.OnMenuFocusGained();
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateNavigationButtons(bool visible = true)
	{
		visible =
			visible &&
			m_WorkshopItem &&
			m_WorkshopItem.GetOffline() &&
			GetSelectedLine() &&
			GetGame().GetInputManager().GetLastUsedInputDevice() != EInputDeviceType.MOUSE;

		super.UpdateNavigationButtons(visible);
	}

	//------------------------------------------------------------------------------------------------
	override void OnLineFocus(SCR_ScriptedWidgetComponent entry)
	{
		super.OnLineFocus(entry);
		UpdateSidePanel();
	}

	//------------------------------------------------------------------------------------------------
	override void OnLineFocusLost(SCR_ScriptedWidgetComponent entry)
	{
		super.OnLineFocusLost(entry);
		UpdateSidePanel();
	}

	//------------------------------------------------------------------------------------------------
	override void OnLineClick(SCR_ContentBrowser_ScenarioLineComponent lineComp)
	{
		if (!lineComp)
			return;

		//! Handle interaction with addon not downloaded
		if (!m_WorkshopItem || !m_WorkshopItem.GetOffline())
		{
			if (GetGame().GetInputManager().GetLastUsedInputDevice() != EInputDeviceType.MOUSE)
				LineInteractionTryDownload(lineComp);
			
			return;
		}

		//! Interaction with addon downloaded
		super.OnLineClick(lineComp);
	}

	//------------------------------------------------------------------------------------------------
	override void OnLineDoubleClick(SCR_ContentBrowser_ScenarioLineComponent lineComp)
	{
		if (!lineComp)
			return;

		//! Handle interaction with addon not downloaded
		if (!m_WorkshopItem || !m_WorkshopItem.GetOffline())
		{
			LineInteractionTryDownload(lineComp);
			return;
		}

		//! Handle interaction with addon dowloaded
		super.OnLineDoubleClick(lineComp);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayInteraction(MissionWorkshopItem scenario)
	{
		if (!m_WorkshopItem || !m_WorkshopItem.GetOffline())
			return;

		super.OnPlayInteraction(scenario);
	}

	//------------------------------------------------------------------------------------------------
	override void Host(MissionWorkshopItem scenario)
	{
		if (!m_WorkshopItem || !m_WorkshopItem.GetOffline())
			return;

		super.Host(scenario);
	}

	//------------------------------------------------------------------------------------------------
	override void Join(MissionWorkshopItem scenario)
	{
		if (!scenario || !m_WorkshopItem || !m_WorkshopItem.GetOffline() || scenario.GetPlayerCount() <= 1)
			return;

		// Interrupt when server config is being edited
		// just temporary solution to prevent endless opening of menus
		if (ServerHostingUI.GetTemporaryConfig())
		{
			SCR_MenuToolsLib.GetEventOnAllMenuClosed().Insert(OnAllMenuCloseJoin);
			SCR_MenuToolsLib.CloseAllMenus({MainMenuUI, ServerBrowserMenuUI});
			return;
		}

		super.Join(scenario);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets favorite state of the Scenario line
	override void SetFavorite(MissionWorkshopItem scenario)
	{
		if (!m_WorkshopItem || !m_WorkshopItem.GetOffline())
			return;

		super.SetFavorite(scenario);
	}

	//------------------------------------------------------------------------------------------------
	//! Called from scenario line component when scenario state changes
	override void OnScenarioStateChanged(SCR_ContentBrowser_ScenarioLineComponent comp)
	{
		UpdateButtons();
		UpdateSidePanel();
	}

	// ---- CALLBACKS ----
	//------------------------------------------------------------------------------------------------
	void Callback_OnGetAsset()
	{
		UpdateGallery();
	}

	//------------------------------------------------------------------------------------------------
	void Callback_OnItemChanged()
	{
		this.UpdateAllWidgets();
	}

	//------------------------------------------------------------------------------------------------
	//! Called when we receive scenarios from backend
	void Callback_OnDownloadScenarios()
	{
		array<MissionWorkshopItem> scenarios = {};
		m_WorkshopItem.GetScenarios(scenarios);

		#ifdef WORKSHOP_DEBUG
		ContentBrowserUI._print(string.Format("Downloaded %1 scenarios", scenarios.Count()));
		#endif

		m_Widgets.m_ScenarioSection.SetVisible(!scenarios.IsEmpty());

		// Delete old entries
		while (m_Widgets.m_ScenariosList.GetChildren())
		{
			m_Widgets.m_ScenariosList.RemoveChild(m_Widgets.m_ScenariosList.GetChildren());
		}

		// Sort missions by name
		SCR_Sorting<MissionWorkshopItem, SCR_CompareMissionName>.HeapSort(scenarios, false);

		if (CreateLines(scenarios, m_Widgets.m_ScenariosList))
			UpdateScenarioLines();
	}

	//------------------------------------------------------------------------------------------------
	void Callback_OnTimeout()
	{
		// Ignore if we are not connected to backend at all
		if (!SCR_WorkshopUiCommon.GetConnectionState())
			return;

		SCR_ConfigurableDialogUi dlg = SCR_CommonDialogs.CreateTimeoutOkDialog();
		dlg.m_OnClose.Insert(Callback_OnTimeOut_OnCloseDialog);
	}

	//------------------------------------------------------------------------------------------------
	void Callback_OnTimeOut_OnCloseDialog()
	{
		m_ParentMenu.CloseMenu();
	}

	// ---- WIDGET CALLBACKS ----
	//------------------------------------------------------------------------------------------------
	protected void OnDeleteButton()
	{
		// Unsubscribe/delete any addon data is available
		if (m_WorkshopItem.GetOffline() || m_WorkshopItem.GetWorkshopItem().GetDownloadingRevision())
			SCR_WorkshopUiCommon.OnUnsubscribeAddonButton(m_WorkshopItem);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDownloadButton()
	{
		// Start new download or continue in downloading
		Revision specificVersion;

		#ifdef WORKSHOP_DEBUG
			string strSpecificVersion = m_Widgets.m_VersionComboBoxComponent0.GetCurrentItem();
			specificVersion = m_WorkshopItem.FindRevision(strSpecificVersion);
		#endif

		if (specificVersion)
			m_DownloadRequest = SCR_WorkshopDownloadSequence.Create(m_WorkshopItem, specificVersion, m_DownloadRequest);
		else
			m_DownloadRequest = SCR_WorkshopDownloadSequence.Create(m_WorkshopItem, m_WorkshopItem.GetLatestRevision(), m_DownloadRequest);
		
		GetGame().GetCallqueue().CallLater(SCR_DownloadManager_Dialog.Create);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDownloadingButton()
	{
		SCR_DownloadManager_Dialog.Create();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSolveIssuesButton()
	{
		// This button does something only if there is a problem
		EWorkshopItemProblem problem = m_WorkshopItem.GetHighestPriorityProblem();

		if (problem == EWorkshopItemProblem.NO_PROBLEM)
			return;

		SCR_WorkshopUiCommon.ExecutePrimaryAction(m_WorkshopItem, m_DownloadRequest);
	}

	//------------------------------------------------------------------------------------------------
	//! By the time when this is called, the button has already been
	//! toggled by the widget library code
	protected void OnVoteUpButton()
	{
		bool newState = m_Widgets.m_VoteUpButtonComponent.GetToggled();
		this.OnVoteButtons(newState, !newState);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnVoteDownButton()
	{
		bool newState = m_Widgets.m_VoteDownButtonComponent.GetToggled();
		this.OnVoteButtons(!newState, !newState);
	}

	//------------------------------------------------------------------------------------------------
	// resetAll - resets the rating
	// voteUp - if not reset rating, vote up or down
	protected void OnVoteButtons(bool voteUp, bool resetAll)
	{
		SCR_ModularButtonComponent bup = m_Widgets.m_VoteUpButtonComponent;
		SCR_ModularButtonComponent bdwn = m_Widgets.m_VoteDownButtonComponent;

		bup.SetToggled(!resetAll && voteUp, false); // Don't invoke the OnToggled again!
		bdwn.SetToggled(!resetAll && !voteUp, false);

		if (resetAll)
		{
			m_WorkshopItem.ResetMyRating();
		}
		else
		{
			m_WorkshopItem.SetMyRating(voteUp);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEnableButtonToggled(SCR_ModularButtonComponent comp)
	{
		SCR_WorkshopUiCommon.OnEnableAddonToggleButton(m_WorkshopItem, comp);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets favorite state of the Addon
	protected void OnAddonFavouriteButtonToggled()
	{
		SCR_WorkshopItem item = m_WorkshopItem;
		bool fav;

		if (item.GetRestricted())
			return;

		fav = !item.GetFavourite();
		item.SetFavourite(fav);
	}

	protected SCR_ModReportDialogComponent m_ModReportDialog;

	//------------------------------------------------------------------------------------------------
	//! This button used in reported and not reported state. It either sends a report or cancels it.
	protected void OnReportButton()
	{
		// Bail if not in backend
		if (!m_WorkshopItem.GetOnline())
			return;

		// Individually reported mod - reported by me
		if (m_WorkshopItem.GetReportedByMe())
		{
			if (m_LoadingOverlayDlg)
				m_LoadingOverlayDlg.Close();

			m_LoadingOverlayDlg = SCR_LoadingOverlayDialog.Create();
			m_LoadingOverlayDlg.m_OnCloseStarted.Insert(Callback_OnLoadMyReportCancel); // Called when user decides to cancel and not wait

			m_WorkshopItem.m_OnMyReportLoaded.Insert(Callback_OnLoadMyReportSuccess);
			m_WorkshopItem.m_OnMyReportLoadError.Insert(Callback_OnLoadMyReportError);
			m_WorkshopItem.LoadReport();

			return;
		}

		m_ModReportDialog = SCR_ModReportDialogComponent.Cast(m_wRoot.FindHandler(SCR_ModReportDialogComponent));

		// Mod with reported author
		if (m_WorkshopItem.GetModAuthorReportedByMe())
		{
			if (m_ModReportDialog)
			{
				m_ModReportDialog.SetItem(m_WorkshopItem);
				m_ModReportDialog.OnSelectReportAuthor();
				m_WorkshopItem.m_OnReportStateChanged.Insert(OnAuthorBlockStateChanged);
			}

			return;
		}

		// Not reported
		if (m_ModReportDialog)
		{
			m_ModReportDialog.OpenSelectReport(m_WorkshopItem);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAuthorBlockStateChanged()
	{
		LoadWorkshopDetails();
		m_WorkshopItem.m_OnReportStateChanged.Remove(OnAuthorBlockStateChanged);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAllMenuCloseJoin()
	{
		SCR_MenuToolsLib.GetEventOnAllMenuClosed().Remove(OnAllMenuCloseJoin);

		//GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.ServerBrowserMenu);
		if (!GetGame().IsPlatformGameConsole())
			GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.ServerHostingDialog);
	}

	//------------------------------------------------------------------------------------------------
	// Unreporting, loading report
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	protected void Callback_OnLoadMyReportCancel()
	{
		m_WorkshopItem.m_OnMyReportLoaded.Remove(Callback_OnLoadMyReportSuccess);	// todo improve error handling :(
		m_WorkshopItem.m_OnMyReportLoadError.Remove(Callback_OnLoadMyReportError);
	}

	//------------------------------------------------------------------------------------------------
	protected void Callback_OnLoadMyReportSuccess()
	{
		m_WorkshopItem.m_OnMyReportLoaded.Remove(Callback_OnLoadMyReportSuccess);
		m_WorkshopItem.m_OnMyReportLoadError.Remove(Callback_OnLoadMyReportError);

		m_LoadingOverlayDlg.CloseAnimated();

		new SCR_CancelMyReportDialog(m_WorkshopItem);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Callback_OnLoadMyReportError()
	{
		m_WorkshopItem.m_OnMyReportLoaded.Remove(Callback_OnLoadMyReportSuccess);
		m_WorkshopItem.m_OnMyReportLoadError.Remove(Callback_OnLoadMyReportError);

		m_LoadingOverlayDlg.CloseAnimated();
		SCR_CommonDialogs.CreateRequestErrorDialog();
	}

	//------------------------------------------------------------------------------------------------
	// Methods for updating state of individual components
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	protected void LoadWorkshopDetails()
	{
		if (!m_WorkshopItem)
			return;

		if (SCR_WorkshopUiCommon.GetConnectionState())
		{
			m_WorkshopItem.m_OnGetAsset.Insert(Callback_OnGetAsset);
			m_WorkshopItem.m_OnChanged.Insert(Callback_OnItemChanged);
			m_WorkshopItem.m_OnScenariosLoaded.Insert(Callback_OnDownloadScenarios);
			m_WorkshopItem.m_OnTimeout.Insert(Callback_OnTimeout);
			m_WorkshopItem.LoadDetails();
		}
		else
		{
			Callback_OnDownloadScenarios();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateAllWidgets()
	{
		UpdateVersionComboBox();
		UpdateButtons();
		UpdateAddonStaticWidgets();
		UpdateScenarioLines();
	}

	//------------------------------------------------------------------------------------------------
	//! updated m_Widgets which depend on state of the mod (so everything except for description, gallery)
	protected void UpdateButtons()
	{
		if (!GetShown())
			return;

		m_WorkshopItem = m_DetailsMenu.m_WorkshopItem;
		SCR_WorkshopItem item = m_WorkshopItem;

		SCR_WorkshopItemActionDownload actionThisItem = item.GetDownloadAction();
		SCR_WorkshopItemActionComposite actionDependencies = item.GetDependencyCompositeAction();
		bool downloading = actionThisItem || actionDependencies;
		EWorkshopItemProblem problem = item.GetHighestPriorityProblem();

		// Label of primary action
		string primaryActionText = SCR_WorkshopUiCommon.GetPrimaryActionName(item);

		// Enable button
		bool enableButtonEnabled = item.GetOffline() && !downloading;
		//m_Widgets.m_EnableButtonComponent.SetVisible(enableButtonEnabled);
		m_Widgets.m_EnableButtonComponent.SetEnabled(enableButtonEnabled);
		m_Widgets.m_EnableButtonComponent.SetToggled(item.GetEnabled(), false);

		bool reportIndividual = item.GetReportedByMe();
		bool reportAuthor = item.GetModAuthorReportedByMe();

		// Report button
		m_Widgets.m_ReportButtonComponent.SetEnabled(item.GetOnline());
		string reportButtonMode = "not_reported";
		if (reportIndividual || reportAuthor)
			reportButtonMode = "reported";
		m_Widgets.m_ReportButtonComponent.SetEffectsWithAnyTagEnabled({"all", reportButtonMode});

		//! Download/Downloading/Delete buttons
		Revision downloadRevision = item.GetWorkshopItem().GetDownloadingRevision();
		bool differentEnvironment = item.GetWorkshopItem().GetBackendEnv() != GetGame().GetBackendApi().GetBackendEnv();

		bool downloaded = !downloading && item.GetOnline() && (item.GetOffline() || differentEnvironment);
		
		m_Widgets.m_DownloadButtonComponent.SetVisible(!downloaded && !downloading);
		m_Widgets.m_DeleteButtonComponent.SetVisible(downloaded);
		m_Widgets.m_DownloadingButtonComponent.SetVisible(downloading);

		// Solve issues button
		// Visible only when there are any issues
		bool solveIssuesVisible = problem != EWorkshopItemProblem.NO_PROBLEM;
		m_Widgets.m_SolveIssuesButton.SetVisible(solveIssuesVisible);
		if (solveIssuesVisible)
		{
			SCR_ButtonEffectText textEffect = SCR_ButtonEffectText.Cast(m_Widgets.m_SolveIssuesButtonComponent.FindEffect("text"));
			textEffect.m_sDefault = primaryActionText;
			textEffect.m_sHovered = primaryActionText;
			textEffect.PropertiesChanged();
		}

		// Vote up/down
		bool enabled = item.GetOnline();
		m_Widgets.m_VoteUpButtonComponent.SetEnabled(enabled);
		m_Widgets.m_VoteDownButtonComponent.SetEnabled(enabled);

		bool rating, ratingSet;
		item.GetMyRating(ratingSet, rating);

		bool voteUpToggled = ratingSet && rating;
		bool voteDownToggled = ratingSet && !rating;

		m_Widgets.m_VoteUpButtonComponent.SetToggled(voteUpToggled, false);
		m_Widgets.m_VoteDownButtonComponent.SetToggled(voteDownToggled, false);

		//! Favourite button
		m_Widgets.m_FavoriteButtonComponent.SetToggled(m_WorkshopItem.GetFavourite(), false);

		//! Navigation buttons
		UpdateNavigationButtons();
	}

	//------------------------------------------------------------------------------------------------
	//! m_Widgets for addon properties like description, name, which can not change
	protected void UpdateAddonStaticWidgets()
	{
		if (!m_WorkshopItem)
			return;

		// Name
		m_Widgets.m_NameText.SetText(m_WorkshopItem.GetName());

		// Description
		string description = m_WorkshopItem.GetDescription();
		m_Widgets.m_DescriptionText.SetText(description);

		// Author
		m_Widgets.m_AuthorNameText.SetText(m_WorkshopItem.GetAuthorName());

		// Rating
		int ratingCount = m_WorkshopItem.GetRatingCount();
		float avgRating = m_WorkshopItem.GetAverageRating();
		avgRating = Math.Round(avgRating * 100.0);
		m_Widgets.m_RatingText.SetText(string.Format("%1%% (%2x)", avgRating, ratingCount));
	}


	//------------------------------------------------------------------------------------------------
	protected void UpdateVersionComboBox()
	{
		#ifdef WORKSHOP_DEBUG
			// Fill version combo box - only for testing!
			if (m_Widgets.m_VersionComboBoxComponent0.GetNumItems() == 0)
			{
				array<string> versions = m_WorkshopItem.GetVersions();
				if (!versions.IsEmpty())
				{
					m_Widgets.m_VersionComboBoxComponent0.ClearAll();
					foreach (string version : versions)
					{
						m_Widgets.m_VersionComboBoxComponent0.AddItem(version);
					}
				}
			}
		#endif
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateGallery()
	{
		if (m_bGalleryUpdated)
			return;

		array<BackendImage> galleryImages = m_WorkshopItem.GetGallery();

		m_Widgets.m_Gallery.SetVisible(!galleryImages.IsEmpty());

		if (!galleryImages.IsEmpty())
		{
			m_Widgets.m_GalleryComponent.SetImages(galleryImages);
			m_bGalleryUpdated = true;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateScenarioLines()
	{
		bool modDownloaded = m_WorkshopItem.GetOffline();
		foreach (SCR_ContentBrowser_ScenarioLineComponent scenarioLine : m_aScenarioLines)
		{
			if (scenarioLine)
			{
				// Show favorites and mouse buttons only when downloaded
				scenarioLine.ShowFavouriteButton(modDownloaded);
				scenarioLine.ShowMouseInteractionButtons(modDownloaded);
				scenarioLine.ShowLastPlayedText(modDownloaded);
				scenarioLine.NotifyScenarioUpdate();
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateSidePanel()
	{
		SCR_ContentBrowser_ScenarioLineComponent lineComp = GetSelectedLine();
		if (lineComp)
		{
			MissionWorkshopItem scenario = lineComp.GetScenario();
			if (scenario)
				m_Widgets.m_ScenarioDetailsPanelComponent.SetScenario(scenario);
		}

		m_Widgets.m_AddonDetailsPanel.SetVisible(!lineComp);
		m_Widgets.m_ScenarioDetailsPanel.SetVisible(lineComp != null);
	}

	//------------------------------------------------------------------------------------------------
	protected void LineInteractionTryDownload(SCR_ContentBrowser_ScenarioLineComponent line)
	{
		// Check if the addon is downloaded
		if (!line || (m_WorkshopItem && m_WorkshopItem.GetOffline()))
			return;

		//! START DOWNLOAD
		// Clicking a line without the addon installed starts the download
		bool inProgress, paused;
		float progress;
		Revision revision;
		m_WorkshopItem.GetDownloadState(inProgress, paused, progress, revision);

		if (!inProgress && !paused)
			OnDownloadButton();
	}
}
