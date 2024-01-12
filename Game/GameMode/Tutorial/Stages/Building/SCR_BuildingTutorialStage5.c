[EntityEditorProps(insertable: false)]
class SCR_BuildingTutorialStage5Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_BuildingTutorialStage5: SCR_BaseCampaignTutorialArlandStage
{
	protected SCR_CharacterInventoryStorageComponent m_PlayerInventory;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_PlayerInventory = SCR_CharacterInventoryStorageComponent.Cast(m_Player.FindComponent(SCR_CharacterInventoryStorageComponent));
		
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{	
		if (!m_TutorialComponent.FindBuiltComposition(m_TutorialComponent.VEHICLE_MAINTENANCE_PREFAB))
		{
			m_TutorialComponent.SetStage(SCR_ECampaignTutorialArlandStage.CONFLICT_BUILDING_PLACE_SERVICE);
			m_TutorialComponent.SetActiveStage(m_TutorialComponent.GetActiveStage()-3);
			SCR_EntityHelper.DeleteEntityAndChildren(this);
		}
		
		if (!m_PlayerInventory)
			return false;
		
		IEntity ent = m_PlayerInventory.GetCurrentItem();
		if (!ent)
			return false;
		
		SCR_CampaignBuildingGadgetToolComponent gadget = SCR_CampaignBuildingGadgetToolComponent.Cast(ent.FindComponent(SCR_CampaignBuildingGadgetToolComponent));
		return gadget;
	}
};
