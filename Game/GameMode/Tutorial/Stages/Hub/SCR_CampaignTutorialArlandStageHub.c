[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStageHubClass: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStageHub : SCR_BaseCampaignTutorialArlandStage
{	
	protected IEntity m_AreaCenter;
	protected static float MAX_DISTANCE = 10000; //100m
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintUIInfo hints;
//		m_fDuration = 9;
		string board;
		
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		for (int x = 1; x <= 3; x++)
		{
			board = "WP_BOARD_" + x.ToString();
			RegisterWaypoint(board);
		}
		
		array<IEntity> rootItems = {};
		
		GetGame().GetCallqueue().CallLater(DelayedHint, 2000, false);
		
		m_AreaCenter = GetGame().GetWorld().FindEntityByName("HUB_AREA_CENTER");
		if (m_AreaCenter)
			GetGame().GetCallqueue().CallLater(CheckVicinity, 1000, true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DelayedHint()
	{
		if (!m_TutorialComponent.GetFirstRun())
		{
			m_TutorialComponent.SetFirstRun(true);
			GetGame().GetCallqueue().CallLater(DelayedPopup, 2000, false, "#AR-Tutorial_ScenarioName-UC", "#AR-Tutorial_Hint_Start", 15, "", "", "", "");
			if (!m_TutorialHintList)
				SetupHintConfig();
			SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CheckVicinity()
	{
		float sqDistance = vector.DistanceSq(m_Player.GetOrigin(), m_AreaCenter.GetOrigin());
		
		if (sqDistance > MAX_DISTANCE)
			m_TutorialComponent.SetActiveConfig(SCR_ETutorialArlandStageMasters.HUB);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_CampaignTutorialArlandStageHub()
	{
		GetGame().GetCallqueue().Remove(CheckVicinity);
	}
};
