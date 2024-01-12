[EntityEditorProps(insertable: false)]
class SCR_BuildingTutorialStage15Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_BuildingTutorialStage15 : SCR_BaseCampaignTutorialArlandStage
{
	protected IEntity m_SupplyTruck, m_WP;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fWaypointCompletionRadius = 50;
		RegisterWaypoint("TownBaseFarm");
		
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
	
		m_SupplyTruck = GetGame().GetWorld().FindEntityByName("BuildingSupplyTruck");
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_WP)
			m_WP = GetGame().GetWorld().FindEntityByName("TownBaseFarm");
		
		return vector.DistanceSq(m_SupplyTruck.GetOrigin(), m_WP.GetOrigin()) <= 2500;
	}
};
