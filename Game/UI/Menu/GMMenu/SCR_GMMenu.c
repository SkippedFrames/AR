//------------------------------------------------------------------------------------------------
class SCR_GMMenuEntry : Managed
{
	bool m_bIsFeatured;
	bool m_bIsRecent;
	bool m_bIsRecommended;
	ref MissionWorkshopItem m_Header;
	SCR_GMMenuTileComponent m_Tile;
	Widget m_wRoot;

	void SCR_GMMenuEntry(MissionWorkshopItem h, bool featured, bool recent, bool recommended)
	{
		m_Header = h;
		m_bIsFeatured = featured;
		m_bIsRecent = recent;
		m_bIsRecommended = recommended;
	}

	void SetTile(SCR_GMMenuTileComponent tile, Widget widget)
	{
		m_Tile = tile;
		m_wRoot = widget;
	}
}

//------------------------------------------------------------------------------------------------
class SCR_GMMenu : ChimeraMenuBase
{
	protected ResourceName m_sLayout = "{02155A85F2DC521F}UI/layouts/Menus/GMMenu/GMMenuTile.layout";
	protected ResourceName m_sConfig = "{CA59D3A983A1BBAB}Configs/GMMenu/GMMenuEntries.conf";
	protected SCR_GalleryComponent m_Gallery;
	protected WorkshopApi m_WorkshopAPI;
	protected SCR_GMMenuTileComponent m_CurrentTile;
	protected Widget m_wRoot;
	protected ref array<ref SCR_GMMenuEntry> m_aEntries = {};

	const int THRESHOLD_RECENTLY_PLAYED = 60 * 60 * 24 * 7;

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		m_wRoot = GetRootWidget();
		m_Gallery = SCR_GalleryComponent.GetGalleryComponent("Gallery", m_wRoot);
		m_WorkshopAPI = GetGame().GetBackendApi().GetWorkshop();

		SCR_InputButtonComponent back = SCR_InputButtonComponent.GetInputButtonComponent("Back", m_wRoot);
		if (back)
			back.m_OnActivated.Insert(OnBack);

		PrepareHeaders();
		PrepareWidgets();

		if (m_aEntries.Count() > 0)
			m_Gallery.SetFocusedItem(0, true);
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnMenuFocusGained()
	{
		if (m_CurrentTile)
			GetGame().GetWorkspace().SetFocusedWidget(m_CurrentTile.GetRootWidget(), true);

		super.OnMenuFocusGained();
	}

	//------------------------------------------------------------------------------------------------
	protected void PrepareHeaders()
	{
		Resource resource = BaseContainerTools.LoadContainer(m_sConfig);
		if (!resource)
			return;

		BaseContainer cont = resource.GetResource().ToBaseContainer();
		if (!cont)
			return;

		array<ResourceName> missions = {};
		cont.Get("m_aGameMasterScenarios", missions);

		if (!missions)
			return;

		foreach (ResourceName str : missions)
		{
			MissionWorkshopItem item = GetMission(str);
			if (!item || !IsUnique(item, m_aEntries))
				continue;

			m_aEntries.Insert(new SCR_GMMenuEntry(item, false, false, false));
		}
	}

	//------------------------------------------------------------------------------------------------
	MissionWorkshopItem GetMission(ResourceName rsc)
	{
		return m_WorkshopAPI.GetInGameScenario(rsc);
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsUnique(MissionWorkshopItem item, array<ref SCR_GMMenuEntry> entries)
	{
		if (!item)
			return false;

		foreach (SCR_GMMenuEntry entry : entries)
		{
			if (!entry || !entry.m_Header)
				continue;

			if (item == entry.m_Header)
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void PrepareWidgets()
	{
		foreach (SCR_GMMenuEntry entry : m_aEntries)
		{
			Widget w = GetGame().GetWorkspace().CreateWidgets(m_sLayout, m_wRoot);
			if (!w)
				continue;

			SCR_GMMenuTileComponent tile = SCR_GMMenuTileComponent.Cast(w.FindHandler(SCR_GMMenuTileComponent));
			if (!tile)
				continue;

			entry.SetTile(tile, tile.m_wRoot);
			tile.m_OnPlay.Insert(OnPlay);
			tile.m_OnContinue.Insert(OnContinue);
			tile.m_OnRestart.Insert(OnRestart);
			tile.m_OnFindServer.Insert(OnFindServer);
			tile.m_OnFocused.Insert(OnTileFocused);
			tile.ShowMission(entry.m_Header, entry.m_bIsFeatured, entry.m_bIsRecent, entry.m_bIsRecommended);

			m_Gallery.AddItem(w);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBack()
	{
		Close();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlay(SCR_GMMenuTileComponent tile)
	{
		GetGame().GetSaveManager().ResetFileNameToLoad();
		TryPlayScenario(tile.m_Item);

		SCR_MenuLoadingComponent.SaveLastMenu(ChimeraMenuPreset.EditorSelectionMenu);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnContinue(SCR_GMMenuTileComponent tile)
	{
		SCR_MissionHeader header = tile.m_Header;
		if (header && !header.GetSaveFileName().IsEmpty())
			GetGame().GetSaveManager().SetFileNameToLoad(header);
		else
			GetGame().GetSaveManager().ResetFileNameToLoad();

		TryPlayScenario(tile.m_Item);

		SCR_MenuLoadingComponent.SaveLastMenu(ChimeraMenuPreset.EditorSelectionMenu);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRestart(SCR_GMMenuTileComponent tile)
	{
		m_CurrentTile = tile;

		SCR_ConfigurableDialogUi dialog = SCR_CommonDialogs.CreateDialog("scenario_restart");
		dialog.m_OnConfirm.Insert(OnRestartConfirmed);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRestartConfirmed()
	{
		GetGame().GetSaveManager().ResetFileNameToLoad();
		SCR_WorkshopUiCommon.TryPlayScenario(m_CurrentTile.m_Item);

		SCR_MenuLoadingComponent.SaveLastMenu(ChimeraMenuPreset.EditorSelectionMenu);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTileFocused(SCR_GMMenuTileComponent tile)
	{
		m_CurrentTile = tile;
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
		TryPlayScenario(m_CurrentTile.m_Item);
	}

	//------------------------------------------------------------------------------------------------
	protected void FindCurrentScenarioServers()
	{
			MissionWorkshopItem scenario = GetGame().GetBackendApi().GetWorkshop().GetInGameScenario(m_CurrentTile.m_Header.GetHeaderResourceName());
			if (!scenario)
				return;

			ServerBrowserMenuUI.OpenWithScenarioFilter(scenario);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayMission()
	{
		// If possible, load the save
		if (!m_CurrentTile || !m_CurrentTile.m_Header)
			return;

		GetGame().GetSaveManager().SetFileNameToLoad(m_CurrentTile.m_Header);
		TryPlayScenario(m_CurrentTile.m_Item);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFindServer(SCR_GMMenuTileComponent tile)
	{
		FindCurrentScenarioServers();
	}
}
