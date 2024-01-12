[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_EnableGlobalSupplyUsageEditorAttribute : SCR_BaseEditorAttribute
{
	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		//~ Disabled for now until logic is added
		return null;
		
		//If opened in global attributes
		if (!IsGameMode(item)) 
			return null;
		
		//~ Todo: Get supplies enabled
		
		//~ Todo: instead of 'true' use current state of supplies enabled or not
		return SCR_BaseEditorAttributeVar.CreateBool(true);
	}
	
	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var) 
			return;
		
		//If opened in global attributes
		if (!IsGameMode(item)) 
			return;
		
		
		//~ Todo: Call function enable supplies (server only). 
		//~ Make sure to Send notification: EDITOR_ATTRIBUTES_ENABLE_GLOBAL_SUPPLY_USAGE or EDITOR_ATTRIBUTES_DISABLE_GLOBAL_SUPPLY_USAGE  if given playerID is not -1
		// EnableSupplies(var.GetBool(), playerID);
	}
}
