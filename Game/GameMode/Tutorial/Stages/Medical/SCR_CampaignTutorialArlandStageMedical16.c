[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStageMedical16Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStageMedical16: SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_bCheckWaypoint = false;
		m_fWaypointHeightOffset = 0.5;
		
		RegisterWaypoint("Figurant");
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
		
		GetGame().GetCallqueue().Remove(m_TutorialComponent.RefreshVictimBloodLevel);
	}
	
	override protected bool GetIsFinished()
	{	
		SCR_CharacterDamageManagerComponent damManager = m_TutorialComponent.GetVictimDamageManager();
		if (!damManager)
			return false;

		return damManager.GetGroupSalineBagged(ECharacterHitZoneGroup.RIGHTARM) || damManager.GetGroupSalineBagged(ECharacterHitZoneGroup.LEFTARM);
	}
};
