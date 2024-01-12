//------------------------------------------------------------------------------------------------
class SCR_ArmorHitZone : ScriptedHitZone
{
	[Attribute(ECharacterHitZoneGroup.VIRTUAL.ToString(), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ECharacterHitZoneGroup))]
	protected ECharacterHitZoneGroup m_eHitZoneGroup;	
};
