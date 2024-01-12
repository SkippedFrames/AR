/**
Entity Entry within the SCR_EntityCatalog. This is meant for NON-EDITABLE ENTITIES ONLY! For editable entities use SCR_EntityCatalogEntry!
*/
//[BaseContainerProps(), BaseContainerCustomDoubleCheckIntResourceNameTitleField("m_bEnabled", "m_sEntityPrefab", 1, "(Item) %1", "DISABLED - (Item) %1")]
[BaseContainerProps(), SCR_BaseContainerCustomInventoryCatalogEntry("m_sEntityPrefab", "m_aEntityDataList", "m_bEnabled")]
class SCR_EntityCatalogInventoryItem : SCR_EntityCatalogEntryNonEditable
{
	//======================================== UI INFO ========================================\\
	//--------------------------------- UI Info not supported ---------------------------------\\
	override SCR_UIInfo GetEntityUiInfo()
	{
		Print("Getting UIInfo from Inventory Items is not supported from Catalog. Get it from InventoryItemComponent after spawning instead!", LogLevel.WARNING);
		
		//~ Returns emtpy UIInfo for now. Also to make sure autotest succeeds
		return new SCR_UIInfo();
	}
};

//~ Custom title to show Type of inventory
class SCR_BaseContainerCustomInventoryCatalogEntry : BaseContainerCustomTitle
{
	protected string m_sPrefabName;
	protected string m_sDataListName;
	protected string m_sEnabledName;

	//------------------------------------------------------------------------------------------------
	void SCR_BaseContainerCustomInventoryCatalogEntry(string prefabName, string dataListName, string enabledName)
	{
		m_sPrefabName = prefabName;
		m_sDataListName = dataListName;
		m_sEnabledName = enabledName;
	}

	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)	
	{	
		bool enabled;
		if (!source.Get(m_sEnabledName, enabled))
			return false;
		
		ResourceName path;
		if (!source.Get(m_sPrefabName, path))
			return false;
		
		if (!path.IsEmpty())
		{
			title = FilePath.StripPath(path.GetPath());
		}
		else 
		{
			title = "NO PREFAB"
		}
		
		array<ref SCR_BaseEntityCatalogData> entityDataList;
		if (!source.Get(m_sDataListName, entityDataList))
			return false; 
		
		string typeTitle, modeTitle;
		SCR_ArsenalItem arsenalItem;
		
		//~ Show specific type info if ArsenalItem
		foreach (SCR_BaseEntityCatalogData data: entityDataList)
		{
			arsenalItem = SCR_ArsenalItem.Cast(data);
			if (arsenalItem)
			{
				//~ Show type
				typeTitle = typename.EnumToString(SCR_EArsenalItemType, arsenalItem.GetItemType());
				title = title + "    ~    " + typeTitle;
				
				//~ Only show mode if not default
				if (arsenalItem.GetItemMode() != SCR_EArsenalItemMode.DEFAULT)
				{
					modeTitle = typename.EnumToString(SCR_EArsenalItemMode, arsenalItem.GetItemMode());
					title = title + " - " + modeTitle
				}
			}
		}
		
		//~ Disabled so do not show types
		if (!enabled)
			title = "DISABLED - " + title;
		
		return true; 
	}
};
