//------------------------------------------------------------------------------------------------
class SCR_SimpleWarningComponent : SCR_ScriptedWidgetComponent
{
	[Attribute("#AR-Account_LoginTimeout")]
	protected string m_sWarning;
	
	[Attribute("warning")]
	protected string m_sIconName;
	
	[Attribute(UIColors.GetColorAttribute(UIColors.WARNING))]
	protected ref Color m_Color;
	
	protected TextWidget m_wWarning;
	protected Widget m_wWarningWrapper;
	protected ImageWidget m_wWarningImage;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_wWarning = TextWidget.Cast(w.FindAnyWidget("WarningText"));
		m_wWarningImage = ImageWidget.Cast(w.FindAnyWidget("WarningImage"));
		m_wWarningWrapper = w.FindAnyWidget("WarningWrapper");
		ResetWarning();
	}
	
	//------------------------------------------------------------------------------------------------
	void ResetWarning()
	{
		SetWarning(m_sWarning, m_sIconName, m_Color);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetWarning(string text, string iconName, Color color)
	{
		if (m_wWarning)
		{
			m_wWarning.SetText(text);
			m_wWarning.SetColor(color);
		}
		
		if (m_wWarningImage)
		{
			m_wWarningImage.LoadImageFromSet(0, UIConstants.ICONS_IMAGE_SET, iconName);
			m_wWarningImage.SetColor(color);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetWarningVisible(bool visible, bool preserveSpace = true)
	{
		if (!preserveSpace)
			m_wRoot.SetVisible(visible);
		
		if (m_wWarningWrapper)
			m_wWarningWrapper.SetVisible(visible);
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_SimpleWarningComponent FindComponentInHierarchy(notnull Widget root)
	{
		ScriptedWidgetEventHandler handler = SCR_WidgetTools.FindHandlerInChildren(root, SCR_SimpleWarningComponent);
		if (handler)
			return SCR_SimpleWarningComponent.Cast(handler);

		return null;
	}

	//------------------------------------------------------------------------------------------------
	static SCR_SimpleWarningComponent FindComponent(notnull Widget w)
	{
		return SCR_SimpleWarningComponent.Cast(w.FindHandler(SCR_SimpleWarningComponent));
	}
}
