// Script File 

[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_HealthVehicleEditorAttribute : SCR_HealthEditorAttribute
{
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		return super.ReadVariable(item, manager);
	}
};
