[EntityEditorProps(insertable: false)]
class SCR_BuildingTutorialStage14Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_BuildingTutorialStage14 : SCR_BaseCampaignTutorialArlandStage
{
	protected Vehicle m_SupplyTruck;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
		
		m_SupplyTruck = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("BuildingSupplyTruck"));
		if (!m_SupplyTruck)
			return;
		
		m_bCheckWaypoint = false;
		RegisterWaypoint(m_SupplyTruck);
		m_TutorialComponent.SetWaypointMiscImage("GETIN", true);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_SupplyTruck)
			return false;
		
		return m_SupplyTruck.GetPilot() == m_Player;
	}
};
