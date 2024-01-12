[EntityEditorProps(insertable: false)]
class SCR_TutorialNavigation12Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialNavigation12 : SCR_BaseCampaignTutorialArlandStage
{
	float m_fAngle;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_fConditionCheckPeriod = 1;
		
		IEntity lighthousePos = GetGame().GetWorld().FindEntityByName("ChurchPos");
		m_fAngle = m_TutorialComponent.GetEntityCompassAngle(lighthousePos);
		
		SCR_HintUIInfo info = m_TutorialHintList.GetHint(m_TutorialComponent.GetStage());
		
		LocalizedString description = info.GetUnformattedDescription();
		
		if (m_fAngle < 0)
			description = string.Format(WidgetManager.Translate(description), m_fAngle + 360);
		else
			description = string.Format(WidgetManager.Translate(description), m_fAngle);
		
		info.SetDescription(description);
		
		SCR_HintManagerComponent.ShowHint(info);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_TutorialComponent.IsMyAngleInRange(m_fAngle, 5);
	}
};
