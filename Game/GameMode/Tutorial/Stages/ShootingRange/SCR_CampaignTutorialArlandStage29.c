[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStage29Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStage29 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_DRIVING_0");
		m_fWaypointHeightOffset = 0;
		m_bCheckWaypoint = false;
		m_fDelay = 1;
		
		foreach (SCR_FiringRangeTarget target : m_TutorialComponent.GetAllTargets())
		{		
			if (target.GetSetDistance() == 100)
			{
				target.SetState(ETargetState.TARGET_UP);
				target.Event_TargetChangeState.Remove(m_TutorialComponent.CountTargetHit);
				target.Event_TargetChangeState.Insert(m_TutorialComponent.CountTargetHit);
			}
		}
				
		SCR_HintManagerComponent.ShowCustomHint("#AR-Tutorial_Hint_FiringTurret" + CreateString("#AR-Keybind_Fire","CharacterFire"), duration: -1);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return (m_TutorialComponent.GetTargetHits() > 5);
	}
};
