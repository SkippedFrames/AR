/*!
Class used to hold UI name.
Data are intentionally *READ ONLY*, because the class is often used on prefabs, not instances.
Instead of adding SetXXX() functions here, consider using specialized inherited class.
*/
[BaseContainerProps(), SCR_BaseContainerLocalizedTitleField("Name")]
class SCR_UIName
{
	[Attribute(uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString Name;
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get The unformatted Name from SCR_HintUIInfo instance.
	\return LocalizedString Name set in SCR_HintUIInfo instance.
	*/
	LocalizedString GetUnformattedName()
	{
		return Name;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Change the Name from SCR_HintUIInfo instance.
	Call SCR_HintManagerComponent.Refresh() to update shown hint to display changed text.
	\param LocalizedString text to which name should be updated to.
	*/
	void SetName(LocalizedString name)
	{
		Name = name;
	}
	
	/*!
	Get name.
	When using it to fill TextWidget, use SetNameTo() if possible.
	\return Name text
	*/
	LocalizedString GetName()
	{
		return Name;
	}
	
	/*!
	Check if the info has a name defined.
	\return True when the name is defined
	*/
	bool HasName()
	{
		return !GetName().IsEmpty();
	}
	
	/*!
	Set name to given text widget.
	When possible, use this function instead of retrieving the name using GetName() and setting it manually.
	Custom UI info classes may be using parameters which would not be applied otherwise.
	\param textWidget Target text widget
	\return True when the name was set
	*/
	bool SetNameTo(TextWidget textWidget)
	{
		if (!textWidget)
			return false;
		
		textWidget.SetText(GetName());
		return true;
	}
	
	/*!
	Print out contents of this UI info
	*/
	void Log(string prefix = string.Empty, LogLevel logLevel = LogLevel.VERBOSE)
	{
		Print(string.Format(prefix + "%1: \"%2\"", Type(), Name), logLevel);
	}
	
	//--- Protected, to be overriden and/or made public by inherited classes
	protected void CopyFrom(SCR_UIName source)
	{
		if (source)
			Name = source.Name;
	}
};
