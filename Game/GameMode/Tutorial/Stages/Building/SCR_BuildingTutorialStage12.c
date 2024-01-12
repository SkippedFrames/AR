[EntityEditorProps(insertable: false)]
class SCR_BuildingTutorialStage12Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_BuildingTutorialStage12: SCR_BaseCampaignTutorialArlandStage
{
	protected SCR_CharacterInventoryStorageComponent m_PlayerInventory;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fDuration = 10;
		m_bCheckWaypoint = false;
		m_bConditionPassCheck = true;
		m_PlayerInventory = SCR_CharacterInventoryStorageComponent.Cast(m_Player.FindComponent(SCR_CharacterInventoryStorageComponent));
		
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_PlayerInventory)
			return false;
		
		IEntity ent = m_PlayerInventory.GetCurrentItem();
		if (!ent)
			return false;
		
		SCR_CampaignBuildingGadgetToolComponent gadget = SCR_CampaignBuildingGadgetToolComponent.Cast(ent.FindComponent(SCR_CampaignBuildingGadgetToolComponent));
		return gadget;
	}
};
