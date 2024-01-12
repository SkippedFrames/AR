//------------------------------------------------------------------------------------------------
//! Commands config root
[BaseContainerProps(configRoot: true)]
class SCR_PlayerCommandsConfig : Managed
{
	[Attribute("", UIWidgets.Object, "Available commands")]
	protected ref array<ref SCR_BaseGroupCommand> m_aCommands;
	
	array<ref SCR_BaseGroupCommand> GetCommands()
	{
		return m_aCommands;
	}
};

//------------------------------------------------------------------------------------------------
//! Commanding menu config root
[BaseContainerProps(configRoot: true)]
class SCR_PlayerCommandingMenuConfig : Managed
{
	[Attribute("", UIWidgets.Object, "Root category of the commanding radial menu")]
	protected ref SCR_PlayerCommandingMenuCategoryElement m_RootCategory;
	
	SCR_PlayerCommandingMenuCategoryElement GetRootCategory()
	{
		return m_RootCategory;
	}
};

//------------------------------------------------------------------------------------------------
//! Commanding menu commanding element class
[BaseContainerProps()]
class SCR_PlayerCommandingMenuCommand : SCR_PlayerCommandingMenuBaseElement
{
	[Attribute("", UIWidgets.EditBox, "Name of the command used from SCR_PlayerCommandsConfig" )]
	protected string m_sCommandName;
	
	[Attribute("", UIWidgets.EditBox, "Name of the command that is displayed in the menu" )]
	protected string m_sCommandDisplayText;
	
	protected string m_sCommandCustomDisplayText;
	
	//------------------------------------------------------------------------------------------------
	string GetCommandName()
	{
		return m_sCommandName;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCommandName(string name)
	{
		m_sCommandName = name;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetCommandDisplayText()
	{
		return m_sCommandDisplayText;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetCommandCustomName()
	{
		return m_sCommandCustomDisplayText;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCommandCustomName(string customName)
	{
		m_sCommandCustomDisplayText = customName;
	}
};

//------------------------------------------------------------------------------------------------
//! Commanding menu base element class
[BaseContainerProps()]
class SCR_PlayerCommandingMenuBaseElement : Managed
{

};

//------------------------------------------------------------------------------------------------
//! Commanding menu base element class
[BaseContainerProps()]
class SCR_PlayerCommandingMenuCategoryElement : SCR_PlayerCommandingMenuBaseElement
{
	[Attribute("", UIWidgets.EditBox, "Name of the category that is displayed in the menu" )]
	protected string m_sCategoryDisplayText;
	
	[Attribute("", UIWidgets.Object, "Elements in the given category")]
	protected ref array<ref SCR_PlayerCommandingMenuBaseElement> m_aElements;
	
	//------------------------------------------------------------------------------------------------
	string GetCategoryDisplayText()
	{
		return m_sCategoryDisplayText;
	}

	//------------------------------------------------------------------------------------------------
	void SetCategoryDisplayText(string displayText)
	{
		m_sCategoryDisplayText = displayText;
	}	
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_PlayerCommandingMenuBaseElement> GetCategoryElements()
	{
		return m_aElements;
	}	
};

class SCR_BaseGroupCommandTitleField : BaseContainerCustomTitleField
{
	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		// Make sure variable exists
		int index = source.GetVarIndex("m_sCommandName");
		if (index == -1)
			return false;

		// Tag string
		source.Get("m_sCommandName", title);

		return true;
	}
}
