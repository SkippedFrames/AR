[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStageMovement15Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStageMovement15 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_LADDER_OFF");
		m_fWaypointCompletionRadius = 3;
		m_fWaypointHeightOffset = 0;
		m_bCheckWaypoint = false;
		m_TutorialComponent.SetWaypointMiscImage("LADDER", true);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return !m_TutorialComponent.CheckCharacterStance(ECharacterCommandIDs.LADDER) && vector.DistanceSq(m_Player.GetOrigin(), GetWaypoint().GetOrigin()) < 2;
	}
};
