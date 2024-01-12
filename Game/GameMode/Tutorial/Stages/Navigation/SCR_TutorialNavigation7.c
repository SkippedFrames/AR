[EntityEditorProps(insertable: false)]
class SCR_TutorialNavigation7Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialNavigation7 : SCR_BaseCampaignTutorialArlandStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
		
		SCR_MapEntity.GetOnMapClose().Remove(m_TutorialComponent.OnMapClose);
		SCR_MapEntity.GetOnMapClose().Insert(m_TutorialComponent.OnMapClose);
		
		SCR_MapEntity mapEnt = SCR_MapEntity.GetMapInstance();
		if (!mapEnt)
			return;
		
		SCR_MapToolMenuUI toolMenuUI = SCR_MapToolMenuUI.Cast(mapEnt.GetMapUIComponent(SCR_MapToolMenuUI));
		if (!toolMenuUI)
			return;	
		
		array<ref SCR_MapToolEntry> tools = toolMenuUI.GetMenuEntries();
		
		Widget toolButton;
		SCR_MapToolEntry rulerTool;
		foreach (int i, SCR_MapToolEntry tool : tools)
		{
			if (tool.m_sIconQuad == "ruler")
			{
				toolButton = GetGame().GetWorkspace().FindAnyWidget("ToolMenuButton"+i);
				rulerTool = tool;
			}
		}
		
		if (!rulerTool.IsEntryActive())
			rulerTool.m_OnClick.Invoke();
		
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return !m_TutorialComponent.GetIsMapOpen();
	}
}
