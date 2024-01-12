class SCR_ContentBrowserActiveFilterEditorUIComponent: ScriptedWidgetComponent
{		
	protected Widget m_wRoot;
	protected Widget m_wBorder;
	protected EEditableEntityLabel m_iLabelReference;
	
	
	
	/*!
	Init button when to set icon and data when it is pressed
	\param label EEditableEntityLabel filter for when it is pressed
	\param uiInfo SCR_UIInfo to set icon
	*/
	void InitFilterButton(EEditableEntityLabel label, SCR_UIInfo uiInfo)
	{
		if (!m_wRoot)
			return;
		
		m_iLabelReference = label;
		
		if (uiInfo)
			uiInfo.SetIconTo(ImageWidget.Cast(m_wRoot.FindAnyWidget("Icon")));
		
		SCR_LinkTooltipTargetEditorUIComponent tooltip = SCR_LinkTooltipTargetEditorUIComponent.Cast(m_wRoot.FindHandler(SCR_LinkTooltipTargetEditorUIComponent));
		if (tooltip)
			tooltip.SetInfo(uiInfo);
	}
	
	/*!
	Get EEditableEntityLabel reference
	\return EEditableEntityLabel
	*/
	EEditableEntityLabel GetLabelReference()
	{
		return m_iLabelReference;
	}
	
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		AnimateWidget.Opacity(m_wBorder, 1, 5);
		return false;
	}
	
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		AnimateWidget.Opacity(m_wBorder, 0, 5);
		return false;
	}
	
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
		m_wBorder = w.FindAnyWidget("Border");
	}
};
