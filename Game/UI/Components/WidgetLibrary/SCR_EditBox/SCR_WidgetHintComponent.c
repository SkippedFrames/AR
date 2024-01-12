/*
Widget component for short message suggesting what needs to be done with widget.
*/

//------------------------------------------------------------------------------------------------
class SCR_WidgetHintComponent : SCR_WLibComponentBase 
{
	[Attribute("Icon", UIWidgets.EditBox, "Widget name to find icon")]
	protected string m_sWidgetIcon;
	
	[Attribute("Message", UIWidgets.EditBox, "Widget name to find message text widget")]
	protected string m_sWidgetMessage;
	
	[Attribute("Bumper", UIWidgets.EditBox, "Widget name to find message text widget")]
	protected string m_sWidgetBumper;
	
	[Attribute("", UIWidgets.EditBox, "Default message to display")]
	protected string m_sDefaultMessage;
	
	[Attribute("0", UIWidgets.EditBox, "True will set bumper widget to visible which will offest the message")]
	protected bool m_bUseBumber;
	
	protected ImageWidget m_wIcon;
	protected TextWidget m_wMessage;
	protected Widget m_wBumper;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_wIcon = ImageWidget.Cast(w.FindAnyWidget(m_sWidgetIcon));
		m_wMessage = TextWidget.Cast(w.FindAnyWidget(m_sWidgetMessage));
		m_wBumper = w.FindAnyWidget(m_sWidgetBumper);
		
		UseDefaultMessage();
		
		if (m_wBumper)
			m_wBumper.SetVisible(m_bUseBumber);
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetVisible(bool visible, bool animate = true)
	{
		if (animate)
		{
			if (!visible || m_wRoot.GetOpacity() == 0)
			{
				m_wRoot.SetVisible(visible);
				return;
			}
			else if (visible || m_wRoot.GetOpacity() == 1)
			{
				m_wRoot.SetVisible(visible);
				return;
			}
		}
		
		super.SetVisible(visible, animate);
	}
	
	//-----------------------------------------------------------/-------------------------------------
	// API
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	void SetMessage(string message)
	{
		if (m_wMessage)
			m_wMessage.SetText(message);
	}
	
	//------------------------------------------------------------------------------------------------
	string GetMessage()
	{
		return m_wMessage.GetText();
	}
	
	//------------------------------------------------------------------------------------------------
	void UseDefaultMessage()
	{
		if (m_wMessage)
			m_wMessage.SetText(m_sDefaultMessage);
	}
}
