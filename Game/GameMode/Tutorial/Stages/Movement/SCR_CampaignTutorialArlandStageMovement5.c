[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStageMovement5Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStageMovement5 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fWaypointHeightOffset = 0.2;
		RegisterWaypoint("WP_WIREMESH");
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		CharacterControllerComponent comp = CharacterControllerComponent.Cast(m_Player.FindComponent(CharacterControllerComponent));
					
		if (comp)
			return comp.GetStance() == ECharacterStance.PRONE;
		else
			return true;
	}
};
