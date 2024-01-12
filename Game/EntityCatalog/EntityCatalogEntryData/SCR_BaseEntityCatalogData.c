/**
Base Class for Entity catalog data. Inherent from this to create your own Entity catalog data
*/
[BaseContainerProps(), BaseContainerCustomStringTitleField("DO NOT USE BASE CLASS!")]
class SCR_BaseEntityCatalogData
{
	[Attribute("1", desc: "Allows to disable the Entity info. SCR_EntityCatalog will ignore the info as if it is null. Used for specific gamemodes and modding.")]
	protected bool m_bEnabled;
	
	//--------------------------------- Is Enabled ---------------------------------\\
	/*!
	If is Enabled
	\return If enabled or disabled
	*/
	bool IsEnabled()
	{
		return m_bEnabled;
	}
	
	//--------------------------------- Init Data ---------------------------------\\
	/*!
	Called by Catalog on creation. Special init for data
	Never called if Data is disabled
	\param entry entry the data is attached to
	*/
	void InitData(notnull SCR_EntityCatalogEntry entry)
	{
		
	}
};
