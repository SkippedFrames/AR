[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_sName", true)]
class SCR_LoadoutArea
{
	[Attribute("Name", desc: "")]
	protected string m_sName;
	
	[Attribute( defvalue: "", uiwidget: UIWidgets.Object, desc: "Area type" )]
	ref LoadoutAreaType m_LoadoutArea;
	
	[Attribute( defvalue: "0", uiwidget: UIWidgets.ComboBox, desc: "Area", enums: ParamEnumArray.FromEnum( ECommonItemType ) )]
	ECommonItemType m_eCommonType;
	
	[Attribute( defvalue: "1", uiwidget: UIWidgets.EditBox, desc: "Row in inventory list" )]
	private int m_iRow;
	
	[Attribute( defvalue: "", desc: "Icon for the Area", params: "edds" )]
	ResourceName m_sIcon;
	
};

[BaseContainerProps(configRoot: true)]
class SCR_InventoryConfig
{
	[Attribute( desc: "Loadout Areas" )]
	private ref array<ref SCR_LoadoutArea> m_aLoadoutAreas;
	
	int GetRowByArea( LoadoutAreaType pArea )
	{ 
		if (!pArea)
			return -1;
		
		foreach ( int iIndex , SCR_LoadoutArea pAreaEntry : m_aLoadoutAreas )
			if ( pAreaEntry.m_LoadoutArea && pAreaEntry.m_LoadoutArea.IsInherited(pArea.Type()) )
				return iIndex; 
		return -1;
	}
	
	int GetRowByCommonItemType( ECommonItemType pItemType )
	{ 
		foreach ( int iIndex , SCR_LoadoutArea pAreaEntry : m_aLoadoutAreas )
			if ( pAreaEntry.m_eCommonType == pItemType )
				return iIndex; 
		return -1;
	}
	
	ResourceName GetIcon( LoadoutAreaType pArea )
	{
		if (!pArea)
			return ResourceName.Empty;
		
		foreach ( int iIndex , SCR_LoadoutArea pAreaEntry : m_aLoadoutAreas )
			if ( pAreaEntry.m_LoadoutArea && pAreaEntry.m_LoadoutArea.IsInherited(pArea.Type()) )
				return pAreaEntry.m_sIcon;
		return ResourceName.Empty;
	}
	
	ResourceName GetIconByRow( int iIndex )
	{
		if ( iIndex > m_aLoadoutAreas.Count() -1 )
			return ResourceName.Empty;
		return m_aLoadoutAreas[ iIndex ].m_sIcon;
	}
	
	LoadoutAreaType GetAreaByRow( int iIndex )
	{
		if (!m_aLoadoutAreas.IsIndexValid(iIndex))
			return null;
		return m_aLoadoutAreas[iIndex].m_LoadoutArea;
	}
	
	void ~SCR_InventoryConfig()
	{
		m_aLoadoutAreas = null;
	}
};

