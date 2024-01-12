/*!
Detects hover condition. When mouse is over this widget for more than threshold amound of time,
hover event handler is fired.
This component relies on SCR_TooltipManagerEntity.
*/

class SCR_HoverDetectorComponent : SCR_ScriptedWidgetComponent
{
	ref ScriptInvoker<SCR_HoverDetectorComponent, Widget> m_OnHoverDetected = new ScriptInvoker();
	ref ScriptInvoker<SCR_HoverDetectorComponent, Widget> m_OnMouseLeave = new ScriptInvoker();

	[Attribute("200", desc: "Time between mouse entering widget and event activation, in milliseconds")]
	protected float m_fHoverDelayMs;

	[Attribute("0", desc: "Triggers OnHover when the widget is focused instead of just on mouse enter")]
	protected bool m_bDetectFocus;

	protected bool m_bMouseOver;

	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		m_bMouseOver = true;
		auto callQueue = GetGame().GetCallqueue();
		callQueue.Remove(OnTimer);
		callQueue.CallLater(OnTimer, m_fHoverDelayMs);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		m_bMouseOver = false;
		auto callQueue = GetGame().GetCallqueue();
		callQueue.Remove(OnTimer);
		m_OnMouseLeave.Invoke(this, m_wRoot);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		bool onFocus = super.OnFocus(w, x, y);

		if (m_bDetectFocus)
			return OnMouseEnter(w, x, y);
		else
			return onFocus;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		bool onFocusLost = super.OnFocusLost(w, x, y);

		if (m_bDetectFocus)
			return OnMouseLeave(w, null, x, y);
		else
			return onFocusLost;
	}

	//------------------------------------------------------------------------------------------------
	void OnTimer()
	{
		// Invoke event if cursor is still over this widget
		if (m_bMouseOver)
			OnHoverDetected();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnHoverDetected()
	{
		m_OnHoverDetected.Invoke(this, m_wRoot);
	}

	//------------------------------------------------------------------------------------------------
	static SCR_HoverDetectorComponent FindComponent(Widget w)
	{
		return SCR_HoverDetectorComponent.Cast(w.FindHandler(SCR_HoverDetectorComponent));
	}
};
