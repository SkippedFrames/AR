[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_ArsenalWeaponsEditorAttribute : SCR_ArsenalBaseEditorAttribute
{
	override void UpdateInterlinkedVariables(SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, bool isInit = false)
	{
		//Set sub labels
		if (isInit)
			manager.SetAttributeAsSubAttribute(SCR_ArsenalAmmunitionModeAttribute);
		
		//~ If Weapons null or only Attachment set SCR_ArsenalAmmunitionModeAttribute disabled
		manager.SetAttributeEnabled(SCR_ArsenalAmmunitionModeAttribute, var && (var.GetInt() != 0 && var.GetInt() != SCR_EArsenalItemType.WEAPON_ATTACHMENT));
		
		super.UpdateInterlinkedVariables(var, manager, isInit);
	}
};
