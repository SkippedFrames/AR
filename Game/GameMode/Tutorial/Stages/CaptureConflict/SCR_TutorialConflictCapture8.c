[EntityEditorProps(insertable: false)]
class SCR_TutorialConflictCapture8Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialConflictCapture8 : SCR_BaseCampaignTutorialArlandStage
{
	SCR_CampaignMilitaryBaseComponent m_Base;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("TownBaseBeauregard");
		m_fWaypointHeightOffset = 0;
		m_bCheckWaypoint = false;
		
		foreach (SCR_FiringRangeTarget target : m_TutorialComponent.GetAllTargets())
		{		
			if (target.GetSetDistance() == 200)
				target.SetState(ETargetState.TARGET_UP);
		}
		
		SCR_HintManagerComponent.HideHint();
		SCR_PopUpNotification.GetInstance().PopupMsg("#AR-Tutorial_Popup_Title-UC", 20, text2: "#AR-Tutorial_Popup_Enemies", category: SCR_EPopupMsgFilter.TUTORIAL);
		
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
		
		m_Base = SCR_CampaignMilitaryBaseComponent.Cast(GetGame().GetWorld().FindEntityByName("TownBaseBeauregard").FindComponent(SCR_CampaignMilitaryBaseComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (m_Base)
			return (m_Base.GetFaction() == SCR_GameModeCampaign.GetInstance().GetFactionByEnum(SCR_ECampaignFaction.BLUFOR));
		else
			return true;
	}
};
