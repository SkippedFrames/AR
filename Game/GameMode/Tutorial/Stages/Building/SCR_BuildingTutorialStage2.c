[EntityEditorProps(insertable: false)]
class SCR_BuildingTutorialStage2Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_BuildingTutorialStage2 : SCR_BaseCampaignTutorialArlandStage
{
	static const int DESIRED_SUPPLIES = 2000;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_BUILDING_INTERACTION");
		m_bCheckWaypoint = false;
		m_fWaypointHeightOffset = 2;
		m_TutorialComponent.SetWaypointMiscImage("CUSTOM", true);
		
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
		
		IEntity baseEnt = GetGame().GetWorld().FindEntityByName("TownBaseFarm");
		if (!baseEnt)
			return;
		
		SCR_CampaignMilitaryBaseComponent baseComp = SCR_CampaignMilitaryBaseComponent.Cast(baseEnt.FindComponent(SCR_CampaignMilitaryBaseComponent));
		if (!baseComp)
			return;
		
		baseComp.SetSupplies(DESIRED_SUPPLIES)
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return IsBuildingModeOpen();
	}
};
