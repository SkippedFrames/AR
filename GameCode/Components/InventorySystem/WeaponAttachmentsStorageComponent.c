class SCR_WeaponAttachmentsStorageComponentClass: WeaponAttachmentsStorageComponentClass
{
};

class SCR_WeaponAttachmentsStorageComponent : WeaponAttachmentsStorageComponent
{
	ref ScriptInvoker m_OnItemAddedToSlotInvoker = new ScriptInvoker();
	ref ScriptInvoker m_OnItemRemovedFromSlotInvoker = new ScriptInvoker();
	
	// Callback when item is added (will be performed locally after server completed the Insert/Move operation)
	override protected void OnAddedToSlot(IEntity item, int slotID)
	{
		if (m_OnItemAddedToSlotInvoker)
			m_OnItemAddedToSlotInvoker.Invoke(item, slotID);
	}
	
	// Callback when item is removed (will be performed locally after server completed the Remove/Move operation)
	override protected void OnRemovedFromSlot(IEntity item, int slotID)
	{
		if (m_OnItemRemovedFromSlotInvoker && !item.IsDeleted())
			m_OnItemRemovedFromSlotInvoker.Invoke(item, slotID);
	}
};
