[BaseContainerProps(configRoot: true)]
class SCR_SupplyCostItemHintUIInfo : SCR_InventoryItemHintUIInfo
{
	protected float m_fItemSupplyCost = -1;
	
	//------------------------------------------------------------------------------------------------
	void SetSupplyCost(float supplyCost)
	{
		m_fItemSupplyCost = supplyCost;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShown(InventoryItemComponent item)
	{
		if (!super.CanBeShown(item))
			return false;
		
		return item && m_fItemSupplyCost >= 0;
	}
	
	//------------------------------------------------------------------------------------------------
	override string GetItemHintName(InventoryItemComponent item)
	{
		return WidgetManager.Translate(GetName(), m_fItemSupplyCost);
	}
}
