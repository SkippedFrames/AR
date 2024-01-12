/*!
UIInfo used by notification system
*/
[BaseContainerProps(), SCR_BaseContainerLocalizedTitleField("Name")]
class SCR_UINotificationInfo : SCR_UIInfo
{	
	[Attribute("0", UIWidgets.ComboBox, "Notification Color", "", ParamEnumArray.FromEnum(ENotificationColor) )]
	protected ENotificationColor m_cNotificationColor;
	
	[Attribute("0", UIWidgets.ComboBox, "Editor Position Data", "", ParamEnumArray.FromEnum(ENotificationSetPositionData) )]
	protected ENotificationSetPositionData m_EditorSetPositionData;
	
	/*!
	Get the notification color enum to set the widgets to in the notification message
	\return Notification widget color in enum that the Notification log converts to a color
	*/
	ENotificationColor GetNotificationColor()
	{
		return m_cNotificationColor;
	}

	/*!
	Get type of: if and how the teleport to notification works
	\return Teleport enum that the Notification message to: Go to location, go to updated location or never go to location
	*/
	ENotificationSetPositionData GetEditorSetPositionData()
	{
		return m_EditorSetPositionData;
	}	
};
