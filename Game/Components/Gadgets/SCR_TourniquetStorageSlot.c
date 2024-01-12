class SCR_TourniquetStorageSlot : SCR_EquipmentStorageSlot
{
	[Attribute("", UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ECharacterHitZoneGroup), desc: "Which hitzone does this slot belong to", category: "Equipped consumables")]
	protected ECharacterHitZoneGroup m_eHitZoneGroup;
	
	//------------------------------------------------------------------------------------------------
	ECharacterHitZoneGroup GetAssociatedHZGroup()
	{
		return m_eHitZoneGroup;
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity GetItem(int ID)
	{
		return GetStorage().Get(ID);
	}
};
