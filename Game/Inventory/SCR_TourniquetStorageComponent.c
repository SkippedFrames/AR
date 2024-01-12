class SCR_TourniquetStorageComponentClass : SCR_EquipmentStorageComponentClass
{
};

class SCR_TourniquetMovedCallback : ScriptedInventoryOperationCallback
{
	SCR_CharacterInventoryStorageComponent m_CharInventoryStorageComp;
	IEntity m_Tourniquet;
	int m_iSlotId;
	BaseInventoryStorageComponent m_Storage;
	bool m_bRemove;
	ref ScriptInvoker<int> m_OnTourniquetMoved;

	//------------------------------------------------------------------------------------------------
	override protected void OnComplete()
	{
		if (m_bRemove)
			m_CharInventoryStorageComp.RemoveItemFromQuickSlot(m_Tourniquet);

		SCR_InventoryMenuUI menu = SCR_InventoryMenuUI.Cast(GetGame().GetMenuManager().FindMenuByPreset(ChimeraMenuPreset.Inventory20Menu));
		if (menu)
		{
			InventoryItemComponent itemComp = InventoryItemComponent.Cast(m_Tourniquet.FindComponent(InventoryItemComponent));
			if (itemComp && itemComp.GetParentSlot())
			{
				BaseInventoryStorageComponent tqStorage = itemComp.GetParentSlot().GetStorage();
				SCR_InventoryStorageBaseUI storageUI = menu.GetStorageUIByBaseStorageComponent(m_Storage);
				if (storageUI)
					storageUI.Refresh();
			}

			SCR_InventoryStorageBaseUI storageUI = menu.GetStorageUIByBaseStorageComponent(m_Storage);
			if (storageUI)
				storageUI.Refresh();

			m_OnTourniquetMoved.Invoke(m_iSlotId);
		}
	}
};

class SCR_TourniquetStorageComponent : SCR_EquipmentStorageComponent
{
	ref SCR_TourniquetMovedCallback m_TourniquetMovedCallback = new SCR_TourniquetMovedCallback();
	
	//------------------------------------------------------------------------------------------------
	void AddTourniquetToSlot(IEntity target, ECharacterHitZoneGroup eHitZoneGroup, IEntity tourniquet)
	{
		SCR_TourniquetStorageComponent tourniquetStorageComp = SCR_TourniquetStorageComponent.Cast(target.FindComponent(SCR_TourniquetStorageComponent));
		if (!tourniquetStorageComp)
			return;		
		
		SCR_InventoryStorageManagerComponent storageMan = SCR_InventoryStorageManagerComponent.Cast(GetOwner().FindComponent(SCR_InventoryStorageManagerComponent));
		if (!storageMan)
			return;
	
		if (!tourniquet)
			return;
		
		SCR_TourniquetStorageSlot tqTargetSlot;
		for (int i, count = tourniquetStorageComp.GetSlotsCount(); i < count; i++)
		{
			tqTargetSlot = SCR_TourniquetStorageSlot.Cast(tourniquetStorageComp.GetSlot(i));
			if (!tqTargetSlot)
				continue;

			if (tqTargetSlot.GetAssociatedHZGroup() != eHitZoneGroup)
				continue;
			
			//Return if slot already contains this particular tourniquet. Likely function got called twice harmlessly
			if (tqTargetSlot.GetItem(i) == tourniquet)
				return;		

			if (tqTargetSlot.GetItem(i))
			{
				Print("TourniquetSlot already contained another item", LogLevel.WARNING);
				return;
			}

			break;
		}
		
		SCR_CharacterInventoryStorageComponent charInventoryStorageComp = SCR_CharacterInventoryStorageComponent.Cast(GetOwner().FindComponent(SCR_CharacterInventoryStorageComponent));
		if (!charInventoryStorageComp)
			return;

		m_TourniquetMovedCallback.m_Tourniquet = tourniquet;
		m_TourniquetMovedCallback.m_CharInventoryStorageComp = charInventoryStorageComp;
		m_TourniquetMovedCallback.m_iSlotId = eHitZoneGroup;

		InventoryItemComponent itemComp = InventoryItemComponent.Cast(tourniquet.FindComponent(InventoryItemComponent));
		m_TourniquetMovedCallback.m_Storage = itemComp.GetParentSlot().GetStorage();
		m_TourniquetMovedCallback.m_bRemove = true;
		
		storageMan.TryMoveItemToStorage(tourniquet, tourniquetStorageComp, tqTargetSlot.GetID(), m_TourniquetMovedCallback);
	}
	
	//------------------------------------------------------------------------------------------------
	bool RemoveTourniquetFromSlot(ECharacterHitZoneGroup eHitZoneGroup, IEntity retrievingCharacter = null)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		if (!character)
			return false;
		
		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(character.GetDamageManager());
		if (!damageMgr)
			return false;
		
		SCR_TourniquetStorageComponent tourniquetStorageComp = SCR_TourniquetStorageComponent.Cast(GetOwner().FindComponent(SCR_TourniquetStorageComponent));
		if (!tourniquetStorageComp)
			return false;

		if (!retrievingCharacter)
			retrievingCharacter = GetOwner();
	
		SCR_InventoryStorageManagerComponent storageMan = SCR_InventoryStorageManagerComponent.Cast(retrievingCharacter.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!storageMan)
			return false;
				
		IEntity targetTourniquet;
		SCR_TourniquetStorageSlot tqTargetSlot;
		for (int i, count = tourniquetStorageComp.GetSlotsCount(); i < count; i++)
		{
			tqTargetSlot = SCR_TourniquetStorageSlot.Cast(tourniquetStorageComp.GetSlot(i));
			if (!tqTargetSlot)
				continue;

			if (tqTargetSlot.GetAssociatedHZGroup() != eHitZoneGroup)
				continue;
			
			targetTourniquet = tqTargetSlot.GetItem(i);
			break;
		}

		BaseInventoryStorageComponent targetStorage = storageMan.FindStorageForItem(targetTourniquet, EStoragePurpose.PURPOSE_DEPOSIT);
		m_TourniquetMovedCallback.m_Storage = targetStorage;
		m_TourniquetMovedCallback.m_bRemove = false;

		return storageMan.TryMoveItemToStorage(targetTourniquet, targetStorage, -1, m_TourniquetMovedCallback);		
	}
	
	override void OnAddedToSlot(IEntity item, int slotID)
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(GetOwner());
		if (!char)
			return;

		SCR_TourniquetStorageSlot tqSlot = SCR_TourniquetStorageSlot.Cast(GetSlot(slotID));
		if (!tqSlot)
			return;

		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(char.GetDamageManager());
		if (!damageMgr)
			return;

		damageMgr.SetTourniquettedGroup(tqSlot.GetAssociatedHZGroup(), true);		
	}
	
	override void OnRemovedFromSlot(IEntity item, int slotID)
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(GetOwner());
		if (!char)
			return;
		
		SCR_TourniquetStorageSlot tqSlot = SCR_TourniquetStorageSlot.Cast(GetSlot(slotID));
		if (!tqSlot)
			return;

		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(char.GetDamageManager());
		if (!damageMgr)
			return;

		damageMgr.SetTourniquettedGroup(tqSlot.GetAssociatedHZGroup(), false);
	}
	
	ScriptInvoker GetOnTourniquetMoved()
	{
		if (!m_TourniquetMovedCallback.m_OnTourniquetMoved)
			m_TourniquetMovedCallback.m_OnTourniquetMoved = new ScriptInvoker();
		return m_TourniquetMovedCallback.m_OnTourniquetMoved;
	}
};
