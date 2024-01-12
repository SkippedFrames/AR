[EntityEditorProps(insertable: false)]
class SCR_TutorialNavigation6Class: SCR_BaseCampaignTutorialArlandStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialNavigation6 : SCR_BaseCampaignTutorialArlandStage
{
	Widget m_wHighlight;
	SCR_MapToolEntry m_RulerTool;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{	
		SCR_MapEntity.GetOnMapOpen().Remove(OnMapOpened);
		SCR_MapEntity.GetOnMapClose().Remove(OnMapClosed);
		SCR_MapEntity.GetOnMapOpen().Insert(OnMapOpened);
		SCR_MapEntity.GetOnMapClose().Insert(OnMapClosed);
		
		SCR_HintManagerComponent.ShowHint(m_TutorialHintList.GetHint(m_TutorialComponent.GetStage()));
		
		HighlightIcon();
		
		m_fDuration = 40;
	}
	//------------------------------------------------------------------------------------------------
	protected void SkipTimer()
	{
		m_fDuration = 0;
		
		if (m_RulerTool)
			m_RulerTool.m_OnClick.Remove(SkipTimer);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void HighlightIcon()
	{
		SCR_MapEntity mapEnt = SCR_MapEntity.GetMapInstance();
		if (!mapEnt)
			return;
		
		SCR_MapToolMenuUI toolMenuUI = SCR_MapToolMenuUI.Cast(mapEnt.GetMapUIComponent(SCR_MapToolMenuUI));
		if (!toolMenuUI)
			return;	
		
		array<ref SCR_MapToolEntry> tools = toolMenuUI.GetMenuEntries();
		
		Widget toolButton;
		foreach (int i, SCR_MapToolEntry tool : tools)
		{
			if (tool.m_sIconQuad == "ruler")
			{
				toolButton = GetGame().GetWorkspace().FindAnyWidget("ToolMenuButton"+i);
				m_RulerTool = tool;
			}
		}
		
		if (!m_RulerTool)
			return;
		
		m_wHighlight = SCR_WidgetHighlightUIComponent.CreateHighlight(toolButton, "{D574871D2C37B255}UI/layouts/Common/WidgetHighlight.layout");
		m_RulerTool.m_OnClick.Insert(SkipTimer);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMapClosed(MapConfiguration config)
	{
		GetGame().GetCallqueue().Remove(HighlightIcon);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMapOpened(MapConfiguration config)
	{
		//Delayed call, so all map widgets are properly initialized
		GetGame().GetCallqueue().CallLater(HighlightIcon, 100);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_TutorialComponent.GetIsMapOpen())
			return true;
		
		return m_RulerTool.IsEntryActive();
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_TutorialNavigation6()
	{
		if (m_RulerTool)
			m_RulerTool.m_OnClick.Remove(SkipTimer);
		
		delete m_wHighlight;
	}
};
