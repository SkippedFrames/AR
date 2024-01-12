[BaseContainerProps(configRoot:true)]
class SCR_ArsenalItemListConfig
{
	[Attribute("", UIWidgets.Object)]
	protected ref array<ref SCR_ArsenalItemStandalone> m_aArsenalItems;
	
	protected ref map<SCR_EArsenalItemType, ref array<SCR_ArsenalItem>> m_mArsenalItemsByType = new map<SCR_EArsenalItemType, ref array<SCR_ArsenalItem>>();
	
	bool GetArsenalItems(out array<ref SCR_ArsenalItemStandalone> arsenalItems)
	{
		arsenalItems = m_aArsenalItems;
		return arsenalItems != null && !arsenalItems.IsEmpty();
	}
	
	bool GetItemTypeAndModeForPrefab(ResourceName prefab, out SCR_EArsenalItemType itemType, out SCR_EArsenalItemMode itemMode)
	{
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(prefab))
			return false;
		
		for (int i = 0, count = m_aArsenalItems.Count(); i < count; i++)
		{
			SCR_ArsenalItem item = m_aArsenalItems[i];
			if (item.GetItemResourceName() == prefab)
			{
				itemType = item.GetItemType();
				itemMode =  item.GetItemMode();
				return true;
			}
		}
		return false;
	}
	
	//----------------------------------------------------------------------------
	/*!
	Get arsenal items filtered by SCR_EArsenalItemType filter, caches values
	\param filter Combined flags for available items for this faction (RIFLE, MAGAZINE, EQUIPMENT, RADIOBACKPACK etc.)
	\param requiresDisplayType Requires the Arsenal data to have display data type (-1 is ignore)
	\return array with availabe arsenal items of give filter types
	*/
	array<SCR_ArsenalItem> GetFilteredArsenalItems(SCR_EArsenalItemType typeFilter, SCR_EArsenalItemMode modeFilter, EArsenalItemDisplayType requiresDisplayType = -1)
	{
		array<SCR_ArsenalItem> filteredItems = new array<SCR_ArsenalItem>();
		
		array<SCR_ArsenalItem> itemsByType = m_mArsenalItemsByType.Get(typeFilter);
		if (!itemsByType)
		{
			itemsByType = new ref array<SCR_ArsenalItem>();
			array<ref SCR_ArsenalItemStandalone> availableArsenalItems;
			if (GetArsenalItems(availableArsenalItems))
			{				
				for (int i = 0, count = availableArsenalItems.Count(); i < count; i++)
				{
					//~ Ignore if no required display data				
					if (requiresDisplayType != -1 && !availableArsenalItems[i].GetDisplayDataOfType(requiresDisplayType))
						continue;
					
					if (availableArsenalItems[i].GetItemType() & typeFilter)
					{
						itemsByType.Insert(availableArsenalItems[i]);
					}
				}
				
				m_mArsenalItemsByType.Insert(typeFilter, itemsByType);
			}
		}
		
		foreach	(SCR_ArsenalItem item : itemsByType)
		{
			if (item.GetItemMode() & modeFilter)
			{
				filteredItems.Insert(item);
			}
		}
		return filteredItems;
	}
	
	bool GetItemModeForPrefab(ResourceName prefab, out SCR_EArsenalItemType itemMode)
	{
		for (int i = 0, count = m_aArsenalItems.Count(); i < count; i++)
		{
			SCR_ArsenalItem item = m_aArsenalItems[i];
			if (item.GetItemResourceName() == prefab)
			{
				itemMode = item.GetItemMode();
				return true;
			}
		}
		return false;
	}
};
