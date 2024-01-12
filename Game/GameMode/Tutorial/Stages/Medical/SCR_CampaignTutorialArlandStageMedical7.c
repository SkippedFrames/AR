[EntityEditorProps(insertable: false)]
class SCR_CampaignTutorialArlandStageMedical7Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTutorialArlandStageMedical7: SCR_BaseCampaignTutorialArlandStage
{
	protected Vehicle m_Jeep;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_bCheckWaypoint = false;

		RegisterWaypoint("AccidentJeep");
		
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
		
		m_Jeep = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("AccidentJeep"));
		m_TutorialComponent.SetWaypointMiscImage("HEAL", true);
	}
		
	override protected bool GetIsFinished()
	{		
		return m_Jeep && !m_Jeep.GetPilot();
	}
};
