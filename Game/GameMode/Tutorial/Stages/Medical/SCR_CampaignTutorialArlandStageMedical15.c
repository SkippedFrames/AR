[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStageMedical15Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStageMedical15: SCR_BaseCampaignTutorialArlandStage
{
	SCR_CharacterInventoryStorageComponent m_PlayerInventoryManager;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_bCheckWaypoint = false;
		m_fWaypointHeightOffset = 0.5;

		RegisterWaypoint("ambulance");
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
		
		m_PlayerInventoryManager = SCR_CharacterInventoryStorageComponent.Cast(m_Player.FindComponent(SCR_CharacterInventoryStorageComponent));
	}
	
	override protected bool GetIsFinished()
	{	
		if (!m_PlayerInventoryManager)
			return false;
		
		IEntity ent = m_PlayerInventoryManager.GetCurrentItem();
		if (!ent)
			return false;
		
		SCR_ConsumableItemComponent consumItem = SCR_ConsumableItemComponent.Cast(ent.FindComponent(SCR_ConsumableItemComponent));
		if (!consumItem)
			return false;
		
		return consumItem.GetConsumableType() == SCR_EConsumableType.SALINE;
	}
};
