/*!
This is a Chat Info Display / Chat HUD.
There is a Chat Panel prefab Inside the layout.
The Chat HUD opens the chat on a button press, and doesn't do anything more interesting.
*/

class SCR_ChatHud : SCR_InfoDisplayExtended
{
	protected SCR_ChatPanel m_ChatPanel;
	protected SCR_InfoDisplaySlotHandler m_slotHandler;
	
	protected int m_iHUDPriorityDefault;
	
	protected const int HEIGHT_DEVIDER = 50;
	
	//------------------------------------------------------------------------------------------------
	override void DisplayUpdate(IEntity owner, float timeSlice)
	{
		if (m_ChatPanel)
		{
			m_ChatPanel.OnUpdateChat(timeSlice);
		}
	}
	
	
	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		InputManager inputMgr = GetGame().GetInputManager();
		inputMgr.AddActionListener("ChatToggle", EActionTrigger.DOWN, Callback_OnToggleAction);
		
		if (!m_wRoot)
			return;
		
		Widget wChatPanel = m_wRoot.FindAnyWidget("ChatPanel");
		
		if (wChatPanel)
			m_ChatPanel = SCR_ChatPanel.Cast(wChatPanel.FindHandler(SCR_ChatPanel));
		
		m_slotHandler = SCR_InfoDisplaySlotHandler.Cast(GetHandler(SCR_InfoDisplaySlotHandler));
		if(!m_slotHandler)
			return;
		
		m_slotHandler.GetSlotUIComponent().GetOnResize().Insert(OnSlotUIResize);
		m_iHUDPriorityDefault = m_slotHandler.GetSlotUIComponent().GetPriority();
		m_ChatPanel.GetOnChatOpen().Insert(OnChatOpen);
		m_ChatPanel.GetOnChatClosed().Insert(OnChatClose);
		
	}
	
	//------------------------------------------------------------------------------------------------
	//! Calculate how many Chat line widgets can be shown when HUD resizes
	protected void OnSlotUIResize()
	{
		int maxLines = (int)m_slotHandler.GetSlotUIComponent().GetHeight() / HEIGHT_DEVIDER;
		maxLines -= 2;
		if (maxLines < 0)
			maxLines = 0;
		
		m_ChatPanel.OnHUDResize(maxLines);
	}
	
	//------------------------------------------------------------------------------------------------
	//! When the chat is open, increase it's priority to make it easier to read through the chat
	void OnChatOpen()
	{
		m_slotHandler.GetSlotUIComponent().SetPriority(m_iHUDPriorityDefault + 1);
		Print(m_slotHandler);
	}
	
	//------------------------------------------------------------------------------------------------
	//! When chat is closed reset it's priority back to default 
	void OnChatClose()
	{
		m_slotHandler.GetSlotUIComponent().SetPriority(m_iHUDPriorityDefault);
	}
	
	//------------------------------------------------------------------------------------------------
	override void DisplayStopDraw(IEntity owner)
	{
		InputManager inputMgr = GetGame().GetInputManager();
		inputMgr.RemoveActionListener("ChatToggle", EActionTrigger.DOWN, Callback_OnToggleAction);
		
		// Widget does not get destroyed by HUD manager when switching characters, so we unregister the chat panel manually.
		SCR_ChatPanelManager.GetInstance().Unregister(m_ChatPanel);
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Called when Enter key is pressed
	protected void Callback_OnToggleAction()
	{	
		if (!m_ChatPanel)
			return;
		
		if (!m_ChatPanel.IsOpen())
		{
			SCR_ChatPanelManager.GetInstance().OpenChatPanel(m_ChatPanel);
		}
	}
};
