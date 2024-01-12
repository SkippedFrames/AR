[EntityEditorProps(insertable: false)]
class SCR_TutorialBallistic1Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

class SCR_TutorialBallistic1: SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_BALLISTIC");
		m_fWaypointCompletionRadius = 2;

    	SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
		//#AR-Tutorial_ShootingRange_MoveToBalisticsProtection
		//MOVE_TO_BALISTIC_PROTECTION
	}
};
