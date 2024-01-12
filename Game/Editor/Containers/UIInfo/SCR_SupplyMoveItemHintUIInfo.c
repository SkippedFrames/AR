[BaseContainerProps(configRoot: true)]
class SCR_SupplyMoveItemHintUIInfo : SCR_BaseSupplyItemHintUIInfo
{
	//------------------------------------------------------------------------------------------------
	override bool CanBeShown(InventoryItemComponent item)
	{
		if (!super.CanBeShown(item))
			return false;
		
		float supplies;
		GetSupplyAmounts(item, supplies);
		
		return supplies > 0;
	}
}
