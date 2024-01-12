enum EMenuAction
{
	ACTION_SELECT,
	ACTION_DESELECT,
	ACTION_UNFOLD,
	ACTION_BACK,
	ACTION_MOVEBETWEEN,
	ACTION_DRAGGED,
	ACTION_DROPPED,
	ACTION_MOVEINSIDE,
	ACTION_OPENCONTAINER
};

enum EStateMenuStorage
{
	STATE_INIT = 0,
	STATE_IDLE,
	STATE_OPENED,
};

enum EStateMenuItem
{
	STATE_INIT,
	STATE_IDLE,
	STATE_MOVING_ITEM_STARTED,
};

enum EDropContainer
{
	ISINSIDE,
	ISNOTINSIDE,
	NOCONTAINER,
};

enum EAttachAction
{
	NONE,
	ATTACH,
	DETACH
};

enum EInvInitStage
{
	BEGIN = 0,
	NAVIGATION_BAR,
	QUICK_SLOTS,
	WEAPON_STORAGE,
	STORAGE_LIST,
	STORAGES,
	HIT_ZONES,
	DONE

};

//------------------------------------------------------------------------------------------------
class SCR_InvCallBack : ScriptedInventoryOperationCallback
{
	SCR_InventoryStorageBaseUI m_pStorageFrom;
	SCR_InventoryStorageBaseUI m_pStorageTo;
	SCR_InventoryStorageBaseUI m_pStorageToFocus;
	SCR_InventoryStorageManagerComponent m_pStorageMan;

	BaseInventoryStorageComponent m_pStorageToDrop;
	BaseInventoryStorageComponent m_pStorageToPickUp;

	EAttachAction m_eAttachAction = EAttachAction.NONE;
	IEntity m_pItem;
	SCR_InventoryMenuUI m_pMenu;
	ResourceName m_sItemToFocus;
	int m_iSlotToFocus;
	bool m_bShouldEquip;
	bool m_bUpdateSlotOnly;
	bool m_bShouldUpdateQuickSlots;

	void InternalComplete()
	{
		OnComplete();
	}
	//------------------------------------------------------------------------------------------------
	protected override void OnFailed()
	{
		if (m_pMenu && m_pItem && m_bShouldEquip)
		{
			m_pMenu.GetCharacterController().TryEquipRightHandItem(m_pItem, EEquipItemType.EEquipTypeWeapon, false);
			m_bShouldEquip = false;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnComplete()
	{
		if (m_pMenu && m_pItem && m_bShouldEquip)
		{
			m_pMenu.GetCharacterController().TryEquipRightHandItem(m_pItem, EEquipItemType.EEquipTypeWeapon, false);
			m_bShouldEquip = false;
		}

		if (m_eAttachAction == EAttachAction.ATTACH)
		{
			if (m_pStorageMan)
				m_pStorageMan.PlayItemSound(m_pItem, SCR_SoundEvent.SOUND_EQUIP);
		}
		else if (m_eAttachAction == EAttachAction.DETACH)
		{
			if (m_pStorageMan)
				m_pStorageMan.PlayItemSound(m_pItem, SCR_SoundEvent.SOUND_UNEQUIP);
		}

		m_eAttachAction = EAttachAction.NONE;

		bool shouldUpdateStorageList;
		if ( m_pStorageFrom )
		{
			if ( m_pMenu )
			{
				if ( m_pStorageFrom == m_pMenu.GetStorageList() )
				{
					shouldUpdateStorageList = true;
				}
				else
				{
					if (m_bUpdateSlotOnly)
					{
						m_pStorageFrom.UpdateSlotUI(m_sItemToFocus);
						m_bUpdateSlotOnly = false;
					}
					else
					{
						BaseInventoryStorageComponent storage = m_pStorageFrom.GetCurrentNavigationStorage();
						auto open = m_pMenu.GetOpenedStorage(storage);
						if (open)
							open.Refresh();
						
						if (m_pItem)
						{
							auto itemStorage = BaseInventoryStorageComponent.Cast(m_pItem.FindComponent(BaseInventoryStorageComponent));
							open = m_pMenu.GetOpenedStorage(itemStorage);
							if (open)
								open.CloseStorage();
						}

						if (m_pStorageFrom)
							m_pStorageFrom.Refresh();
					}
				}
			}
		}
		else
		{
			if ( m_pMenu )
			{
				shouldUpdateStorageList = true;
			}
		}

		if ( m_pStorageTo )
		{
			if ( m_pMenu )
			{
				if ( m_pStorageTo == m_pMenu.GetStorageList() )
				{
					shouldUpdateStorageList = true;

					auto itemStorage = BaseInventoryStorageComponent.Cast(m_pItem.FindComponent(BaseInventoryStorageComponent));
					SCR_InventoryOpenedStorageUI open = m_pMenu.GetOpenedStorage(itemStorage);
					if (open)
						open.CloseStorage();
				}
				else
				{
					m_pStorageTo.Refresh();
					BaseInventoryStorageComponent storage = m_pStorageTo.GetCurrentNavigationStorage();
					auto open = m_pMenu.GetOpenedStorage(storage);
					if (open)
						open.Refresh();
				}
			}
		}

		if (shouldUpdateStorageList)
		{
			m_pMenu.ShowStoragesList();
			m_pMenu.ShowAllStoragesInList();
		}

		if ( m_pMenu )
		{ 
			if ( m_pStorageFrom != m_pMenu.GetLootStorage() && m_pStorageTo != m_pMenu.GetLootStorage() )
				m_pMenu.RefreshLootUIListener();

			SCR_InventoryStorageBaseUI pStorage = m_pMenu.GetActualStorageInCharacterStorageUI();
			if ( pStorage )
			{
				if ( pStorage.Type() == SCR_InventoryStorageWeaponsUI ) 
					pStorage = m_pMenu.GetStorageList();
				//pStorage.Refresh();
			}

			m_pMenu.NavigationBarUpdate();
		}

		if (m_pStorageToFocus)
		{
			m_pMenu.FocusOnSlotInStorage(m_pStorageToFocus, m_iSlotToFocus);
			m_iSlotToFocus = -1;
		}
		
		if (!m_bShouldEquip && m_pStorageTo && m_pStorageTo.IsInherited(SCR_InventoryStorageLootUI))
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_CONTAINER_DIFR_DROP);

		if (m_pStorageToDrop)
		{
			if (m_pMenu && m_bShouldUpdateQuickSlots)
				m_pMenu.ShowQuickSlotStorage();
			m_pStorageToDrop = null;
		}

		if (m_pStorageToPickUp)
		{
			if (m_pMenu && m_bShouldUpdateQuickSlots)
				m_pMenu.ShowQuickSlotStorage();
			m_pStorageToPickUp = null;
		}

		m_bShouldUpdateQuickSlots = false;
	}
	
};

//------------------------------------------------------------------------------------------------
//! UI Script
//! Inventory Menu UI Layout
class SCR_InventoryMenuUI : ChimeraMenuBase
{	
	
	#ifndef DISABLE_INVENTORY
	static protected ResourceName							m_ItemPreviewManagerPrefab = "{9F18C476AB860F3B}Prefabs/World/Game/ItemPreviewManager.et";
	protected SCR_InventoryStorageManagerComponent			m_InventoryManager 		= null;
	protected SCR_CharacterInventoryStorageComponent		m_StorageManager 		= null;
	protected ChimeraCharacter								m_Player;
	protected Widget										m_wContainer;
	protected Widget										m_wStorageList, m_wGadgetStorage, m_wLootStorage, m_wOpenedStorage, m_wWeaponSlots;
	protected Widget  										m_wAttachmentStorage;
	protected Widget										m_widget;
	
	protected SCR_InventoryInspectionUI						m_InspectionScreen				= null;
	
	protected SCR_InventorySlotUI							m_pFocusedSlotUI				= null;
	protected SCR_InventorySlotUI							m_pSelectedSlotUI				= null;
	protected SCR_InventorySlotUI							m_pPrevSelectedSlotUI			= null;

	protected SCR_InventorySlotStorageUI					m_pFocusedSlotStorageUI			= null;
	protected SCR_InventorySlotStorageUI					m_pSelectedSlotStorageUI		= null;
	protected SCR_InventorySlotStorageUI					m_pSelectedSlotStoragePrevUI	= null;

	protected SCR_InventoryStorageBaseUI					m_pStorageBaseUI				= null;
	protected SCR_InventoryStoragesListUI					m_pStorageListUI				= null;
	protected SCR_InventoryStorageGadgetsUI					m_pGadgetsBaseUI				= null;
	protected SCR_InventoryStorageBaseUI					m_pStorageLootUI				= null;
	protected SCR_InventoryWeaponSlotsUI					m_pWeaponStorage				= null;
	protected EquipedWeaponStorageComponent					m_pWeaponStorageComp			= null;
	protected ref array<SCR_InventoryOpenedStorageUI> 		m_aOpenedStoragesUI				= {};
	protected SCR_InventoryAttachmentStorageUI				m_pAttachmentStorageUI			= null;

	protected SCR_InventoryItemInfoUI						m_pItemInfo						= null;
	protected SCR_InventoryDamageInfoUI						m_pDamageInfo					= null;
	protected SCR_InventoryStorageBaseUI					m_pActiveStorageUI				= null;
	protected SCR_InventoryStorageBaseUI					m_pActiveHoveredStorageUI		= null;
	protected SCR_InventoryStorageBaseUI					m_pPrevActiveStorageUI			= null;
	protected ref array<SCR_InventorySlotUI>				m_aShownStorages				= new ref array<SCR_InventorySlotUI>();	//used for storing pointers on storages which are displayed on screen ( for the refreshing purposes )
	protected GridLayoutWidget								m_wStoragesContainer;
	protected const int										STORAGE_AREA_COLUMNS			= 2;
	protected int											m_iStorageListCounter			= 0;
	protected int											m_iVicinityDiscoveryRadius		= 0;
	//protected static bool									m_bColdStart = true;			// uncomment to enable the expand / collapse feature of storages

	protected ref SCR_InventorySlotUI						m_pInspectedSlot				= null;
	protected ref SCR_InventoryGearInspectionPointUI		m_pGearInspectionPointUI		= null;
	protected Widget										m_wAttachmentContainer			= null;
	
	protected ProgressBarWidget							m_wProgressBarWeight;
	protected ProgressBarWidget							m_wProgressBarWeightItem;

	const string											SUPPLY_STORAGE_LAYOUT	= "{1DA3820E61D50EA1}UI/layouts/Menus/Inventory/SupplyInventoryContainerGrid.layout";
	const string 											BACKPACK_STORAGE_LAYOUT	= "{06E9285D68D190EF}UI/layouts/Menus/Inventory/InventoryContainerGrid.layout";
	const string											WEAPON_STORAGE_LAYOUT	= "{7B28D87B8A1ADD41}UI/layouts/Menus/Inventory/InventoryWeaponSlots.layout";
	const string 											GADGETS_STORAGE_LAYOUT 	= "{265189B87ED5CD10}UI/layouts/Menus/Inventory/InventoryGadgetsPanel.layout";
	const string 											STORAGES_LIST_LAYOUT 	= "{FC579324F5E4B3A3}UI/layouts/Menus/Inventory/InventoryCharacterGrid.layout";
	const string 											ITEM_INFO			 	= "{AE8B7B0A97BB0BA8}UI/layouts/Menus/Inventory/InventoryItemInfo.layout";
	const string 											DAMAGE_INFO			 	= "{55AFA256E1C20FB2}UI/layouts/Menus/Inventory/InventoryDamageInfo.layout";
	const string											ACTION_LAYOUT 			= "{81BB7785A3987196}UI/layouts/Menus/Inventory/InventoryActionNew.layout";
	
	const string											STORAGE_LAYOUT_CLOSE_WIDGET_NAME = "CloseStorageBtn";

	const ResourceName										HITZONE_CONTAINER_LAYOUT= "{36DB099B4CDF8FC2}UI/layouts/Menus/Inventory/Medical/HitZonePointContainer.layout";

	[Attribute("{01E150D909447632}Configs/Damage/DamageStateConfig.conf", desc: "Config to get visual data from", params: "conf class=SCR_DamageStateConfig")]
	protected ref SCR_DamageStateConfig m_DamageStateConfig;
	
	
	protected EStateMenuStorage								m_EStateMenuStorage = EStateMenuStorage.STATE_IDLE; // is this useful for anything?
	protected EStateMenuItem								m_EStateMenuItem = EStateMenuItem.STATE_IDLE;
	protected string 										m_sFSMStatesNames[10]={"init", "idle", "item selected", "storage selected", "STATE_STORAGE_OPENED", "storage unfolded", "move started", "move finished", "closing", "STATE_UNKNOWN" };
	protected SCR_NavigationBarUI							m_pNavigationBar			= null;
	protected SCR_InputButtonComponent						m_CloseButton;
	
	protected ResourceName 									m_sSupplyCostUIInfoPrefab = "{4D8296CB3CB3B8BF}Configs/Inventory/SupplyCost_ItemUIInfo.conf";
	protected ref SCR_SupplyCostItemHintUIInfo 				m_SupplyCostUIInfo;

	//variables dedicated to move an item from storage to storage
	protected IEntity 											m_pItem;
	protected BaseInventoryStorageComponent						m_pDisplayedStorage, m_pLastCurrentNavStorage;
	protected BaseInventoryStorageComponent						m_pStorageFrom, m_pStorageTo;	//last known storages from the last move operation
//	protected SCR_InventorySlotUI	 							m_pStorageUIFrom;
	protected SCR_InventorySlotStorageUI						m_pStorageUITo;	//last known storagesUI from the last move operation
	protected SCR_CharacterVicinityComponent 					m_pVicinity;
	ItemPreviewWidget 											m_wPlayerRender, m_wPlayerRenderSmall;
	PreviewRenderAttributes										m_PlayerRenderAttributes;
	protected ButtonWidget										m_wButtonShowAll;
	protected SCR_InventoryCharacterWidgetHelper 				m_pCharacterWidgetHelper;
	protected ItemPreviewManagerEntity 							m_pPreviewManager;
	protected bool												m_bDraggingEnabled;
	protected FrameWidget										m_wDragDropContainer;
	protected SCR_SlotUIComponent 								m_pDragDropFrameSlotUI;
	protected RenderTargetWidget								m_pDragDropPreviewImage;
	protected ref array<SCR_InventoryStorageBaseUI>  			m_aStorages = {};
	protected TextWidget										m_wTotalWeightText;
	protected bool												m_bLocked = false;	//helper variable

	//Item/Weapon Switching
	protected SCR_InventoryStorageQuickSlotsUI			m_pQuickSlotStorage;
	protected Widget									m_wQuickSlotStorage;
	protected ref SCR_InvCallBack						m_pCallBack = new SCR_InvCallBack();
	const int											WEAPON_SLOTS_COUNT = 4;

	protected bool												m_bIsUsingGamepad;
	protected float 											m_fX, m_fY;	//debug;
	protected bool												m_bShowIt = true;
	protected int												m_iMouseX, m_iMouseY
	const int													DRAG_THRESHOLD 			= 5;

	//other character's information
	protected SCR_CharacterControllerComponent					m_CharController;
	protected Widget											m_wCharFeatureBleeding;
	protected Widget											m_wCharFeatureWounded;
	//protected ProgressBarWidget								m_wInfoStamina;	// Preparation for showing the stamina level in inventory
	protected ref array<HitZone> 								m_aBleedingHitZones = {};

	protected bool 												m_bWasJustTraversing;
	protected bool 												m_bStorageSwitchMode;
	protected SCR_InventorySlotUI 								m_pItemToAssign;

	protected Widget											m_wAttachmentPointsContainer;
	protected ref array<SCR_InventoryHitZonePointContainerUI> 	m_aHitZonePoints = {};

	protected SCR_InventoryHitZonePointUI 						m_bleedingHandlerGlobal;
	protected SCR_InventoryHitZonePointUI 						m_fractureHandlerGlobal;
	protected SCR_CountingTimerUI		 						m_morphineTimerHandlerGlobal;
	protected SCR_CountingTimerUI		 						m_salineTimerHandlerGlobal;

	protected SCR_CharacterDamageManagerComponent				m_CharDamageManager;
	protected const int											CHARACTER_HITZONES_COUNT = 7;

	protected SCR_InventorySpinBoxComponent 					m_AttachmentSpinBox;

	protected bool 												m_bProcessInitQueue = false;
	protected EInvInitStage 									m_eInitStage = EInvInitStage.BEGIN;
	protected SCR_LoadingOverlay 								m_LoadingOverlay;

	//------------------------------------------------------------------------------------------------
	ItemPreviewManagerEntity GetItemPreviewManager()
	{
		return m_pPreviewManager;
	}

	//------------------------------------------------------------------------------------------------
	protected void InitializeCharacterHitZones()
	{
		if (m_AttachmentSpinBox)
			m_AttachmentSpinBox.ClearAll();

		Widget bleeding = m_widget.FindAnyWidget("BleedingGlobal");
		Widget fracture = m_widget.FindAnyWidget("FractureGlobal");
		Widget morphineTimer = m_widget.FindAnyWidget("MorphineTimer");
		Widget salineTimer = m_widget.FindAnyWidget("SalineTimer");

		m_CharController = SCR_CharacterControllerComponent.Cast(m_Player.GetCharacterController());
		m_CharDamageManager = SCR_CharacterDamageManagerComponent.Cast(m_Player.GetDamageManager());
		if (bleeding)
			m_bleedingHandlerGlobal = SCR_InventoryHitZonePointUI.Cast(bleeding.FindHandler(SCR_InventoryHitZonePointUI));
		
		if (fracture)
			m_fractureHandlerGlobal = SCR_InventoryHitZonePointUI.Cast(fracture.FindHandler(SCR_InventoryHitZonePointUI));
		
		if (morphineTimer)
			m_morphineTimerHandlerGlobal = SCR_CountingTimerUI.Cast(morphineTimer.FindHandler(SCR_CountingTimerUI));
		
		if (salineTimer)
			m_salineTimerHandlerGlobal = SCR_CountingTimerUI.Cast(salineTimer.FindHandler(SCR_CountingTimerUI));
	}

	protected bool InitHitZones()
	{
		int hzToInit = m_aHitZonePoints.Count();
		if (hzToInit == CHARACTER_HITZONES_COUNT)
			return true;

		Widget w = GetGame().GetWorkspace().CreateWidgets(HITZONE_CONTAINER_LAYOUT, m_wAttachmentPointsContainer);
		SCR_InventoryHitZonePointContainerUI point = SCR_InventoryHitZonePointContainerUI.Cast(w.FindHandler(SCR_InventoryHitZonePointContainerUI));
		point.InitializeHitZoneUI(m_pStorageListUI.GetStorage(), this, hzToInit + 1, m_Player);
		m_aHitZonePoints.Insert(point);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void HideCharacterHitZones()
	{
		if (m_AttachmentSpinBox)
			m_AttachmentSpinBox.ClearAll();
		if (m_bleedingHandlerGlobal)
			m_bleedingHandlerGlobal.GetRootWidget().SetVisible(false);		
		
		if (m_fractureHandlerGlobal)
			m_fractureHandlerGlobal.GetRootWidget().SetVisible(false);	
		
		if (m_morphineTimerHandlerGlobal)
			m_morphineTimerHandlerGlobal.GetRootWidget().SetVisible(false);

		if (m_salineTimerHandlerGlobal)
			m_salineTimerHandlerGlobal.GetRootWidget().SetVisible(false);

		foreach (SCR_InventoryHitZonePointContainerUI point : m_aHitZonePoints)
		{
			point.GetRootWidget().RemoveFromHierarchy();
		}
		
		m_aHitZonePoints.Clear();
	}

	//------------------------------------------------------------------------------------------------
	// !
	protected void InitializeCharacterInformation()
	{
		SCR_CharacterBloodHitZone charBloodHZ = SCR_CharacterBloodHitZone.Cast(m_CharDamageManager.GetBloodHitZone());
		charBloodHZ.GetOnDamageStateChanged().Insert(OnDamageStateChanged);

		m_CharDamageManager.GetOnDamageOverTimeRemoved().Insert(OnDamageStateChanged);
		m_CharDamageManager.GetOnDamageOverTimeAdded().Insert(OnDamageStateChanged);
		m_CharDamageManager.GetOnDamageStateChanged().Insert(OnDamageStateChanged);
		OnDamageStateChanged();
	}
	
	//------------------------------------------------------------------------------------------------
	void AddItemToAttachmentSelection(string item, Managed data = null)
	{
		if (item.IsEmpty())
			return;
		if (m_AttachmentSpinBox.FindItem(item) < 0)
			m_AttachmentSpinBox.AddItem(item, data);

		SetAttachmentSpinBoxActive(m_bIsUsingGamepad);
	}

	//------------------------------------------------------------------------------------------------
	void RemoveItemFromAttachmentSelection(string item)
	{
		int index = m_AttachmentSpinBox.FindItem(item);
		if (index > -1)
			m_AttachmentSpinBox.RemoveItem(index);
		
		SetAttachmentSpinBoxActive(m_bIsUsingGamepad);
	}

	//------------------------------------------------------------------------------------------------
	SCR_InventorySpinBoxComponent GetAttachmentSpinBoxComponent()
	{
		return m_AttachmentSpinBox;
	}

	//------------------------------------------------------------------------------------------------
	// !
	void OnDamageStateChanged()
	{
		SCR_CharacterBloodHitZone charBloodHZ = SCR_CharacterBloodHitZone.Cast(m_CharDamageManager.GetBloodHitZone());

		//TODO@FAC change this strong treshold and make better condition when effects work
		bool bleedingVisible = (charBloodHZ.GetHealthScaled() <= charBloodHZ.GetDamageStateThreshold(ECharacterBloodState.STRONG));
		float bleedingAmount = Math.InverseLerp(
			charBloodHZ.GetDamageStateThreshold(ECharacterBloodState.UNCONSCIOUS),
			charBloodHZ.GetDamageStateThreshold(ECharacterBloodState.UNDAMAGED),
			charBloodHZ.GetHealthScaled());

		if (m_bleedingHandlerGlobal)
		{
			m_bleedingHandlerGlobal.GetRootWidget().SetVisible(bleedingVisible);
			m_bleedingHandlerGlobal.SetBloodLevelProgress(bleedingAmount);
		}		
		
		if (m_fractureHandlerGlobal)
			m_fractureHandlerGlobal.GetRootWidget().SetVisible(m_CharDamageManager.GetMovementDamage() > 0);
		
		if (m_morphineTimerHandlerGlobal)
		{
			ScriptedHitZone headHZ = ScriptedHitZone.Cast(m_CharDamageManager.GetHeadHitZone());
			if (headHZ)
				m_morphineTimerHandlerGlobal.GetRootWidget().SetVisible(headHZ.GetDamageOverTime(EDamageType.HEALING) < 0);		
		}
		
		if (m_salineTimerHandlerGlobal)
			m_salineTimerHandlerGlobal.GetRootWidget().SetVisible(charBloodHZ.GetDamageOverTime(EDamageType.HEALING) < 0);
	}

	//------------------------------------------------------------------------------------------------
	// !
	void RegisterUIStorage( SCR_InventorySlotUI pStorageUI )
	{
		if ( m_aShownStorages.Find( pStorageUI ) > -1 )
			return;

		if ( SCR_InventorySlotStorageUI.Cast( pStorageUI ) || SCR_InventorySlotWeaponUI.Cast( pStorageUI ) )
			m_aShownStorages.Insert( pStorageUI );
	}

	//------------------------------------------------------------------------------------------------
	// !
	void UnregisterUIStorage( SCR_InventorySlotUI pStorageUI )
	{
		if ( pStorageUI )
			m_aShownStorages.RemoveItem( pStorageUI );
	}

	//------------------------------------------------------------------------------------------------
	void SetActiveStorage( SCR_InventoryStorageBaseUI storageUI )
	{
		m_pPrevActiveStorageUI = m_pActiveStorageUI;
		m_pActiveStorageUI = storageUI;
		if( m_pActiveStorageUI )
		{
			m_pFocusedSlotUI = m_pActiveStorageUI.GetFocusedSlot();
		}
	}

	//------------------------------------------------------------------------------------------------
	//---Get total weight from all storages (weapon, gadget, deposit)
	protected float GetTotalWeight()
	{
		if (!m_InventoryManager)
			return 0;
		
		return m_InventoryManager.GetTotalWeightOfAllStorages();
	}

	//------------------------------------------------------------------------------------------------
	SCR_InventoryStorageManagerComponent GetInventoryStorageManager()
	{
		return m_InventoryManager;
	}

	//------------------------------------------------------------------------------------------------
	//! Actualy opened UI storage in character's storage
	SCR_InventoryStorageBaseUI GetActualStorageInCharacterStorageUI() { return m_pStorageBaseUI; }
	SCR_InventoryStorageBaseUI GetStorageList() { return m_pStorageListUI; }
	SCR_InventoryStorageBaseUI GetLootStorage() { return m_pStorageLootUI; }

	//------------------------------------------------------------------------------------------------
	//! Returns whether interaction with inventory items is possible
	bool GetCanInteract()
	{
		if (GetInspectionScreen())
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshPlayerWidget()
	{
		if (!m_pPreviewManager)
			return;
		if (m_wPlayerRender)
			m_pPreviewManager.SetPreviewItem(m_wPlayerRender, m_Player, m_PlayerRenderAttributes);
		if (m_wPlayerRenderSmall)
			m_pPreviewManager.SetPreviewItem(m_wPlayerRenderSmall, m_Player, m_PlayerRenderAttributes);
	}

	protected bool ProcessInitQueue()
	{
		switch (m_eInitStage)
		{
			case EInvInitStage.BEGIN:
			{
				m_eInitStage = EInvInitStage.NAVIGATION_BAR;
			} break;

			case EInvInitStage.NAVIGATION_BAR:
			{
				m_pNavigationBar.SetAllButtonEnabled(false);
				m_pNavigationBar.FillFromConfig();
				NavigationBarUpdate();
				m_eInitStage = EInvInitStage.QUICK_SLOTS;
			} break;

			case EInvInitStage.QUICK_SLOTS:
			{
				InitQuickSlots();
				m_eInitStage = EInvInitStage.WEAPON_STORAGE;
			} break;

			case EInvInitStage.WEAPON_STORAGE:
			{
				ShowEquipedWeaponStorage();
				m_eInitStage = EInvInitStage.STORAGE_LIST;
			} break;

			case EInvInitStage.STORAGE_LIST:
			{
				ShowStoragesList();
				m_eInitStage = EInvInitStage.STORAGES;
			} break;

			case EInvInitStage.STORAGES:
			{
				ShowAllStoragesInList();
				m_eInitStage = EInvInitStage.HIT_ZONES;
			} break;

			case EInvInitStage.HIT_ZONES:
			{
				if (InitHitZones())
				{
					m_wPlayerRender.SetVisible(true);

					m_eInitStage = EInvInitStage.DONE;
				}
			} break;

			case EInvInitStage.DONE:
			{
				OnQueueProcessed();
				return true;
			}
		}

		return false;
	}

	protected void OnQueueProcessed()
	{
		m_bProcessInitQueue = false;
		m_wAttachmentPointsContainer.SetVisible(true);
		UpdateCharacterPreview();
		SetStorageSwitchMode(m_bIsUsingGamepad);
		if (m_LoadingOverlay)
			m_LoadingOverlay.GetRootWidget().RemoveFromHierarchy();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);

		if (m_bProcessInitQueue)
		{
			if (!ProcessInitQueue())
				return;
		}

		if (m_InspectionScreen)
		{
			m_InspectionScreen.UpdateView(tDelta);
			return;
		}

		if (m_pCharacterWidgetHelper && m_pPreviewManager && m_PlayerRenderAttributes && m_pCharacterWidgetHelper.Update(tDelta, m_PlayerRenderAttributes))
		{
			UpdatePreview();
		}
	}

	//------------------------------------------------------------------------------------------------
	void UpdatePreview()
	{
		if (m_pInspectedSlot)
			UpdateGearInspectionPreview();
		else
			UpdateCharacterPreview();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateCharacterPreview()
	{
		if (!m_wPlayerRender)
			return;

		m_pPreviewManager.SetPreviewItem(m_wPlayerRender, m_Player, m_PlayerRenderAttributes);

		for (int i, count = m_aHitZonePoints.Count(); i < count; i++)
		{
			SCR_InventoryHitZonePointContainerUI hp = m_aHitZonePoints[i];
			if (!hp)
				continue;
			TNodeId id = hp.GetBoneIndex();

			vector transform[4];
			Math3D.MatrixIdentity4(transform);
			float xOffset = 0;
			if (hp.GetHitZoneGroup() == ECharacterHitZoneGroup.HEAD)
				xOffset = -40;

			vector screenPos;
			if (m_wPlayerRender.TryGetItemNodePositionInWidgetSpace(id, transform, screenPos))
			{
 				FrameSlot.SetPos(hp.GetRootWidget(), screenPos[0] + xOffset, screenPos[1]);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateGearInspectionPreview()
	{	
		if (!m_pInspectedSlot)
		{
			InspectItem(null);
			return;
		}
		
		InventoryItemComponent itemComp = m_pInspectedSlot.GetInventoryItemComponent();
		
		if (!itemComp)
		{
			InspectItem(null);
			return;
		}
		
		m_pPreviewManager.SetPreviewItem(m_wPlayerRender, itemComp.GetOwner(), m_PlayerRenderAttributes, true);
		
		if (m_wPlayerRender && m_pGearInspectionPointUI)
			m_pGearInspectionPointUI.UpdatePreviewSlotWidgets(m_wPlayerRender);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		if( !Init() )
		{
			Action_CloseInventory();
			return;
		}

		GetGame().SetViewDistance(GetGame().GetMinimumViewDistance());

		ShowVicinity();

		m_bProcessInitQueue = true;

		if (m_pPreviewManager)
		{
			m_wPlayerRender = ItemPreviewWidget.Cast( m_widget.FindAnyWidget( "playerRender" ) );
			auto collection = m_StorageManager.GetAttributes();
			if (collection)
				m_PlayerRenderAttributes = PreviewRenderAttributes.Cast(collection.FindAttribute(SCR_CharacterInventoryPreviewAttributes));
			SizeLayoutWidget wPlayerRenderSmallRoot = SizeLayoutWidget.Cast( m_widget.FindAnyWidget( "playerRenderSmallRoot" ) );
			if ( wPlayerRenderSmallRoot )
				m_wPlayerRenderSmall = ItemPreviewWidget.Cast( wPlayerRenderSmallRoot.FindAnyWidget( "item" ) );

			m_pCharacterWidgetHelper = SCR_InventoryCharacterWidgetHelper(m_wPlayerRender, GetGame().GetWorkspace() );
		}

		Widget wrap = m_widget.FindAnyWidget( "WrapLayoutShow" );
		m_wButtonShowAll = ButtonWidget.Cast( wrap.FindAnyWidget( "ItemButton" ) );
		if( m_wButtonShowAll )
		{
			m_wButtonShowAll.AddHandler( new SCR_InventoryButton( EInventoryButton.BUTTON_SHOW_DEFAULT, this ) );
		}

		if( m_pNavigationBar )
			m_pNavigationBar.m_OnAction.Insert(OnAction);

		GetGame().GetInputManager().AddActionListener("Inventory_Drag", EActionTrigger.DOWN, Action_DragDown);
		GetGame().GetInputManager().AddActionListener("Inventory", EActionTrigger.DOWN, Action_CloseInventory);
		InitAttachmentSpinBox();
		OnInputDeviceIsGamepad(!GetGame().GetInputManager().IsUsingMouseAndKeyboard());
		GetGame().OnInputDeviceIsGamepadInvoker().Insert(OnInputDeviceIsGamepad);		
		
		SetAttachmentSpinBoxActive(m_bIsUsingGamepad);
		
		ResetHighlightsOnAvailableStorages();
		SetOpenStorage();
		UpdateTotalWeightText();
		
		InitializeCharacterHitZones();
		InitializeCharacterInformation();
		UpdateCharacterPreview();
	}

	void InitAttachmentSpinBox()
	{
		if (m_AttachmentSpinBox)
		{
			m_AttachmentSpinBox.ClearAll();
			return;
		}

		Widget w = m_widget.FindAnyWidget("AttachmentSpinBox");
		if (!w)
		{
			Print("Cannot find AttachmentSpinBox widget in hierarchy!", LogLevel.ERROR);
			return;
		}

		m_AttachmentSpinBox = SCR_InventorySpinBoxComponent.Cast(w.FindHandler(SCR_InventorySpinBoxComponent));
		m_AttachmentSpinBox.m_OnChanged.Insert(NavigationBarUpdateGamepad);
	}


	//------------------------------------------------------------------------------------------------
	protected void SetStorageSwitchMode(bool enabled)
	{
		m_bStorageSwitchMode = enabled;
		foreach (SCR_InventoryStorageBaseUI storage : m_aStorages)
		{
			if (storage)
				storage.ActivateStorageButton(m_bStorageSwitchMode);
		}

		m_pStorageLootUI.ActivateStorageButton(m_bStorageSwitchMode);
		if (m_pStorageListUI)
			m_pStorageListUI.ActivateStorageButton(m_bStorageSwitchMode);
		if (m_pQuickSlotStorage)
			m_pQuickSlotStorage.ActivateStorageButton(m_bStorageSwitchMode);

		foreach (SCR_InventoryOpenedStorageUI storage : m_aOpenedStoragesUI)
		{
			storage.ActivateStorageButton(m_bStorageSwitchMode);
		}

		if (m_bStorageSwitchMode)
		{
			HideItemInfo();
			Widget btnToFocus = m_pStorageLootUI.GetButtonWidget();
			if (m_pActiveStorageUI)
				btnToFocus = m_pActiveStorageUI.GetButtonWidget();
			GetGame().GetWorkspace().SetFocusedWidget(btnToFocus);
		}
		else
		{
			if (m_pActiveStorageUI)
			{
				m_pActiveStorageUI.ShowContainerBorder(false);
				FocusOnSlotInStorage(m_pActiveStorageUI);
			}
		}

		NavigationBarUpdate();
	}

	//------------------------------------------------------------------------------------------------
	protected void ToggleStorageSwitchMode()
	{
		SetStorageSwitchMode(!m_bStorageSwitchMode);
	}

	protected void SetAttachmentSpinBoxActive(bool enable)
	{
		if (!m_AttachmentSpinBox)
			return;

		int itemCount = m_AttachmentSpinBox.GetNumItems();
		bool shouldBeActive = (enable && (itemCount > 0));
		m_AttachmentSpinBox.SetEnabled(shouldBeActive);
		m_AttachmentSpinBox.GetRootWidget().SetVisible(shouldBeActive);

		Widget leftArea = m_widget.FindAnyWidget("StorageLootSlot");
		Widget rightArea = m_widget.FindAnyWidget("StoragesListSlot");
		Widget openStorages = m_widget.FindAnyWidget("OpenedStorages");

		if (shouldBeActive)
		{
			openStorages.SetNavigation(WidgetNavigationDirection.RIGHT, WidgetNavigationRuleType.EXPLICIT, "AttachmentSpinBox");
			leftArea.SetNavigation(WidgetNavigationDirection.RIGHT, WidgetNavigationRuleType.EXPLICIT, "AttachmentSpinBox");
			rightArea.SetNavigation(WidgetNavigationDirection.LEFT, WidgetNavigationRuleType.EXPLICIT, "AttachmentSpinBox");
			m_AttachmentSpinBox.GetRootWidget().SetNavigation(WidgetNavigationDirection.RIGHT, WidgetNavigationRuleType.EXPLICIT, "StoragesListSlot");
		}
		else
		{
			openStorages.SetNavigation(WidgetNavigationDirection.RIGHT, WidgetNavigationRuleType.EXPLICIT, "StoragesListSlot");
			leftArea.SetNavigation(WidgetNavigationDirection.RIGHT, WidgetNavigationRuleType.EXPLICIT, "StoragesListSlot");
			rightArea.SetNavigation(WidgetNavigationDirection.LEFT, WidgetNavigationRuleType.EXPLICIT, "StorageLootSlot");
		}
	}

	//------------------------------------------------------------------------------------------------
	bool CanFocusOnSlotInStorage(SCR_InventoryStorageBaseUI storage, int slotIndex)
	{
		if (!storage)
			return false;

		array<SCR_InventorySlotUI> slots = {};
		storage.GetSlots(slots);
		return slots.IsIndexValid(slotIndex);
	}

	//------------------------------------------------------------------------------------------------
	void FocusOnSlotInStorage(SCR_InventoryStorageBaseUI storage, int slotIndex = 0)
	{
		if (!storage)
			return;

		array<SCR_InventorySlotUI> slots = {};
		storage.GetSlots(slots);

		if (slots.IsIndexValid(slotIndex) && slots[slotIndex])
		{
			GetGame().GetWorkspace().SetFocusedWidget(slots[slotIndex].GetButtonWidget());
		}
		else
		{
			int slotToFocus = (slots.Count()-1);
			if (slots.IsIndexValid(slotToFocus) && slots[slotToFocus])
				GetGame().GetWorkspace().SetFocusedWidget(slots[slotToFocus].GetButtonWidget());
			else
				GetGame().GetWorkspace().SetFocusedWidget(null);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! returns true if the init procedure was success
	protected bool Init()
	{
		m_widget = GetRootWidget();

		if ( m_widget == null )
			return false;

		Widget loading = m_widget.FindAnyWidget("LoadingOverlay");
		if (loading)
		{
			loading.SetVisible(true);
			m_LoadingOverlay = SCR_LoadingOverlay.Cast(loading.FindHandler(SCR_LoadingOverlay));
		}

		//Get player
		PlayerController playerController = GetGame().GetPlayerController();
		if ( playerController != null )
		{
			m_Player = ChimeraCharacter.Cast(playerController.GetControlledEntity());
			if ( m_Player != null )
			{
				ChimeraWorld world = ChimeraWorld.CastFrom(m_Player.GetWorld());
				if (!world)
					return false;
				
				m_pPreviewManager = world.GetItemPreviewManager();
				if (!m_pPreviewManager)
				{
					Resource rsc = Resource.Load(m_ItemPreviewManagerPrefab);
					if (rsc.IsValid())
						GetGame().SpawnEntityPrefabLocal(rsc, world);
					
					m_pPreviewManager = world.GetItemPreviewManager();
				}

				m_CharController = SCR_CharacterControllerComponent.Cast(m_Player.GetCharacterController());
				if (m_CharController)
				{
					m_CharController.m_OnLifeStateChanged.Insert(LifeStateChanged);
					if (m_CharController.GetLifeState() != ECharacterLifeState.ALIVE)
						return false;
				}
				
				SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
				if (gameMode)
					gameMode.GetOnControllableDeleted().Insert(OnControllableDeleted);
				
				//Inventory Manager
				m_InventoryManager = SCR_InventoryStorageManagerComponent.Cast( m_Player.FindComponent( SCR_InventoryStorageManagerComponent ) );
				m_StorageManager = SCR_CharacterInventoryStorageComponent.Cast( m_Player.FindComponent( SCR_CharacterInventoryStorageComponent ) );
				if( !m_StorageManager )
					return false;
				m_pWeaponStorageComp = EquipedWeaponStorageComponent.Cast(m_StorageManager.GetWeaponStorage());
				if( !m_pWeaponStorageComp )
					return false;
				if ( m_InventoryManager )
				{
					m_InventoryManager.m_OnItemAddedInvoker.Insert(OnItemAddedListener);
					m_InventoryManager.m_OnItemRemovedInvoker.Insert(OnItemRemovedListener);
				}

				m_wProgressBarWeight = ProgressBarWidget.Cast( m_widget.FindAnyWidget( "ProgressBarWeight" ) );
				if( m_wProgressBarWeight )
				{
					m_wProgressBarWeight.SetMin( 0.0 );
					m_wProgressBarWeight.SetMax( m_StorageManager.GetMaxLoad() );
					m_wProgressBarWeight.SetCurrent( m_InventoryManager.GetTotalWeightOfAllStorages() );
				};

				m_wProgressBarWeightItem = ProgressBarWidget.Cast( m_widget.FindAnyWidget( "ProgressBarWeightItem" ) );
				if( m_wProgressBarWeightItem )
				{
					m_wProgressBarWeightItem.SetMin( 0.0 );
					m_wProgressBarWeightItem.SetMax( m_StorageManager.GetMaxLoad() );
					m_wProgressBarWeightItem.SetCurrent( 0.0 );
				};

				Widget wNaviBar = m_widget.FindAnyWidget( "Footer" );
				if( wNaviBar )
					m_pNavigationBar = SCR_NavigationBarUI.Cast( wNaviBar.FindHandler( SCR_NavigationBarUI ) );
				m_pVicinity = SCR_CharacterVicinityComponent.Cast(m_Player.FindComponent(SCR_CharacterVicinityComponent));
				if (m_pVicinity)
				{
					m_pVicinity.OnVicinityUpdateInvoker.Insert(RefreshLootUIListener);
					m_iVicinityDiscoveryRadius = m_pVicinity.GetDiscoveryRadius();
				}

				m_wStoragesContainer = GridLayoutWidget.Cast( m_widget.FindAnyWidget( "StorageGrid" ) );
				m_wTotalWeightText = TextWidget.Cast( m_widget.FindAnyWidget("TotalWeightText") );
			}
			else
			{
				return false;
			}

		}
		else
		{
			return false;
		}

		m_wDragDropContainer = FrameWidget.Cast( m_widget.FindAnyWidget( "DragDropContainer" ) );
		if ( m_wDragDropContainer )
		{
			m_pDragDropFrameSlotUI = SCR_SlotUIComponent.Cast( m_wDragDropContainer.FindHandler( SCR_SlotUIComponent ) );
			m_pDragDropPreviewImage = RenderTargetWidget.Cast( m_wDragDropContainer.FindAnyWidget( "item" ) );
		}

		m_wAttachmentPointsContainer = m_widget.FindAnyWidget("AttachmentPoints");

		return true;
	}

	//------------------------------------------------------------------------------------------------
	void OnAction( SCR_InputButtonComponent comp, string action, SCR_InventoryStorageBaseUI pParentStorage = null, int traverseStorageIndex = -1 )
	{
		switch (action)
		{
			case "Inventory":
			{
				Action_CloseInventory();
			}
			break;

			case "Inventory_UnassignFromQuickSlot":
			{
				Action_QuickSlotUnassign();
			} break;

			case "Inventory_AssignToQuickSlot":
			{
				Action_QuickSlotAssign();
			} break;

			case "Inventory_Drop":
			{
				Action_MoveBetween();
			} break;

			case "Inventory_Equip":
			{
				Action_EquipItem();
			} break;

			case "Inventory_Select":
			{
				if (m_pAttachmentStorageUI && m_AttachmentSpinBox.IsFocused())
				{
					FocusOnSlotInStorage(m_pAttachmentStorageUI);
					return;
				}

				if (m_bStorageSwitchMode && m_bIsUsingGamepad)
				{
					SetStorageSwitchMode(false);
					return;
				}
				if (m_pFocusedSlotUI && !m_pFocusedSlotUI.IsSlotSelected())
					Action_SelectItem();
				else
					Action_DeselectItem();
			} break;

			case "Inventory_Deselect":
			{
				Action_DeselectItem();
			} break;

			case "Inventory_OpenStorage":
			{
				Action_UnfoldItem();
			} break;

			case "Inventory_OpenNewStorage":
			{
				Action_OpenContainer();
			} break;

			case "Inventory_StepBack":
			{
				Action_StepBack( pParentStorage, traverseStorageIndex );
			} break;

			case "Inventory_Inspect":
			{
				Action_Inspect();
			} break;

			case "Inventory_Use":
			{
				Action_UseItem();
			} break;

			case "Inventory_Move":
			{
				if (m_bStorageSwitchMode)
				{
					Action_Drop();
					return;
				}
				Action_MoveItemToStorage();
			} break;

			case "Inventory_Swap":
			{
				if (m_bStorageSwitchMode)
				{
					ToggleStorageSwitchMode();
					FocusOnSlotInStorage(m_pActiveStorageUI);
					return;
				}
				Action_SwapItems(m_pSelectedSlotUI, m_pFocusedSlotUI);
			} break;
			
			case "Inventory_DetachItem":
			{
				SCR_InventoryHitZonePointContainerUI hzContainer = SCR_InventoryHitZonePointContainerUI.Cast(m_AttachmentSpinBox.GetCurrentItemData());
				if (hzContainer)
					hzContainer.RemoveTourniquetFromSlot();
			} break;

			case "InventoryEscape":
			{
				Action_TryCloseInventory();
			} break;
		}
	}

	//------------------------------------------------------------------------------------------------
	// ! Default view of the character's inventory
	void ShowDefault()
	{
		ShowEquipedWeaponStorage();
		if ( !m_pSelectedSlotStorageUI )
		{
			SimpleFSM( EMenuAction.ACTION_BACK );
			return;
		}
		m_pSelectedSlotStorageUI.SetSelected( false );

		if ( m_pSelectedSlotUI )
		{
			m_pSelectedSlotUI.SetSelected( false );
			m_pSelectedSlotUI = null;
		}
		if ( m_pSelectedSlotStorageUI )
			m_pSelectedSlotStorageUI = null;

		FilterOutStorages( false );
	}

	//------------------------------------------------------------------------------------------------
	protected int CloseOpenedStorage()
	{
		if ( !m_wContainer )
			return -1;	//any storage opened
		return CloseStorage( m_wContainer );
	}

	//------------------------------------------------------------------------------------------------
	protected int CloseStorage( SCR_InventoryStorageBaseUI pStorageUI )
	{		
		//if (!pStorageUI)
			//return -1;
		Widget w = pStorageUI.GetRootWidget();
		if ( !w )
			return -1;
		
		//~ Close linked storages one frame later to make sure the the current storage is closed
		CloseLinkedStorages(pStorageUI.GetStorage());
		
		return CloseStorage( w );
	}

	//------------------------------------------------------------------------------------------------
	protected int CloseStorage( notnull Widget w )
	{
		auto storageUIHandler = SCR_InventoryStorageBaseUI.Cast( w.FindHandler( SCR_InventoryStorageBaseUI ) );
		if ( !storageUIHandler )
			return -1;	//some storage opened, but it does not have any handler ( wrong )

		int iLastShownPage = storageUIHandler.GetLastShownPage();
		m_pLastCurrentNavStorage = storageUIHandler.GetCurrentNavigationStorage();
		w.RemoveHandler( storageUIHandler );	//remove the handler from the widget
		w.RemoveFromHierarchy();
		
		return iLastShownPage;
	}

	//------------------------------------------------------------------------------------------------
	// ! Hides/Unhides ( not closes ) the storage
	protected void ToggleStorageContainerVisibility( notnull SCR_InventorySlotUI pSlot )
	{
		BaseInventoryStorageComponent pStorage = pSlot.GetAsStorage();
		if ( !pStorage )
			return;
		SCR_InventoryStorageBaseUI pStorageUI = GetStorageUIByBaseStorageComponent( pStorage );
		if ( !pStorageUI )
			return;
		pStorageUI.ToggleShow();
	}

	//------------------------------------------------------------------------------------------------
	// ! Shows the content of the 1st available storage
	void ShowStorage()
	{
		if( !m_StorageManager )
			return;

		array<SCR_UniversalInventoryStorageComponent> pStorages = new array<SCR_UniversalInventoryStorageComponent>();
		m_StorageManager.GetStorages( pStorages );

		if( pStorages.Count() == 0 )
			return;
		SCR_UniversalInventoryStorageComponent pStorage = pStorages.Get( 0 );
		ShowStorage( pStorage );
	}

	//------------------------------------------------------------------------------------------------
	//!
	void ShowStorage( BaseInventoryStorageComponent storage, LoadoutAreaType area = null )
	{
		if ( !m_wStoragesContainer )
			return;
		m_wContainer =  GetGame().GetWorkspace().CreateWidgets( BACKPACK_STORAGE_LAYOUT, m_wStoragesContainer );
		int iRow = Math.Floor( m_iStorageListCounter / STORAGE_AREA_COLUMNS );
		int iCol = m_iStorageListCounter % STORAGE_AREA_COLUMNS;
		m_iStorageListCounter++;

		m_wStoragesContainer.SetRowFillWeight( iRow, 0 );
		m_wStoragesContainer.SetColumnFillWeight( iCol, 0 );

		GridSlot.SetColumn( m_wContainer, iCol );
		GridSlot.SetRow( m_wContainer, iRow );
		GridSlot.SetColumnSpan( m_wContainer, 1 );
		GridSlot.SetRowSpan( m_wContainer, 1 );

		if ( !m_wContainer )
			return;
		if ( storage.Type() == ClothNodeStorageComponent )
		{
			m_wContainer.AddHandler( new SCR_InventoryStorageLBSUI( storage, area, this, 0, null) );
			m_pStorageBaseUI = SCR_InventoryStorageBaseUI.Cast( m_wContainer.FindHandler( SCR_InventoryStorageLBSUI ) );
		}
		else if ( storage.Type() == EquipedWeaponStorageComponent )
		{
			m_wContainer.AddHandler( new SCR_InventoryStorageWeaponsUI( m_StorageManager.GetWeaponStorage(), area, this ) );
			m_pStorageBaseUI = SCR_InventoryStorageWeaponsUI.Cast( m_wContainer.FindHandler( SCR_InventoryStorageWeaponsUI ) );
		}
		else
		{
			m_wContainer.AddHandler( new SCR_InventoryStorageBackpackUI( storage, area, this, 0, null ) );
			m_pStorageBaseUI = SCR_InventoryStorageBaseUI.Cast( m_wContainer.FindHandler( SCR_InventoryStorageBaseUI ) );
		}

		m_aStorages.Insert( m_pStorageBaseUI );
	}

	//------------------------------------------------------------------------------------------------
	void ShowVicinity(bool compact = false) // if true, vicinity will have only 4 rows instead of 6
	{
		if (!m_pVicinity)
		{
			Print("No vicnity component on character!", LogLevel.DEBUG);
			return;
		}

		if ( m_wLootStorage )
		{
			m_wLootStorage.RemoveHandler( m_wLootStorage.FindHandler( SCR_InventoryStorageLootUI ) );	//remove the handler from the widget
			m_wLootStorage.RemoveFromHierarchy();
		}

		Widget parent = m_widget.FindAnyWidget( "StorageLootSlot" );
		m_wLootStorage =  GetGame().GetWorkspace().CreateWidgets( BACKPACK_STORAGE_LAYOUT, parent );
		if ( !m_wLootStorage )
			return;

		if (compact)
			m_wLootStorage.AddHandler( new SCR_InventoryStorageLootUI( null, null, this, 0, null, m_Player, 4, 6 ) );
		else
			m_wLootStorage.AddHandler( new SCR_InventoryStorageLootUI( null, null, this, 0, null, m_Player ) );
		m_pStorageLootUI = SCR_InventoryStorageBaseUI.Cast( m_wLootStorage.FindHandler( SCR_InventoryStorageLootUI ) );
	}

	//------------------------------------------------------------------------------------------------
	void ShowAttachmentStorage(InventorySearchPredicate searchPredicate, bool closeOnly = false)
	{
		if (m_wAttachmentStorage)
		{
			m_wAttachmentStorage.RemoveHandler(m_wAttachmentStorage.FindHandler(SCR_InventoryAttachmentStorageUI));
			m_wAttachmentStorage.RemoveFromHierarchy();
			if (closeOnly)
				return;
		}

		Widget parent = m_widget.FindAnyWidget("AttachmentStorage");
		if (!parent)
			return;

		m_wAttachmentStorage = GetGame().GetWorkspace().CreateWidgets(BACKPACK_STORAGE_LAYOUT, parent);
		if (!m_wAttachmentStorage)
			return;

		SCR_InventoryAttachmentStorageUI handler = new SCR_InventoryAttachmentStorageUI(m_pStorageListUI.GetStorage(), null, this, 0, null, searchPredicate);
		m_wAttachmentStorage.AddHandler(handler);
		m_pAttachmentStorageUI = handler;
	}

	//------------------------------------------------------------------------------------------------
	void CloseAttachmentStorage()
	{
		if (m_wAttachmentStorage)
		{
			m_wAttachmentStorage.RemoveHandler(m_wAttachmentStorage.FindHandler(SCR_InventoryAttachmentStorageUI));
			m_wAttachmentStorage.RemoveFromHierarchy();
		}
	}

	//------------------------------------------------------------------------------------------------
	SCR_InventoryAttachmentStorageUI GetAttachmentStorageUI()
	{
		return m_pAttachmentStorageUI;
	}

	//------------------------------------------------------------------------------------------------
	void RemoveAttachmentStorage(SCR_InventoryAttachmentStorageUI attStorage)
	{
		CloseStorage(attStorage);
	}

	//------------------------------------------------------------------------------------------------
	// shows opened storage in a new container
	void OpenStorageAsContainer(BaseInventoryStorageComponent storage, bool showVicinity = true, bool hideCloseButton = false)
	{
		foreach (SCR_InventoryOpenedStorageUI openedStorage : m_aOpenedStoragesUI)
		{
			if (openedStorage.GetStorage() == storage)
			{
				// if storage is already open, close it instead
				RemoveOpenStorage(openedStorage);
				return;
			}
		}

		SCR_InventoryOpenedStorageUI handler = CreateOpenedStorageUI(storage);

		Widget parent = m_widget.FindAnyWidget("OpenedStorages");
		Widget newStorage;

		if (SCR_InventoryStorageContainerUI.Cast(handler) || SCR_InventoryOpenedStorageArsenalUI.Cast(handler))
			newStorage = GetGame().GetWorkspace().CreateWidgets(SUPPLY_STORAGE_LAYOUT, parent);
		else
			newStorage = GetGame().GetWorkspace().CreateWidgets(BACKPACK_STORAGE_LAYOUT, parent);

		newStorage.AddHandler(handler);

		ButtonWidget closeBtn = handler.ActivateCloseStorageButton();

		ScrollLayoutWidget scroll = ScrollLayoutWidget.Cast(m_widget.FindAnyWidget("LeftAreaScroll"));

		if (scroll)
			scroll.SetSliderPos(0, 0);

		m_aOpenedStoragesUI.Insert(handler);
		
		if (showVicinity)
			ShowVicinity(true);
		
		if (hideCloseButton)
		{
			Widget closeButton = newStorage.FindAnyWidget(STORAGE_LAYOUT_CLOSE_WIDGET_NAME);
			if (closeButton)
				closeButton.SetVisible(false);
		}
		
		OpenLinkedStorages(storage, showVicinity, true);
	}

	//------------------------------------------------------------------------------------------------
	void RemoveOpenStorage(SCR_InventoryOpenedStorageUI openedStorage)
	{
		CloseStorage(openedStorage);
		m_aOpenedStoragesUI.RemoveItem(openedStorage);
		if (m_aOpenedStoragesUI.IsEmpty())
		{
			ShowVicinity();
		}
	}

	//------------------------------------------------------------------------------------------------
	void RefreshLootUIListener()
	{
		if (!m_pVicinity || !m_pStorageLootUI)
			return;
		
		m_pStorageLootUI.Refresh();
		if (m_aOpenedStoragesUI.IsEmpty())
			return;
		
		vector playerOrigin = m_Player.GetOrigin();
		vector entityBoundsMaxs, entityBoundsMins;
		SCR_InventoryOpenedStorageUI storageUI
		for (int index = m_aOpenedStoragesUI.Count() - 1; index >= 0; index--)
		{
			storageUI = m_aOpenedStoragesUI.Get(index);
			if (!storageUI)
				continue;
			
			storageUI.GetStorage().GetOwner().GetWorldBounds(entityBoundsMins, entityBoundsMaxs);
			if (!Math3D.IntersectionSphereAABB(playerOrigin, m_iVicinityDiscoveryRadius, entityBoundsMins, entityBoundsMaxs))
				RemoveOpenStorage(storageUI);
		}
	}

	//------------------------------------------------------------------------------------------------
	void ShowStoragesList()
	{
		if( m_wStorageList )
		{
			m_wStorageList.RemoveHandler( m_wStorageList.FindHandler( SCR_InventoryStoragesListUI ) );	//remove the handler from the widget
			m_wStorageList.RemoveFromHierarchy();
		}

		Widget parent = m_widget.FindAnyWidget( "StoragesListSlot" );
		m_wStorageList =  GetGame().GetWorkspace().CreateWidgets( STORAGES_LIST_LAYOUT, parent );

		if( !m_wStorageList )
			return;

		m_wStorageList.AddHandler( new SCR_InventoryStoragesListUI( m_StorageManager, null, this ) );
		m_pStorageListUI = SCR_InventoryStoragesListUI.Cast( m_wStorageList.FindHandler( SCR_InventoryStoragesListUI ) );
	}

	//------------------------------------------------------------------------------------------------
	void ShowAllStoragesInList()
	{
		m_iStorageListCounter = 0;
		array<SCR_InventorySlotUI> aSlotsUI = {};
		m_pStorageListUI.GetSlots( aSlotsUI );
		if ( !m_aStorages.IsEmpty() )
		{
			foreach ( SCR_InventoryStorageBaseUI pStorage : m_aStorages )
			{
				if ( !pStorage )
					continue;
				CloseStorage( pStorage );
			}
		}
		SortSlotsByLoadoutArea( aSlotsUI );
		m_aStorages.Resize( aSlotsUI.Count() );

		foreach ( SCR_InventorySlotUI pSlot : aSlotsUI )
		{
			if ( !pSlot )
				continue;

			BaseInventoryStorageComponent pStorage = pSlot.GetAsStorage();
			if ( pStorage )
			{
				ShowStorage( pStorage, pSlot.GetLoadoutArea() );

				/* Enable to have the expand / collapse feature
				if ( m_StorageManager.GetIsStorageShown( pStorage ) )
					m_pStorageBaseUI.Show( true );
				else
					m_pStorageBaseUI.Show( false );
				*/
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_InventoryStorageBaseUI GetStorageUIFromVicinity(BaseInventoryStorageComponent storage)
	{
		if (!storage)
			return null;
		array<SCR_InventorySlotUI> slots = {};
		m_pStorageLootUI.GetSlots(slots);
		foreach (SCR_InventorySlotUI slot : slots)
		{
			if (slot && slot.GetStorageComponent() == storage)
				return slot.GetStorageUI();
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_InventoryOpenedStorageUI CreateOpenedStorageUI(BaseInventoryStorageComponent storage)
	{
		IEntity storageOwner = storage.GetOwner();
		
		SCR_ArsenalInventoryStorageManagerComponent arsenalManagerComponent //! v
			= SCR_ArsenalInventoryStorageManagerComponent.Cast(storageOwner.FindComponent(SCR_ArsenalInventoryStorageManagerComponent));
		
		if (arsenalManagerComponent)
			return new SCR_InventoryOpenedStorageArsenalUI(storage, null, this, 0, {storage});
		
		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.FindResourceComponent(storageOwner);
		
		if (resourceComponent)
		{
			//% Remove the hard coded type for the resources type.
			SCR_ResourceContainer container = resourceComponent.GetContainer(EResourceType.SUPPLIES);
			
			if (container)
				return new SCR_InventoryStorageContainerUI(storage, null, this, 0, {storage});
		}
		
		return new SCR_InventoryOpenedStorageUI(storage, null, this, 0, {storage});
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_InventoryStorageBaseUI GetStorageUIByBaseStorageComponent( BaseInventoryStorageComponent pStorage )
	{
		if (!pStorage)
			return m_pStorageLootUI;
		
		if (pStorage == m_pWeaponStorageComp)
			return m_pWeaponStorage;
		
		BaseInventoryStorageComponent parentStorage;

		if (pStorage && pStorage.GetParentSlot())
			parentStorage = pStorage.GetParentSlot().GetStorage();

		if (ClothNodeStorageComponent.Cast(parentStorage))
		{
			foreach (SCR_InventoryStorageBaseUI pStorageUI : m_aStorages)
			{
				if (pStorageUI && pStorageUI.GetStorage() == parentStorage)
					return pStorageUI;
			}
		}

		foreach (SCR_InventoryStorageBaseUI pStorageUI : m_aStorages)
		{
			if (pStorageUI && pStorageUI.GetStorage() == pStorage)
				return pStorageUI;
		}

		foreach (SCR_InventoryStorageBaseUI pStorageUI : m_aOpenedStoragesUI)
		{
			if (pStorageUI && pStorageUI.GetStorage() == pStorage)
				return pStorageUI;
		}
		
		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected void SortSlotsByLoadoutArea( out array<SCR_InventorySlotUI> aSlots )
	{
		array<SCR_InventorySlotUI> tmpSlots = {};
		tmpSlots.Copy( aSlots );
		aSlots.Clear();

		foreach ( SCR_InventorySlotUI pSlotUI : tmpSlots )
		{
			if ( !SCR_InventorySlotStorageUI.Cast( pSlotUI ) || pSlotUI.GetLoadoutArea() == null )
				continue;
			aSlots.Insert(pSlotUI);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowGadgetStorage()
	{
		if( m_wGadgetStorage && m_wContainer )
		{
			m_wContainer.RemoveHandler( m_wGadgetStorage.FindHandler( SCR_InventoryStorageGadgetsUI ) );	//remove the handler from the widget
			m_wGadgetStorage.RemoveFromHierarchy();
		}

		Widget parent = m_widget.FindAnyWidget( "GadgetsGridSlot" );
		m_wGadgetStorage =  GetGame().GetWorkspace().CreateWidgets( GADGETS_STORAGE_LAYOUT, parent );

		if( !m_wGadgetStorage )
			return;

		//m_wGadgetStorage.AddHandler( new SCR_InventoryStorageGadgetsUI( m_StorageManager.GetGadgetsStorage(), ELoadoutArea.ELA_None, this ) );
		m_pGadgetsBaseUI = SCR_InventoryStorageGadgetsUI.Cast( m_wGadgetStorage.FindHandler( SCR_InventoryStorageGadgetsUI ) );
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowEquipedWeaponStorage()
 	{
		
		if ( m_wWeaponSlots )
		{
			m_wWeaponSlots.RemoveHandler( m_wWeaponSlots.FindHandler( SCR_InventoryWeaponSlotsUI ) );	//remove the handler from the widget
			m_wWeaponSlots.RemoveFromHierarchy();
		}

		Widget parent = m_widget.FindAnyWidget( "WeaponSlots" );
		m_wWeaponSlots = GetGame().GetWorkspace().CreateWidgets( WEAPON_STORAGE_LAYOUT, parent );

		if ( !m_wWeaponSlots )
			return;

		m_wWeaponSlots.AddHandler( new SCR_InventoryWeaponSlotsUI( m_StorageManager.GetWeaponStorage(), null, this ) );
		m_pStorageBaseUI = SCR_InventoryWeaponSlotsUI.Cast( m_wWeaponSlots.FindHandler( SCR_InventoryWeaponSlotsUI ) );
		m_pWeaponStorage = SCR_InventoryWeaponSlotsUI.Cast( m_pStorageBaseUI );
		m_pWeaponStorageComp = EquipedWeaponStorageComponent.Cast( m_pWeaponStorage.GetStorage() );
 	}

	//------------------------------------------------------------------------------------------------
	void ShowItemInfo( string sName = "", string sDescr = "", float sWeight = 0.0, SCR_InventoryUIInfo uiInfo = null )
	{
		if ( !m_pItemInfo )
		{
			//Widget parent = m_widget.FindAnyWidget( "SoldierInfo" );
			Widget infoWidget = GetGame().GetWorkspace().CreateWidgets(ITEM_INFO, m_widget);
			if ( !infoWidget )
				return;

			infoWidget.AddHandler( new SCR_InventoryItemInfoUI() );
			m_pItemInfo = SCR_InventoryItemInfoUI.Cast( infoWidget.FindHandler( SCR_InventoryItemInfoUI ) );
		}

		if( !m_pItemInfo )
			return;

		Widget w = WidgetManager.GetWidgetUnderCursor();
		if (!w)
		{
			w = m_pFocusedSlotUI.GetButtonWidget();
		}

		m_pItemInfo.Show( 0.6, w, m_bIsUsingGamepad );
		m_pItemInfo.SetName( sName );
		m_pItemInfo.SetDescription( sDescr );
		
		m_pItemInfo.SetWeight( sWeight );
		if (uiInfo && uiInfo.IsIconVisible())
			m_pItemInfo.SetIcon(uiInfo.GetIconPath(), uiInfo.GetIconColor());
		else
			m_pItemInfo.ShowIcon(false);
		
		array<SCR_InventoryItemHintUIInfo> hintsInfo = {};
		
		//~ Add hints
		if (uiInfo)
			uiInfo.GetItemHintArray(hintsInfo);
		
		//~ Arsenal supply cost hint if item is in an arsenal storage
		if (m_SupplyCostUIInfo)
		{
			SCR_ArsenalInventorySlotUI arsenalSlot = SCR_ArsenalInventorySlotUI.Cast(m_pFocusedSlotUI);
			if (arsenalSlot)
			{
				m_SupplyCostUIInfo.SetSupplyCost(arsenalSlot.GetItemSupplyCost());
				hintsInfo.InsertAt(m_SupplyCostUIInfo, 0);
			}
		}

		//~ If has hints show them
		if (!hintsInfo.IsEmpty())
			m_pItemInfo.SetItemHints(m_pFocusedSlotUI.GetInventoryItemComponent(), hintsInfo );
			
		int targetPosX, targetPosY;

		float x, y;
		w.GetScreenPos(x, y);

		if (x == 0 && y == 0)
		{
			m_pItemInfo.Hide();
			return;
		}

		float width, height;
		w.GetScreenSize(width, height);

		float screenSizeX, screenSizeY;
		GetGame().GetWorkspace().GetScreenSize(screenSizeX, screenSizeY);

		float infoWidth, infoHeight;
		m_pItemInfo.GetInfoWidget().GetScreenSize(infoWidth, infoHeight);

		targetPosX = x;
		targetPosY = y + height;

		float offsetX = (screenSizeX - infoWidth - targetPosX);
		if (offsetX < 0)
			targetPosX += offsetX;
		float offsetY = (screenSizeY - infoHeight - targetPosY);
		if (offsetY < 0)
			targetPosY += offsetY;

		m_pItemInfo.Move(
			GetGame().GetWorkspace().DPIUnscale(targetPosX), 
			GetGame().GetWorkspace().DPIUnscale(targetPosY));
	}
	
	//------------------------------------------------------------------------------------------------
	void ShowDamageInfo(string sName, SCR_InventoryDamageUIInfo damageInfo)
	{
		Widget w = WidgetManager.GetWidgetUnderCursor();
		if (!w && m_AttachmentSpinBox)
		{
			SCR_InventoryHitZonePointContainerUI currentHZContainer = SCR_InventoryHitZonePointContainerUI.Cast(m_AttachmentSpinBox.GetCurrentItemData());
			if (currentHZContainer)
				w = currentHZContainer.GetRootWidget().FindAnyWidget("HitZoneButton");
		}
		
		if (!w)
			return;
		
		if (!m_pDamageInfo)
 	 	{
		 	Widget infoWidget = GetGame().GetWorkspace().CreateWidgets(DAMAGE_INFO, m_widget);
			if ( !infoWidget )
				return;
		
			infoWidget.AddHandler( new SCR_InventoryDamageInfoUI() );
			m_pDamageInfo = SCR_InventoryDamageInfoUI.Cast( infoWidget.FindHandler( SCR_InventoryDamageInfoUI ) );
		}
	
		if (!m_pDamageInfo)
 	 		return;
		
		m_pDamageInfo.Show( 0.6, w, m_bIsUsingGamepad );
		m_pDamageInfo.SetName( sName );
		
		if (damageInfo)
		{
			m_pDamageInfo.SetDamageStateVisible(damageInfo.m_bDamageIconVisible, damageInfo.m_bDamageRegenerating, damageInfo.m_sDamageIntensity, damageInfo.m_sDamageText);
			m_pDamageInfo.SetBleedingStateVisible(damageInfo.m_bBleedingIconVisible, damageInfo.m_sBleedingText);
			m_pDamageInfo.SetTourniquetStateVisible(damageInfo.m_bTourniquetIconVisible);
			m_pDamageInfo.SetSalineBagStateVisible(damageInfo.m_bSalineBagIconVisible);
			m_pDamageInfo.SetFractureStateVisible(damageInfo.m_bFractureIconVisible, damageInfo.m_bFractureIcon2Visible);
		}

		int iMouseX, iMouseY;

		float x, y;
		w.GetScreenPos(x, y);

		float width, height;
		w.GetScreenSize(width, height);

		float screenSizeX, screenSizeY;
		GetGame().GetWorkspace().GetScreenSize(screenSizeX, screenSizeY);

		float infoWidth, infoHeight;
		m_pDamageInfo.GetInfoWidget().GetScreenSize(infoWidth, infoHeight);

		iMouseX = x;
		iMouseY = y + height;
		if (x + infoWidth > screenSizeX)
			iMouseX = screenSizeX - infoWidth - width * 0.5; // offset info if it would go outside of the screen

		m_pDamageInfo.Move( GetGame().GetWorkspace().DPIUnscale( iMouseX ), GetGame().GetWorkspace().DPIUnscale( iMouseY ) );
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool GetDamageInfo()
	{
 	 	Widget infoWidget = GetGame().GetWorkspace().CreateWidgets(DAMAGE_INFO, m_widget);
		if ( !infoWidget )
			return false;

		infoWidget.AddHandler( new SCR_InventoryDamageInfoUI() );
		m_pDamageInfo = SCR_InventoryDamageInfoUI.Cast( infoWidget.FindHandler( SCR_InventoryDamageInfoUI ) );
		
		return m_pDamageInfo;
	}

	//------------------------------------------------------------------------------------------------
	void HideItemInfo()
	{
		if ( !m_pItemInfo )
			return;
		m_pItemInfo.Hide();
	}
	
	//------------------------------------------------------------------------------------------------
	void HideDamageInfo()
	{
		if ( !m_pDamageInfo )
			return;
		m_pDamageInfo.Hide();
	}

	//------------------------------------------------------------------------------------------------
	void SetSlotFocused( SCR_InventorySlotUI pFocusedSlot, SCR_InventoryStorageBaseUI pFromStorageUI, bool bFocused )
	{
		if( bFocused )
		{
			InputManager pInputManager = GetGame().GetInputManager();
			if ( !( pInputManager && pInputManager.IsUsingMouseAndKeyboard() ) )
			{
				if ( m_pActiveStorageUI != pFromStorageUI )
				{
					if ( m_pActiveStorageUI )
					{
						SCR_InventorySlotUI pLastFocusedSlot = pFromStorageUI.GetLastFocusedSlot();
						pFromStorageUI.SetSlotFocused( pLastFocusedSlot );
						if ( pLastFocusedSlot )
						{
							m_bLocked = true;
							GetGame().GetWorkspace().SetFocusedWidget( pLastFocusedSlot.GetButtonWidget(), true );
							m_bLocked = false;
						}
					}

					m_pActiveStorageUI = pFromStorageUI;
				}
			}

			m_pFocusedSlotUI = pFocusedSlot;
			SetFocusedSlotEffects();
		}
		else
		{
			if ( !m_bLocked )
			{
				if ( m_pActiveStorageUI )
					m_pActiveStorageUI.SetLastFocusedSlot( pFocusedSlot	 );

				HideItemInfo();
				m_pFocusedSlotUI = null;
				NavigationBarUpdate();
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SetFocusedSlotEffects()
	{
		if( !m_pFocusedSlotUI )
		{
			if( m_wProgressBarWeightItem )
				m_wProgressBarWeightItem.SetCurrent( 0.0 );
			return;
		}

		//show info about the item
		InventoryItemComponent invItemComp = m_pFocusedSlotUI.GetInventoryItemComponent();
		if ( !invItemComp )
			return;
		auto attribs = SCR_ItemAttributeCollection.Cast( invItemComp.GetAttributes() );

		if ( !attribs )
			return;
		UIInfo itemInfo = attribs.GetUIInfo();
		if ( !itemInfo )
			HideItemInfo();
		else
		{
			SCR_InventoryUIInfo inventoryInfo = SCR_InventoryUIInfo.Cast(itemInfo);
			
			if (inventoryInfo)
				ShowItemInfo( inventoryInfo.GetInventoryItemName(invItemComp), inventoryInfo.GetInventoryItemDescription(invItemComp), invItemComp.GetTotalWeight(), inventoryInfo);
			else 
				ShowItemInfo( itemInfo.GetName(), itemInfo.GetDescription(), invItemComp.GetTotalWeight(), null);
		}
	
		//show the weight on the progressbar
		//TODO: overlap or add on the end, depending on if the item is already in the storage or is going to be added
		if( m_wProgressBarWeightItem )
		{
			float weight = invItemComp.GetTotalWeight();
			m_wProgressBarWeightItem.SetCurrent( weight );
		};

		NavigationBarUpdate();
	}

	//------------------------------------------------------------------------------------------------
	//!
	protected EDropContainer IsFocusedItemInsideDropContainer()
	{
		if ( !m_pActiveHoveredStorageUI )
			return EDropContainer.NOCONTAINER;
		if ( m_pActiveHoveredStorageUI.GetFocusedSlot() == m_pFocusedSlotUI )
			return EDropContainer.ISINSIDE;

		return EDropContainer.ISNOTINSIDE;
	}

	//------------------------------------------------------------------------------------------------
	protected bool CanSwapItems(SCR_InventorySlotUI slot1, SCR_InventorySlotUI slot2)
	{
		if (!slot1 || !slot2)
			return false;

		return m_InventoryManager.CanSwapItemStorages(
			slot1.GetInventoryItemComponent().GetOwner(),
			slot2.GetInventoryItemComponent().GetOwner()
		);
	}

	//------------------------------------------------------------------------------------------------
	protected bool CanMoveItem(SCR_InventorySlotUI slot1, SCR_InventorySlotUI slot2)
	{
		if (!slot1 || !slot2)
			return false;

		return m_InventoryManager.CanMoveItemToStorage(
			slot1.GetInventoryItemComponent().GetOwner(),
			slot2.GetStorageComponent()
		);
	}

	//------------------------------------------------------------------------------------------------
	protected bool CanMoveItem(SCR_InventorySlotUI slot1, SCR_InventoryStorageBaseUI slot2)
	{
		if (!slot1 || !slot2)
			return false;

		return m_InventoryManager.CanMoveItemToStorage(
			slot1.GetInventoryItemComponent().GetOwner(),
			slot2.GetStorage()
		);
	}

	//------------------------------------------------------------------------------------------------
	void NavigationBarUpdate()
	{
		if (!m_pNavigationBar)
			return;

		if (m_bIsUsingGamepad)
		{
			NavigationBarUpdateGamepad();
			return;
		}

		m_pNavigationBar.SetAllButtonEnabled( false );
		m_pNavigationBar.SetButtonEnabled( "ButtonBack", true );

		SCR_InventoryHitZoneUI hzSlot = SCR_InventoryHitZoneUI.Cast(m_pActiveHoveredStorageUI);
		m_pNavigationBar.SetButtonEnabled("ButtonRemoveTourniquet", (hzSlot && hzSlot.IsTourniquetted()));			

		if ( !m_pFocusedSlotUI )
			return;
		if ( m_pFocusedSlotUI.GetStorageUI() == m_pQuickSlotStorage )
		{
			if (m_pFocusedSlotUI.GetInventoryItemComponent())
				m_pNavigationBar.SetButtonEnabled("ButtonQuickSlotUnassign", true);
			return;
		}
		InventoryItemComponent itemComp = m_pFocusedSlotUI.GetInventoryItemComponent();
		bool arsenalItem = IsStorageArsenal(m_pFocusedSlotUI.GetStorageUI().GetCurrentNavigationStorage());
		if (itemComp && itemComp.GetOwner() && !arsenalItem)
			m_pNavigationBar.SetButtonEnabled("ButtonInspect", (itemComp.GetOwner().FindComponent(SCR_WeaponAttachmentsStorageComponent) != null));
		
		m_pNavigationBar.SetButtonEnabled( "ButtonSelect", true );
		m_pNavigationBar.SetButtonEnabled( "ButtonDrop", m_pFocusedSlotUI.IsDraggable() );

		if (itemComp)
		{	
			SCR_ConsumableItemComponent consumableComp = SCR_ConsumableItemComponent.Cast(itemComp.GetOwner().FindComponent(SCR_ConsumableItemComponent));
			if (consumableComp && consumableComp.GetConsumableEffect() && consumableComp.GetConsumableEffect().CanApplyEffect(m_Player, m_Player))
				m_pNavigationBar.SetButtonEnabled("ButtonUse");
		}

		bool flag = m_pFocusedSlotUI.GetStorageUI() == m_pStorageLootUI;
		bool isArsenal = IsStorageArsenal(m_pStorageLootUI.GetCurrentNavigationStorage());
		
		if (m_aOpenedStoragesUI.Contains(SCR_InventoryOpenedStorageUI.Cast(m_pFocusedSlotUI.GetStorageUI())))
		{
			isArsenal = false;
			flag = false;
		}
		
		if (isArsenal)
		{
			m_pNavigationBar.SetButtonEnabled("ButtonBuy", flag);
			m_pNavigationBar.SetButtonEnabled("ButtonSell", !flag);
			m_pNavigationBar.SetButtonEnabled("ButtonPickup", false);
			m_pNavigationBar.SetButtonEnabled("ButtonDrop", false);
		}
		else
		{
			m_pNavigationBar.SetButtonEnabled("ButtonBuy", false);
			m_pNavigationBar.SetButtonEnabled("ButtonSell", false);
			m_pNavigationBar.SetButtonEnabled("ButtonPickup", flag);
			m_pNavigationBar.SetButtonEnabled("ButtonDrop", !flag);
		}
		
		m_pNavigationBar.SetButtonEnabled( "ButtonStepBack", true );
	
		HandleSlottedItemFunction();
	}

	//------------------------------------------------------------------------------------------------
	void HandleSlottedItemFunction()
	{
		string sAction = "#AR-Inventory_Select";
		bool arsenalItem = IsStorageArsenal(m_pFocusedSlotUI.GetStorageUI().GetCurrentNavigationStorage()); // hotfix for disabling opening open action in arsenal storages

		switch ( m_pFocusedSlotUI.GetSlotedItemFunction() )
		{
			case ESlotFunction.TYPE_GADGET:
				// m_pNavigationBar.SetButtonEnabled( "ButtonEquip", true );
				break;
			case ESlotFunction.TYPE_WEAPON:
				m_pNavigationBar.SetButtonEnabled( "ButtonEquip", true );

				InventoryItemComponent itemComp = m_pFocusedSlotUI.GetInventoryItemComponent();

				if (!itemComp)
				 	return;
				
				IEntity item = itemComp.GetOwner();

				if (item)
				{
					WeaponComponent weaponComp = WeaponComponent.Cast(item.FindComponent(WeaponComponent));

					if (weaponComp &&
						weaponComp.GetWeaponType() != EWeaponType.WT_FRAGGRENADE &&
						weaponComp.GetWeaponType() != EWeaponType.WT_SMOKEGRENADE &&
						weaponComp.Type() != SCR_MineWeaponComponent)
					{
						m_pNavigationBar.SetButtonEnabled( "ButtonOpenStorage", !arsenalItem );
						m_pNavigationBar.SetButtonEnabled( "ButtonOpenAsContainer", !arsenalItem );
					}
				}

				break;
			case ESlotFunction.TYPE_MAGAZINE:
				// TODO: show the Reload action
				//m_pNavigationBar.SetButtonEnabled( "ButtonUse", true );
				break;
			case ESlotFunction.TYPE_CONSUMABLE:
				// TODO: show the Consume action
				m_pNavigationBar.SetButtonEnabled( "ButtonUse", true );

				break;
			case ESlotFunction.TYPE_STORAGE:
				if( m_EStateMenuItem == EStateMenuItem.STATE_MOVING_ITEM_STARTED && m_pFocusedSlotUI != m_pSelectedSlotUI )
				{
					sAction = "#AR-Inventory_Move";
					//m_pNavigationBar.SetButtonEnabled( "ButtonSelect", false );
					//m_pNavigationBar.SetButtonEnabled( "ButtonMove", true );
				}
				// Enable in case the storage is not "togglable" - can be only shown and only opening another storage will close it
				/*else if ( m_EStateMenuStorage == EStateMenuStorage.STATE_OPENED && m_pFocusedSlotUI == m_pSelectedSlotUI && m_pFocusedSlotUI.Type() != SCR_InventorySlotStorageEmbeddedUI)
				{
					m_pNavigationBar.SetButtonEnabled( "ButtonSelect", false );
				}*/
				else if ( m_pFocusedSlotUI.Type() == SCR_InventorySlotStorageEmbeddedUI || m_pFocusedSlotUI.Type() == SCR_SupplyInventorySlotUI)
				{
					m_pNavigationBar.SetButtonEnabled( "ButtonOpenStorage", !arsenalItem );
					m_pNavigationBar.SetButtonEnabled( "ButtonOpenAsContainer", !arsenalItem );
				}
				
				break;

			case ESlotFunction.TYPE_HEALTH:
				// TODO: show the Heal action
				m_pNavigationBar.SetButtonEnabled( "ButtonUse", true );
				
				break;
		}

		HandleSelectButtonState( sAction );
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateItemSplitDialog(int maxVal, IEntity entityTo, IEntity entityFrom)
	{
		SCR_ItemSplitDialog dialog = SCR_ItemSplitDialog.Create(maxVal, entityTo, entityFrom);

		dialog.m_OnConfirm.Insert(OnItemSplitDialogConfirm);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnItemSplitDialogConfirm(SCR_ItemSplitDialog dialog)
	{		
		if (OnItemSplitDialogConfirm_Merge(dialog))
			return;
		
		if (OnItemSplitDialogConfirm_Create(dialog))
			return;
	}

	//------------------------------------------------------------------------------------------------
	protected bool OnItemSplitDialogConfirm_Merge(inout notnull SCR_ItemSplitDialog dialog)
	{
		SCR_ResourcePlayerControllerInventoryComponent resourceInventoryComp = SCR_ResourcePlayerControllerInventoryComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_ResourcePlayerControllerInventoryComponent));
		
		if (!resourceInventoryComp)
			return false;
		
		IEntity entityFrom = dialog.GetEntityFrom();
		IEntity entityTo = dialog.GetEntityTo();
		
		SCR_ResourceComponent resourceComponentFrom = SCR_ResourceComponent.FindResourceComponent(entityFrom);
		
		if (!resourceComponentFrom)
			return false;
		
		if (!entityTo)
			return false;
		
		SCR_ResourceComponent resourceComponentTo = SCR_ResourceComponent.FindResourceComponent(entityTo);
		
		if (!resourceComponentTo)
			return false;
		
		resourceInventoryComp.Rpc(resourceInventoryComp.RpcAsk_MergeContainerWithContainerPartial, Replication.FindId(resourceComponentFrom), Replication.FindId(resourceComponentTo), EResourceType.SUPPLIES, dialog.GetSliderValue());
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool OnItemSplitDialogConfirm_Create(inout notnull SCR_ItemSplitDialog dialog)
	{
		SCR_ResourcePlayerControllerInventoryComponent resourceInventoryComp = SCR_ResourcePlayerControllerInventoryComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_ResourcePlayerControllerInventoryComponent));
		
		if (!resourceInventoryComp)
			return false;
		
		IEntity entityFrom = dialog.GetEntityFrom();
		IEntity entityTo = dialog.GetEntityTo();
		
		SCR_ResourceComponent resourceComponentFrom = SCR_ResourceComponent.FindResourceComponent(entityFrom);
		
		if (!resourceComponentFrom)
			return false;
		
		if (!entityTo)
		{
			resourceInventoryComp.Rpc(resourceInventoryComp.RpcAsk_CreatePhysicalContainerWithContainer, Replication.FindId(resourceComponentFrom), Replication.FindId(null), Replication.FindId(null), EResourceType.SUPPLIES, dialog.GetSliderValue());
			
			return true;
		}
		
		SCR_ResourceComponent resourceComponentTo = SCR_ResourceComponent.FindResourceComponent(entityTo);
		
		//! Failsafe for not processing the same case needed in for merging containers.
		if (resourceComponentTo)
			return false;
		
		SCR_InventoryStorageManagerComponent invManagerTo;
		IEntity entityParentTo = entityTo.GetParent();
		
		if (entityParentTo)
			invManagerTo = SCR_InventoryStorageManagerComponent.Cast(entityParentTo.FindComponent(SCR_InventoryStorageManagerComponent));
		
		if (!invManagerTo)
			return false;
		
		BaseInventoryStorageComponent storageTo = BaseInventoryStorageComponent.Cast(entityTo.FindComponent(BaseInventoryStorageComponent));
		
		if (!storageTo)
			return false;
		
		resourceInventoryComp.Rpc(resourceInventoryComp.RpcAsk_CreatePhysicalContainerWithContainer, Replication.FindId(resourceComponentFrom), Replication.FindId(invManagerTo), Replication.FindId(storageTo), EResourceType.SUPPLIES, dialog.GetSliderValue());
		
		return true;
	}



	//------------------------------------------------------------------------------------------------
	void NavigationBarUpdateGamepad()
	{
		m_pNavigationBar.SetAllButtonEnabled(false);
		m_pNavigationBar.SetButtonEnabled("ButtonBack", true);
		m_pNavigationBar.SetButtonEnabled("ButtonSelect", true);

		SCR_InventoryHitZoneUI hzSlot = m_AttachmentSpinBox.GetFocusedHZPoint();

		m_pNavigationBar.SetButtonEnabled("ButtonRemoveTourniquet",
			(hzSlot && hzSlot.IsTourniquetted()) &&
			m_AttachmentSpinBox.IsFocused()
		);		
		
		if (m_pActiveStorageUI == m_pAttachmentStorageUI)
		{
			m_pNavigationBar.SetButtonEnabled("ButtonUse", true);
			return;
		}

		if (m_bStorageSwitchMode)
		{
			m_pNavigationBar.SetButtonActionName("ButtonBack", "#AR-Inventory_Close");
			bool shouldShowMove = (m_pSelectedSlotUI != null) && m_pSelectedSlotUI.IsDraggable();
			if (m_pActiveStorageUI)
				shouldShowMove &= m_pActiveStorageUI.IsStorageHighlighted();
			m_pNavigationBar.SetButtonEnabled("ButtonMove", shouldShowMove);
			m_pNavigationBar.SetButtonEnabled("ButtonSelect", !m_pSelectedSlotUI);
		}
		else
		{
			m_pNavigationBar.SetButtonEnabled("ButtonMove", m_pSelectedSlotUI != null);
			m_pNavigationBar.SetButtonEnabled("ButtonSwap", m_pSelectedSlotUI != null);
		}

		if (!m_bStorageSwitchMode &&
			m_pActiveStorageUI != m_pStorageLootUI &&
			m_pActiveStorageUI != m_pStorageListUI)
		{
			m_pNavigationBar.SetButtonEnabled("ButtonQuickSlotAssign", true);
		}

		bool isQuickSlotStorage = (m_pActiveStorageUI == m_pQuickSlotStorage);
		if (isQuickSlotStorage)
		{
			bool itmToAssign = m_pItemToAssign != null;
			m_pNavigationBar.SetAllButtonEnabled(false);
			m_pNavigationBar.SetButtonEnabled("ButtonClose", true);
			if (m_bStorageSwitchMode)
				m_pNavigationBar.SetButtonEnabled("ButtonSelect", true);

			m_pNavigationBar.SetButtonEnabled("ButtonQuickSlotAssign", itmToAssign);
			m_pNavigationBar.SetButtonEnabled("ButtonQuickSlotUnassign",
				!itmToAssign &&
				m_pFocusedSlotUI != null
			);
		}

		if (!m_pFocusedSlotUI)
			return;

		InventoryItemComponent itemComp = m_pFocusedSlotUI.GetInventoryItemComponent();
		bool arsenalItem = IsStorageArsenal(m_pFocusedSlotUI.GetStorageUI().GetCurrentNavigationStorage());
		if (itemComp && itemComp.GetOwner() && !arsenalItem)
			m_pNavigationBar.SetButtonEnabled("ButtonInspect", (itemComp.GetOwner().FindComponent(SCR_WeaponAttachmentsStorageComponent) != null));

		m_pNavigationBar.SetButtonActionName("ButtonBack", "#AR-Menu_Back");

		m_pNavigationBar.SetButtonEnabled("ButtonDrop",
			(m_pFocusedSlotUI != null) &&
			!isQuickSlotStorage &&
			m_pFocusedSlotUI.IsDraggable() &&
			!m_AttachmentSpinBox.IsFocused()
		);

		bool flag = m_pFocusedSlotUI.GetStorageUI() == m_pStorageLootUI;
		m_pNavigationBar.SetButtonEnabled("ButtonPickup", flag);

		if (!isQuickSlotStorage)
		{
			m_pNavigationBar.SetButtonEnabled("ButtonDrop", !flag);
			HandleSlottedItemFunction();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void HandleSelectButtonState( string sAction = "#AR-Inventory_Select" )
	{
		//TODO: this can be done better
		if ( sAction == "#AR-Inventory_Move" )
			m_pNavigationBar.SetButtonActionName( "ButtonSelect", sAction );
		else
		{
			if ( !m_pFocusedSlotUI.IsSlotSelected() )
				m_pNavigationBar.SetButtonActionName( "ButtonSelect", sAction );
			else
				m_pNavigationBar.SetButtonActionName( "ButtonSelect", "#AR-Inventory_Deselect" );
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the inspection screen UI
	SCR_InventoryInspectionUI GetInspectionScreen()
	{
		return m_InspectionScreen;
	}

	//------------------------------------------------------------------------------------------------
	//! Opens the inspection screen for the item slot
	void InspectItem(SCR_InventorySlotUI itemSlot)
	{
		if (m_pGearInspectionPointUI)
		{	
			m_pGearInspectionPointUI.ClearSlots();
			m_wAttachmentContainer.RemoveHandler(m_pGearInspectionPointUI);
			m_pGearInspectionPointUI = null;
			m_pInspectedSlot = null;
		}

		if (!itemSlot)
		{
			CloseAttachmentStorage();
			SetAttachmentSpinBoxActive(m_bIsUsingGamepad);
			m_PlayerRenderAttributes.RotateItemCamera(Vector(0, 0, 0), Vector(0, 0, 0), Vector(0, 0, 0)); // reset rotation
			InitializeCharacterHitZones();
			UpdateCharacterPreview();
			return;
		}
		
		InventoryItemComponent itemComp = itemSlot.GetInventoryItemComponent();
		
		if (!itemComp)
		{
			InitializeCharacterHitZones();
			CloseAttachmentStorage();
			SetAttachmentSpinBoxActive(m_bIsUsingGamepad);
			UpdateCharacterPreview(); 
			return;
		}
		
		IEntity item = itemComp.GetOwner();
		
		SCR_WeaponAttachmentsStorageComponent weaponAttachmentStorage = SCR_WeaponAttachmentsStorageComponent.Cast(item.FindComponent(SCR_WeaponAttachmentsStorageComponent));
		
		if (weaponAttachmentStorage)
		{
			m_PlayerRenderAttributes.RotateItemCamera(Vector(0, 90, 0), Vector(0, 90, 0), Vector(0, 90, 0)); // rotate inspected weapon to side view
			m_pInspectedSlot = itemSlot;
			InspectWeapon(weaponAttachmentStorage);
			UpdateGearInspectionPreview();
			return;	
		}
		
		/*	Preparations for gear inspection
		ClothNodeStorageComponent clothNodeStorage = ClothNodeStorageComponent.Cast(item.FindComponent(ClothNodeStorageComponent));
		
		if (clothNodeStorage)
		{
			m_pInspectedSlot = itemSlot;
			InspectGear(clothNodeStorage);
			UpdateGearInspectionPreview();
			return;
		}*/
	}
	
	//------------------------------------------------------------------------------------------------
	void InspectWeapon(SCR_WeaponAttachmentsStorageComponent weaponAttachmentStorage)
	{
		CloseAttachmentStorage();
		HideCharacterHitZones();
		SetAttachmentSpinBoxActive(m_bIsUsingGamepad);
		Widget inspectionFrame = m_widget.FindAnyWidget("AttachmentPoints");
		m_wAttachmentContainer = GetGame().GetWorkspace().CreateWidgets(BACKPACK_STORAGE_LAYOUT, inspectionFrame);
		m_pGearInspectionPointUI = new SCR_InventoryGearInspectionPointUI(weaponAttachmentStorage, null, this, frameSlot: inspectionFrame);
		m_wAttachmentContainer.AddHandler(m_pGearInspectionPointUI);
	}
	
	/*	Preparations for gear inspection
	void InspectGear(ClothNodeStorageComponent clothNodeStorage)
	{
		Widget inspectionFrame = m_widget.FindAnyWidget("AttachmentPoints");
		m_wAttachmentContainer = GetGame().GetWorkspace().CreateWidgets(BACKPACK_STORAGE_LAYOUT, inspectionFrame);
		m_pGearInspectionPointUI = new SCR_InventoryGearInspectionPointUI(clothNodeStorage, null, this, frameSlot: inspectionFrame);
		m_wAttachmentContainer.AddHandler(m_pGearInspectionPointUI);
	}*/
	
	SCR_InventoryGearInspectionPointUI GetGearInspectionUI()
	{
		return m_pGearInspectionPointUI;
	}

	//------------------------------------------------------------------------------------------------
	//! shows only the storages the item can be stored into
	protected void FilterOutStorages( bool bShow = true )
	{
		//Get all slots from the storage list UI
		array<SCR_InventorySlotUI> pSlotsInListUI = new array<SCR_InventorySlotUI>();
		m_pStorageListUI.GetSlots( pSlotsInListUI );
		//foreach ( SCR_InventorySlotUI pSlotFromUI : pSlotsInListUI )
		//	RegisterUIStorage( pSlotFromUI );

		if ( m_pStorageLootUI )
			m_pStorageLootUI.GetSlots( pSlotsInListUI );
		if ( GetActualStorageInCharacterStorageUI() )
			GetActualStorageInCharacterStorageUI().GetSlots( pSlotsInListUI );
		if ( m_pQuickSlotStorage )
			m_pQuickSlotStorage.GetSlots( pSlotsInListUI );
		if ( m_pWeaponStorage )
			m_pWeaponStorage.GetSlots( pSlotsInListUI );

		BaseInventoryStorageComponent pStorageTo;
		foreach ( SCR_InventorySlotUI pStorageSlotUI : pSlotsInListUI )
		{
			if ( !pStorageSlotUI )
				continue;

			pStorageTo = pStorageSlotUI.GetAsStorage();

			if ( bShow )
			{
				if ( !m_pSelectedSlotUI )
					continue;
				if ( pStorageSlotUI.GetAsStorage() == m_pSelectedSlotUI.GetStorageUI().GetCurrentNavigationStorage() )	//it's the same storage as the selected item comes from
				{
					pStorageSlotUI.SetEnabledForMove( 0 );
					continue;
				}

				InventoryItemComponent pInventoryItem = m_pSelectedSlotUI.GetInventoryItemComponent();
				if ( !pInventoryItem )
					continue;

				IEntity pItem = pInventoryItem.GetOwner();

				if ( pStorageTo )
				{
					bool bCanStore = true;

					if ( m_InventoryManager.CanInsertItemInActualStorage( pItem, pStorageTo ) )
					{
						if ( pStorageTo.Type() == SCR_InventoryStorageWeaponsUI )
							if ( !m_pWeaponStorageComp.Contains( pStorageTo.GetOwner() ) )
								bCanStore = false;

						if ( bCanStore )
							pStorageSlotUI.SetEnabledForMove( 1 );
						else
							pStorageSlotUI.SetEnabledForMove( 0 );
					}
					else
					{
						pStorageSlotUI.SetEnabledForMove( 0 );
					}
				}
				else
				{
					pStorageSlotUI.SetEnabledForMove( 0 );
				}

			}
			else
			{
				pStorageSlotUI.SetEnabledForMove( 2 );
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void FilterOutItems( bool bFilterOut )
	{
		array<SCR_InventorySlotUI> aItemsUIInLoot = {};

		//if ( m_pStorageLootUI )
			//aItemsUIInLoot.InsertAll( m_pStorageLootUI.GetUISlots() );
		if ( m_pStorageListUI )
			aItemsUIInLoot.InsertAll( m_pStorageListUI.GetUISlots() );
		//if ( m_pStorageBaseUI )
			//aItemsUIInLoot.InsertAll( m_pStorageBaseUI.GetUISlots() );

		PrintFormat( "INV: Filtering out items %1", 1.5 );

		foreach ( SCR_InventorySlotUI pSlot : aItemsUIInLoot )
		{
			if ( !pSlot.GetStorageComponent() )
			{
				if ( bFilterOut )
				{
					PrintFormat( "INV: Disabling slot %1", pSlot );
					pSlot.SetEnabledForMove( 0 );
				}
				else
				{
					PrintFormat( "INV: Reseting slot %1", pSlot );
					pSlot.SetEnabledForMove( 2 );
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Action_TryCloseInventory()
	{
		if (m_bWasJustTraversing)
		{
			m_bWasJustTraversing = false;
			return;
		}

		if (m_bIsUsingGamepad)
		{
			if (!m_bStorageSwitchMode)
			{
				SetStorageSwitchMode(true);
				return;
			}
			else
			{
				if (m_pSelectedSlotUI)
				{
					DeselectSlot();				
				}
				else
				{
					Action_CloseInventory();
					return;
				}

				SetStorageSwitchMode(false);
				ResetHighlightsOnAvailableStorages();
				FocusOnSlotInStorage(m_pActiveStorageUI);
				return;
			}
		}
		else
		{
			DeselectSlot();
		}

		Action_CloseInventory();
	}

	//------------------------------------------------------------------------------------------------
	protected void Action_CloseInventory()
	{
		array<BaseInventoryStorageComponent> traverseStorage = {};
		if (m_wLootStorage)
		{
			SCR_InventoryStorageBaseUI storageUIHandler = SCR_InventoryStorageBaseUI.Cast( m_wLootStorage.FindHandler( SCR_InventoryStorageBaseUI ) );
			storageUIHandler.GetTraverseStorage(traverseStorage);
		}

		if (!traverseStorage.IsEmpty())
		{
			BaseInventoryStorageComponent storage = traverseStorage[0];
			if (storage)
			{
				IEntity entity = storage.GetOwner();
				m_InventoryManager.PlayItemSound(entity, SCR_SoundEvent.SOUND_CONTAINER_CLOSE);
			}
		}
		else
		{
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_CLOSE);
		}
		
		GetGame().GetInputManager().RemoveActionListener("Inventory_Drag", EActionTrigger.DOWN, Action_DragDown);
		GetGame().GetInputManager().RemoveActionListener("Inventory", EActionTrigger.DOWN, Action_CloseInventory);
		//m_bColdStart = false; 
		if (m_pVicinity)
		{
			m_pVicinity.ManipulationComplete();
			m_iVicinityDiscoveryRadius = 0;
		}

		auto menuManager = GetGame().GetMenuManager();
		auto menu = ChimeraMenuPreset.Inventory20Menu;

		auto inventoryMenu = menuManager.FindMenuByPreset( menu ); // prototype inventory
		if (inventoryMenu)
			menuManager.CloseMenuByPreset( menu );
		
		if  (m_PlayerRenderAttributes)
			m_PlayerRenderAttributes.ResetDeltaRotation();

		if (m_Player)
		{
			m_CharController = SCR_CharacterControllerComponent.Cast(m_Player.GetCharacterController());
			if (m_CharController)
				m_CharController.m_OnLifeStateChanged.Remove(LifeStateChanged);
		}
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
			gameMode.GetOnControllableDeleted().Remove(OnControllableDeleted);
		
		if (m_pCharacterWidgetHelper)
			m_pCharacterWidgetHelper.Destroy();
		
		m_pCharacterWidgetHelper = null;

		HideItemInfo();
		HideDamageInfo();
		
		if ( m_pItemInfo )
			m_pItemInfo.Destroy();
		
		if ( m_pDamageInfo )
			m_pDamageInfo.Destroy();
		
		m_pItemInfo = null;
		m_pDamageInfo = null;

		if (m_InventoryManager)
			m_InventoryManager.OnInventoryMenuClosed();
	}
				
	//------------------------------------------------------------------------------------------------
	//! Method inserted on LifeStateChanged invoker on the character controller
	//! When LifeState changes, we may want to close the inventory
	void LifeStateChanged(ECharacterLifeState lifeState)
	{
		if (lifeState != ECharacterLifeState.ALIVE)
			Action_CloseInventory();
	}

	//------------------------------------------------------------------------------------------------
	//Only for cases when character is removed before dying
	protected void OnControllableDeleted(IEntity controllable)
	{
		if (m_Player && m_Player == controllable)
			Action_CloseInventory();
	}

	//------------------------------------------------------------------------------------------------
	//!
	protected void SimpleFSM( EMenuAction EAction = EMenuAction.ACTION_SELECT  )
	{
		switch (EAction)
		{
			case EMenuAction.ACTION_MOVEINSIDE:
			{
				Action_MoveItemToStorage(m_pActiveStorageUI);
				if (m_bIsUsingGamepad)
					SetStorageSwitchMode(true);
				NavigationBarUpdate();
				if (m_pSelectedSlotUI)
				{
					m_pSelectedSlotUI.SetSelected(false);
					m_pSelectedSlotUI = null;
				}
				FocusOnSlotInStorage(m_pActiveStorageUI);
				ResetHighlightsOnAvailableStorages();
			} break;

			case EMenuAction.ACTION_SELECT:
			{
				if (!m_pFocusedSlotUI)
					return;

				if (m_pSelectedSlotUI)
					m_pSelectedSlotUI.SetSelected(false);

				if (m_bIsUsingGamepad)
				{
					m_pSelectedSlotUI = m_pFocusedSlotUI;
					m_pSelectedSlotUI.SetSelected(true);
					HighlightAvailableStorages(m_pSelectedSlotUI);
				}

				NavigationBarUpdate();
				if (m_bIsUsingGamepad)
					SetStorageSwitchMode(true);
			} break;

			case EMenuAction.ACTION_DESELECT:
			{
				if (m_pSelectedSlotUI)
				{
					ResetHighlightsOnAvailableStorages();
					m_pSelectedSlotUI.SetSelected(false);
					m_pSelectedSlotUI = null;
				}

				NavigationBarUpdate();
			} break;

			case EMenuAction.ACTION_DRAGGED:
			{
				m_EStateMenuItem = EStateMenuItem.STATE_MOVING_ITEM_STARTED;
			} break;

			case EMenuAction.ACTION_DROPPED:
			{
				Action_Drop();
				if (m_pActiveHoveredStorageUI)
					m_pActiveHoveredStorageUI.ShowContainerBorder(false);
				m_EStateMenuItem = EStateMenuItem.STATE_IDLE;
				return;
			} break;

			case EMenuAction.ACTION_MOVEBETWEEN:
			{
				if (m_pFocusedSlotUI == m_pInspectedSlot)
				{
					InspectItem(null);
				}			
				
				if (m_pFocusedSlotUI)
				{
					m_pSelectedSlotUI = m_pFocusedSlotUI;
					SCR_InventoryStorageBaseUI pStorage = m_pFocusedSlotUI.GetStorageUI();

					if (SCR_InventoryOpenedStorageUI.Cast(pStorage) && m_aOpenedStoragesUI.Contains(SCR_InventoryOpenedStorageUI.Cast(pStorage)))
					{
						MoveBetweenFromVicinity();
						SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_CONTAINER_DIFR_DROP);
					}
					else if (pStorage != m_pStorageLootUI)
					{
						MoveBetweenToVicinity();
						SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_VICINITY_DROP_CLICK);
					}
					else
					{
						MoveBetweenFromVicinity();
						SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_PICKUP_CLICK);
					}
					m_pSelectedSlotUI = null;
				}
				ResetHighlightsOnAvailableStorages();
			} break;

			case EMenuAction.ACTION_UNFOLD:
			{
				if ( m_pFocusedSlotUI.GetStorageUI() == m_pStorageListUI ) //if it is slot in the "storage list ui"
				{
					if ( BaseInventoryStorageComponent.Cast( m_pFocusedSlotUI.GetAsStorage() ) )	// and if it is a storage
					{
						//ShowStorage( m_pFocusedSlotUI.GetAsStorage() ); 		//show the content of the actualy selected
						ToggleStorageContainerVisibility( m_pFocusedSlotUI );
						m_EStateMenuStorage = EStateMenuStorage.STATE_OPENED;
					}
					else
					{
						//CloseOpenedStorage();	// if it is not storage, show nothing
						//ToggleStorageContainerVisibility( m_pFocusedSlotUI.GetAsStorage() );
						m_EStateMenuStorage = EStateMenuStorage.STATE_IDLE;
					}
					
				}
				else
				{
					TraverseActualSlot();
					NavigationBarUpdate();
					m_EStateMenuStorage = EStateMenuStorage.STATE_OPENED;
				}
			} break;

			case EMenuAction.ACTION_OPENCONTAINER:
			{
				OpenAsNewContainer();
			} break;
		}

		if (!m_bIsUsingGamepad)
			m_pSelectedSlotUI = m_pFocusedSlotUI;

		HideItemInfo();
	}

	//------------------------------------------------------------------------------------------------
	protected void Action_OpenContainer()
	{
		SimpleFSM(EMenuAction.ACTION_OPENCONTAINER);
	}

	//------------------------------------------------------------------------------------------------
	//!
	protected void Action_Drop()
	{
		bool onDrop = false;
		if (m_pFocusedSlotUI)
			onDrop = m_pFocusedSlotUI.OnDrop(m_pSelectedSlotUI);

		Widget underCursor = WidgetManager.GetWidgetUnderCursor();
		if (underCursor && underCursor.GetName() == "HitZoneButton")
		{
			SCR_InventoryHitZonePointContainerUI hzContainer = SCR_InventoryHitZonePointContainerUI.Cast(m_AttachmentSpinBox.GetCurrentItemData());
			if (m_pSelectedSlotUI && hzContainer)
			{
				hzContainer.GetStorage().OnDrop(m_pSelectedSlotUI);
			}
		}
		
		if ( !onDrop && IsFocusedItemInsideDropContainer() != EDropContainer.NOCONTAINER )		//dropped to a container
		{
			if (m_pSelectedSlotUI == m_pInspectedSlot)
				InspectItem(null);
			
			if ( m_pFocusedSlotUI )
			{
				if ( IsFocusedItemInsideDropContainer() == EDropContainer.ISINSIDE )
				{
					if ( SCR_InventorySlotStorageUI.Cast( m_pFocusedSlotUI ) )	// storage
					{
						MoveItemToStorageSlot();
					}
					else if ( SCR_InventorySlotWeaponSlotsUI.Cast( m_pFocusedSlotUI ) )	// weapon slot
					{
						EquipWeaponIntoWeaponSlot();
					}
					else if ( SCR_InventorySlotUI.Cast( m_pFocusedSlotUI ) )	// simple slot
					{
						if ( SCR_InventoryStorageQuickSlotsUI.Cast( m_pFocusedSlotUI.GetStorageUI() ) )	//quick slot
						{
							//m_pFocusedSlotUI.m_iQuickSlotIndex
							SetItemToQuickSlotDrop();
						}
						else
						{
							MoveItemToStorageSlot();
							/*
							if ( m_pFocusedSlotUI.GetInventoryItemComponent() )
							{
								MoveItemToStorageSlot();
							}
							else
							{
								//we are inserting into empy slot
								m_InventoryManager.EquipGadget( m_pSelectedSlotUI.GetInventoryItemComponent().GetOwner() );
							}
								*/
						}

					}
				}
			}
			else
			{
				// just container
				if (m_pActiveHoveredStorageUI)
					m_pActiveHoveredStorageUI.OnDrop(m_pSelectedSlotUI);
				if (!onDrop)
					MoveItem();
			}
		}
		else
		{
			if (WidgetManager.GetWidgetUnderCursor() == m_wPlayerRender)
			{
				EquipDraggedItem(true);
			}
			else
			{
				//dropped outside of a container
				RemoveItemFromQuickSlotDrop();
			}
		}
		
		ResetHighlightsOnAvailableStorages();
	}

	//------------------------------------------------------------------------------------------------
	void Action_QuickSlotAssign()
	{
		if (m_pItemToAssign && m_pActiveStorageUI == m_pQuickSlotStorage)
		{
			int slotId = m_pQuickSlotStorage.GetFocusedSlotId() + 1;
			SetItemToQuickSlot(slotId, m_pItemToAssign);
			FocusOnSlotInStorage(m_pQuickSlotStorage, slotId - 1);
			m_pItemToAssign = null;
		}
		else
		{
			if (m_pActiveStorageUI != m_pStorageLootUI && m_pActiveStorageUI != m_pQuickSlotStorage)
			{
				m_pItemToAssign = m_pFocusedSlotUI;
				FocusOnSlotInStorage(m_pQuickSlotStorage, 4);
			}
		}

		NavigationBarUpdate();
	}

	//------------------------------------------------------------------------------------------------
	void Action_QuickSlotUnassign()
	{
		if (!m_pFocusedSlotUI)
			return;

		InventoryItemComponent itemComp = m_pFocusedSlotUI.GetInventoryItemComponent();
		if (!itemComp)
			return;

		IEntity item = itemComp.GetOwner();
		if (!item)
			return;

		int slotId = m_pQuickSlotStorage.GetFocusedSlotId();
		slotId = Math.Clamp(slotId, 0, m_StorageManager.GetQuickSlotItems().Count());
		m_pItemToAssign = null;
		m_StorageManager.RemoveItemFromQuickSlot(item);
		ShowQuickSlotStorage();
		FocusOnSlotInStorage(m_pQuickSlotStorage, slotId);
		NavigationBarUpdate();
	}

	//------------------------------------------------------------------------------------------------
	//!
	void MoveItemToStorageSlot()
	{
		if (MoveItemToStorageSlot_VirtualArsenal())
			return;
		
		if (MoveItemToStorageSlot_ResourceContainer())
			return;

		if (!m_pSelectedSlotUI)
			return;
		
		InventoryItemComponent pComp = m_pSelectedSlotUI.GetInventoryItemComponent();
		if (!pComp)
			return;

		IEntity pItem = pComp.GetOwner();
		if (!m_InventoryManager.CanMoveItem(pItem))
			return;
		
		m_pCallBack.m_pItem = pItem;
		m_pCallBack.m_pMenu = this;
		m_pCallBack.m_pStorageFrom = m_pSelectedSlotUI.GetStorageUI();
		m_pCallBack.m_pStorageTo = m_pFocusedSlotUI.GetStorageUI();

		BaseInventoryStorageComponent pStorageFromComponent = m_pCallBack.m_pStorageFrom.GetCurrentNavigationStorage();
		BaseInventoryStorageComponent pStorageToComponent = m_pFocusedSlotUI.GetAsStorage();
		
		if (!pStorageToComponent)
			pStorageToComponent = m_pCallBack.m_pStorageTo.GetStorage();
		
		if (IsStorageArsenal(pStorageToComponent))
		{
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_DROP_ERROR);
			return;
		}
		
		bool shouldEquip = m_pCallBack.m_pStorageTo == m_pStorageListUI;
		bool equip = shouldEquip && m_InventoryManager.EquipAny(m_StorageManager , pItem, 0, m_pCallBack);
		
		if (!equip)
			m_InventoryManager.InsertItem( pItem, pStorageToComponent, pStorageFromComponent, m_pCallBack );
		else
			m_InventoryManager.PlayItemSound(pItem, SCR_SoundEvent.SOUND_EQUIP);
		/*
		if ( pItem.FindComponent( SCR_GadgetComponent ) )
			m_InventoryManager.EquipGadget( pItem, m_pCallBack );
		else
		*/
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void EquipWeaponIntoWeaponSlot()
	{
		if (EquipWeaponIntoWeaponSlot_VirtualArsenal())
			return;

		if (!m_pSelectedSlotUI)
			return;
		
		InventoryItemComponent pComp = m_pSelectedSlotUI.GetInventoryItemComponent();
		if (!pComp)
			return;

		SCR_InventorySlotWeaponSlotsUI weaponSlot = SCR_InventorySlotWeaponSlotsUI.Cast(m_pFocusedSlotUI);
		if (!weaponSlot)
			return;
		
		IEntity pItem = pComp.GetOwner();
		if (!m_InventoryManager.CanMoveItem(pItem))
			return;
		
		m_pCallBack.m_pItem = pItem;
		m_pCallBack.m_pMenu = this;
		m_pCallBack.m_pStorageFrom = m_pSelectedSlotUI.GetStorageUI();
		m_pCallBack.m_pStorageTo = m_pFocusedSlotUI.GetStorageUI();
		
		InventoryItemComponent pItemToReplaceComp = m_pFocusedSlotUI.GetInventoryItemComponent();
		IEntity pItemToReplace;
		if (pItemToReplaceComp)
			pItemToReplace = pItemToReplaceComp.GetOwner();
		
		if (pItemToReplace)
		{
			BaseInventoryStorageComponent itemToReplaceStorage = BaseInventoryStorageComponent.Cast(pItemToReplace.FindComponent(BaseInventoryStorageComponent));
		
			if (itemToReplaceStorage && m_InventoryManager.CanInsertItemInStorage(pItem, itemToReplaceStorage, -1))
			{
				m_InventoryManager.TryInsertItemInStorage(pItem, itemToReplaceStorage, -1, m_pCallBack);
				return;
			}
		}
		
		InventoryStorageSlot itemParentSlot = pComp.GetParentSlot();
		
		if (pItemToReplace && pItem)
		{
			if (itemParentSlot && IsStorageArsenal(itemParentSlot.GetStorage()))
			{
				m_InventoryManager.TryReplaceAndDropItemAtSlot(m_pWeaponStorageComp, pItem, weaponSlot.GetWeaponSlotIndex(), m_pCallBack, true, true);
				return;
			}
