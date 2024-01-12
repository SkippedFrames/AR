[EntityEditorProps(insertable: false)]
class SCR_TutorialConflictCapture21Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialConflictCapture21 : SCR_BaseCampaignTutorialArlandStage
{
	SCR_CampaignMilitaryBaseComponent m_Base;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		IEntity baseEnt = GetGame().GetWorld().FindEntityByName("MainBaseMossHill");
		if (!baseEnt)
			return;
		
		RegisterWaypoint(baseEnt);
		m_bCheckWaypoint = false;
		m_Base = SCR_CampaignMilitaryBaseComponent.Cast(baseEnt.FindComponent(SCR_CampaignMilitaryBaseComponent));
		
		if (!m_Base)
			return;
		
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
		}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_Base)
			return false;
		
		return (m_Base.GetFaction() == SCR_GameModeCampaign.GetInstance().GetFactionByEnum(SCR_ECampaignFaction.BLUFOR));
	}
};
