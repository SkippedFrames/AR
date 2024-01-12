[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStageMedical17Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStageMedical17: SCR_BaseCampaignTutorialArlandStage
{
	protected SCR_CharacterInventoryStorageComponent m_PlayerInventoryManager;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_bCheckWaypoint = false;
		RegisterWaypoint("Ambulance");
		m_fWaypointHeightOffset = 0.5;
		
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
		
		m_PlayerInventoryManager = SCR_CharacterInventoryStorageComponent.Cast(m_Player.FindComponent(SCR_CharacterInventoryStorageComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{	
		if (!m_PlayerInventoryManager)
			return false;
		
		IEntity ent = m_PlayerInventoryManager.GetCurrentItem();
		if (!ent)
			return false;
		
		SCR_ConsumableItemComponent consumItem = SCR_ConsumableItemComponent.Cast(ent.FindComponent(SCR_ConsumableItemComponent));
		
		return consumItem && consumItem.GetConsumableType() == SCR_EConsumableType.MORPHINE;
	}
};
