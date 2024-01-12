//------------------------------------------------------------------------------------------------

class SCR_ServerInfoPanelComponent : ScriptedWidgetComponent
{
	SBServerInfo m_ServerInfo;
	
	TextWidget m_ScenarioName;
	TextWidget m_ScenarioSummary;
	ImageWidget m_ScenarioImage;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_ScenarioName = TextWidget.Cast(w.FindAnyWidget(""));
		m_ScenarioSummary = TextWidget.Cast(w.FindAnyWidget("TextDescription"));
		m_ScenarioImage = ImageWidget.Cast(w.FindAnyWidget("Image"));
		
	}
	
	void SetServerInfo(SBServerInfo info)
	{
		m_ServerInfo = info;
		
		m_ScenarioName.SetText(info.Name);
		m_ScenarioSummary.SetText(info.Description);
		//m_ScenarioImage.LoadImageTexture(info.);
		
	}
};
