[BaseContainerProps()]
class SCR_FuelItemHintsUIInfo : SCR_InventoryItemHintUIInfo
{
	//------------------------------------------------------------------------------------------------
	override bool CanBeShown(InventoryItemComponent item)
	{
		if (!item || !super.CanBeShown(item))
			return false;
		
		return FuelManagerComponent.Cast(item.GetOwner().FindComponent(FuelManagerComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override string GetItemHintName(InventoryItemComponent item)
	{
		if (!item)
			return super.GetItemHintName(item);
		
		FuelManagerComponent fuelManager = FuelManagerComponent.Cast(item.GetOwner().FindComponent(FuelManagerComponent));
		if (!fuelManager)
			return super.GetItemHintName(item);
		
		float currentFuel = fuelManager.GetTotalFuel();
		float maxFuel = fuelManager.GetTotalMaxFuel();
		
		string currentFuelString, maxFuelString;
		
		//~ Set dec lenght to either 2 or 0
		if (currentFuel - (int)currentFuel != 0)
			currentFuelString = currentFuel.ToString(lenDec: 2);
		else 
			currentFuelString = currentFuel.ToString(lenDec: 0);
		
		//~ Set dec lenght to either 2 or 0
		if (maxFuel - (int)maxFuel != 0)
			maxFuelString = maxFuel.ToString(lenDec: 2);
		else 
			maxFuelString = maxFuel.ToString(lenDec: 0);
		
		
		return WidgetManager.Translate(GetName(), currentFuelString, maxFuelString);
	}
}
