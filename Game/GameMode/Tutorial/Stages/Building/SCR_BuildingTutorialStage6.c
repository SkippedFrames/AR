[EntityEditorProps(insertable: false)]
class SCR_BuildingTutorialStage6Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_BuildingTutorialStage6 : SCR_BaseCampaignTutorialArlandStage
{
	protected SCR_CampaignMilitaryBaseComponent m_MilitaryBaseComponent;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
		
		m_MilitaryBaseComponent = SCR_CampaignMilitaryBaseComponent.Cast(GetGame().GetWorld().FindEntityByName("TownBaseFarm").FindComponent(SCR_CampaignMilitaryBaseComponent));
		if (!m_MilitaryBaseComponent)
			return;
		
		IEntity ent = m_TutorialComponent.FindBuiltComposition(m_TutorialComponent.VEHICLE_MAINTENANCE_PREFAB);
		
		m_bCheckWaypoint = false;
		RegisterWaypoint(ent);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_TutorialComponent.FindBuiltComposition("{543C31BC05F032B6}PrefabsEditable/Auto/Compositions/Slotted/SlotFlatSmall/E_VehicleMaintenance_S_US_01.et"))
		{
			m_TutorialComponent.SetStage(SCR_ECampaignTutorialArlandStage.CONFLICT_BUILDING_ENTER_BUILDING);
			m_TutorialComponent.SetActiveStage(m_TutorialComponent.GetActiveStage()-4);
			SCR_EntityHelper.DeleteEntityAndChildren(this);
		}
		
		if (!m_MilitaryBaseComponent)
			return false;
		
		SCR_ServicePointComponent service = m_MilitaryBaseComponent.GetServiceByType(SCR_EServicePointType.LIGHT_VEHICLE_DEPOT);
		
		return service;
	}
};
