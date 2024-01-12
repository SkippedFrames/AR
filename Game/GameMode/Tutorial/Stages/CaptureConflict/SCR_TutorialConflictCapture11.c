[EntityEditorProps(insertable: false)]
class SCR_TutorialConflictCapture11Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialConflictCapture11 : SCR_BaseCampaignTutorialArlandStage
{
	IEntity m_MHQ, m_WP;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fWaypointCompletionRadius = 10;
		RegisterWaypoint("WP_CONFLICT_SEIZING_2");
		
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
	
		m_MHQ = GetGame().GetWorld().FindEntityByName("MobileHQ");
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_WP)
			m_WP = GetGame().GetWorld().FindEntityByName("WP_CONFLICT_SEIZING_2");
		
		return vector.DistanceSq(m_MHQ.GetOrigin(), m_WP.GetOrigin()) <= 100;
	}
};
