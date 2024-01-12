/*
Component for a line in scenario browser.
*/
class SCR_ContentBrowser_ScenarioLineComponent : SCR_ListMenuEntryComponent
{
	protected ref SCR_ContentBrowser_ScenarioLineWidgets m_Widgets = new SCR_ContentBrowser_ScenarioLineWidgets;

	protected MissionWorkshopItem m_Mission;
	protected ref SCR_MissionHeader m_Header;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
		m_Widgets.Init(w);

		if (!GetGame().InPlayMode())
			return;

		// Mouse interaction buttons
		m_FavComponent = m_Widgets.m_FavouriteButtonComponent;

		m_aMouseButtons.Insert(m_FavComponent);
		m_aMouseButtons.Insert(m_Widgets.m_PlayComponent);
		m_aMouseButtons.Insert(m_Widgets.m_ContinueComponent);
		m_aMouseButtons.Insert(m_Widgets.m_RestartComponent);
		m_aMouseButtons.Insert(m_Widgets.m_HostComponent);
		m_aMouseButtons.Insert(m_Widgets.m_FindServersComponent);

		m_Widgets.m_PlayComponent.m_OnClicked.Insert(OnPlay);
		m_Widgets.m_ContinueComponent.m_OnClicked.Insert(OnContinue);
		m_Widgets.m_RestartComponent.m_OnClicked.Insert(OnRestart);
		m_Widgets.m_HostComponent.m_OnClicked.Insert(OnHost);
		m_Widgets.m_FindServersComponent.m_OnClicked.Insert(OnFindServers);

		SCR_ServicesStatusHelper.RefreshPing();
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Insert(OnCommStatusCheckFinished);
		
		super.HandlerAttached(w);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Remove(OnCommStatusCheckFinished);
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateModularButtons()
	{
		if (!m_Mission)
			return;

		// Visibility
		bool mp = m_Mission.GetPlayerCount() > 1;
		bool bIsAddonReady = true;
		
		if (!m_Mission.GetOwner())
			m_Header = SCR_MissionHeader.GetMissionHeader(m_Mission);
		else
			bIsAddonReady = m_Mission.GetOwner().IsReadyToRun();
		
		bool canContinue = m_Header && GetGame().GetSaveManager().HasLatestSave(m_Header);
		bool show = m_bMouseButtonsEnabled && m_bFocused && bIsAddonReady;

		m_Widgets.m_Play.SetVisible(show && !canContinue);
		m_Widgets.m_Continue.SetVisible(show && canContinue);
		m_Widgets.m_Restart.SetVisible(canContinue);
		
		m_Widgets.m_Host.SetVisible(show && !GetGame().IsPlatformGameConsole() && mp);
		m_Widgets.m_Host.SetEnabled(SCR_ServicesStatusHelper.AreMultiplayerServicesAvailable());
		
		m_Widgets.m_FindServers.SetVisible(show && mp);
		m_Widgets.m_FindServers.SetEnabled(SCR_ServicesStatusHelper.AreMultiplayerServicesAvailable());

		super.UpdateModularButtons();
	}

	//------------------------------------------------------------------------------------------------
	override bool SetFavorite(bool favorite)
	{
		if (!m_Mission || !super.SetFavorite(favorite))
			return false;

		m_Mission.SetFavorite(favorite);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateAllWidgets()
	{
		if (!m_Mission)
			return;

		// Fav button
		if (m_Mission)
			SetFavorite(m_Mission.IsFavorite());

		//! Name
		m_Widgets.m_NameText.SetText(m_Mission.Name());

		//! Game mode
		/*
		// Not supported yet - todo
		*/

		//! Type/player count
		int playerCount = m_Mission.GetPlayerCount();
		bool mp = playerCount > 1;
		bool sp = !mp;

		m_Widgets.m_SinglePlayerImage.SetVisible(!m_bMouseButtonsEnabled && sp);
		m_Widgets.m_MultiPlayerImage.SetVisible(!m_bMouseButtonsEnabled && mp);
		m_Widgets.m_PlayerCountText.SetText(playerCount.ToString());

		//! Last played time text
		int timeSinceLastPlayedSeconds = m_Mission.GetTimeSinceLastPlay();
		string timeSinceLastPlayText;

		if (timeSinceLastPlayedSeconds > 0)
			timeSinceLastPlayText = SCR_FormatHelper.GetTimeSinceEventImprecise(timeSinceLastPlayedSeconds);
		else
			timeSinceLastPlayText = "-";

		m_Widgets.m_LastPlayedText.SetText(timeSinceLastPlayText);

		//! Source addon
		WorkshopItem sourceAddon = m_Mission.GetOwner();
		m_Widgets.m_SourceImageOfficial.SetVisible(sourceAddon == null);
		m_Widgets.m_SourceImageCommunity.SetVisible(sourceAddon != null);
		m_Widgets.m_SourceNameTextOfficial.SetVisible(sourceAddon == null);
		m_Widgets.m_SourceNameTextCommunity.SetVisible(sourceAddon != null);
		string sourceAddonText = string.Empty;
		if (sourceAddon)
			m_Widgets.m_SourceNameTextCommunity.SetText(sourceAddon.Name());
		else
			m_Widgets.m_SourceNameTextOfficial.SetText("#AR-Editor_Attribute_OverlayLogo_Reforger");

		//! Modular buttons
		UpdateModularButtons();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlay()
	{
		if (m_OnMouseInteractionButtonClicked)
			m_OnMouseInteractionButtonClicked.Invoke("Play");
	}

	//------------------------------------------------------------------------------------------------
	protected void OnContinue()
	{
		if (m_OnMouseInteractionButtonClicked)
			m_OnMouseInteractionButtonClicked.Invoke("Continue");
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRestart()
	{
		if (m_OnMouseInteractionButtonClicked)
			m_OnMouseInteractionButtonClicked.Invoke("Restart");
	}

	//------------------------------------------------------------------------------------------------
	protected void OnHost()
	{
		if (m_OnMouseInteractionButtonClicked)
			m_OnMouseInteractionButtonClicked.Invoke("Host");
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFindServers()
	{
		if (m_OnMouseInteractionButtonClicked)
			m_OnMouseInteractionButtonClicked.Invoke("FindServers");
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCommStatusCheckFinished(SCR_ECommStatus status, float responseTime, float lastSuccessTime, float lastFailTime)
	{
		UpdateModularButtons();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Must be called when the scenario properties have been updated (set/clear favourite or similar)
	void NotifyScenarioUpdate()
	{
		UpdateAllWidgets();
	}

	//------------------------------------------------------------------------------------------------
	void SetScenario(MissionWorkshopItem mission)
	{
		m_Mission = mission;

		UpdateAllWidgets();
	}

	//------------------------------------------------------------------------------------------------
	MissionWorkshopItem GetScenario()
	{
		return m_Mission;
	}

	//------------------------------------------------------------------------------------------------
	void ShowFavouriteButton(bool show)
	{
		m_Widgets.m_FavouriteButtonOverlay.SetVisible(show);
	}

	//------------------------------------------------------------------------------------------------
	void ShowName(bool show)
	{
		m_Widgets.m_NameText.SetVisible(show);
	}

	//------------------------------------------------------------------------------------------------
	void ShowPlayerCount(bool show)
	{
		m_Widgets.m_PlayerCount.SetVisible(show);
	}

	//------------------------------------------------------------------------------------------------
	void ShowLastPlayedText(bool show)
	{
		m_Widgets.m_LastPlayedTextSize.SetVisible(show);
	}

	//------------------------------------------------------------------------------------------------
	void ShowSource(bool show)
	{
		m_Widgets.m_Source.SetVisible(show);
	}

	//------------------------------------------------------------------------------------------------
	static SCR_ContentBrowser_ScenarioLineComponent FindComponent(Widget w)
	{
		return SCR_ContentBrowser_ScenarioLineComponent.Cast(w.FindHandler(SCR_ContentBrowser_ScenarioLineComponent));
	}
}
