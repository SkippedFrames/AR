[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStageShooting7Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStageShooting7 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fDelay = 1;
		m_TutorialComponent.SetPlayerStanceToCheck(ECharacterStance.PRONE);
		
		foreach (SCR_FiringRangeTarget target : m_TutorialComponent.GetAllTargets())
		{		
			if (target.GetSetDistance() == 125)
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
