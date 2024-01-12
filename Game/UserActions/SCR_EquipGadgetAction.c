//------------------------------------------------------------------------------------------------
//! modded version for to be used with the inventory 2.0 
class SCR_EquipGadgetAction: SCR_InventoryAction
{
	#ifndef DISABLE_INVENTORY
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		SCR_InventoryStorageManagerComponent pInventoryManager = SCR_InventoryStorageManagerComponent.Cast( user.FindComponent( SCR_InventoryStorageManagerComponent ) );
		if ( !pInventoryManager )
			return false;
		
		SCR_EquipmentStorageComponent storageComp = SCR_EquipmentStorageComponent.Cast(pInventoryManager.FindStorageForItem(m_Item.GetOwner(), EStoragePurpose.PURPOSE_EQUIPMENT_ATTACHMENT));
		if (!storageComp)
		{
			SetCannotPerformReason("#AR-Inventory_NoSlot");
			return false;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void PerformActionInternal(SCR_InventoryStorageManagerComponent manager, IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (CanBePerformedScript(pUserEntity))
			manager.EquipGadget( pOwnerEntity );
	}
	#endif
};
