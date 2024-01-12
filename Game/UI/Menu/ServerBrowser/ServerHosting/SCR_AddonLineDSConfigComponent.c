//----------------------------------------------------------------------------------------------
class SCR_AddonLineDSConfigComponent : SCR_AddonLineBaseComponent
{
	protected const string BUTTON_UP = "m_UpButton";
	protected const string BUTTON_DOWN = "m_DownButton";
	
	protected bool m_bWidgetEnabled = false; // Doesn't reflect enable state of item
	
	protected SCR_ModularButtonComponent m_ButtonUp;
	protected SCR_ModularButtonComponent m_ButtonDown;
	
	protected bool m_bOnBottom;
	
	protected ref ScriptInvoker<SCR_AddonLineBaseComponent> Event_OnButtonUp;
	protected ref ScriptInvoker<SCR_AddonLineBaseComponent> Event_OnButtonDown;

	//------------------------------------------------------------------------------------------------
	protected void InvokeEventOnButtonUp()
	{
		if (Event_OnButtonUp)
			Event_OnButtonUp.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetEventOnButtonUp()
	{
		if (!Event_OnButtonUp)
			Event_OnButtonUp = new ScriptInvoker();

		return Event_OnButtonUp;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void InvokeEventOnButtonDown()
	{
		if (Event_OnButtonDown)
			Event_OnButtonDown.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetEventOnButtonDown()
	{
		if (!Event_OnButtonDown)
			Event_OnButtonDown = new ScriptInvoker();

		return Event_OnButtonDown;
	}
	
	//----------------------------------------------------------------------------------------------
	// Ovreride
	//----------------------------------------------------------------------------------------------
	
	//----------------------------------------------------------------------------------------------
	override void Init(SCR_WorkshopItem item)
	{
		// Get ordering widgets 
		Widget wButtonUp = m_wRoot.FindAnyWidget(BUTTON_UP);
		if (wButtonUp)
			m_ButtonUp = SCR_ModularButtonComponent.Cast(wButtonUp.FindHandler(SCR_ModularButtonComponent));
		
		if (m_ButtonUp)
			m_ButtonUp.m_OnClicked.Insert(InvokeEventOnButtonUp);

		Widget wButtonDown = m_wRoot.FindAnyWidget(BUTTON_DOWN);
		if (wButtonDown)
			m_ButtonDown = SCR_ModularButtonComponent.Cast(wButtonDown.FindHandler(SCR_ModularButtonComponent));
		
		if (m_ButtonDown)
			m_ButtonDown.m_OnClicked.Insert(InvokeEventOnButtonDown);
		
		// Setup rest 
		super.Init(item);
		HandleEnableButtons(m_bWidgetEnabled);
	}
	
	//----------------------------------------------------------------------------------------------
	override void UpdateAllWidgets()
	{
		super.UpdateAllWidgets();
		
		// Hide action buttons
		if (m_ActionButtons) 
			m_ActionButtons.ShowAllButtons(false);
		
		m_Widgets.m_HorizontalState.SetVisible(false);
		m_Widgets.m_UpdateButton.SetVisible(false);
		
		// Display oredering widgets if enabled
		bool onTop = m_wRoot.GetZOrder() == 0;
		
		if (m_ButtonUp)
		{
			m_ButtonUp.GetRootWidget().SetVisible(m_bWidgetEnabled && m_bMouseOver);
			m_ButtonUp.GetRootWidget().SetEnabled(!onTop);
		}
		
		if (m_ButtonDown)
		{
			m_ButtonDown.GetRootWidget().SetVisible(m_bWidgetEnabled && m_bMouseOver);
			m_ButtonDown.GetRootWidget().SetEnabled(!m_bOnBottom);
		}
	}
	
	//----------------------------------------------------------------------------------------------
	override void OnEnableButton()
	{
		super.OnEnableButton();
	}
	
	//----------------------------------------------------------------------------------------------
	override void OnDisableButton()
	{
		super.OnDisableButton();
	}

	//----------------------------------------------------------------------------------------------
	// API
	//----------------------------------------------------------------------------------------------
	
	//----------------------------------------------------------------------------------------------
	void SetWidgetEnabled(bool enabled)
	{
		m_bWidgetEnabled = enabled;	
	}
	
	//----------------------------------------------------------------------------------------------
	bool GetWidgetEnabled()
	{
		return m_bWidgetEnabled;
	}
	
	//----------------------------------------------------------------------------------------------
	void SetOnBottom(bool onBottom)
	{
		m_bOnBottom = onBottom;
	}
}
