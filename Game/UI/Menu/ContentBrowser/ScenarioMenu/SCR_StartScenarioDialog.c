/*
Dialog with confirmation for starting a scenario
*/
class SCR_StartScenarioDialog : DialogUI
{
	ref SCR_StartScenarioDialogWidgets widgets = new SCR_StartScenarioDialogWidgets;
	protected MissionWorkshopItem m_Scenario;
	
	//------------------------------------------------------------------------------------------------
	static SCR_StartScenarioDialog CreateForScenario(MissionWorkshopItem scenario)
	{
		SCR_StartScenarioDialog dlg = SCR_StartScenarioDialog.Cast(GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.StartScenarioDialog, DialogPriority.CRITICAL));
		dlg.Init(scenario);
		return dlg;
	}
	
	
	
	// ----------------------------------- Protected and private -------------------------------------
	
	
	
	//------------------------------------------------------------------------------------------------
	protected void Init(MissionWorkshopItem scenario)
	{
		m_Scenario = scenario;
		SCR_MissionHeader header = SCR_MissionHeader.Cast(scenario.GetHeader());
		
		// Name
		if (header)
			widgets.m_ScenarioNameText.SetText(header.m_sName);
		else if (m_Scenario)
			widgets.m_ScenarioNameText.SetText(scenario.Name());
		
		// Image
		if (header)
		{
			string image = header.m_sIcon;
			if (!image.IsEmpty())
			{
				widgets.m_ScenarioImage.LoadImageTexture(0, image, false, true);
			}
			else
				widgets.m_ScenarioImage.SetVisible(false);
		}
		else
			widgets.m_ScenarioImage.SetVisible(false);
		
		
		// Hide host button for SP scenario
		if (header)
		{
			if (!header.IsMultiplayer())
			{
				widgets.m_HostButton.SetVisible(false);
				widgets.m_HostButton.SetEnabled(false);
			}
		}
	}
	
	
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		widgets.Init(GetRootWidget());
		widgets.m_PlayButtonComponent.m_OnActivated.Insert(OnPlay);
		widgets.m_HostButtonComponent.m_OnActivated.Insert(OnHost);
		widgets.m_JoinButtonComponent.m_OnActivated.Insert(OnJoin);
	}
	
	
	//------------------------------------------------------------------------------------------------
	void OnJoin()
	{
		if (!m_Scenario)
			return;
		
		ServerBrowserMenuUI menu = ServerBrowserMenuUI.Cast(GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.ServerBrowserMenu, 0, true, true));
	}
	
	
	//------------------------------------------------------------------------------------------------
	void OnPlay()
	{
		if (!m_Scenario)
			return;
		
		m_Scenario.Play();
	}
	
	
	//------------------------------------------------------------------------------------------------
	void OnHost()
	{
		if (!m_Scenario)
			return;
		
		ref DSConfig config = new DSConfig;
		ref DSGameConfig game = new DSGameConfig;
		config.game = game;
		game.scenarioId = m_Scenario.Id();
		game.maxPlayers = m_Scenario.GetPlayerCount();
		game.name = m_Scenario.Name() + " - " + System.GetMachineName();
		WorkshopItem hostedMod = m_Scenario.GetOwner();
		if (hostedMod)
			game.hostedScenarioModId = hostedMod.Id();
		
		ref array<WorkshopItem> offlineMods = new array<WorkshopItem>;
		ref array<ref DSMod> modIds = new array<ref DSMod>;
		GetGame().GetBackendApi().GetWorkshop().GetOfflineItems(offlineMods);
		foreach(auto mod : offlineMods)
		{
			ref DSMod modData = new DSMod;
			modData.modId = mod.Id();
			modData.name = mod.Name();
			modData.version = mod.GetActiveRevision().GetVersion();
			modIds.Insert(modData);
		}
		config.game.mods = modIds;
		m_Scenario.Host(config);
	}
	
	
	//------------------------------------------------------------------------------------------------
	override protected void OnConfirm()
	{
		// Do nothing, confirm button is disabled
	}
	
};
