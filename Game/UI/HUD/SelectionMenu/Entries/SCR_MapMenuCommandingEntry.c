//------------------------------------------------------------------------------------------------
class SCR_MapMenuCommandingEntry : SCR_SelectionMenuEntry
{
	string m_sIdentifier;	// string identifier
						
	//------------------------------------------------------------------------------------------------
	string GetEntryIdentifier()
	{
		return m_sIdentifier;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_MapMenuCommandingEntry(string identifier = "")
	{
		m_sIdentifier = identifier;
	}
};

//------------------------------------------------------------------------------------------------
class SCR_MapMenuRequestedTaskEntry : SCR_SelectionMenuEntry
{
	SCR_RequestedTaskSupportEntity m_SupportClass;
	
	//------------------------------------------------------------------------------------------------
	void SetSupportClass(SCR_RequestedTaskSupportEntity supportClass)
	{
		m_SupportClass = supportClass;
	}
	
	bool CanBeShownScript(IEntity user, BaseSelectionMenu sourceMenu)
	{
		return m_SupportClass.CanRequest();
	}
};
