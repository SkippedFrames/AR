//------------------------------------------------------------------------------------------------
class SCR_MineAddFlagAction : ScriptedUserAction
{
	[Attribute("0 0 0.19")]
	protected vector m_vFlagOffset;
	
	//------------------------------------------------------------------------------------------------
	IEntity FindFlagInInventory(IEntity user)
	{
		InventoryStorageManagerComponent storageManager = InventoryStorageManagerComponent.Cast(user.FindComponent(InventoryStorageManagerComponent));
		if (!storageManager)
			return null;
		
		array<typename> components = {};
		components.Insert(SCR_MineFlagComponent);
		return storageManager.FindItemWithComponents(components, EStoragePurpose.PURPOSE_ANY);
	}
	
	//------------------------------------------------------------------------------------------------
	void PlaceFlag(IEntity user, IEntity owner)
	{
		RplComponent userRplComponent = RplComponent.Cast(user.FindComponent(RplComponent));
		if (!userRplComponent)
			return;
		
		InventoryStorageManagerComponent storageManager = InventoryStorageManagerComponent.Cast(user.FindComponent(InventoryStorageManagerComponent));
		if (!storageManager)
			return;
		
		SCR_CharacterInventoryStorageComponent storage = SCR_CharacterInventoryStorageComponent.Cast(user.FindComponent(SCR_CharacterInventoryStorageComponent));
		if (!storage)
			return;
		
		IEntity flag = FindFlagInInventory(user);
		
		if (!flag)
			return;
		
		SCR_PlaceableInventoryItemComponent itemComponent = SCR_PlaceableInventoryItemComponent.Cast(flag.FindComponent(SCR_PlaceableInventoryItemComponent));
		
		RplComponent rplComponent = RplComponent.Cast(flag.FindComponent(RplComponent));
		if (rplComponent && !rplComponent.IsProxy())
		{
			vector mat[4];
			owner.GetTransform(mat);
			itemComponent.PlaceItem(mat[0], mat[1], mat[2], mat[3] + m_vFlagOffset);
		}
		
		SCR_SoundManagerEntity soundManagerEntity = GetGame().GetSoundManagerEntity();
		if (soundManagerEntity)
			soundManagerEntity.CreateAndPlayAudioSource(flag, "SOUND_MINEFLAG_PLACE");
				
		if (userRplComponent.IsOwner())
			storageManager.TryRemoveItemFromStorage(flag, itemComponent.GetParentSlot().GetStorage());
		
		SCR_MineWeaponComponent mine = SCR_MineWeaponComponent.Cast(owner.FindComponent(SCR_MineWeaponComponent));
		if (mine)
			mine.SetFlag(flag);
	}
	
	//------------------------------------------------------------------------------------------------
	override event void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		PlaceFlag(pUserEntity, pOwnerEntity);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		SCR_MineWeaponComponent mine = SCR_MineWeaponComponent.Cast(GetOwner().FindComponent(SCR_MineWeaponComponent));
		if (!mine || mine.IsFlagged())
			return false;
		
		SCR_PressureTriggerComponent mineTriggerComponent = SCR_PressureTriggerComponent.Cast(GetOwner().FindComponent(SCR_PressureTriggerComponent));
		if (!mineTriggerComponent || !mineTriggerComponent.IsActivated())
			return false;
		
		SCR_MineAwarenessComponent mineAwarenessComponent = SCR_MineAwarenessComponent.GetLocalInstance();
		if (!mineAwarenessComponent)
			return super.CanBeShownScript(user);
		
		if (!mineAwarenessComponent.IsDetected(GetOwner()) || !FindFlagInInventory(user))
			return false;
		
		return super.CanBeShownScript(user);
	}
	
};
