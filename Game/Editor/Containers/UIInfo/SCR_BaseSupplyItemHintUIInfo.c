[BaseContainerProps()]
class SCR_BaseSupplyItemHintUIInfo : SCR_InventoryItemHintUIInfo
{		
	//------------------------------------------------------------------------------------------------
	protected void GetSupplyAmounts(InventoryItemComponent item, out float supplies, out float maxSupplies = -1)
	{
		supplies = 0;	
		maxSupplies = 0;
		
		if (!item)
			return;
		
		SCR_ResourceComponent resourceComp = GetResourceComponent(item);
		if (!resourceComp)
			return;
				
		SCR_ResourceConsumer consumer = GetConsumer(resourceComp);
		if (!consumer)
			return;
		
		supplies = consumer.GetAggregatedResourceValue();
		maxSupplies = consumer.GetAggregatedMaxResourceValue();
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_ResourceConsumer GetConsumer(SCR_ResourceComponent resourceComp)
	{
		if (!resourceComp)
			return null;
		
		SCR_ResourceConsumer consumer = resourceComp.GetConsumer(EResourceGeneratorID.DEFAULT_STORAGE, EResourceType.SUPPLIES);
		if (consumer)
			return consumer;
		
		consumer = resourceComp.GetConsumer(EResourceGeneratorID.VEHICLE_UNLOAD, EResourceType.SUPPLIES);
		if (consumer)
			return consumer;
		
		consumer = resourceComp.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
		if (consumer)
			return consumer;
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_ResourceComponent GetResourceComponent(InventoryItemComponent item)
	{
		if (!item)
			return null;
		
		return SCR_ResourceComponent.FindResourceComponent(item.GetOwner());
	}
}
