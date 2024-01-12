[EntityEditorProps(insertable: false)]
class SCR_TutorialNavigation4Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialNavigation4 : SCR_BaseCampaignTutorialArlandStage
{
	Widget m_wHighlight;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{	
		m_fDuration = 20;
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
		
		Widget toolMenu = GetGame().GetWorkspace().FindAnyWidget("ToolMenu");
		
		if (toolMenu)
			m_wHighlight = SCR_WidgetHighlightUIComponent.CreateHighlight(toolMenu, "{D574871D2C37B255}UI/layouts/Common/WidgetHighlight.layout");
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_TutorialNavigation4()
	{
		delete m_wHighlight;
	}
};
