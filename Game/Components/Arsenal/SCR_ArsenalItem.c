[BaseContainerProps(configRoot: true), BaseContainerCustomCheckIntTitleField("m_bEnabled", "Arsenal Data", "DISABLED - Arsenal Data", 1)]
class SCR_ArsenalItem : SCR_BaseEntityCatalogData
{	
	[Attribute("2", desc: "Type of the arsenal item. An arsenal will only spawn items of types that it allows to be spawned. The item will not show up if it is not allowed. Eg: FieldDressing = HEAL", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(SCR_EArsenalItemType))]
	protected SCR_EArsenalItemType m_eItemType;
	
	[Attribute("2", desc: "Item mode of arsenal, set this to what the behaviour is for the item. EG: FieldDressing = CONSUMABLE as it is used up or M16 with attachments = WEAPON_VARIENTS as it is not a default M16. Check other items in the faction config to see how it works.", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(SCR_EArsenalItemMode))]
	protected SCR_EArsenalItemMode m_eItemMode;
	
	[Attribute("10", desc: "Supply cost of item when using the resupply action and/or taking them from the arsenal inventory. Note in overwrite arsenal config this value is ignored and still taken from catalog", params: "0 inf 1")]
	protected int m_iSupplyCost;
	
	[Attribute(desc: "Display data for SCR_ArsenalDisplayComponent. If Arsenal item has display data of the correct type for the entity with SCR_ArsenalDisplayComponent than it can be displayed on said entity")]
	protected ref array<ref SCR_ArsenalItemDisplayData> m_aArsenalDisplayData;
	
	protected SCR_EntityCatalogEntry m_EntryParent;
	protected ref Resource m_ItemResource;
	
	SCR_EArsenalItemType GetItemType()
	{
		return m_eItemType;
	}
	
	SCR_EArsenalItemMode GetItemMode()
	{
		return m_eItemMode;
	}
	
	ResourceName GetItemResourceName()
	{
		if (!m_EntryParent)
			return string.Empty;	
		
		return m_EntryParent.GetPrefab();
	}
	
	Resource GetItemResource()
	{
		return m_ItemResource;
	}
	
	/*!
	Get display data of given type for displaying items on arsenal display
	\param displayType type of display data to find
	\return Display data, can be null if not found
	*/
	SCR_ArsenalItemDisplayData GetDisplayDataOfType(EArsenalItemDisplayType displayType)
	{
		foreach (SCR_ArsenalItemDisplayData displayData : m_aArsenalDisplayData)
		{
			if (displayData.GetDisplayType() == displayType)
				return displayData;
		}
		
		return null;
	}
	
	//--------------------------------- Direct Getter general or any faction ---------------------------------\\
	/*!
	Get supply cost of arsenal item
	\return Supplycost
	*/
	int GetSupplyCost()
	{
		return m_iSupplyCost;
	}
	
	//--------------------------------- Init Data ---------------------------------\\
	override void InitData(notnull SCR_EntityCatalogEntry entry)
	{
		m_EntryParent = entry;
		
		m_ItemResource = Resource.Load(m_EntryParent.GetPrefab());
	}
};
