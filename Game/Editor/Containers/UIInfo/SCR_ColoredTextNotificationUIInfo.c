[BaseContainerProps(), SCR_BaseContainerLocalizedTitleField("Name")]
class SCR_ColoredTextNotificationUIInfo : SCR_UINotificationInfo
{
	[Attribute("0", UIWidgets.ComboBox, "Notification Text Color", "", ParamEnumArray.FromEnum(ENotificationColor) )]
	protected ENotificationColor m_iNotificationTextColor;
	
	/*!
	Get the notification color enum to set the text to in the notification message
	\return Notification text color in enum that the Notification log converts to a color
	*/
	ENotificationColor GetNotificationTextColor()
	{
		return m_iNotificationTextColor;
	}
};
