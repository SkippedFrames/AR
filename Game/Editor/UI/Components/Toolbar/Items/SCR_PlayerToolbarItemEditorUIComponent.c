class SCR_PlayerToolbarItemEditorUIComponent: SCR_EntityToolbarItemEditorUIComponent
{
	[Attribute()]
	protected string m_sPlayerNameWidgetName;
	
	protected TextWidget m_wPlayerName;
	
	//------------------------------------------------------------------------------------------------
	override void SetEntity(SCR_EditableEntityComponent entity, Widget widget, SCR_EditableEntitySlotManagerUIComponent slotManager)
	{
		super.SetEntity(entity, widget, slotManager);
		
		SCR_EditablePlayerDelegateComponent delegate = SCR_EditablePlayerDelegateComponent.Cast(entity);
		if (!delegate) return;
				
		int playerID = delegate.GetPlayerID();
		
		m_wPlayerName = TextWidget.Cast(widget.FindAnyWidget(m_sPlayerNameWidgetName));
		if (m_wPlayerName)
			m_wPlayerName.SetText(GetGame().GetPlayerManager().GetPlayerName(playerID));
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		super.OnFocus(w, x, y);
		
		if (m_wPlayerName)
			m_wPlayerName.SetColor(UIColors.HIGHLIGHTED);
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		super.OnFocusLost(w, x, y);
		
		if (m_wPlayerName)
			m_wPlayerName.SetColor(Color.White);
		
		return false;
	}
};
