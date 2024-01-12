[EntityEditorProps(category: "GameScripted/Gadgets", description: "Gadget base", color: "0 0 255 255")]
class SCR_GadgetComponentClass: ScriptGameComponentClass
{
};

//------------------------------------------------------------------------------------------------
//! Gadget type 
enum EGadgetType
{
	MAP = 1,
	COMPASS = 2,
	BINOCULARS = 4,
	FLASHLIGHT = 8,
	RADIO = 16,
	RADIO_BACKPACK = 32,
	WRISTWATCH = 64,
	CONSUMABLE = 128,
	BUILDING_TOOL = 256,
	SUPPORT_STATION = 512
};

//------------------------------------------------------------------------------------------------
//! Gadget mode
enum EGadgetMode
{
	ON_GROUND = 0,	// ground
	IN_STORAGE,		// within storage but not slotted
	IN_SLOT,		// in equipment slot
	IN_HAND,		// held in left hand
	LAST
};

//------------------------------------------------------------------------------------------------
// @NOTE(Leo) : short term animation solution
// from conversation with @Théo Escamez: //for now we have 4 items> compass adrianov, compass SY183, Radio ANPRC68 and Radio R148
//! Gadget anim variable
enum EGadgetAnimVariable
{
	NONE = 0,
	ADRIANOV = 1, 
	SY183 = 2,
	ANPRC68 = 3,
	R148 = 4,
	MX991 = 5,
	FLASHLIGHT_SOVIET_01 = 6,
	M22 = 7,
	B12 = 8,
	MAP = 9
};

//------------------------------------------------------------------------------------------------
//! Gadget base class
//------------------------------------------------------------------------------------------------
class SCR_GadgetComponent : ScriptGameComponent
{			
	[Attribute("", UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EGadgetAnimVariable), desc: "Gadget anim variable", category: "Gadget")]
	protected EGadgetAnimVariable m_eAnimVariable;
	
	[Attribute("0 0 0", UIWidgets.Coords, desc: "Adjusted position of prefab within equipment slot, for when items placed intto a same slot have different sizes like flashlights", category: "Gadget")]
	protected vector m_vEquipmentSlotOffset;
	
	bool m_bFocused;
	protected bool m_bActivated = false;					// current state if the gadget can be toggled on	
	protected EGadgetMode m_iMode = EGadgetMode.ON_GROUND;	// curent gadget mode
	protected IEntity m_CharacterOwner;						// current entity in posession of this gadget
				
	//------------------------------------------------------------------------------------------------
	EGadgetAnimVariable GetAnimVariable()
	{
		return m_eAnimVariable;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Is gadget toggled on/off
	//! \return current state on/off
	bool IsToggledOn()
	{
		return m_bActivated;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Event called from SCR_GadgetManagerComponent through RPC request
	//! \param state is gadget state: true - active / false - inactive
	void OnToggleActive(bool state)
	{}
	
	//------------------------------------------------------------------------------------------------
	//! InventoryItemComponent event
	void OnParentSlotChanged(InventoryStorageSlot oldSlot, InventoryStorageSlot newSlot)
	{
		if (newSlot == null)	// ground
		{
			IEntity parentOwner, owner;
			
			BaseInventoryStorageComponent storageComp = oldSlot.GetStorage();
			if (storageComp)
				parentOwner = storageComp.GetOwner();
			else 
				parentOwner = oldSlot.GetOwner();	// LoadoutSlotInfo wont return storage comp 
			
			owner = parentOwner;
			
			while (owner != null)
			{
				parentOwner = owner;
				owner = owner.GetParent();
			}
			
			if ( !SCR_ChimeraCharacter.Cast(parentOwner) )	// remove from char is currently handled by gadget manager
				SCR_GadgetManagerComponent.SetGadgetModeStashed(this, EGadgetMode.ON_GROUND);
		}
		else if (oldSlot == null)
		{
			IEntity parentOwner, owner;
			
			BaseInventoryStorageComponent storageComp = newSlot.GetStorage();
			if (storageComp)
				parentOwner = storageComp.GetOwner();
			else 
				parentOwner = newSlot.GetOwner();	// LoadoutSlotInfo wont return storage comp
			
			owner = parentOwner;
			
			while (owner != null)
			{
				parentOwner = owner;
				owner = owner.GetParent();
			}
			
			
			if ( !SCR_ChimeraCharacter.Cast(parentOwner) ) // add to char is currently handled by gadget manager
				SCR_GadgetManagerComponent.SetGadgetModeStashed(this, EGadgetMode.IN_STORAGE);
		}
	}
		
	//------------------------------------------------------------------------------------------------
	//! Gadget mode change event
	//! \param mode is the target mode being switched to
	void OnModeChanged(EGadgetMode mode, IEntity charOwner)
	{
		// Clear last mode
		ModeClear(m_iMode);
		
		// Update current mode
		m_iMode = mode;
		
		// Set new mode
		ModeSwitch(mode, charOwner);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set gadget mode 
	//! \param mode is the target mode being switched to
	protected void ModeSwitch(EGadgetMode mode, IEntity charOwner)
	{
		UpdateVisibility(mode);
		
		// if removing from inventory
		if (mode == EGadgetMode.ON_GROUND)
			m_CharacterOwner = null;
		else
			m_CharacterOwner = charOwner;
				
		if (mode == EGadgetMode.IN_HAND)
		{
			ActivateGadgetFlag();		// TODO not needed for everything? could leave this to individual gadget decision
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Clear gadget mode
	//! \param mode is the mode being cleared
	protected void ModeClear(EGadgetMode mode)
	{
		if (mode == EGadgetMode.IN_HAND)
		{	
			DeactivateGadgetFlag();
		}
	}
						
	//------------------------------------------------------------------------------------------------
	//! Set gadget active flag
	void ActivateGadgetFlag()
	{
		IEntity owner = GetOwner();
		if (!owner)
			return;
		
		InventoryItemComponent itemComponent = InventoryItemComponent.Cast(owner.FindComponent(InventoryItemComponent));
		if (itemComponent)
			itemComponent.ActivateOwner(true);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Unset gadget active flag
	void DeactivateGadgetFlag()
	{
		IEntity owner = GetOwner();
		if (!owner)
			return;
		
		InventoryItemComponent itemComponent = InventoryItemComponent.Cast(owner.FindComponent(InventoryItemComponent));
		if (itemComponent)
			itemComponent.ActivateOwner(false);
	}
	

	//------------------------------------------------------------------------------------------------
	//! Set visibility when show/hide hand animation starts/finishes
	// \param inHand states whether the gadget is currently held in hand
	void UpdateVisibility(EGadgetMode mode)
	{		
		// mode1
		InventoryItemComponent itemComponent = InventoryItemComponent.Cast(GetOwner().FindComponent(InventoryItemComponent));
		if (!itemComponent)
			return;
			
		// if in hand or visible while slotted
		if ( mode == EGadgetMode.IN_HAND || (mode == EGadgetMode.IN_SLOT && IsVisibleEquipped()) )
		{	
			// slotted,  Set positioning of visible slotted gadgets on character -> configured in item prefab to allow gadgets of different sizes
			if (mode != EGadgetMode.IN_HAND)
			{			
				InventoryStorageSlot slot = itemComponent.GetParentSlot();
				ItemAnimationAttributes animAttr = ItemAnimationAttributes.Cast(itemComponent.FindAttribute(ItemAnimationAttributes));
				if (slot && animAttr)
				{
					vector matLS[4];
					animAttr.GetAdditiveTransformLS(matLS);
					matLS[3] = matLS[3] + m_vEquipmentSlotOffset;
					slot.SetAdditiveTransformLS(matLS); 
				}
				
				EquipmentStorageSlot equipSlot = EquipmentStorageSlot.Cast(slot);
				if (equipSlot && equipSlot.IsOccluded())
				{
					itemComponent.HideOwner();	// slotted but occluded
					return;
				}
			}
			
			itemComponent.ShowOwner();
		}
		// if slotted and not visible or not slotted
		else
		{
			if (mode == EGadgetMode.ON_GROUND)
				itemComponent.ShowOwner();
			else 
				itemComponent.HideOwner();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Synchronize gadget state
	//! \param state is target state true = active, false = inactive
	void ToggleActive(bool state)
	{		
		if (!m_CharacterOwner)
			return;
		
		RplComponent rplComponent = RplComponent.Cast(m_CharacterOwner.FindComponent(RplComponent));
		if (!rplComponent || !rplComponent.IsOwner())
			return;					// NOT owner of the character in possession of this gadget
				
		// Client side
		rplComponent = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (rplComponent && rplComponent.IsProxy())
			OnToggleActive(state);	// activate client side to avoid server delay
			
		// Sync
		if (m_CharacterOwner)
			SCR_GadgetManagerComponent.GetGadgetManager(m_CharacterOwner).AskToggleGadget(this, state);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Action listener callback
	void ActivateAction()
	{}
	
	//------------------------------------------------------------------------------------------------
	//! Toggle gadget focus state
	//! \param enable is target state
	void ToggleFocused(bool enable)
	{	
		if (enable)
			m_bFocused = true;
		else
			m_bFocused = false;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Get IEntity in possession of this Gadget
	//! \return returns entity m_CharacterOwner
	IEntity GetCharacterOwner()
	{
		return m_CharacterOwner;
	}
				
	//------------------------------------------------------------------------------------------------
	//! Get gadget type
	//! \return Returns gadget type
	EGadgetType GetType()
	{
		return 0;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Get gadget mode
	//! \return Returns gadget mode
	EGadgetMode GetMode()
	{
		return m_iMode;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Can be held in hand
	//! \return Returns true if heldable
	bool CanBeHeld()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Can be toggled/activated
	//! \return Returns true if toggleable
	bool CanBeToggled()
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gadget has a raised animation version
	//! \return Returns true if raisable
	bool CanBeRaised()
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gadget uses ADS controls from gadget raisable context
	//! \return Returns true if using the controls
	bool IsUsingADSControls()
	{
		return false;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Visible when attached to character gear, further condition may determine visibility, such as if the equipment slot is obstructed by something else
	//! \return Returns true if visible in equipment slot
	bool IsVisibleEquipped()
	{
		return false;
	}
	
	//Called by Gadgets system
	void Update(float timeSlice);
	
	protected void ConnectToGadgetsSystem()
	{
		World world = GetOwner().GetWorld();
		GadgetsSystem gadgetSystem = GadgetsSystem.Cast(world.FindSystem(GadgetsSystem));
		if (!gadgetSystem)
			return;
		
		gadgetSystem.Register(this);
	}
	
	protected void DisconnectFromGadgetsSystem()
	{
		World world = GetOwner().GetWorld();
		GadgetsSystem gadgetSystem = GadgetsSystem.Cast(world.FindSystem(GadgetsSystem));
		if (!gadgetSystem)
			return;
		
		gadgetSystem.Unregister(this);
	}
	
	//------------------------------------------------------------------------------------------------	
	override bool RplSave(ScriptBitWriter writer)
	{
		writer.WriteIntRange(m_iMode, 0, EGadgetMode.LAST-1);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		reader.ReadIntRange(m_iMode, 0, EGadgetMode.LAST-1);
		
		UpdateVisibility(m_iMode);
			
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		DisconnectFromGadgetsSystem();
		
		super.OnDelete(owner);
	}
						
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask( owner, EntityEvent.INIT);
		
		InventoryItemComponent invComp = InventoryItemComponent.Cast(GetOwner().FindComponent(InventoryItemComponent));
		if (invComp)
			invComp.m_OnParentSlotChangedInvoker.Insert(OnParentSlotChanged);
	}
};
