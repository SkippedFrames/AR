[EntityEditorProps(insertable: false)]
class SCR_BuildingTutorialStage18Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_BuildingTutorialStage18 : SCR_BaseCampaignTutorialArlandStage
{
	protected IEntity m_Base;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("TeleportRadio2");
		m_fWaypointCompletionRadius = 5;
		
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
		
		m_Base = GetGame().GetWorld().FindEntityByName("TownBaseFarm");
		if (!m_Base)
			return;
		
		GetGame().GetCallqueue().CallLater(CheckVicinity, 1000, true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CheckVicinity()
	{
		vector playerPos = m_Player.GetOrigin();
		float sqDistance = vector.DistanceSq(playerPos, m_Base.GetOrigin());
		
		if (sqDistance > 10000)
			m_TutorialComponent.SetActiveConfig(SCR_ETutorialArlandStageMasters.HUB);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_BuildingTutorialStage18()
	{
		GetGame().GetCallqueue().Remove(CheckVicinity);
	}
};
