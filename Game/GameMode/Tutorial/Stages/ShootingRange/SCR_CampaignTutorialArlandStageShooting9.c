[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStageShooting9Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStageShooting9 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_FIREPOZ_2");
		m_fWaypointHeightOffset = 0;
		m_bCheckWaypoint = false;
		m_fDelay = 1;
		m_TutorialComponent.SetCheckLeaning(true);
		m_TutorialComponent.SetPlayerDeployedCheck(false);
				
		foreach (SCR_FiringRangeTarget target : m_TutorialComponent.GetAllTargets())
		{		
			if (target.GetSetDistance() == 10)
			{
				target.SetState(ETargetState.TARGET_UP);
				target.SetAutoResetTarget(true);
				target.Event_TargetChangeState.Remove(m_TutorialComponent.CountTargetHit);
				target.Event_TargetChangeState.Insert(m_TutorialComponent.CountTargetHit);
			}
		}
		
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return (m_TutorialComponent.GetTargetHits() > 4);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Reset()
	{
		m_TutorialComponent.StageReset_RifleRespawn();
	}
};
