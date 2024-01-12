//------------------------------------------------------------------------------------------------
//! Controls invoker resgistration for gadget manager
class SCR_GadgetInvokersInitState
{
	SCR_GadgetManagerComponent m_GadgetManager;
	SCR_CharacterControllerComponent m_Controller;
	
	bool m_bIsControlledInit = false;	// init flag for controlled entity invokers
	bool m_bIsDefaultInit = false;		// init flag for default invokers
	bool m_bIsControlledEnt = false;	// controlled entity flag
	
	//------------------------------------------------------------------------------------------------
	bool IsInit()
	{
		if (m_bIsControlledEnt)
			return m_bIsDefaultInit && m_bIsControlledInit;
		
		return m_bIsDefaultInit;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Invokers for all characters
	void InitInvokers(IEntity character, SCR_CharacterControllerComponent controller)
	{
		if (m_bIsDefaultInit)
			return;
		
		m_Controller = controller;
		SCR_InventoryStorageManagerComponent invManager = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		if (invManager)
		{			
			m_Controller.m_OnGadgetStateChangedInvoker.Insert(m_GadgetManager.OnGadgetStateChanged);
			m_Controller.m_OnControlledByPlayer.Insert(m_GadgetManager.OnControlledByPlayer);
			invManager.m_OnItemAddedInvoker.Insert(m_GadgetManager.OnItemAdded); 
			invManager.m_OnItemRemovedInvoker.Insert(m_GadgetManager.OnItemRemoved); 
			
			m_bIsDefaultInit = true;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Invokers for controlled character only
	void InitControlledInvokers(IEntity character, SCR_CharacterControllerComponent controller)
	{
		if (m_bIsControlledInit)
			return;
		
		m_Controller = controller;
		m_Controller.GetOnPlayerDeathWithParam().Insert(m_GadgetManager.OnPlayerDeath);
		m_Controller.m_OnGadgetFocusStateChangedInvoker.Insert(m_GadgetManager.OnGadgetFocusStateChanged);
		
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
			editorManager.GetOnOpened().Insert(m_GadgetManager.OnEditorOpened);
					
		m_bIsControlledEnt = true;
		m_bIsControlledInit = true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Cleanup invokers when entity is destroyed
	void CleanupInvokers(GenericEntity character)
	{	
		if (m_Controller)
		{
			m_Controller.m_OnGadgetStateChangedInvoker.Remove(m_GadgetManager.OnGadgetStateChanged);
			m_Controller.m_OnControlledByPlayer.Remove(m_GadgetManager.OnControlledByPlayer);
		}
				
		SCR_InventoryStorageManagerComponent invManager = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		if (invManager)
		{
			invManager.m_OnItemAddedInvoker.Remove(m_GadgetManager.OnItemAdded); 
			invManager.m_OnItemRemovedInvoker.Remove(m_GadgetManager.OnItemRemoved); 
		}
		
		m_bIsDefaultInit = false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Cleanup local invokers when entity is no longer controlled
	void CleanupLocalInvokers(GenericEntity character)
	{	
		if (m_Controller)
		{
			m_Controller.GetOnPlayerDeathWithParam().Remove(m_GadgetManager.OnPlayerDeath);
			m_Controller.m_OnGadgetFocusStateChangedInvoker.Remove(m_GadgetManager.OnGadgetFocusStateChanged);
		}
		
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
			editorManager.GetOnOpened().Remove(m_GadgetManager.OnEditorOpened);
		
		m_bIsControlledInit = false;
		m_bIsControlledEnt = false;
	}
				
	//------------------------------------------------------------------------------------------------
	void Clear(GenericEntity entity)
	{
		if (!entity)
			return;
		
		CleanupLocalInvokers(entity);
		CleanupInvokers(entity);
	}
		
	//------------------------------------------------------------------------------------------------
	void SCR_GadgetInvokersInitState(notnull SCR_GadgetManagerComponent gadgetManager)
	{
		m_GadgetManager = gadgetManager;
	}
};

//------------------------------------------------------------------------------------------------
[EntityEditorProps(category: "GameScripted/Gadgets", description: "Gadget manager", color: "0 0 255 255")]
class SCR_GadgetManagerComponentClass: ScriptGameComponentClass
{
	//------------------------------------------------------------------------------------------------
	static override bool DependsOn(string className)
	{
		if (className == "SCR_CharacterInventoryStorageComponent")	// enforce load order here to be able to access storage
			return true;
		
		return false;
	}
};

//------------------------------------------------------------------------------------------------
class SCR_GadgetManagerComponent : ScriptGameComponent
{			
	int m_iRadiosFlags = EGadgetType.RADIO | EGadgetType.RADIO_BACKPACK;
	
	ref ScriptInvoker<SCR_GadgetComponent> m_OnGadgetAdded = new ScriptInvoker();			// called when gadget is added to inventory
	ref ScriptInvoker<SCR_GadgetComponent> m_OnGadgetRemoved = new ScriptInvoker();			// called when gadget is removed from inventory
	protected static ref ScriptInvoker<IEntity, SCR_GadgetManagerComponent> s_OnGadgetInitDone = new ScriptInvoker();	// invoked after gadget init is done on a newly spawned character
	
	protected bool m_bIsGadgetADS;		// is gadget currently in raised state
	protected IEntity m_HeldGadget;
	protected SCR_GadgetComponent m_HeldGadgetComponent;
	protected SCR_GadgetComponent m_HiddenGadgetComponent;
	protected InputManager m_InputManager;
	protected SCR_InventoryStorageManagerComponent m_InventoryStorageMgr;
	protected SCR_CharacterControllerComponent m_Controller;
	protected SCR_VONController m_VONController;
	protected ref SCR_GadgetInvokersInitState m_pInvokersState = new ref SCR_GadgetInvokersInitState(this);
	
	protected ref array<ref array <SCR_GadgetComponent>> m_aInventoryGadgetTypes = {};	// array of gadget types > array of gadget components
	protected ref map<EGadgetType, int> m_aGadgetArrayMap = new map<EGadgetType, int>; 	// map of gadget types -> gadget type array ID
	
	//TODO	
	protected SCR_GadgetComponent m_LastHeldGadgetComponent; // hack to make thingy work, unhack me 

	//------------------------------------------------------------------------------------------------
	// GET/SET METHODS
	//------------------------------------------------------------------------------------------------
	static ScriptInvoker GetOnGadgetInitDoneInvoker() { return s_OnGadgetInitDone; }
	
	//------------------------------------------------------------------------------------------------
	//! Get gadget manager of entity
	//! \return gadget manager
	static SCR_GadgetManagerComponent GetGadgetManager(IEntity entity)
	{		
		GenericEntity player = GenericEntity.Cast(entity);
		if (!player)
			return null;
		
		SCR_GadgetManagerComponent gadgetManager = SCR_GadgetManagerComponent.Cast( player.FindComponent(SCR_GadgetManagerComponent) );		

		return gadgetManager;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get held (currently in hand) gadget
	//! \return held gadget entity
	IEntity GetHeldGadget()
	{
		if (m_HeldGadget)
			return m_HeldGadget;
		else if (m_HiddenGadgetComponent)
			return m_HiddenGadgetComponent.GetOwner();
		else
			return null;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Get is player in gadget ADS
	//! \return if gadget is in ADS
	bool GetIsGadgetADS()
	{
		return m_bIsGadgetADS;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get held (currently in hand) gadget component
	//! \return held gadget component
	SCR_GadgetComponent GetHeldGadgetComponent()
	{
		if (m_HeldGadgetComponent)
			return m_HeldGadgetComponent;
		
		return m_HiddenGadgetComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set mode of a gadget
	//! When changing mode FROM EGadgetMode.IN_HAND to inventory, target should be EGadgetMode.IN_STORAGE -> it will do EGadgetMode.IN_SLOT automatically if it is slotted
	//! \param gadget is the subject
	//! \param targetMode is the mode being switched into
	//! \param doFocus determines whether the gadget will becomes focused straight away
	void SetGadgetMode(IEntity gadget, EGadgetMode targetMode, bool doFocus = false)
	{		
		if (!gadget)
			return;
		
		SCR_GadgetComponent gadgetComp = SCR_GadgetComponent.Cast( gadget.FindComponent(SCR_GadgetComponent) );
		if (!gadgetComp)
			return;
		
		if ( targetMode == EGadgetMode.IN_HAND && !gadgetComp.CanBeHeld() )
			return;
		
		// switch mode rules
		if (!CanChangeGadgetMode(gadget, gadgetComp, targetMode))
			return;
		
		if ( targetMode == EGadgetMode.IN_HAND || gadgetComp.GetMode() == EGadgetMode.IN_HAND || gadgetComp == m_HiddenGadgetComponent )	// If going TO or FROM mode1 -> anims need to happen first. Mode is synched from finish events instead of here 
		{					
			if (!m_Controller)
				return;
							
			if (targetMode == EGadgetMode.IN_HAND)	// Gadget to hand
			{
				if (m_Controller.GetWeaponADSInput())
					m_Controller.SetWeaponADSInput(false);

				if (gadgetComp.CanBeRaised())
					m_Controller.TakeGadgetInLeftHand(gadget, gadgetComp.GetAnimVariable(), doFocus);
				else
					m_Controller.TakeGadgetInLeftHand(gadget, gadgetComp.GetAnimVariable());
			}
			else									// Gadget from hand
			{
				if (targetMode == EGadgetMode.ON_GROUND)	// if moving from hand to storage, SetGadgetMode will call hand->ground ground->slot , so we cannot wait for anim to finish to switch mode
					m_Controller.RemoveGadgetFromHand(true);
				else
					m_Controller.RemoveGadgetFromHand(gadgetComp == m_HiddenGadgetComponent); 
				
				if (gadgetComp.GetType() == EGadgetType.MAP) // TODO hotfix against incorrect toggle set when spamming input, will be solved when map has unfocused & focused mode
					return;
								
				if (gadgetComp.m_bFocused)	// close focus state
					gadgetComp.ToggleFocused(false);
			}
		}
		else	// If switching between NONE and MODE0
			gadgetComp.OnModeChanged(targetMode, GetOwner());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Static method for setting gadgets to ground/storage modes for containers such as vehicles which do not have gadget manager
	//! \param gadgetComp is the subject component
	//! \param targetMode is the mode being switched into
	static void SetGadgetModeStashed(SCR_GadgetComponent gadgetComp, EGadgetMode targetMode)
	{
		if (targetMode != EGadgetMode.ON_GROUND && targetMode != EGadgetMode.IN_STORAGE)
			return;
		
		gadgetComp.OnModeChanged(targetMode, null);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Remove gadget held in hand
	void RemoveHeldGadget()
	{
		if (m_HeldGadget)
			SetGadgetMode(m_HeldGadget, EGadgetMode.IN_SLOT); // whether the gadget is actually slotted or put to storage is handled in OnGadgetStateChanged
		else if (m_HiddenGadgetComponent)
			SetGadgetMode(m_HiddenGadgetComponent.GetOwner(), EGadgetMode.IN_SLOT);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Activate held gadget 
	protected void OnActivateHeldGadget()
	{
		if (m_HeldGadgetComponent)
			m_HeldGadgetComponent.ActivateAction();
		else if (m_HiddenGadgetComponent)
			m_HiddenGadgetComponent.ActivateAction();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Toggle held gadget on/off
	//! \param state true = ON / false = OFF
	void ToggleHeldGadget(bool state)
	{
		if (m_HeldGadgetComponent)
			m_HeldGadgetComponent.ToggleActive(state);
		else if (m_HiddenGadgetComponent)
			m_HiddenGadgetComponent.ToggleActive(state);
	}
						
	//------------------------------------------------------------------------------------------------
	//! Get all owned gadgets by type	
	//! \param type is the type of gadget being searched
	//! \return Returns array of gadget of the given type or null if not found
	array<SCR_GadgetComponent> GetGadgetsByType(EGadgetType type)
	{
		int gadgetArrayID = m_aGadgetArrayMap.Get(type);
		if (gadgetArrayID != -1)
		{
			array<SCR_GadgetComponent> arr = new array<SCR_GadgetComponent>;
			arr.Copy(m_aInventoryGadgetTypes[gadgetArrayID]);
			
			return arr;
		}
		
		return null;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Get first owned gadget by type
	//! \param type is the type of gadget being searched
	//! \return Returns gadget of given type or null if not found
	IEntity GetGadgetByType(EGadgetType type)
	{
		array<SCR_GadgetComponent> gadgets = GetGadgetsByType(type);
		if (!gadgets || gadgets.IsEmpty())
			return null;
		
		foreach (SCR_GadgetComponent comp : gadgets)
		{
			if (comp)
				return comp.GetOwner();
			else 
				Print(GetOwner().ToString() + " SCR_GadgetManager::GetGadgetsByType returned null entry in an array of " + typename.EnumToString(EGadgetType, type) + " gadgets", LogLevel.ERROR); // this should not happen
		}

		return null;
	}
			
	//------------------------------------------------------------------------------------------------
	//! Get first owned gadget assigned in quickslot by type
	//! \param type is the type of gadget being searched
	//! \return Returns gadget of given type or null if not found
	IEntity GetQuickslotGadgetByType(EGadgetType type)
	{
		SCR_CharacterInventoryStorageComponent charStorage = SCR_CharacterInventoryStorageComponent.Cast( GetOwner().FindComponent(SCR_CharacterInventoryStorageComponent) );
		array<IEntity> quickslotEntities = charStorage.GetQuickSlotItems();
		
		foreach (IEntity entity : quickslotEntities )
		{
			if (!entity)	// empty slots will be null
				continue;
			
			SCR_GadgetComponent gadgetComp = SCR_GadgetComponent.Cast( entity.FindComponent(SCR_GadgetComponent) );
			if (gadgetComp)
			{
				if (gadgetComp.GetType() == type)
					return entity;
			}
		}
				
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check whether gadget is owned by this entity
	//! \return true if owned 
	bool IsGadgetOwned(SCR_GadgetComponent gadgetComp)
	{
		int gadgetArrayID = m_aGadgetArrayMap.Get(gadgetComp.GetType());
		if (gadgetArrayID != 1)
		{
			return m_aInventoryGadgetTypes[gadgetArrayID].Contains(gadgetComp);
		}
		
		return false;
	}
								
	//------------------------------------------------------------------------------------------------
	// MANAGER METHODS
	//------------------------------------------------------------------------------------------------
	//! Rules for gadget mode change
	//! \param gadget is the subject
	//! \param gadget is the subjects gadget component
	//! \param targetMode is the wanted mode
	protected bool CanChangeGadgetMode(IEntity gadget, SCR_GadgetComponent gadgetComp, EGadgetMode targetMode)
	{								
		if ( targetMode == EGadgetMode.IN_HAND || gadgetComp.GetMode() == EGadgetMode.IN_HAND )	// If going TO or FROM mode1 -> anims need to happen first. Mode is synched from finish events instead of here 
		{
			CompartmentAccessComponent compAccess = CompartmentAccessComponent.Cast(GetOwner().FindComponent(CompartmentAccessComponent));	// if in vehicle
			if (compAccess && compAccess.IsInCompartment())
			{
				InventoryItemComponent invItemComponent = InventoryItemComponent.Cast(gadget.FindComponent(InventoryItemComponent));
				if (!invItemComponent)
					return false;
					
				CharacterModifierAttributes charModifData = CharacterModifierAttributes.Cast(invItemComponent.FindAttribute(CharacterModifierAttributes));
				if (!charModifData)
					return false;
				
				if (targetMode == EGadgetMode.IN_HAND && !charModifData.CanBeEquippedInVehicle())
					return false;
			}
		}
	
		return true; 
	}
			
	//------------------------------------------------------------------------------------------------
	//! Update currently held(in hand) gadget
	//! \param gadget is the subject
	protected void UpdateHeldGadget(IEntity gadget)
	{
		// no need ot update
		if (gadget && gadget == m_HeldGadget)
			return;
		
		m_bIsGadgetADS = false;
		
		// clear
		if (!gadget)
		{
			if (m_HeldGadgetComponent && m_HeldGadgetComponent.m_bFocused)
				OnGadgetFocusStateChanged(m_HeldGadget, false);
			
			m_HeldGadget = null;
			m_LastHeldGadgetComponent = m_HeldGadgetComponent;
			m_HeldGadgetComponent = null;
			
			CharacterInputContext inputCtx = m_Controller.GetInputContext();
			IEntity hiddenGadget = inputCtx.GetWantedLefHandGadgetEntity();
			if (hiddenGadget)
			{
				SCR_GadgetComponent gadgetComp = SCR_GadgetComponent.Cast( hiddenGadget.FindComponent(SCR_GadgetComponent) );
				if (!gadgetComp)
					return;
				
				m_HiddenGadgetComponent = gadgetComp;
				
				ConnectToGadgetsManagerSystem();
			}
			else
			{
				m_HiddenGadgetComponent = null;
			}
		}
		else
		{
			SCR_GadgetComponent gadgetComp = SCR_GadgetComponent.Cast( gadget.FindComponent(SCR_GadgetComponent) );
			if (!gadgetComp)
				return;
			
			m_HeldGadget = gadget;
			m_HeldGadgetComponent = gadgetComp;
			m_HiddenGadgetComponent = null;
			ConnectToGadgetsManagerSystem();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Additonal pre SetGadgetMode logic for local player, modfying the behavior based on input used & current held gadget state 
	//! \param gadget is the subject 
	//! \param inputVal determines type of input we want to execute, click/hold version
	void HandleInput(IEntity gadget, float inputVal)
	{
		EGadgetMode targetMode = EGadgetMode.IN_HAND;
		bool doFocus = inputVal != 1;	// input modifier from config
		
		if (m_HeldGadget == gadget)		// already holding this gadget
			targetMode = EGadgetMode.IN_STORAGE;
		
		SCR_GadgetComponent gadgetComp = SCR_GadgetComponent.Cast( gadget.FindComponent(SCR_GadgetComponent) );
		if (!gadgetComp)
			return;
		
		if (inputVal != 1 && gadgetComp.CanBeToggled())
			gadgetComp.ToggleActive(!gadgetComp.IsToggledOn());
		else
			SetGadgetMode(gadget, targetMode, doFocus);
	} 
	
	//------------------------------------------------------------------------------------------------
	//! Clear toggle state for cases where it needs to be done without waiting for animation
	protected void ClearToggleState()
	{
		m_bIsGadgetADS = false;
		if (m_HeldGadgetComponent)
		{
			m_HeldGadgetComponent.ToggleFocused(false);
			m_Controller.SetGadgetRaisedModeWanted(false); 
		}
	}
		
	//------------------------------------------------------------------------------------------------
	// EVENTS
	//------------------------------------------------------------------------------------------------
	//! SCR_InventoryStorageManagerComponent, called on every add to slot, not only to inventory
	void OnItemAdded(IEntity item, BaseInventoryStorageComponent storageOwner)
	{		
		SCR_GadgetComponent gadgetComp = SCR_GadgetComponent.Cast(item.FindComponent(SCR_GadgetComponent));
		if (!gadgetComp)
			return;
		
		EGadgetType type = gadgetComp.GetType();
		
		int gadgetArrayID = m_aGadgetArrayMap.Get(type);
		if (gadgetArrayID == -1)
			return;
		
		m_aInventoryGadgetTypes[gadgetArrayID].Insert(gadgetComp);
		
		// Check whether the gadget is in equipment slot (OnAddedToSlot will happen before this)
		EquipmentStorageComponent equipmentComp = EquipmentStorageComponent.Cast(storageOwner);
		if (equipmentComp)
		{
			InventoryStorageSlot storageSlot = equipmentComp.FindItemSlot(item);
			if (storageSlot)
				SetGadgetMode(item, EGadgetMode.IN_SLOT);
		}
		else if (type == EGadgetType.RADIO_BACKPACK) 	// special case for backpack radios, always in (character) slot
			SetGadgetMode(item, EGadgetMode.IN_SLOT);
		else
			SetGadgetMode(item, EGadgetMode.IN_STORAGE); 
				
		m_OnGadgetAdded.Invoke(gadgetComp);
		
		if (m_VONController && (type & m_iRadiosFlags))	// add entries to VONController
		{
			BaseRadioComponent radioComp = BaseRadioComponent.Cast(gadgetComp.GetOwner().FindComponent(BaseRadioComponent));
			if (!radioComp)
				return;
			
			// Put all transceivers (AKA) channels in the VoN menu
			int count = radioComp.TransceiversCount();
			for (int i = 0; i < count; i++)
			{
				SCR_VONEntryRadio radioEntry = new SCR_VONEntryRadio();
				radioEntry.SetRadioEntry(radioComp.GetTransceiver(i), i + 1, gadgetComp);
				m_VONController.AddEntry(radioEntry);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_InventoryStorageManagerComponent, called on every remove from slot, not only from inventory
	void OnItemRemoved(IEntity item, BaseInventoryStorageComponent storageOwner)
	{
		SCR_GadgetComponent gadgetComp = SCR_GadgetComponent.Cast(item.FindComponent(SCR_GadgetComponent));
		if (!gadgetComp)
			return;
		
		InventoryItemComponent invComp = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (!invComp)
			return;
				
		EGadgetType type = gadgetComp.GetType();
		
		if (invComp.GetParentSlot())
			SetGadgetMode(item, EGadgetMode.IN_STORAGE);	
		else
			SetGadgetMode(item, EGadgetMode.ON_GROUND);
		
		int gadgetArrayID = m_aGadgetArrayMap.Get(gadgetComp.GetType());
		if (gadgetArrayID == -1)
			return;
		
		int gadgetPos = m_aInventoryGadgetTypes[gadgetArrayID].Find(gadgetComp);
		if (gadgetPos != -1)
			m_aInventoryGadgetTypes[gadgetArrayID].Remove(gadgetPos);
		
		m_OnGadgetRemoved.Invoke(gadgetComp);
		
		if (m_VONController && (type == EGadgetType.RADIO || type == EGadgetType.RADIO_BACKPACK))	// remove entries from VONController
		{
			array<ref SCR_VONEntry> entries = m_VONController.GetVONEntries();
			
			for (int i = entries.Count() - 1; i >= 0; --i)
			{
				SCR_VONEntryRadio entry = SCR_VONEntryRadio.Cast(entries[i]);
				if (entry && entry.GetGadget() == gadgetComp)
					m_VONController.RemoveEntry(entries[i]);	// multiple entries per gadget possible
			}
		}
	}
			
	//------------------------------------------------------------------------------------------------
	//! SCR_CharacterControllerComponent event
	void OnPlayerDeath(SCR_CharacterControllerComponent charController, IEntity instigatorEntity, notnull Instigator instigator)
	{		
		if (!charController || charController != m_Controller)
			return;
				
		ClearToggleState();
		m_pInvokersState.Clear(GetOwner());
		UnregisterVONEntries();
		UnregisterInputs();
		DisconnectFromGadgetsManagerSystem();
	}
		
	//------------------------------------------------------------------------------------------------
	//! SCR_CharacterControllerComponent event
	void OnControlledByPlayer(IEntity owner, bool controlled)
	{		
		if (System.IsConsoleApp())	// hotfix for this being called on DS when other players control their characters
			return;
		
		if (controlled && owner != SCR_PlayerController.Cast(GetGame().GetPlayerController()).GetLocalControlledEntity()) // same as hotfix above but for hosted server, if we get controlled=true for entity which isnt ours, discard
			controlled = false;
		
		if (!controlled)
		{
			m_pInvokersState.CleanupLocalInvokers(GetOwner());
			UnregisterInputs();
			UnregisterVONEntries();
			DisconnectFromGadgetsManagerSystem();
		}
		else if (owner)
		{			
			m_pInvokersState.InitControlledInvokers(owner, m_Controller);
			RegisterInputs();
			RegisterVONEntries();
			ConnectToGadgetsManagerSystem();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! CharacterControllerComponent event
	void OnGadgetStateChanged(IEntity gadget, bool isInHand, bool isOnGround)
	{
		SCR_GadgetComponent gadgetComp = SCR_GadgetComponent.Cast(gadget.FindComponent(SCR_GadgetComponent));
		if (!gadgetComp)
			return;
		
		if (isInHand)
		{
			UpdateHeldGadget(gadget);
			gadgetComp.OnModeChanged(EGadgetMode.IN_HAND, GetOwner());
		}
		else
		{
			UpdateHeldGadget(null);
			
			if (isOnGround)	// this means it has been moved directly to ground from hand
			{
				gadgetComp.OnModeChanged(EGadgetMode.ON_GROUND, GetOwner());
				return;
			}
					
			// Check whether this gadget is slotted in a parent's equipment storage in order to set correct mode
			IEntity parent = gadget.GetParent();
			if (parent)
			{
				InventoryItemComponent item = InventoryItemComponent.Cast(gadget.FindComponent(InventoryItemComponent));
				if (item)
				{
					EquipmentStorageSlot storageSlot = EquipmentStorageSlot.Cast(item.GetParentSlot());
					if (storageSlot && storageSlot.GetOwner() == parent)
					{
						gadgetComp.OnModeChanged(EGadgetMode.IN_SLOT, GetOwner());
						return;
					}
				}
			}
					
			gadgetComp.OnModeChanged(EGadgetMode.IN_STORAGE, GetOwner());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! CharacterControllerComponent event, used only for local character
	void OnGadgetFocusStateChanged(IEntity gadget, bool isFocused)
	{
		SCR_GadgetComponent gadgetComponent;
		if (gadget)
		{
			gadgetComponent = m_HeldGadgetComponent;
		}
		else
		{
			// when gadget is switched, this gets called after gadget state change with null TODO preferably this shouldnt happen :(
			gadgetComponent = m_LastHeldGadgetComponent;
			isFocused = false;
		}
		
		m_bIsGadgetADS = isFocused;
		
		// TODO hotfix against incorrect toggle set when spamming input, will be solved when map has unfocused & focused mode
		if (!gadgetComponent 
			|| ( !isFocused && gadgetComponent.GetType() == EGadgetType.MAP && gadgetComponent.GetMode() == EGadgetMode.IN_HAND ) )
			return;
		
		gadgetComponent.ToggleFocused(isFocused);
		
		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (controller)
			controller.SetGadgetFocus(isFocused);
	}
			
	//------------------------------------------------------------------------------------------------
	//! SCR_EditorManagerEntity event  //TODO we shouldnt need this, probably needs to be dependent on camera instead
	void OnEditorOpened()
	{
		ClearToggleState();
	}

	//------------------------------------------------------------------------------------------------
	// INPUTS & INIT
	//------------------------------------------------------------------------------------------------
	//! Input action callback
	protected void OnGadgetInput(float value, EActionTrigger reason)
	{
		EGadgetType input = GetGadgetInputAction();	
		if (input == 0)
			return;
			
		// Search quickslots first, then inventory
		IEntity gadget = GetQuickslotGadgetByType(input);
		if (!gadget)
			gadget = GetGadgetByType(input);
		
		if (!gadget)
			return;
		
		HandleInput(gadget, value);
	}

	//------------------------------------------------------------------------------------------------
	//! Explicit flashlight toggle action
	protected void ActionFlashlightToggle(float value, EActionTrigger reason)
	{
		// Search quickslots first, then inventory
		IEntity flashlight = GetQuickslotGadgetByType(EGadgetType.FLASHLIGHT);
		if (!flashlight)
			flashlight = GetGadgetByType(EGadgetType.FLASHLIGHT);
		
		if (!flashlight)
			return;

		SCR_GadgetComponent gadgetComp = SCR_GadgetComponent.Cast(flashlight.FindComponent(SCR_GadgetComponent));
		if (gadgetComp)
			gadgetComp.ActivateAction();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Input action callback
	protected void OnPauseMenu()
	{
		if (SCR_MapGadgetComponent.Cast(m_HeldGadgetComponent))
			SetGadgetMode( m_HeldGadget, EGadgetMode.IN_STORAGE ); // TODO update state transition
	}
	
	//------------------------------------------------------------------------------------------------
	//! Input action callback
	protected void OnGadgetADS()
	{
		SetGadgetADS(!m_bIsGadgetADS);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Input action callback
	protected void OnGadgetADSHold(float value, EActionTrigger reason)
	{
		SetGadgetADS(reason == EActionTrigger.DOWN);
	}

	//------------------------------------------------------------------------------------------------
	//! API for setting raised gadget mode
	void SetGadgetADS(bool gadgetADS)
	{
		if (!m_Controller)
			return;
				
		if (m_HeldGadgetComponent && m_HeldGadgetComponent.CanBeRaised())
		{
			m_Controller.SetGadgetRaisedModeWanted(gadgetADS);
		}
		else if (m_HiddenGadgetComponent && m_HiddenGadgetComponent.CanBeRaised())
		{
			if (gadgetADS)
				m_Controller.RecoverHiddenGadget(false, false);

			m_Controller.SetGadgetRaisedModeWanted(gadgetADS);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gather initial gadgets
	//! \param character is the query target
	protected void RegisterInitialGadgets(IEntity character)
	{
		array<IEntity> foundItems = {};
		array<typename> componentsQuery = {SCR_GadgetComponent};
	
		m_InventoryStorageMgr = SCR_InventoryStorageManagerComponent.Cast( character.FindComponent(SCR_InventoryStorageManagerComponent) );
		if (m_InventoryStorageMgr)
			m_InventoryStorageMgr.FindItemsWithComponents(foundItems, componentsQuery, EStoragePurpose.PURPOSE_ANY);
	
		int count = foundItems.Count();
		for (int i = 0; i < count; i++)
		{
			SCR_GadgetComponent gadgetComp = SCR_GadgetComponent.Cast( foundItems[i].FindComponent(SCR_GadgetComponent) );
			int gadgetArrayID = m_aGadgetArrayMap.Get(gadgetComp.GetType());
			if (gadgetArrayID == -1)
				continue;
			
			m_aInventoryGadgetTypes[gadgetArrayID].Insert(gadgetComp);

			if (gadgetComp.GetMode() != EGadgetMode.ON_GROUND)	// initial gadget for client can/will already have set mode members by replication, set the entire mode process properly here
			{
				SetGadgetMode(foundItems[i], gadgetComp.GetMode());
				continue;
			}
			
			// initial gadgets for host will likely be added to storage before manager starts catching OnAdded events and will default to ground  
			InventoryItemComponent invItemComp = InventoryItemComponent.Cast( foundItems[i].FindComponent(InventoryItemComponent) );
			if (invItemComp)
			{
				EquipmentStorageSlot storageSlot = EquipmentStorageSlot.Cast(invItemComp.GetParentSlot());	// Check whether the gadget is in equipment slot (OnAddedToSlot will happen before this)
				if (storageSlot || gadgetComp.GetType() == EGadgetType.RADIO_BACKPACK)	// backpack slot is not EquipmentStorageSlot but we consider it as such
				{
					SetGadgetMode(foundItems[i], EGadgetMode.IN_SLOT);
					continue;
				}
			}
			
			SetGadgetMode(foundItems[i], EGadgetMode.IN_STORAGE);
		}
		
		s_OnGadgetInitDone.Invoke(character, this);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Register radio gadgets into the VON system as entries
	protected void RegisterVONEntries()
	{
		if (!GetGame().GetPlayerController())
			return;
		
		m_VONController = SCR_VONController.Cast(GetGame().GetPlayerController().FindComponent(SCR_VONController));
		if (!m_VONController)
			return;
		
		array<SCR_GadgetComponent> radiosArray = {};
		radiosArray.Copy(GetGadgetsByType(EGadgetType.RADIO)); 					// squad radios
		radiosArray.InsertAll(GetGadgetsByType(EGadgetType.RADIO_BACKPACK)); 	// backpack radio

		foreach (SCR_GadgetComponent radio : radiosArray)
		{
			BaseRadioComponent radioComp = BaseRadioComponent.Cast(radio.GetOwner().FindComponent(BaseRadioComponent));
			int count = radioComp.TransceiversCount();
			for (int i = 0 ; i < count; ++i)	// Get all individual transceivers (AKA channels) from the radio
			{
				SCR_VONEntryRadio radioEntry = new SCR_VONEntryRadio();
				radioEntry.SetRadioEntry(radioComp.GetTransceiver(i), i + 1, radio);
				
				m_VONController.AddEntry(radioEntry);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UnregisterVONEntries()
	{
		if (!m_VONController)
			return;
		
		array<SCR_GadgetComponent> radiosArray = {};
		radiosArray.Copy(GetGadgetsByType(EGadgetType.RADIO)); 					// squad radios
		radiosArray.InsertAll(GetGadgetsByType(EGadgetType.RADIO_BACKPACK)); 	// backpack radio
		array<ref SCR_VONEntry> entries = m_VONController.GetVONEntries(); 
			
		foreach (SCR_GadgetComponent radio : radiosArray)
		{
			for (int i = entries.Count() - 1; i >= 0; --i)
			{
				SCR_VONEntryRadio entry = SCR_VONEntryRadio.Cast(entries[i]);
				if (entry && entry.GetGadget() == radio)
					m_VONController.RemoveEntry(entries[i]);	// multiple entries per gadget possible
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Register input actions
	protected void RegisterInputs()
	{
		m_InputManager = GetGame().GetInputManager();
		if (!m_InputManager)
			return;
								
		// Gadgets
		m_InputManager.AddActionListener("GadgetMap", EActionTrigger.DOWN, OnGadgetInput);
		m_InputManager.AddActionListener("MapEscape", EActionTrigger.DOWN, OnPauseMenu);
		m_InputManager.AddActionListener("GadgetCompass", EActionTrigger.DOWN, OnGadgetInput);
		m_InputManager.AddActionListener("GadgetBinoculars", EActionTrigger.DOWN, OnGadgetInput);
		m_InputManager.AddActionListener("GadgetFlashlight", EActionTrigger.DOWN, OnGadgetInput);
		m_InputManager.AddActionListener("GadgetFlashlightToggle", EActionTrigger.DOWN, ActionFlashlightToggle);
		m_InputManager.AddActionListener("GadgetWatch", EActionTrigger.DOWN, OnGadgetInput);
		m_InputManager.AddActionListener("GadgetADS", EActionTrigger.DOWN, OnGadgetADS);
		m_InputManager.AddActionListener("GadgetADSHold", EActionTrigger.DOWN, OnGadgetADSHold);
		m_InputManager.AddActionListener("GadgetADSHold", EActionTrigger.UP, OnGadgetADSHold);
		m_InputManager.AddActionListener("GadgetCancel", EActionTrigger.DOWN, RemoveHeldGadget);
		m_InputManager.AddActionListener("GadgetActivate", EActionTrigger.DOWN, OnActivateHeldGadget);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Register input actions
	protected void UnregisterInputs()
	{
		m_InputManager = GetGame().GetInputManager();
		if (!m_InputManager)
			return;
		
		m_InputManager.RemoveActionListener("GadgetMap", EActionTrigger.DOWN, OnGadgetInput);
		m_InputManager.RemoveActionListener("MapEscape", EActionTrigger.DOWN, OnPauseMenu);
		m_InputManager.RemoveActionListener("GadgetCompass", EActionTrigger.DOWN, OnGadgetInput);
		m_InputManager.RemoveActionListener("GadgetBinoculars", EActionTrigger.DOWN, OnGadgetInput);
		m_InputManager.RemoveActionListener("GadgetFlashlight", EActionTrigger.DOWN, OnGadgetInput);
		m_InputManager.RemoveActionListener("GadgetFlashlightToggle", EActionTrigger.DOWN, ActionFlashlightToggle);
		m_InputManager.RemoveActionListener("GadgetWatch", EActionTrigger.DOWN, OnGadgetInput);
		m_InputManager.RemoveActionListener("GadgetADS", EActionTrigger.DOWN, OnGadgetADS);
		m_InputManager.RemoveActionListener("GadgetADSHold", EActionTrigger.DOWN, OnGadgetADSHold);
		m_InputManager.RemoveActionListener("GadgetADSHold", EActionTrigger.UP, OnGadgetADSHold);
		m_InputManager.RemoveActionListener("GadgetCancel", EActionTrigger.DOWN, RemoveHeldGadget);
		m_InputManager.RemoveActionListener("GadgetActivate", EActionTrigger.DOWN, OnActivateHeldGadget);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Convert invoked input to gadget type
	//! \return gadget type
	protected EGadgetType GetGadgetInputAction()
	{
		if ( m_InputManager.GetActionTriggered("GadgetMap") )			
			return EGadgetType.MAP;
		
		if ( m_InputManager.GetActionTriggered("GadgetCompass") )
			return EGadgetType.COMPASS;
		
		if ( m_InputManager.GetActionTriggered("GadgetBinoculars") )
			return EGadgetType.BINOCULARS;
		
		if ( m_InputManager.GetActionTriggered("GadgetFlashlight") )
			return EGadgetType.FLASHLIGHT;
		
		if ( m_InputManager.GetActionTriggered("GadgetWatch") )
			return EGadgetType.WRISTWATCH;
		
		return 0;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Initialize component
	//! \param owner is the owner entity
	protected void InitComponent(IEntity owner)
	{ 		
		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast( owner.FindComponent(SCR_CharacterControllerComponent) );	
		if (!controller)
			return;
				
		m_Controller = controller;
	
		m_pInvokersState.InitInvokers(owner, m_Controller); // default (all entities with gadget manager) invokers

		if (SCR_PlayerController.GetLocalControlledEntity() == owner) 	
			m_pInvokersState.InitControlledInvokers(owner, m_Controller);						// local (controlled entity) invokers, ignore owned non controlled characters
		
		// Owner specific behavior 
		if (m_pInvokersState.IsInit())
		{
			RegisterInitialGadgets(owner);
			
			if (m_pInvokersState.m_bIsControlledEnt)
			{
				RegisterInputs();
				RegisterVONEntries();
			}
			else
				DisconnectFromGadgetsManagerSystem();	// if not controlled entity, clear frame
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// RPC
	//------------------------------------------------------------------------------------------------
	//! Ask method for toggle synchronization called from individual gadgets
	//! \param gadgetComp is gadget component
	//! \param state is desired toggle state, true: active 
	void AskToggleGadget(SCR_GadgetComponent gadgetComp, bool state)
	{				
		RplId id = Replication.FindId(gadgetComp);
		
		if ( id && id.IsValid() )
			Rpc(RPC_AskToggleGadget, id, state);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Ask RPC to server for gadget toggle
	//! \param id is unique Rpl id of the gadget 
	//! \param state is desired toggle state, true: active 
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
    protected void RPC_AskToggleGadget(RplId id, bool state)
	{	
		// TODO sanity check
					
		SCR_GadgetComponent gadgetComp = SCR_GadgetComponent.Cast(Replication.FindItem(id));
		if (!gadgetComp)
			return;
				
		gadgetComp.OnToggleActive(state);
		Rpc(RPC_DoToggleGadget, id, state);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Do RPC to clients for gadget toggle 
	//! \param id is unique Rpl id of the gadget 
	//! \param state is desired toggle state, true: active 
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast, RplCondition.NoOwner)]
    protected void RPC_DoToggleGadget(RplId id, bool state)
    {						
		// TODO sanity check
		
		SCR_GadgetComponent gadgetComp = SCR_GadgetComponent.Cast(Replication.FindItem(id));
		if (!gadgetComp)
			return;
		
		gadgetComp.OnToggleActive(state);
    }
	
	protected void ConnectToGadgetsManagerSystem()
	{
		World world = GetOwner().GetWorld();
		GadgetManagersSystem gadgetManagersSystem = GadgetManagersSystem.Cast(world.FindSystem(GadgetManagersSystem));
		if (!gadgetManagersSystem)
			return;
		
		gadgetManagersSystem.Register(this);
	}
	
	protected void DisconnectFromGadgetsManagerSystem()
	{
		World world = GetOwner().GetWorld();
		GadgetManagersSystem gadgetManagersSystem = GadgetManagersSystem.Cast(world.FindSystem(GadgetManagersSystem));
		if (!gadgetManagersSystem)
			return;
		
		gadgetManagersSystem.Unregister(this);
	}
	
	override void OnDelete(IEntity owner)
	{
		DisconnectFromGadgetsManagerSystem();
		
		super.OnDelete(owner);
	}
		
	#ifndef DISABLE_GADGETS	
		//------------------------------------------------------------------------------------------------	
		//! runs only on local client
		void Update(float timeSlice)
		{			
			if (!m_pInvokersState.IsInit())
				InitComponent(GetOwner());

			// context
			if (!m_InputManager || !(m_HeldGadgetComponent || m_HiddenGadgetComponent))
			{
				DisconnectFromGadgetsManagerSystem();
				return;
			}

			SCR_GadgetComponent gadgetComp;
			if (m_HeldGadgetComponent)
				gadgetComp = m_HeldGadgetComponent;
			else 
				gadgetComp = m_HiddenGadgetComponent;

			m_InputManager.ActivateContext("GadgetContext");

			if (gadgetComp.CanBeToggled())
				m_InputManager.ActivateContext("GadgetContextToggleable");

			if (gadgetComp.IsUsingADSControls())
			{
				// Cancel gadget ADS while actively driving vehicle
				SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(GetOwner());
				if (character && character.IsDriving(1))
				{
					if (m_bIsGadgetADS)
						SetGadgetADS(false);
				}
				else
				{
					m_InputManager.ActivateContext("GadgetContextRaisable");
				}
			}

			if (gadgetComp.GetType() == EGadgetType.MAP)
				m_InputManager.ActivateContext("GadgetMapContext");
		}

		//------------------------------------------------------------------------------------------------
		override void OnPostInit(IEntity owner)
		{
			if ( g_Game.InPlayMode() )
				ConnectToGadgetsManagerSystem();

			// init arrays
			typename gadgetTypes = EGadgetType;
			int count = gadgetTypes.GetVariableCount();
			for ( int i = 0; i < count; i++ )
			{
				array <SCR_GadgetComponent> gadgets = new array <SCR_GadgetComponent>;
				m_aInventoryGadgetTypes.Insert(gadgets);
				m_aGadgetArrayMap.Insert( Math.Pow(2, i), i); // EGadgetType flag + array ID
			}
		}		
	#endif
};
