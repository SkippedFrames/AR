[ComponentEditorProps(category: "GameScripted/Inventory", description: "Inventory 2.0", icon: HYBRID_COMPONENT_ICON)]
class SCR_CharacterInventoryStorageComponentClass: CharacterInventoryStorageComponentClass
{
};

//------------------------------------------------------------------------------------------------
//! default scritped storage for the character

const ref array<EWeaponType>		WEAPON_TYPES_THROWABLE	= {EWeaponType.WT_FRAGGRENADE, EWeaponType.WT_SMOKEGRENADE};

enum EItemType
{
	IT_NONE = 16385
};

//------------------------------------------------------------------------------------------------
class SCR_InvEquipCB : SCR_InvCallBack
{
	CharacterControllerComponent m_Controller;

	protected override void OnComplete()
	{
		m_Controller.TryEquipRightHandItem(m_pItem, EEquipItemType.EEquipTypeWeapon, false);
		m_pItem = null;
	}
	
	protected override void OnFailed()
	{
		m_pItem = null;
	}
};

//------------------------------------------------------------------------------------------------
class SCR_EquipNextGrenadeCB : SCR_InvCallBack
{
	SCR_InventoryStorageManagerComponent m_InvMan;

	//------------------------------------------------------------------------------------------------
	protected override void OnComplete()
	{
		if (m_pItem)
			m_InvMan.EquipWeapon(m_pItem, null, false);
		
		m_pItem = null;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnFailed()
	{
		m_pItem = null;
	}
};	

class SCR_CharacterInventoryStorageComponent : CharacterInventoryStorageComponent
{
	//TODO: define this on loadout level. This is temporary and will be removed!
	[Attribute( "0", UIWidgets.EditBox, "How much weight the character can carry")]
	protected float m_fMaxWeight;
	
	//TODO: define this on loadout level. This is temporary and will be removed!
	[Attribute( "0", UIWidgets.EditBox, "How much volume the character can carry")]
	protected float m_fMaxVolume;
	
	#ifndef DISABLE_INVENTORY
	
	private BaseInventoryStorageComponent m_pLootStorage;
	protected ref array<IEntity>						m_aQuickSlots = { null, null, null, null, null, null, null, null, null, null };
	protected ref map<IEntity, int> 					m_mSlotHistory = new map<IEntity, int>();
	protected ref array<IEntity>						m_aWeaponQuickSlotsStorage = {}; //Is used to store first four quickslots of turrets.
	protected ref array< int >							m_aQuickSlotsHistory = new array< int >();	//here we'll be remembering the items stored
	// protected ref array<EntityComponentPrefabData>		m_aPrefabsData = { null, null, null, null, null, null, null, null, null, null }; // todo: figure out the intentions
	protected string				 					m_pLoadout;
	protected static const int							GADGET_OFFSET = 9999;	//EWeaponType && EGadgetType might be have the same number, offset it ( not nice (i agree) )
	/*
	protected ref array<ref array<int>>					m_aDefaultRiflemanQuickSlots = 	{	{ EWeaponType.WT_RIFLE, EWeaponType.WT_SNIPERRIFLE, EWeaponType.WT_MACHINEGUN },
																							{ EWeaponType.WT_HANDGUN, EWeaponType.WT_ROCKETLAUNCHER, EWeaponType.WT_GRENADELAUNCHER, ( EGadgetType.BINOCULARS ) + GADGET_OFFSET  },
																							{ ( EGadgetType.FLASHLIGHT ) + GADGET_OFFSET },
																							{ EWeaponType.WT_FRAGGRENADE, EWeaponType.WT_SMOKEGRENADE },
																							{ ( EGadgetType.MAP ) + GADGET_OFFSET },
																							{ ( EGadgetType.COMPASS ) + GADGET_OFFSET } 
																						};
	*/
	protected static ref array<ref array<int>>			DEFAULT_QUICK_SLOTS =			{	{ EWeaponType.WT_RIFLE, EWeaponType.WT_SNIPERRIFLE, EWeaponType.WT_MACHINEGUN },
																							{ EWeaponType.WT_RIFLE, EWeaponType.WT_ROCKETLAUNCHER, EWeaponType.WT_GRENADELAUNCHER, EWeaponType.WT_SNIPERRIFLE, EWeaponType.WT_MACHINEGUN },
																							{ EWeaponType.WT_HANDGUN },
																							{ EWeaponType.WT_FRAGGRENADE },
																							{ EWeaponType.WT_SMOKEGRENADE },
																							{ EGadgetType.CONSUMABLE + GADGET_OFFSET + SCR_EConsumableType.BANDAGE },
																							{ EGadgetType.CONSUMABLE + GADGET_OFFSET + SCR_EConsumableType.TOURNIQUET },
																							{ EGadgetType.CONSUMABLE + GADGET_OFFSET + SCR_EConsumableType.MORPHINE, EGadgetType.CONSUMABLE + GADGET_OFFSET + SCR_EConsumableType.SALINE },
																							{ EGadgetType.RADIO + GADGET_OFFSET }, // Preferably as GadgetRadio action, then it can be saline
																							{ EGadgetType.BUILDING_TOOL + GADGET_OFFSET } // To be replaced with engineering tool
																						};

	protected ref array<typename> m_aBlockedSlots = {};
	protected ref array<BaseInventoryStorageComponent> m_aStoragesInStorageList = {};		//here we remember the opened storages in the Inventory menu ( in the Storages list area )
	protected SCR_CompartmentAccessComponent m_CompartmentAcessComp;
	protected BaseInventoryStorageComponent m_WeaponStorage;
	
	protected ref SCR_InvEquipCB m_Callback = new SCR_InvEquipCB();
	
	//------------------------------------------------------------------------ USER METHODS ------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//TODO: define this on loadout level. This is temporary and will be removed!
	float GetMaxLoad() { return m_fMaxWeight; }
	
	//------------------------------------------------------------------------------------------------
	// !
	BaseInventoryStorageComponent GetWeaponStorage()
	{
		if (!m_WeaponStorage)
			m_WeaponStorage = BaseInventoryStorageComponent.Cast(GetOwner().FindComponent(EquipedWeaponStorageComponent));
		
		return m_WeaponStorage;
	}
	
	//------------------------------------------------------------------------------------------------
	// !
	InventoryItemComponent GetItemFromLoadoutSlot( LoadoutAreaType eSlot )
	{
		// Not all attached Entities to slots are storages.
		// For instance boots - even though attached to character storage slot by themselves represent item, not storage
		// However if entity attached to slot it is guaranteed to have InventoryItemComponent
		InventoryStorageSlot slot = GetSlotFromArea(eSlot.Type());
		if (!slot)
			return null;
		
		IEntity entity = slot.GetAttachedEntity();
		if (!entity)
			return null;
		
		return InventoryItemComponent.Cast(entity.FindComponent(InventoryItemComponent));
	}
	//------------------------------------------------------------------------------------------------
	// !
	BaseInventoryStorageComponent GetStorageFromLoadoutSlot( LoadoutAreaType eSlot )
	{
		return BaseInventoryStorageComponent.Cast( GetItemFromLoadoutSlot(eSlot) );
	}
	
	//------------------------------------------------------------------------------------------------
	// 
	protected bool HasStorageComponent( IEntity pEntity )
	{
		return GetStorageComponentFromEntity( pEntity ) != null;	
	}
	
	
	//------------------------------------------------------------------------------------------------
	// ! returns all topmost storages
	void GetStorages( out notnull array<SCR_UniversalInventoryStorageComponent> storagesInInventory )
	{
		array<IEntity> pEntities = new array<IEntity>();
		int iNrOfStorages = GetAll( pEntities );
		
		foreach ( IEntity pEntity: pEntities )
		{
			SCR_UniversalInventoryStorageComponent pUniComp = GetStorageComponentFromEntity( pEntity );
			if( pUniComp )
				storagesInInventory.Insert( pUniComp );
		}
	}
		
	//------------------------------------------------------------------------------------------------
	void GetBlockedSlots(out notnull array<typename> blockedSlots)
	{
		blockedSlots.Copy(m_aBlockedSlots);
	}

	//------------------------------------------------------------------------------------------------
	// ! get the item inventory component 
	SCR_UniversalInventoryStorageComponent GetStorageComponentFromEntity( IEntity pEntity )
	{
		if ( pEntity == null )
			return null;
		
		return SCR_UniversalInventoryStorageComponent.Cast(pEntity.FindComponent( SCR_UniversalInventoryStorageComponent ));	
	}

	//------------------------------------------------------------------------------------------------
	// !
	void SetLootStorage( IEntity pOwner )
	{
		if( !pOwner )
		{
			m_pLootStorage = null;
			return;
		}
		m_pLootStorage = BaseInventoryStorageComponent.Cast( pOwner.FindComponent( BaseInventoryStorageComponent ) );
	}

	BaseInventoryStorageComponent GetLootStorage()
	{
		return m_pLootStorage;
	}
	
	//------------------------------------------------------------------------------------------------
	// ! returns the visibility state of the adequate UI container - if the storage was previously shown in the inventory
	bool GetIsStorageShown( notnull BaseInventoryStorageComponent pStorage )
	{
		return m_aStoragesInStorageList.Find( pStorage ) != -1;
	}
	//------------------------------------------------------------------------------------------------
	// ! 
	void SetStorageAsShown( notnull BaseInventoryStorageComponent pStorage )
	{
		if ( !GetIsStorageShown( pStorage ) )
		{
			m_aStoragesInStorageList.Insert( pStorage );
		}
	}
	//------------------------------------------------------------------------------------------------
	// !
	void SetStorageAsHidden( notnull BaseInventoryStorageComponent pStorage )
	{
		m_aStoragesInStorageList.RemoveItem( pStorage );
	}
	
	//------------------------------------------------------------------------------------------------
	// !
	static int GetItemType( IEntity pItem )
	{
		int iItemType = -1;
		//Weapons:
		BaseWeaponComponent weaponComponent = BaseWeaponComponent.Cast( pItem.FindComponent( BaseWeaponComponent ) );
		if ( weaponComponent )
		{
			iItemType = weaponComponent.GetWeaponType();
		}
		else
		{
			//Gadgets:
			SCR_GadgetComponent gadgetComponent = SCR_GadgetComponent.Cast( pItem.FindComponent( SCR_GadgetComponent ) );
			if ( gadgetComponent )
			{
				EGadgetType gadgetType = gadgetComponent.GetType();
				int consumableOffset = 0;
				if (gadgetType == EGadgetType.CONSUMABLE)
				{
					SCR_ConsumableItemComponent consumable = SCR_ConsumableItemComponent.Cast(gadgetComponent);
					if (consumable)
						consumableOffset = consumable.GetConsumableType();
				}
				iItemType = gadgetComponent.GetType() + GADGET_OFFSET + consumableOffset;
			}
		}
				
		return iItemType;
	}
	
	//------------------------------------------------------------------------------------------------
	// !
	bool ItemBelongsToSlot( int iItemType, int iSlotIndex )
	{
		return DEFAULT_QUICK_SLOTS[iSlotIndex].Contains(iItemType);
	}

	//------------------------------------------------------------------------------------------------
	int GetLastQuickSlotId(IEntity ent)
	{
		int result = m_mSlotHistory.Get(ent);
		m_mSlotHistory.Remove(ent);
		return result;
	}

	//------------------------------------------------------------------------------------------------
	int GetEntityQuickSlot(IEntity item)
	{
		int itemType = GetItemType(item);
		foreach (int i, ref array<int> allowedTypes: DEFAULT_QUICK_SLOTS)
		{
			if (allowedTypes.Contains(itemType))
				return i;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	bool IsItemAlreadyInQuickSlot(IEntity item)
	{
		int itemType = GetItemType(item);
		if (itemType < 0)
			return false;
		int slotId = GetEntityQuickSlot(item);
		if (slotId < 0)
			return false;
		return m_aQuickSlots[slotId] == item;
	}

	//------------------------------------------------------------------------------------------------
	// !
	int StoreItemToQuickSlot( notnull IEntity pItem, int iSlotIndex = -1, bool isForced = false )
	{
		int iItemType = GetItemType( pItem );
		if ( iSlotIndex == -1 ) //we don't know what slot we put the item into. Check first if we remember the type of the item
		{	
			InventoryItemComponent itemComp = InventoryItemComponent.Cast(pItem.FindComponent(InventoryItemComponent));
			InventoryStorageSlot parentSlot;
			if (itemComp)
				parentSlot = itemComp.GetParentSlot();
			
			if (parentSlot && EquipedWeaponStorageComponent.Cast(parentSlot.GetStorage()))
			{
				iSlotIndex = parentSlot.GetID();
			}
			else
			{			
				for (int iLoop, cnt = m_aQuickSlots.Count(); iLoop < cnt; iLoop++)	//go through the all quick slots
				{
					if ( m_aQuickSlots[ iLoop ] ) //do it only for the empty slots
						continue;
					if ( iItemType == m_aQuickSlotsHistory[ iLoop ] ) // there was aready something in the slot with this index
					{
						iSlotIndex = iLoop;
						break;
					}	
					else
					{
						//there was nothing before, put the item into slot defined by the template ( DEFAULT_QUICK_SLOTS )
						if ( ItemBelongsToSlot( iItemType, iLoop ) )
						{
							iSlotIndex = iLoop;
							break;
						}
					}					
				}
			}
		}
				
		if ( iSlotIndex == -1 )	//any suitable slot not found, do not insert into quick slot
			return -1;
		
		if (!isForced)
		{
			TurretCompartmentSlot turretCompartment = TurretCompartmentSlot.Cast(GetCurrentCompartment());
			if (turretCompartment)
			{
				if (iSlotIndex < SCR_InventoryMenuUI.WEAPON_SLOTS_COUNT)
					return -1;
				
				array<IEntity> turretWeapons = {};
				GetTurretWeaponsList(turretCompartment, turretWeapons);
				if (turretWeapons.Contains(pItem))
					return -1;
			}
		}
		
		if ( pItem == m_aQuickSlots.Get( iSlotIndex ) )	//if there's already the item in the slot, remove it
		{
			m_aQuickSlots.Set( iSlotIndex, null );
		}
		else
		{
			int iOldIndex = RemoveItemFromQuickSlot( pItem );
			if ( 0 <= iOldIndex && iOldIndex < m_aQuickSlotsHistory.Count() )
				m_aQuickSlotsHistory[ iOldIndex ] = 0;	//in case the item is already in slot and we shift the item into a different slot
			m_aQuickSlots.Set( iSlotIndex, pItem );
			
			m_aQuickSlotsHistory[ iSlotIndex ] = iItemType; // remember it
		}

		return iSlotIndex;
	}
	
	//------------------------------------------------------------------------------------------------
	// !
	int RemoveItemFromQuickSlot( IEntity pItem )
	{
		int iIndex = m_aQuickSlots.Find( pItem );
		if ( iIndex != -1 )
			m_aQuickSlots.Set( iIndex, null );
		
		return iIndex;
	}
	
	//------------------------------------------------------------------------------------------------
	// !
	void RemoveItemFromQuickSlotAtIndex(int iIndex)
	{
		if (iIndex >= 0 && m_aQuickSlots.Get(iIndex))
			m_aQuickSlots.Set(iIndex, null);
	}
	
	//------------------------------------------------------------------------------------------------
	// !
	array<IEntity> GetQuickSlotItems()
	{ 
		return m_aQuickSlots; 
	}

	//------------------------------------------------------------------------------------------------
	IEntity GetItemFromQuickSlot(int index)
	{
		if (!m_aQuickSlots.IsIndexValid(index))
			return null;

		return m_aQuickSlots[index];
	}

	//------------------------------------------------------------------------------------------------
	//! Get currently held item. If character holds gadget, gadget is returned, otherwise current weapon.
	IEntity GetCurrentItem()
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		if (!character)
			return null;
		
		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return null;
		
		IEntity gadget = controller.GetAttachedGadgetAtLeftHandSlot();
		if (gadget)
			return gadget;
		
		BaseWeaponManagerComponent weaponManager = controller.GetWeaponManagerComponent();
		if (!weaponManager)
			return null;
		
		BaseWeaponComponent weapon = weaponManager.GetCurrentWeapon();
		if (weapon)
			return weapon.GetOwner();
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get selected item. If there is no active switching, currently held item is returned instead.
	IEntity GetSelectedItem()
	{
		// Selection is in progress
		if (m_Callback.m_pItem)
			return m_Callback.m_pItem;
	
		return GetCurrentItem();	
	}
	
	//------------------------------------------------------------------------------------------------
	//! Unequip currently held item. Not allowed while switching to another item.
	void UnequipCurrentItem()
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		if (!character)
			return;
		
		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return;
		
		if (controller.IsChangingItem())
			return;
		
		if (controller.IsGadgetInHands())
			controller.RemoveGadgetFromHand();
		else
			controller.SelectWeapon(null);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Drop currently held item. Not allowed while switching to another item.
	void DropCurrentItem()
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		if (!character)
			return;
		
		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return;
		
		if (controller.IsChangingItem())
			return;
		
		SCR_InventoryStorageManagerComponent storageManager = SCR_InventoryStorageManagerComponent.Cast(controller.GetInventoryStorageManager());
		if (!storageManager)
			return;
		
		IEntity itemEnt;
		
		// TODO: Also drop hidden sticky gadget
		if (controller.IsGadgetInHands())
		{
			// TODO: Equip another gadget or consumable of same type
			itemEnt = controller.GetAttachedGadgetAtLeftHandSlot();
			storageManager.TryRemoveItemFromInventory(itemEnt);
			return;
		}
		
		BaseWeaponManagerComponent weaponManager = controller.GetWeaponManagerComponent();
		if (!weaponManager)
			return;
		
		WeaponSlotComponent currentSlot = weaponManager.GetCurrentSlot();
		if (!currentSlot)
			return;
		
		SCR_EquipNextGrenadeCB callback = new SCR_EquipNextGrenadeCB();
		
		EWeaponType type = currentSlot.GetWeaponType();
		itemEnt = currentSlot.GetWeaponEntity();
		
		if (!storageManager.CanMoveItem(itemEnt))
			return;
		
		callback.m_InvMan = storageManager;		
		
		if (WEAPON_TYPES_THROWABLE.Contains(type))
			callback.m_pItem = storageManager.FindNextWeaponOfType(type, itemEnt, true);
		
		storageManager.SetInventoryLocked(true);
		controller.DropWeapon(currentSlot);
		storageManager.SetInventoryLocked(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when item is added to slot, update armored attributes when item with armorData is equipped
	protected override void OnAddedToSlot(IEntity item, int slotID)
	{
		super.OnAddedToSlot(item, slotID);
		
		UpdateBlockedSlots(item, slotID, true);
		
		EditArmoredAttributes(item, slotID);

		#ifdef DEBUG_INVENTORY20
		// Loadout manager is taking care of this since there are some items that shouldn't be visible when attached to slot, some have different meshes for different states.
		// Consider glasses in first person view - they deffinitely should be disabled
		// it is slightly more complex then this
			
		InventoryItemComponent itemComponent = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (!itemComponent) 
			return;
		
		SCR_UniversalInventoryStorageComponent storageComponent = GetStorageComponentFromEntity(item);
		if (!storageComponent)
			return;
		
		SCR_ItemAttributeCollection attr = SCR_ItemAttributeCollection.Cast(storageComponent.GetAttributes());
		if( !attr )
			return;
		
		UIInfo UIinfoItem = attr.GetUIInfo();
		if( !UIinfoItem )
			return;
		
		PrintFormat( "INV: item %1 was added. It's weight is: %2, and total weight of item/storage is: %3", UIinfoItem.GetName(), attr.GetWeight(), storageComponent.GetTotalWeight() );
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnRemovedFromSlot(IEntity item, int slotID)
	{
		super.OnRemovedFromSlot(item, slotID);
			
		UpdateBlockedSlots(item, slotID, false);
		
		EditArmoredAttributes(item, slotID, true);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateBlockedSlots(IEntity item, int slotID, bool added)
	{
		BaseLoadoutClothComponent loadoutComp = BaseLoadoutClothComponent.Cast(item.FindComponent(BaseLoadoutClothComponent));
		
		if (!loadoutComp)
			return;
		
		array<typename> blockedSlots = {};
		
		loadoutComp.GetBlockedSlots(blockedSlots);
		
		if (blockedSlots.IsEmpty())
			return;
		
		if (added)
		{
			foreach (typename blockedSlot: blockedSlots)
			{
				m_aBlockedSlots.Insert(blockedSlot);
			}
		}
		else
		{
			foreach (typename blockedSlot: blockedSlots)
			{
				m_aBlockedSlots.RemoveItem(blockedSlot);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	bool IsAreaBlocked(typename areaType)
	{
		return m_aBlockedSlots.Contains(areaType);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Take the data from the armor attribute, and store them in map on damagemanager
	protected void EditArmoredAttributes(IEntity item, int slotID, bool remove = false)
	{
		InventoryItemComponent itemComponent = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (!itemComponent) 
			return;
		
		SCR_ItemAttributeCollection attributes = SCR_ItemAttributeCollection.Cast(itemComponent.GetAttributes());
		if (!attributes)
			return;
		
		SCR_ArmoredClothItemData armorAttr = SCR_ArmoredClothItemData.Cast(attributes.FindAttribute(SCR_ArmoredClothItemData));
		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(GetOwner().FindComponent(SCR_CharacterDamageManagerComponent));
		if (armorAttr && damageMgr)
			damageMgr.UpdateArmorDataMap(armorAttr, remove);
	}
	
	//------------------------------------------------------------------------------------------------
	// !
	void HandleOnItemAddedToInventory( IEntity item, BaseInventoryStorageComponent storageOwner )
	{
		int targetQuickSlot = StoreItemToQuickSlot(item);
		if (targetQuickSlot > -1 && SCR_WeaponSwitchingBaseUI.s_bOpened)
			SCR_WeaponSwitchingBaseUI.RefreshQuickSlots(targetQuickSlot);
	}
	
	//------------------------------------------------------------------------------------------------
	// !
	void HandleOnItemRemovedFromInventory( IEntity item, BaseInventoryStorageComponent storageOwner )
	{
		m_mSlotHistory.Set(item, m_aQuickSlots.Find(item));
		RemoveItemFromQuickSlot( item );
	}
	
	//------------------------------------------------------------------------------------------------	
	bool CanUseItem(notnull IEntity item, ESlotFunction slotFunction = ESlotFunction.TYPE_GENERIC)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		if (!character)
			return false;
		
		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return false;
		
		// Autodetect slot function if not provided
		if (slotFunction == ESlotFunction.TYPE_GENERIC)
		{
			if (item.FindComponent(MagazineComponent))
				slotFunction = ESlotFunction.TYPE_MAGAZINE;
			else if (item.FindComponent(BaseWeaponComponent))
				slotFunction = ESlotFunction.TYPE_WEAPON;
			else if (item.FindComponent(SCR_GadgetComponent))
				slotFunction = ESlotFunction.TYPE_GADGET;
		}
		
		switch (slotFunction)
		{
			case ESlotFunction.TYPE_MAGAZINE:
			{
				if (!character.IsInVehicle())
					return CanReloadCurrentWeapon(item);
				
				return false;
			}
			
			case ESlotFunction.TYPE_WEAPON:
			{
				if (TurretCompartmentSlot.Cast(GetCurrentCompartment()))
				{
					BaseWeaponComponent currentWeapon = GetCurrentTurretWeapon();
					return currentWeapon && currentWeapon.GetOwner(); // TODO: != item
				}
				else
				{
					return controller.GetCanFireWeapon(); // TODO: Has multiple muzzles or has next grenade type
				}
				
				return false;
			}
			
			case ESlotFunction.TYPE_GADGET:
			{
				if (character.IsInVehicleADS())
					return false;
				
				return controller.CanEquipGadget(item);
			}
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------	
	bool UseItem(notnull IEntity item, ESlotFunction slotFunction = ESlotFunction.TYPE_GENERIC)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		if (!character)
			return false;
		
		// Autodetect slot function
		if (slotFunction == ESlotFunction.TYPE_GENERIC)
		{
			if (item.FindComponent(MagazineComponent))
				slotFunction = ESlotFunction.TYPE_MAGAZINE;
			else if (item.FindComponent(BaseWeaponComponent))
				slotFunction = ESlotFunction.TYPE_WEAPON;
			else if (item.FindComponent(SCR_GadgetComponent))
				slotFunction = ESlotFunction.TYPE_GADGET;
		}
		
		switch (slotFunction)
		{
			case ESlotFunction.TYPE_MAGAZINE:
			{
				RemoveItemFromQuickSlot(item);

				SCR_MagazinePredicate predicate = new SCR_MagazinePredicate();
				predicate.magWellType = MagazineComponent.Cast(item.FindComponent(MagazineComponent)).GetMagazineWell().Type();
				array<IEntity> magazines = {};

				InventoryStorageManagerComponent invMan = InventoryStorageManagerComponent.Cast(GetOwner().FindComponent(InventoryStorageManagerComponent));
				if (invMan)
					invMan.FindItems(magazines, predicate);

				foreach (IEntity nextMag : magazines)
				{
					if (nextMag != item)
					{
						StoreItemToQuickSlot(nextMag);
						break;
					}
				}

				return ReloadCurrentWeapon(item);
			}
			
			case ESlotFunction.TYPE_WEAPON:
			{
				CharacterControllerComponent controller = character.GetCharacterController();
				if (!controller)
					return false;
				
				m_Callback.m_pItem = item;
				m_Callback.m_Controller = controller;
				
				SCR_GadgetManagerComponent gadgetMgr = SCR_GadgetManagerComponent.GetGadgetManager(character);
				if (gadgetMgr)
					gadgetMgr.RemoveHeldGadget();
				
				TurretCompartmentSlot turretCompartment = TurretCompartmentSlot.Cast(GetCurrentCompartment());
				if (turretCompartment)
				{	
					TurretControllerComponent turretController = TurretControllerComponent.Cast(turretCompartment.GetController());
					if (!turretController)
						return false;
					
					array<WeaponSlotComponent> turretWeaponSlots = {};		
					GetTurretWeaponSlots(turretCompartment, turretWeaponSlots);
					foreach (WeaponSlotComponent weaponSlot: turretWeaponSlots)
					{
						if (weaponSlot.GetWeaponEntity() == item)
							return turretController.SelectWeapon(character, weaponSlot);
					}
				}
				else
				{
					SCR_InventoryStorageManagerComponent inventoryManager = SCR_InventoryStorageManagerComponent.Cast(controller.GetInventoryStorageManager());
					if (!inventoryManager)
						return false;
					
					BaseWeaponComponent currentWeapon;
					BaseWeaponManagerComponent manager = controller.GetWeaponManagerComponent();
					if (manager)
						currentWeapon = manager.GetCurrentWeapon();
					
					// Swap grenade type only if reselecting the same grenade slot
					BaseWeaponComponent itemWeapon = BaseWeaponComponent.Cast(item.FindComponent(BaseWeaponComponent));
					if (itemWeapon && itemWeapon.CanBeEquipped(controller) != ECanBeEquippedResult.OK)
					{
						return false;
					}
					else if (currentWeapon && itemWeapon && currentWeapon.GetWeaponType() == itemWeapon.GetWeaponType() && WEAPON_TYPES_THROWABLE.Contains(itemWeapon.GetWeaponType()))
					{
						// Equip different type of grenade
						IEntity nextGrenade = inventoryManager.FindNextWeaponOfType(itemWeapon.GetWeaponType(), currentWeapon.GetOwner());
						if (nextGrenade)
							m_Callback.m_pItem = nextGrenade;
					}
					// Currently selected weapon can have alternative muzzle
					else if (currentWeapon && currentWeapon.GetOwner() == item && !controller.IsGadgetInHands())
					{
						// Select next muzzle of a selected weapon
						int nextMuzzleID = SCR_WeaponLib.GetNextMuzzleID(currentWeapon); 
						if (nextMuzzleID != -1)
							return controller.SetMuzzle(nextMuzzleID);
						
						return false;
					}
					
					// TODO: Interrupt current equipping process now
					if (GetWeaponStorage() && GetWeaponStorage().Contains(m_Callback.m_pItem))
					{
						controller.TryEquipRightHandItem(m_Callback.m_pItem, EEquipItemType.EEquipTypeWeapon, false);
						m_Callback.m_pItem = null;
					}
					else
					{
						inventoryManager.EquipWeapon(m_Callback.m_pItem, m_Callback, false);
					}
					
					return true;
				}
				break;
			}
			
			case ESlotFunction.TYPE_GADGET:
			{
				if (character.IsInVehicleADS())
					return false;
				
				// need to run through manager
				// TODO kamil: this doesnt call setmode when switching to other item from gadget (no direct call to scripted togglefocused for example, possibly other issues?)
				SCR_GadgetManagerComponent gadgetMgr = SCR_GadgetManagerComponent.GetGadgetManager(character);
				if (gadgetMgr)
					gadgetMgr.SetGadgetMode(item, EGadgetMode.IN_HAND);
				else
					return false;
				return true;
			}
		} 
		return false;
	}
	
	//------------------------------------------------------------------------------------------------	
	protected bool CanReloadCurrentWeapon(notnull IEntity item)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		if (!character)
			return false;
		
		CharacterControllerComponent controller = character.GetCharacterController();	
		if (!controller)
			return false;
		
		BaseWeaponManagerComponent weaponManager = controller.GetWeaponManagerComponent();
		if (!weaponManager)
			return false;
		
		BaseWeaponComponent currentWeapon = weaponManager.GetCurrent();
		if (!currentWeapon)
			return false;
		
		if (!currentWeapon.IsReloadPossible())
			return false;
		
		BaseMuzzleComponent currentMuzzle = currentWeapon.GetCurrentMuzzle();
		if (!currentMuzzle)
			return false;
		
		BaseMagazineWell currentMagWell = currentMuzzle.GetMagazineWell();
		if (!currentMagWell)
			return false;
		
		MagazineComponent magazine = MagazineComponent.Cast(item.FindComponent(MagazineComponent));
		if (!magazine)
			return false;
		
		if (magazine.GetMagazineWell().Type() == currentMagWell.Type())
			return true;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------	
	protected bool ReloadCurrentWeapon(IEntity item)
	{
		if (!item)
			return false;
		
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		if (!character)
			return false;
		
		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return false;
		
		return controller.ReloadWeaponWith(item);
	}
	
	protected void GetPlayersWeapons( notnull inout array<IEntity> outWeapons )
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		if (!character)
			return;
		
		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return;
		
		BaseWeaponManagerComponent weaponManager = controller.GetWeaponManagerComponent();
		if (weaponManager)
			weaponManager.GetWeaponsList(outWeapons);
	}
	
	//------------------------------------------------------------------------------------------------
	protected BaseCompartmentSlot GetCurrentCompartment()
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		if (!character)
			return null;
		
		CompartmentAccessComponent compAccess = character.GetCompartmentAccessComponent();
		if (!compAccess || !compAccess.IsInCompartment())
			return null;
		
		return compAccess.GetCompartment();
	}
	
	//------------------------------------------------------------------------------------------------
	protected BaseWeaponComponent GetCurrentTurretWeapon()
	{
		BaseCompartmentSlot compartment = GetCurrentCompartment();
		if (!compartment)
			return null;
		
		TurretControllerComponent controller = TurretControllerComponent.Cast(compartment.GetController());
		if (!controller)
			return null;
		
		BaseWeaponManagerComponent weaponManager = controller.GetWeaponManager();
		if (!weaponManager)
			return null;
		
		return weaponManager.GetCurrentWeapon();
	}
	
	//------------------------------------------------------------------------------------------------
	// !
	void InitAsPlayer(IEntity pOwner, bool pControlled)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(pOwner);
		if (!character)
			return;
		
		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return;
		
		SCR_InventoryStorageManagerComponent pInventoryManager = SCR_InventoryStorageManagerComponent.Cast(controller.GetInventoryStorageManager());
		if ( !pInventoryManager )
			return;
			
		if( pControlled )
		{
			int i = DEFAULT_QUICK_SLOTS.Count();
			if ( m_aQuickSlotsHistory.Count() <  i )
			{
				for ( int m = 0; m < i; m++ )
					m_aQuickSlotsHistory.Insert( 0 );	//default is none	
			}
			
			//Insert weapons to quick slots as first, because they can't be equipped into hands directly from different storage than EquippedWeaponStorage
			//TODO: optimalize ( hotfix for ability to select grenades from quickslot )
			
			array<IEntity> outWeapons = {};
			GetPlayersWeapons( outWeapons );
			foreach ( IEntity weapon : outWeapons )
				StoreItemToQuickSlot( weapon );

			// There may be unequipped grenades among default quick item slots as well
			array<IEntity> outItems = {};
			pInventoryManager.GetItems( outItems );
			foreach ( IEntity pItem : outItems )
			{
				if (!pItem)
					continue;
				
				if (IsItemAlreadyInQuickSlot(pItem))
					continue;
				
				StoreItemToQuickSlot( pItem );
			}
			
			pInventoryManager.m_OnItemAddedInvoker.Insert( HandleOnItemAddedToInventory );
			pInventoryManager.m_OnItemRemovedInvoker.Insert( HandleOnItemRemovedFromInventory );

			SCR_CharacterControllerComponent charController = SCR_CharacterControllerComponent.Cast(controller);
			if (charController)
				charController.m_OnItemUseEndedInvoker.Insert(OnItemUsed);
			
			m_CompartmentAcessComp = SCR_CompartmentAccessComponent.Cast(character.GetCompartmentAccessComponent());
			if (!m_CompartmentAcessComp)
				return;
			
			if (m_CompartmentAcessComp)
			{
				m_CompartmentAcessComp.GetOnCompartmentEntered().Insert(OnCompartmentEntered);
				m_CompartmentAcessComp.GetOnCompartmentLeft().Insert(OnCompartmentLeft);
			}
		}
		else
		{
			pInventoryManager.m_OnItemAddedInvoker.Remove( HandleOnItemAddedToInventory );
			pInventoryManager.m_OnItemRemovedInvoker.Remove( HandleOnItemRemovedFromInventory );
			
			if (m_CompartmentAcessComp)
			{
				m_CompartmentAcessComp.GetOnCompartmentEntered().Remove(OnCompartmentEntered);
				m_CompartmentAcessComp.GetOnCompartmentLeft().Remove(OnCompartmentLeft);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	// Called when item is used by the player
	protected void OnItemUsed(IEntity item, bool successful, SCR_ConsumableEffectAnimationParameters animParams)
	{
		// If the item isn't consumable, return
		if (!SCR_ConsumableItemComponent.Cast(item.FindComponent(SCR_ConsumableItemComponent)))
			return;
		
		// restock used medical item back to its quick slot
		int quickSlot = m_aQuickSlots.Find(item);
		if (quickSlot == -1)
			return;

		RemoveItemFromQuickSlotAtIndex(quickSlot);
		int itemType = GetItemType(item);

		typename t = BaseWeaponComponent;
		if (itemType > GADGET_OFFSET)
			t = SCR_GadgetComponent;

		SCR_ItemTypeSearchPredicate itemSearch = new SCR_ItemTypeSearchPredicate(t, itemType, item);
		array<IEntity> items = {};

		InventoryStorageManagerComponent invMan = InventoryStorageManagerComponent.Cast(GetOwner().FindComponent(InventoryStorageManagerComponent));
		if (!invMan)
			return;

		invMan.FindItems(items, itemSearch);

		if (!items.IsEmpty())
		{
			foreach (IEntity itm : items)
			{
				if (itm == item)
					continue;
				StoreItemToQuickSlot(itm, quickSlot);
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! SCR_CompartmentAccessComponent event
	protected void OnCompartmentEntered(IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		BaseCompartmentSlot compartment = manager.FindCompartment(slotID, mgrID);
		
		if (m_CompartmentAcessComp && m_CompartmentAcessComp.IsGettingIn())
		{
			m_aWeaponQuickSlotsStorage.Clear();
			for (int i; i < SCR_InventoryMenuUI.WEAPON_SLOTS_COUNT && i < m_aQuickSlots.Count(); i++)
				m_aWeaponQuickSlotsStorage.Insert(m_aQuickSlots[i]);
		}
		
		if (!TurretCompartmentSlot.Cast(compartment) && !m_aWeaponQuickSlotsStorage.IsEmpty())
		{
			RemoveItemsFromWeaponQuickSlots();
			
			IEntity quickSlotEntity;
			SCR_InventoryStorageManagerComponent pInventoryManager = SCR_InventoryStorageManagerComponent.Cast(GetOwner().FindComponent( SCR_InventoryStorageManagerComponent));
			
			for (int i; i < m_aWeaponQuickSlotsStorage.Count(); i++)
			{
				quickSlotEntity = m_aWeaponQuickSlotsStorage[i];
				
				if (!quickSlotEntity)
					continue;
				
				if (!pInventoryManager.Contains(quickSlotEntity))
					continue;
				
				StoreItemToQuickSlot(quickSlotEntity, i, true);
			}
			
			SCR_WeaponSwitchingBaseUI.RefreshQuickSlots();
			
			return;
		}

		array<WeaponSlotComponent> turretWeaponSlots = {};
		if (GetTurretWeaponSlots(compartment, turretWeaponSlots) > 0)
		{
			RemoveItemsFromWeaponQuickSlots();
			
			foreach (int i, WeaponSlotComponent weaponSlot: turretWeaponSlots)
			{
				IEntity weaponSlotEntity = weaponSlot.GetWeaponEntity();
				if (!weaponSlotEntity)
					continue;

				StoreItemToQuickSlot(weaponSlotEntity, i, true);
			}
		}
		
		SCR_WeaponSwitchingBaseUI.RefreshQuickSlots();
	}

	//------------------------------------------------------------------------------------------------
	//! SCR_CompartmentAccessComponent event
	protected void OnCompartmentLeft(IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		BaseCompartmentSlot compartment = manager.FindCompartment(slotID, mgrID);
		
		if (!TurretCompartmentSlot.Cast(compartment))
		{
			m_aWeaponQuickSlotsStorage.Clear();
			for (int i; i < SCR_InventoryMenuUI.WEAPON_SLOTS_COUNT && i < m_aQuickSlots.Count(); i++)
				m_aWeaponQuickSlotsStorage.Insert(m_aQuickSlots[i]);
		}
		
		if (m_CompartmentAcessComp && m_CompartmentAcessComp.IsGettingOut())
		{
			RemoveItemsFromWeaponQuickSlots();
			
			IEntity quickSlotEntity;
			SCR_InventoryStorageManagerComponent pInventoryManager = SCR_InventoryStorageManagerComponent.Cast(GetOwner().FindComponent( SCR_InventoryStorageManagerComponent));
			
			for (int i; i < m_aWeaponQuickSlotsStorage.Count(); i++)
			{
				quickSlotEntity = m_aWeaponQuickSlotsStorage[i];
				
				if (!quickSlotEntity)
					continue;
				
				if (!pInventoryManager.Contains(quickSlotEntity))
					continue;
				
				StoreItemToQuickSlot(quickSlotEntity, i, true);
			}
			
			m_aWeaponQuickSlotsStorage.Clear();
		}
		
		SCR_WeaponSwitchingBaseUI.RefreshQuickSlots();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RemoveItemsFromWeaponQuickSlots()
	{
		for (int i = 0; i < SCR_InventoryMenuUI.WEAPON_SLOTS_COUNT; i++)
			RemoveItemFromQuickSlotAtIndex(i);
	}
	
	//------------------------------------------------------------------------------------------------
	protected int GetTurretWeaponsList(BaseCompartmentSlot compartment, out array<IEntity> weaponsList)
	{
		TurretControllerComponent turretController = TurretControllerComponent.Cast(compartment.GetController());
		if (!turretController)
			return -1;
		
		BaseWeaponManagerComponent weaponManager = turretController.GetWeaponManager();
		if (weaponManager)
			return weaponManager.GetWeaponsList(weaponsList);
		
		return -1;
	}
	
	
	//------------------------------------------------------------------------------------------------
	protected int GetTurretWeaponSlots(BaseCompartmentSlot compartment, out array<WeaponSlotComponent> weaponSlots)
	{
		TurretControllerComponent turretController = TurretControllerComponent.Cast(compartment.GetController());
		if (!turretController)
			return -1;
		
		BaseWeaponManagerComponent weaponManager = turretController.GetWeaponManager();
		if (weaponManager)
			return weaponManager.GetWeaponsSlots(weaponSlots);
		
		return -1;
	}
	
	//------------------------------------------------------------------------ COMMON METHODS ----------------------------------------------------------------------
	
	#else
	float GetMaxLoad();
	BaseInventoryStorageComponent GetWeaponStorage();
	InventoryItemComponent GetItemFromLoadoutSlot( ELoadoutArea eSlot );
	BaseInventoryStorageComponent GetStorageFromLoadoutSlot( ELoadoutArea eSlot );
	protected bool HasStorageComponent( IEntity pEntity );
	void GetStorages( out notnull array<SCR_UniversalInventoryStorageComponent> storagesInInventory );
	SCR_UniversalInventoryStorageComponent GetStorageComponentFromEntity( IEntity pEntity );
	void SetLootStorage( IEntity pOwner );
	BaseInventoryStorageComponent GetLootStorage();
	//override bool CanStoreItem(IEntity item, int slotID);
	//override bool CanRemoveItem(IEntity item);
	//protected override void OnRemovedFromSlot(IEntity item, int slotID);
	//override InventoryStorageSlot GetEmptySlotForItem( IEntity item );
	//protected override void OnAddedToSlot(IEntity item, int slotID);
	#endif
};
