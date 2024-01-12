[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_SupportStationUseSuppliesEditorAttribute : SCR_BaseEditorAttribute
{
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{					
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity)
			return null;
		
		SCR_BaseSupportStationComponent supportStation = SCR_BaseSupportStationComponent.Cast(editableEntity.GetOwner().FindComponent(SCR_BaseSupportStationComponent));
		if (!supportStation)
			return null;
		
		if (!supportStation.GetResourceConsumer())
			return null;
		
		return SCR_BaseEditorAttributeVar.CreateBool(supportStation.IsUsingSupplies());
	}
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{						
		if (!var)
			return;
		
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity)
			return;
		
		SCR_BaseSupportStationComponent supportStation = SCR_BaseSupportStationComponent.Cast(editableEntity.GetOwner().FindComponent(SCR_BaseSupportStationComponent));
		if (!supportStation)
			return;
		
		if (!supportStation.GetResourceConsumer())
			return;
		
		supportStation.SetUseSupplies(var.GetBool());
	}
};
