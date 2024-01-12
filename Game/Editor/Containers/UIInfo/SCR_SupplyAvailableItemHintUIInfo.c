[BaseContainerProps(configRoot: true)]
class SCR_SupplyAvailableItemHintUIInfo : SCR_BaseSupplyItemHintUIInfo
{
	[Attribute("0", desc: "If true will not show this hint if the item is howered over in vicinity")]
	protected bool m_bHideIfInvicinity;
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShown(InventoryItemComponent item)
	{
		if (!item)
			return false;
		
		InventoryStorageSlot parentSlot = item.GetParentSlot();
		//~ Hide if in vicinity
		if (m_bHideIfInvicinity && !parentSlot)
			return false;
		
		 if (!super.CanBeShown(item))
			return false;
		
		//~ Make sure it is not shown if stored in arsenal
		if (parentSlot)
		{
			BaseInventoryStorageComponent storage = parentSlot.GetStorage();
			if (!storage || SCR_ArsenalComponent.Cast(storage.GetOwner().FindComponent(SCR_ArsenalComponent)))
				return false;
		}

		SCR_ResourceComponent resourceComp = GetResourceComponent(item);
		if (!resourceComp)
			return false;
		
		return resourceComp.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void GetSupplyAmounts(InventoryItemComponent item, out float supplies, out float maxSupplies = -1)
	{
		supplies = 0;	
		maxSupplies = 0;
		
		if (!item)
			return;
		
		SCR_ResourceComponent resourceComp = GetResourceComponent(item);
		if (!resourceComp)
			return;
		
		SCR_ResourceConsumer consumer = resourceComp.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
		if (!consumer)
			return;
		
		supplies = consumer.GetAggregatedResourceValue();
		maxSupplies = consumer.GetAggregatedMaxResourceValue();
	}
	
	//------------------------------------------------------------------------------------------------
	override string GetItemHintName(InventoryItemComponent item)
	{
		if (!item)
			return super.GetItemHintName(item);
		
		float supplies;
		GetSupplyAmounts(item, supplies);
		
		return WidgetManager.Translate(GetName(), supplies);
	}
}
