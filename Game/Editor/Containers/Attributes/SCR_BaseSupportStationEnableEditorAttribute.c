[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_BaseSupportStationEnableEditorAttribute : SCR_BaseEditorAttribute
{
	//------------------------------------------------------------------------------------------------
	//~ Overwrite this
	protected bool IsValidSupportStation(ESupportStationType supportStationType)
	{
		Print("SCR_BaseSupportStationEnableEditorAttribute should never be used. Inherit instead!", LogLevel.ERROR);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_BaseSupportStationComponent GetSupportStationFromSlottedVehicleEntities(IEntity parent)
	{
		SlotManagerComponent slotManager = SlotManagerComponent.Cast(parent.FindComponent(SlotManagerComponent));
		if (!slotManager)
			return null;
		
		array<EntitySlotInfo> slotInfos = {};
		slotManager.GetSlotInfos(slotInfos);
		IEntity slotEntity;
		
		SCR_BaseSupportStationComponent supportStation;
		
		foreach (EntitySlotInfo slotInfo : slotInfos)
		{
			slotEntity = slotInfo.GetAttachedEntity();
			if (!slotEntity)
				continue;
			
			supportStation = SCR_BaseSupportStationComponent.Cast(slotEntity.FindComponent(SCR_BaseSupportStationComponent));
			if (supportStation)
			{
				if (IsValidSupportStation(supportStation.GetSupportStationType()))
					return supportStation;
				else 
					supportStation = null;
			}
		}
		
		//~ Not found
		return supportStation;
	}
	
	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{					
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity)
			return null;

		SCR_BaseSupportStationComponent supportStation = SCR_BaseSupportStationComponent.Cast(editableEntity.GetOwner().FindComponent(SCR_BaseSupportStationComponent));
		if (supportStation && !IsValidSupportStation(supportStation.GetSupportStationType()))
			supportStation = null;
		
		//~ If vehicle check if supportStation is on slotted entities
		if (!supportStation && editableEntity.GetEntityType() == EEditableEntityType.VEHICLE)
			supportStation = GetSupportStationFromSlottedVehicleEntities(editableEntity.GetOwner());
		
		if (!supportStation)
			return null;
		
		return SCR_BaseEditorAttributeVar.CreateBool(supportStation.IsEnabled());
	}
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{						
		if (!var)
			return;
		
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity)
			return;
		
		SCR_BaseSupportStationComponent supportStation = SCR_BaseSupportStationComponent.Cast(editableEntity.GetOwner().FindComponent(SCR_BaseSupportStationComponent));
		if (supportStation && !IsValidSupportStation(supportStation.GetSupportStationType()))
			supportStation = null;
		
		//~ If vehicle check if supportStation is on slotted entities
		if (!supportStation && editableEntity.GetEntityType() == EEditableEntityType.VEHICLE)
			supportStation = GetSupportStationFromSlottedVehicleEntities(editableEntity.GetOwner());
		
		if (!supportStation)
			return;
		
		supportStation.SetEnabled(var.GetBool());
	}
};
