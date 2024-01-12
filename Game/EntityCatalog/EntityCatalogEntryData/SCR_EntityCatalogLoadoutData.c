
/**
Info for Player Loadout
*/
[BaseContainerProps(configRoot: true), BaseContainerCustomCheckIntTitleField("m_bEnabled", "LoadoutData", "DISABLED - LoadoutData", 1)]
class SCR_EntityCatalogLoadoutData: SCR_BaseEntityCatalogData
{
	//~ NOTE! SCR_EntityCatalogEntry Now support Labels. Instead of defining roles here use EEditableEntityLabel.ROLE_RIFLEMAN etc instead!
	//[Attribute("1", desc: "", uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(ERespawnRole))]
	//protected ERespawnRole m_eRoles;
	
	[Attribute(desc: "Name, description and icon of Respawn loadout")]
	protected ref SCR_UIInfo m_UiInfo;

	//--------------------------------- Get Role Flags ---------------------------------\\
	/*!
	Get Role flag of Respawn loadout
	\return Role flag
	*/
	/*ERespawnRole GetRoles()
	{
		return m_eRoles;
	}*/
	
	//--------------------------------- Has Role Flags ---------------------------------\\
	/*!
	Check if loadout has the given role
	\return true if loadout has the given role
	*/
	/*bool HasRole(ERespawnRole role)
	{
		return SCR_Enum.HasFlag(GetRoles(), role);
	}*/
	
	//--------------------------------- Get UI Info ---------------------------------\\
	/*!
	Get UIInfo of loadout
	\return UiInfo
	*/
	SCR_UIInfo GetUiInfo()
	{
		return m_UiInfo;
	}
};

//--------------------------------- Role enums ---------------------------------\\
/*enum ERespawnRole
{
	RIFLEMAN 		= 1,
	MEDIC 			= 2,
	MACHINEGUNNER 	= 4,
	AT_SPECIALIST 	= 8,
	GRENADIER 		= 16,
};*/
