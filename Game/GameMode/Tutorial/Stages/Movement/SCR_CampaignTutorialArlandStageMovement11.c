[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStageMovement11Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStageMovement11 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_WALL2");
		m_fWaypointCompletionRadius = 5;
		m_fWaypointHeightOffset = 0.3;
		GetGame().GetCallqueue().Remove(m_TutorialComponent.Check3rdPersonViewUsed);
		m_TutorialComponent.SetWaypointMiscImage("JUMP", true);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_TutorialComponent.CheckCharacterStance(ECharacterCommandIDs.CLIMB);
	}
};
