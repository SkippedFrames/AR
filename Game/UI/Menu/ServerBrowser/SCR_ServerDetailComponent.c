// Component for handling server detail panel in server browser
// 
//------------------------------------------------------------------------------------------------

class SCR_ServerDetailsComponent : ScriptedWidgetComponent
{
	// Widget names
	protected const string WIDGET_SCENARIO_IMAGE = "Image";
	protected const string WIDGET_SCENARIO_DESC = "TextDescription";
	
	// Widget
	protected Widget m_wRoot;
	protected ImageWidget m_wScenarioImage; 
	protected TextWidget m_wScenarioDecs;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		// Root 
		m_wRoot = w;
		if(!m_wRoot)
			return;
		
		// Widgets 
		FindWidgets();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void FindWidgets()
	{
		m_wScenarioImage = ImageWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_SCENARIO_IMAGE));
		m_wScenarioDecs = TextWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_SCENARIO_DESC));
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateRoomDetails(Room room)
	{
		// Image setup
		if (!m_wScenarioImage)
			return;
		
		//m_wScenarioImage.SetImage();
		
		// Description setup
		if (!m_wScenarioDecs)
			return;
		
		//m_wScenarioDecs.SetText();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Pass server details info and show it in text
	void SetServerDetails(SBServerInfo serverInfo)
	{
		/*if(m_wServerName)
			m_wServerName.SetText(serverInfo.Name);*/
		
		//SetEntryInfo(m_wPlayers, serverInfo.NumPlayers.ToString() + "/" + serverInfo.MaxPlayers.ToString());
		//SetEntryInfo(m_wIP, serverInfo.HostIp);
		//SetEntryInfo(m_wPort, serverInfo.HostPort.ToString());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetEntryInfo(Widget w, string str)
	{
		if(!w)
			return;
		
		TextWidget wInfo = TextWidget.Cast(w.FindAnyWidget("Info"));
		if(!wInfo)
			return;
		
		wInfo.SetText(str);
	}
};
