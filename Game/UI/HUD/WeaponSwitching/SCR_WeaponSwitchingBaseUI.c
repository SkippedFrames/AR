[EntityEditorProps(category: "GameScripted/UI/HUD/WeaponSwitchingBar", description: "Concept of quick selection bar")]


class SCR_WeaponSwitchingBaseUI: SCR_InfoDisplay
{
	protected static ResourceName							m_ItemPreviewManagerPrefab = "{9F18C476AB860F3B}Prefabs/World/Game/ItemPreviewManager.et"; // This could be attribute
	protected static Widget									s_wQuickSlotStorage;
	protected static ref SCR_InventoryStorageQuickSlotsUI	s_pQuickSlotStorage;

	static bool 										s_bOpened;

	[Attribute( "{A1E61EF091EAC47C}UI/layouts/Menus/Inventory/InventoryQuickSlotsGrid.layout" )]
	protected string m_sQuickSlotGridLayout;
	
	//------------------------------------------------------------------------ USER METHODS ----------------------------------------------------------------------
	
	
	//------------------------------------------------------------------------------------------------
	static void RefreshQuickSlots(int id = -1)
	{
		if (s_pQuickSlotStorage)
		{
			if (id < 0)
				s_pQuickSlotStorage.RefreshQuickSlots();
			else
				s_pQuickSlotStorage.RefreshSlot(id);
			s_pQuickSlotStorage.HighlightLastSelectedSlot();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Init( IEntity owner )
	{
		s_bOpened = false;

		if (!owner || !m_wRoot)
			return;

		ChimeraWorld world = ChimeraWorld.CastFrom(owner.GetWorld());
		if (world)
		{
			//instantiate the preview manager
			if (!world.GetItemPreviewManager())
			{
				Resource rsc = Resource.Load(m_ItemPreviewManagerPrefab);
				if (rsc.IsValid())
					GetGame().SpawnEntityPrefabLocal(rsc, world);
			}
		}

		InputManager inputManager = GetGame().GetInputManager();;
        inputManager.AddActionListener("Inventory_WeaponSwitching", EActionTrigger.UP, QuickBarClosedOnly );
        inputManager.AddActionListener("Inventory_WeaponSwitching", EActionTrigger.DOWN, Action_OpenQuickSelectionBar );
        inputManager.AddActionListener("Inventory_WeaponSwitching", EActionTrigger.UP, Action_CloseQuickSelectionBar );

        for (int i = 0; i < 10; ++i)
        {
			inputManager.AddActionListener("SwitchWeaponCategory" + i.ToString(), EActionTrigger.DOWN, Action_OpenQuickSelectionBar);
			inputManager.AddActionListener("SwitchWeaponCategory" + i.ToString(), EActionTrigger.UP, Action_CloseQuickSelectionBar);
        }
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveActionListeners()
	{
		InputManager inputManager = GetGame().GetInputManager();
        inputManager.RemoveActionListener("Inventory_WeaponSwitching", EActionTrigger.UP, QuickBarClosedOnly );
        inputManager.RemoveActionListener("Inventory_WeaponSwitching", EActionTrigger.DOWN, Action_OpenQuickSelectionBar );
        inputManager.RemoveActionListener("Inventory_WeaponSwitching", EActionTrigger.UP, Action_CloseQuickSelectionBar );

		for (int i = 0; i < 10; ++i)
		{
			inputManager.RemoveActionListener("SwitchWeaponCategory" + i.ToString(), EActionTrigger.DOWN, Action_OpenQuickSelectionBar);
			inputManager.RemoveActionListener("SwitchWeaponCategory" + i.ToString(), EActionTrigger.UP, Action_CloseQuickSelectionBar);
		}
	}

	//------------------------------------------------------------------------------------------------
    void Action_OpenQuickSelectionBar()
    {
		if (s_bOpened)
			return;

		ChimeraCharacter character = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		CharacterControllerComponent controller;

		if (character)
			controller = character.GetCharacterController();

		if (controller && controller.IsUnconscious())
			return;		
		
    		GetGame().GetInputManager().AddActionListener("CharacterSwitchWeapon", EActionTrigger.VALUE, Action_SelectSlot );
    		s_bOpened = true;
    		GetGame().GetCallqueue().Remove( ShowQuickSlots );		// if there's a delayed Show method from the previous quick bar usage, purge it
		if ( !m_wRoot )
			return;
		if( s_wQuickSlotStorage )
		{
			s_wQuickSlotStorage.RemoveHandler( s_wQuickSlotStorage.FindHandler( SCR_InventoryStorageQuickSlotsUI ) );
			s_wQuickSlotStorage.RemoveFromHierarchy();
		}
		Widget parent = m_wRoot.FindAnyWidget( "QuickSlots" );
		s_wQuickSlotStorage =  GetGame().GetWorkspace().CreateWidgets( m_sQuickSlotGridLayout, parent );
		if( !s_wQuickSlotStorage )
			return;
		s_pQuickSlotStorage = new SCR_InventoryStorageQuickSlotsUI( null, null, null );
		s_wQuickSlotStorage.AddHandler( s_pQuickSlotStorage );
		s_pQuickSlotStorage.SetInitialQuickSlot();
		s_pQuickSlotStorage.HighlightLastSelectedSlot();
		
		Show( true, UIConstants.FADE_RATE_DEFAULT );
		BlurWidget wBlur = BlurWidget.Cast( m_wRoot.FindAnyWidget( "wBlur" ) );
		if ( wBlur ) 
			wBlur.SetVisible( true );

		SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_HOTKEY_OPEN);
		
		if (!controller)
			return;
		
		SCR_InventoryStorageManagerComponent inventory = SCR_InventoryStorageManagerComponent.Cast(controller.GetInventoryStorageManager());
		if (inventory)
			inventory.m_OnQuickBarOpenInvoker.Invoke(true);
    }
	
    //------------------------------------------------------------------------------------------------
    void Action_CloseQuickSelectionBar()
    {
    	GetGame().GetInputManager().RemoveActionListener("CharacterSwitchWeapon", EActionTrigger.VALUE, Action_SelectSlot );
    	s_bOpened = false;
		if ( !m_wRoot )
			return;	
		BlurWidget wBlur = BlurWidget.Cast( m_wRoot.FindAnyWidget( "wBlur" ) );
		if ( wBlur ) 
			wBlur.SetVisible( false );
		
		if ( s_pQuickSlotStorage && s_pQuickSlotStorage.UseItemInSlot() )
			GetGame().GetCallqueue().CallLater( ShowQuickSlots, 2000, false, false, UIConstants.FADE_RATE_DEFAULT, true );
		else
			Show( false, UIConstants.FADE_RATE_DEFAULT );
		
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if (!character)
			return;

		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return;

		SCR_InventoryStorageManagerComponent inventory = SCR_InventoryStorageManagerComponent.Cast(controller.GetInventoryStorageManager());
		if (inventory)
			inventory.m_OnQuickBarOpenInvoker.Invoke(false);
    }

	//------------------------------------------------------------------------------------------------
	// called when the quickbar is closed by letting go the tab key
	protected void QuickBarClosedOnly()
	{
		if (s_pQuickSlotStorage)
			s_pQuickSlotStorage.SetQuickBarClosed();
	}

	//------------------------------------------------------------------------------------------------
    void Action_SelectSlot()
	{
		if (!s_bOpened || !s_pQuickSlotStorage)
			return;
		
		int targetSlot = -1;

		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager.GetActionTriggered("SwitchWeaponCategory1"))
			targetSlot = 0;
		else if (inputManager.GetActionTriggered("SwitchWeaponCategory2"))
			targetSlot = 1;
		else if (inputManager.GetActionTriggered("SwitchWeaponCategory3"))
			targetSlot = 2;
		else if (inputManager.GetActionTriggered("SwitchWeaponCategory4"))
			targetSlot = 3;
		else if (inputManager.GetActionTriggered("SwitchWeaponCategory5"))
			targetSlot = 4;
		else if (inputManager.GetActionTriggered("SwitchWeaponCategory6"))
			targetSlot = 5;
		else if (inputManager.GetActionTriggered("SwitchWeaponCategory7"))
			targetSlot = 6;
		else if (inputManager.GetActionTriggered("SwitchWeaponCategory8"))
			targetSlot = 7;
		else if (inputManager.GetActionTriggered("SwitchWeaponCategory9"))
			targetSlot = 8;
		else if (inputManager.GetActionTriggered("SwitchWeaponCategory0"))
			targetSlot = 9;

		if ( targetSlot == - 1 )
			s_pQuickSlotStorage.SelectSlot( inputManager.GetActionValue("CharacterSwitchWeapon") );
		else
			s_pQuickSlotStorage.SelectSlot( targetSlot );
	}

	//------------------------------------------------------------------------------------------------
	override void UpdateValues(IEntity owner, float timeSlice)
	{
		if (!s_bOpened)
			return;

	    GetGame().GetInputManager().ActivateContext("WeaponSelectionContext");
	}

	protected void ShowQuickSlots(bool show, float speed = UIConstants.FADE_RATE_DEFAULT)
	{
		Show(show, speed);
	}
		
	//------------------------------------------------------------------------------------------------
	override event void OnStartDraw(IEntity owner)
	{
		super.OnStartDraw(owner);
		Init(owner);
		Show(false, 0);
	}	
	
	//------------------------------------------------------------------------------------------------
	override event void OnStopDraw(IEntity owner)
	{
		super.OnStopDraw(owner);
		RemoveActionListeners();
		s_pQuickSlotStorage = null;
	}
};

class SCR_QuickSlotRefreshCB : ScriptedInventoryOperationCallback
{
	override void OnComplete()
	{
		SCR_WeaponSwitchingBaseUI.RefreshQuickSlots();
	}
}
