[EntityEditorProps(category: "GameScripted/UI/Inventory")]
class SCR_VehicleInventoryStorageManagerComponentClass : ScriptedInventoryStorageManagerComponentClass
{
}

class SCR_VehicleInventoryStorageManagerComponent : ScriptedInventoryStorageManagerComponent
{
	//------------------------------------------------------------------------------------------------
	protected override void FillInitialStorages(out array<BaseInventoryStorageComponent> storagesToAdd)
	{
		super.FillInitialStorages(storagesToAdd);

		if (!GetGame().InPlayMode())
			return;

		SlotManagerComponent slotManager = SlotManagerComponent.Cast(GetOwner().FindComponent(SlotManagerComponent));
		if (!slotManager)
			return;

		array<EntitySlotInfo> slots = {};
		slotManager.GetSlotInfos(slots);

		foreach (EntitySlotInfo info : slots)
		{
			IEntity ent = info.GetAttachedEntity();
			if (!ent)
				continue;
			BaseInventoryStorageComponent invComp = BaseInventoryStorageComponent.Cast(ent.FindComponent(BaseInventoryStorageComponent));
			if (invComp)
			{
				storagesToAdd.Insert(invComp);
			}
		}
	}
}
