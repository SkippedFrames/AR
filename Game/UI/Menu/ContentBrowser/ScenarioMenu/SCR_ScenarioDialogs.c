/*!
Classes for Scenario dialogs
*/
void ScriptInvokerMissionWorkshopItemMethod(MissionWorkshopItem item);
typedef func ScriptInvokerMissionWorkshopItemMethod;
typedef ScriptInvokerBase<ScriptInvokerMissionWorkshopItemMethod> ScriptInvokerMissionWorkshopItem;

//------------------------------------------------------------------------------------------------
class SCR_ScenarioDialogs
{
	static protected const ResourceName DIALOGS_CONFIG = "{F020A20CC93DB3C7}Configs/ContentBrowser/ScenarioDialogs.conf";

	//------------------------------------------------------------------------------------------------
	static SCR_ConfigurableDialogUi CreateDialog(string presetName)
	{
		return SCR_ConfigurableDialogUi.CreateFromPreset(DIALOGS_CONFIG, presetName);
	}

	//------------------------------------------------------------------------------------------------
	static SCR_ScenarioConfirmationDialogUi CreateScenarioConfirmationDialog(MissionWorkshopItem scenario, ScriptInvokerBool onFavoritesResponse = null)
	{
		SCR_ScenarioConfirmationDialogUi dialogUI = new SCR_ScenarioConfirmationDialogUi(scenario, onFavoritesResponse);
		SCR_ConfigurableDialogUi.CreateFromPreset(DIALOGS_CONFIG, "SCENARIO_CONFIRMATION", dialogUI);

		return dialogUI;
	}
}

//------------------------------------------------------------------------------------------------
class SCR_ScenarioConfirmationDialogUi : SCR_ConfigurableDialogUi
{
	MissionWorkshopItem m_Scenario;

	protected SCR_InputButtonComponent m_Favorite;
	protected SCR_InputButtonComponent m_Host;
	protected SCR_InputButtonComponent m_FindServers;
	
	protected SCR_ModularButtonComponent m_FavoriteStarButton;

	//! If true, the dialog itself will set the scenario favorite state, otherwise it will live it to the menu or handler class
	protected bool m_bHandleFavoriting = true;

	protected ref ScriptInvokerMissionWorkshopItem m_OnFavorite;

	//This should probably be a setting in SCR_HorizontalScrollAnimationComponent, as this is a bandaid solution to the title flickering
	protected const int MAX_TITLE_LENGTH = 55;

	//------------------------------------------------------------------------------------------------
	void SCR_ScenarioConfirmationDialogUi(MissionWorkshopItem scenario, ScriptInvokerBool onFavoritesResponse = null)
	{
		m_Scenario = scenario;

		if (onFavoritesResponse)
		{
			m_bHandleFavoriting = false;
			onFavoritesResponse.Insert(UpdateFavoriteWidgets);
		}
	}

	//! OVERRIDES
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		super.OnMenuOpen(preset);

		if (!m_Scenario)
			return;

		// Connection state
		SCR_ServicesStatusHelper.RefreshPing();
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Insert(OnCommStatusCheckFinished);
		
		//! Update visuals
		SetTitle(m_Scenario.Name());

		Widget backgroundImageBackend = GetRootWidget().FindAnyWidget("BackgroundImageBackend");
		if (backgroundImageBackend)
		{
			SCR_ScenarioBackendImageComponent backendImageComp = SCR_ScenarioBackendImageComponent.Cast(backgroundImageBackend.FindHandler(SCR_ScenarioBackendImageComponent));
			if (backendImageComp)
				backendImageComp.SetScenarioAndImage(m_Scenario, m_Scenario.Thumbnail());
		}

		//! Content layout
		Widget contentLayoutRoot = GetContentLayoutRoot(GetRootWidget());
		if (!contentLayoutRoot)
			return;

		Widget singlePlayerImage = contentLayoutRoot.FindAnyWidget("SinglePlayerImageOverlay");
		Widget multiPlayerImage = contentLayoutRoot.FindAnyWidget("MultiPlayerImageOverlay");
		TextWidget playerCountText = TextWidget.Cast(contentLayoutRoot.FindAnyWidget("PlayerCountText"));
		TextWidget playerCountLabelText = TextWidget.Cast(contentLayoutRoot.FindAnyWidget("PlayerCountLabelText"));

		Widget sourceImageOfficial = contentLayoutRoot.FindAnyWidget("SourceImageOfficialOverlay");
		Widget sourceImageCommunity = contentLayoutRoot.FindAnyWidget("SourceImageCommunityOverlay");
		TextWidget sourceNameTextOfficial = TextWidget.Cast(contentLayoutRoot.FindAnyWidget("SourceNameTextOfficial"));
		TextWidget sourceNameTextCommunity = TextWidget.Cast(contentLayoutRoot.FindAnyWidget("SourceNameTextCommunity"));

		//! Type and player count
		int playerCount = m_Scenario.GetPlayerCount();
		bool mp = playerCount > 1;
		singlePlayerImage.SetVisible(!mp);
		multiPlayerImage.SetVisible(mp);
		if (mp)
		{
			playerCountText.SetText(playerCount.ToString());
			playerCountLabelText.SetText("#AR-ServerBrowser_ServerPlayers");
		}

		//! Source addon
		bool isSourceAddonValid;
		WorkshopItem sourceAddon = m_Scenario.GetOwner();

		if (sourceAddon)
		{
			isSourceAddonValid = true;
			sourceNameTextCommunity.SetText(sourceAddon.Name());
		}
		else
		{
			isSourceAddonValid = false;
		}

		sourceImageOfficial.SetVisible(!isSourceAddonValid);
		sourceImageCommunity.SetVisible(isSourceAddonValid);
		sourceNameTextOfficial.SetVisible(!isSourceAddonValid);
		sourceNameTextCommunity.SetVisible(isSourceAddonValid);

		//! Buttons
		SCR_MissionHeader header = SCR_MissionHeader.Cast(MissionHeader.ReadMissionHeader(m_Scenario.Id()));
		bool canBeLoaded = header && GetGame().GetSaveManager().HasLatestSave(header);

		SCR_InputButtonComponent confirm = FindButton("confirm");
		if (confirm)
			confirm.SetVisible(!canBeLoaded);

		SCR_InputButtonComponent load = FindButton("load");
		if (load)
			load.SetVisible(canBeLoaded);

		SCR_InputButtonComponent restart = FindButton("restart");
		if (restart)
			restart.SetVisible(canBeLoaded, false);

		m_FindServers = FindButton("join");
		if (m_FindServers)
		{
			m_FindServers.SetVisible(mp, false);
			SCR_ServicesStatusHelper.SetConnectionButtonEnabled(m_FindServers, SCR_ServicesStatusHelper.SERVICE_BI_BACKEND_MULTIPLAYER);
		}

		m_Host = FindButton("host");
		if (m_Host)
		{
			m_Host.SetVisible(mp && !GetGame().IsPlatformGameConsole() /*&& SCR_ContentBrowser_ScenarioSubMenu.GetHostingAllowed()*/, false);
			SCR_ServicesStatusHelper.SetConnectionButtonEnabled(m_Host, SCR_ServicesStatusHelper.SERVICE_BI_BACKEND_MULTIPLAYER);
		}

		m_Favorite = FindButton("favorite");
		if (m_Favorite)
			m_Favorite.m_OnActivated.Insert(OnFavoritesButton);

		//! Star button
		Widget favButton = m_wRoot.FindAnyWidget("FavoriteButton");
		if (favButton)
		{
			m_FavoriteStarButton = SCR_ModularButtonComponent.FindComponent(favButton);
			if (m_FavoriteStarButton)
				m_FavoriteStarButton.m_OnClicked.Insert(OnFavoritesButton);
		}

		//! Favorites widgets update
		UpdateFavoriteWidgets(m_Scenario.IsFavorite());
	}

	//----------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Remove(OnCommStatusCheckFinished);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnButtonPressed(SCR_InputButtonComponent button)
	{
		super.OnButtonPressed(button);

		if (m_sLastPressedButtonTag != "favorite")
			Close();
	}

	//------------------------------------------------------------------------------------------------
	override void SetTitle(string text)
	{
		super.SetTitle(text);

		Widget titleFrame = m_wRoot.FindAnyWidget("TitleFrame");
		if (!titleFrame)
			return;

		SCR_HorizontalScrollAnimationComponent scrollComp = SCR_HorizontalScrollAnimationComponent.Cast(titleFrame.FindHandler(SCR_HorizontalScrollAnimationComponent));
		if (!scrollComp)
			return;

		if (text.Length() < MAX_TITLE_LENGTH)
			scrollComp.AnimationStop();
		else
			scrollComp.AnimationStart();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCommStatusCheckFinished(SCR_ECommStatus status, float responseTime, float lastSuccessTime, float lastFailTime)
	{
		if (m_FindServers)
			SCR_ServicesStatusHelper.SetConnectionButtonEnabled(m_FindServers, SCR_ServicesStatusHelper.SERVICE_BI_BACKEND_MULTIPLAYER);

		if (m_Host)
			SCR_ServicesStatusHelper.SetConnectionButtonEnabled(m_Host, SCR_ServicesStatusHelper.SERVICE_BI_BACKEND_MULTIPLAYER);
	}
	
	//! PROTECTED
	//------------------------------------------------------------------------------------------------
	protected void OnFavoritesButton()
	{
		if (!m_Scenario)
			return;

		if (!m_bHandleFavoriting && m_OnFavorite)
		{
			m_OnFavorite.Invoke(m_Scenario);
			return;
		}

		bool isFavorite = !m_Scenario.IsFavorite();
		m_Scenario.SetFavorite(isFavorite);
		UpdateFavoriteWidgets(isFavorite);
	}


	//------------------------------------------------------------------------------------------------
	protected void UpdateFavoriteWidgets(bool isFavorite)
	{
		// Footer Button
		string label = UIConstants.FAVORITE_LABEL_ADD;
		if (isFavorite)
			label = UIConstants.FAVORITE_LABEL_REMOVE;

		m_Favorite.SetLabel(label);

		// Star Button
		if (m_FavoriteStarButton)
			m_FavoriteStarButton.SetToggled(isFavorite, false);
	}

	//! PUBLIC
	//------------------------------------------------------------------------------------------------
	MissionWorkshopItem GetScenario()
	{
		return m_Scenario;
	}
	
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerMissionWorkshopItem GetOnFavorite()
	{
		if (!m_OnFavorite)
			m_OnFavorite = new ScriptInvokerMissionWorkshopItem();

		return m_OnFavorite;
	}
}
