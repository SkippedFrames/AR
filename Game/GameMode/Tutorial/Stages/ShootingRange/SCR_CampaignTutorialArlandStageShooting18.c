[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStageShooting18Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStageShooting18 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fDelay = 1;
		RegisterWaypoint("WP_TARGETS_CLOSER");
		m_TutorialComponent.SetWaypointMiscImage("ATTACK", true);
		m_bCheckWaypoint = false;
		foreach (SCR_FiringRangeTarget target : m_TutorialComponent.GetAllTargets())
		{
			if (target.GetSetDistance() == 600)
			{
				target.SetState(ETargetState.TARGET_UP);
				target.Event_TargetChangeState.Remove(m_TutorialComponent.CountTargetHit);
				target.Event_TargetChangeState.Insert(m_TutorialComponent.CountTargetHit);
			}
		}
		
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return (m_TutorialComponent.GetTargetHits() > 2);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_RifleRespawn();
	}
};
