[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStageMedical13Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStageMedical13: SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_bCheckWaypoint = false;
		m_fWaypointHeightOffset = 0.5;
		
		RegisterWaypoint("Figurant");
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{	
		SCR_CharacterDamageManagerComponent damManager = m_TutorialComponent.GetVictimDamageManager();
		if (!damManager)
			return false;
		
		return !damManager.GetGroupDamageOverTime(ECharacterHitZoneGroup.UPPERTORSO, EDamageType.BLEEDING);
	}
};
