[EntityEditorProps(insertable: false)]
class SCR_BuildingTutorialStage9Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_BuildingTutorialStage9 : SCR_BaseCampaignTutorialArlandStage
{
	protected IEntity m_SupplyTruck, m_SupplyDepot;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fWaypointCompletionRadius = 10;
		RegisterWaypoint("FIA_SupplyDepot");
		m_TutorialComponent.SetWaypointMiscImage("GETOUT", true);
		
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage(), "sooner"));
	
		m_SupplyTruck = GetGame().GetWorld().FindEntityByName("BuildingSupplyTruck");
		m_SupplyDepot = GetGame().GetWorld().FindEntityByName("FIA_SupplyDepot");
		
		GetGame().GetCallqueue().CallLater(SCR_HintManagerComponent.ShowHint, 30000, false, m_TutorialHintList.GetHint(m_TutorialComponent.GetStage(), "later"));
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_SupplyDepot)
			return false;
		
		return vector.DistanceSq(m_SupplyTruck.GetOrigin(), m_SupplyDepot.GetOrigin()) <= 100;
	}
};
