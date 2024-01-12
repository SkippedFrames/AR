//------------------------------------------------------------------------------------------------
class SCR_MapMarkerSquadLeaderComponent : SCR_MapMarkerDynamicWComponent
{
	bool m_bIsHovered;
	protected bool m_bIsInit;
	protected Widget m_wMarkerVLayout;
	protected Widget m_wGroupInfo;
	protected Widget m_wGroupInfoList;
	protected TextWidget m_wGroupFrequency;
	
	protected ref array<Widget> m_aGroupMemberEntries = new array<Widget>;
	
	[Attribute("{CCD81F58E9D6EEA6}UI/layouts/Map/MapMarkerGroupInfo.layout", desc: "group info layout")]
	protected ResourceName m_sGroupInfoLayout;
	
	[Attribute("{B09864CA15145CD3}UI/layouts/Map/MapMarkerGroupInfoLine.layout", desc: "group info line layout")]
	protected ResourceName m_sLineLayout;
		
	[Attribute("lineText", desc: "line text widget")]
	protected string m_sLineTextWidgetName;
	
	[Attribute("lineIcon", desc: "line icon widget")]
	protected string m_sLineIconWidgetName;
	
	[Attribute("40", desc: "pixels, group info offset")]
	protected int m_iGroupInfoOffset;
		
	//------------------------------------------------------------------------------------------------
	//! Differentiates visuals between our group and the others
	void SetGroupActive(bool state)
	{
		if (state)
			m_wMarkerVLayout.SetOpacity(1);
		else 
			m_wMarkerVLayout.SetOpacity(0.75);
	}
			
	//------------------------------------------------------------------------------------------------
	void UpdateGroupInfoPosition(int screenX, int screenY)
	{
		if (m_wGroupInfo)
			FrameSlot.SetPos(m_wGroupInfo, GetGame().GetWorkspace().DPIUnscale(screenX), GetGame().GetWorkspace().DPIUnscale(screenY) - m_iGroupInfoOffset);	// needs unscaled coords
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseButtonDown(Widget w, int x, int y, int button)
	{
		if (button != 0)	// LMB only
			return true;
		
		GetGame().OpenGroupMenu();
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		m_wMarkerText.SetColor(GUIColors.ORANGE);
		
		SCR_AIGroup group = SCR_MapMarkerSquadLeader.Cast(m_MarkerEnt).GetGroup();
		if (group)
		{
			if (!m_bIsInit)
			{
				m_wGroupInfo = GetGame().GetWorkspace().CreateWidgets(m_sGroupInfoLayout, m_wRoot.GetParent());
				if (!m_wGroupInfo)
					return false;
				
				m_wGroupInfoList = m_wGroupInfo.FindAnyWidget("groupInfoList");
				m_wGroupFrequency = TextWidget.Cast(m_wGroupInfo.FindAnyWidget("groupFrequency"));
				
				int capacity = group.GetMaxMembers();
				
				for (int i = 0; i < capacity; i++)
				{
					m_aGroupMemberEntries.Insert(GetGame().GetWorkspace().CreateWidgets(m_sLineLayout, m_wGroupInfoList));
				}
				
				m_bIsInit = true;
			}
			
			float fFrequency = Math.Round(group.GetRadioFrequency() * 0.1) * 0.01; 	// Format the frequency text: round and convert to 2 digits with one possible decimal place (39500 -> 39.5)			
			m_wGroupFrequency.SetText(fFrequency.ToString(3, 1));
			
			int playerCount = group.GetPlayerCount();
			array<int> membersCopy = new array<int>;
			membersCopy.Copy(group.GetPlayerIDs());
			
			PlayerManager pManager = GetGame().GetPlayerManager();
			int leaderID = group.GetLeaderID();
			
			// leader entry first, order of IDs is not guaranteed
			foreach (int id : membersCopy)
			{
				if (id == leaderID)
				{
					Widget entry = m_aGroupMemberEntries[0];
					TextWidget txtW = TextWidget.Cast(entry.FindWidget(m_sLineTextWidgetName));
					txtW.SetText(pManager.GetPlayerName(id));
					entry.SetVisible(true);
					
					if (GetGame().GetPlayerController().GetPlayerId() == id)
						txtW.SetColor(GUIColors.ORANGE);
					else 
						txtW.SetColor(GUIColors.DEFAULT);
					
					ImageWidget imgW = ImageWidget.Cast(entry.FindWidget(m_sLineIconWidgetName));
					imgW.SetOpacity(1);
					
					membersCopy.RemoveItem(id);
					break;	
				}
			}
			
			// members
			foreach (int i, Widget entry :  m_aGroupMemberEntries) 
			{
				if (i == 0)		// leader
					continue;
				
				if (i < playerCount)
				{
					TextWidget txtW = TextWidget.Cast(entry.FindWidget(m_sLineTextWidgetName));
					txtW.SetText(pManager.GetPlayerName(membersCopy[i-1]));
					entry.SetVisible(true);
					
					if (GetGame().GetPlayerController().GetPlayerId() == membersCopy[i-1])
						txtW.SetColor(GUIColors.ORANGE);
					else 
						txtW.SetColor(GUIColors.DEFAULT);
					
					ImageWidget imgW = ImageWidget.Cast(entry.FindWidget(m_sLineIconWidgetName));
					imgW.SetOpacity(0);
					
					continue;
				} 
				
				entry.SetVisible(false);
			}
			
			m_wGroupInfo.SetVisible(true);
		}
		
		m_bIsHovered = true;
		
		return true;
	}
	
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		m_wMarkerText.SetColor(m_TextColor);
		m_wGroupInfo.SetVisible(false);
		m_bIsHovered = false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_wMarkerVLayout = m_wRoot.FindWidget("markerVLayout");
	}
}
