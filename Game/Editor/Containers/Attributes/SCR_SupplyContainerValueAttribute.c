[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_SupplyContainerValueAttribute : SCR_BaseValueListEditorAttribute
{
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity)
			return null;

		GenericEntity entity = editableEntity.GetOwnerScripted();
		if (!entity)
			return null;

		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.FindResourceComponent(entity);
		if (!resourceComponent)
			return null;

		//:| Dont display Attribute for encapsulated Containers.
		SCR_ResourceContainer supplyContainer = resourceComponent.GetContainer(EResourceType.SUPPLIES);
		if (!supplyContainer || supplyContainer.IsEncapsulated())
			return null;

		SCR_ResourceInteractor supplyInteractor;
		SCR_ResourceConsumer supplyConsumer;
		SCR_ResourceGenerator supplyGenerator;
		GetInteractors(editableEntity.GetEntityType(entity), resourceComponent, supplyConsumer, supplyGenerator, supplyInteractor);
		if (!supplyInteractor)
			return null;

		float currentValue = supplyInteractor.GetAggregatedResourceValue();
		float maxValue = supplyInteractor.GetAggregatedMaxResourceValue();

		return SCR_BaseEditorAttributeVar.CreateFloat((currentValue / maxValue) * 100);
	}

	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity)
			return;

		GenericEntity entity = editableEntity.GetOwnerScripted();
		if (!entity)
			return;

		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.FindResourceComponent(entity);
		if (!resourceComponent)
			return;

		//:| Dont display Attribute for encapsulated Containers.
		SCR_ResourceContainer supplyContainer = resourceComponent.GetContainer(EResourceType.SUPPLIES);
		if (!supplyContainer || supplyContainer.IsEncapsulated())
			return;

		SCR_ResourceInteractor supplyInteractor;
		SCR_ResourceConsumer supplyConsumer;
		SCR_ResourceGenerator supplyGenerator;
		GetInteractors(editableEntity.GetEntityType(entity), resourceComponent, supplyConsumer, supplyGenerator, supplyInteractor);
		if (!supplyInteractor)
			return;

		float percentage = var.GetFloat();
		float maxValue = supplyInteractor.GetAggregatedMaxResourceValue();
		float newValue = (maxValue * percentage) / 100;

		supplyContainer.SetResourceValueEx(newValue);
	}

	void GetInteractors(EEditableEntityType entityType, notnull SCR_ResourceComponent resourceComponent, out SCR_ResourceConsumer consumer, out SCR_ResourceGenerator generator, out SCR_ResourceInteractor anyInteractor)
	{
		//:| Vehicles have a different type of consumer and generator, hence the branching below.
		if (entityType == EEditableEntityType.VEHICLE)
		{
			consumer = resourceComponent.GetConsumer(EResourceGeneratorID.VEHICLE_UNLOAD, EResourceType.SUPPLIES);
			anyInteractor = consumer;

			generator = resourceComponent.GetGenerator(EResourceGeneratorID.VEHICLE_LOAD, EResourceType.SUPPLIES);
			if (!anyInteractor)
				anyInteractor = generator;
		}
		else
		{
			consumer = resourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT_STORAGE, EResourceType.SUPPLIES);
			anyInteractor = consumer;

			generator = resourceComponent.GetGenerator(EResourceGeneratorID.DEFAULT_STORAGE, EResourceType.SUPPLIES);
			if (!anyInteractor)
				anyInteractor = generator;
		}
	}
}
