void OnPlayerEnterCompartmentMethod(ChimeraCharacter playerCharacter, IEntity compartmentEntity);
typedef func OnPlayerEnterCompartmentMethod;
typedef ScriptInvokerBase<OnPlayerEnterCompartmentMethod> OnPlayerEnterCompartment;

void OnPlayerExitCompartmentMethod(ChimeraCharacter playerCharacter, IEntity compartmentEntity);
typedef func OnPlayerExitCompartmentMethod;
typedef ScriptInvokerBase<OnPlayerEnterCompartmentMethod> OnPlayerExitCompartment;

[EntityEditorProps(category: "GameScripted/Vehicle", description: "CompartmentAccessComponent", color: "0 0 255 255")]
class SCR_CompartmentAccessComponentClass: CompartmentAccessComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CompartmentAccessComponent : CompartmentAccessComponent
{	
	//! Local invokers for a specific vehicle
	private ref ScriptInvoker m_OnCompartmentEntered;
	private ref ScriptInvoker m_OnCompartmentLeft;
	
	protected ref OnPlayerEnterCompartment m_OnPlayerEnterCompartment;
	protected ref OnPlayerExitCompartment m_OnPlayerExitCompartment;
	
	protected const int WAIT_FOR_VEHICLE_MAX_TRIES = 10;
	protected static int s_iWaitForVehicleTries = 0;
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnCompartmentEntered(bool createNew = true)
	{
		if (!m_OnCompartmentEntered && createNew)
			m_OnCompartmentEntered = new ScriptInvoker();
		return m_OnCompartmentEntered;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnCompartmentLeft(bool createNew = true)
	{
		if (!m_OnCompartmentLeft && createNew)
			m_OnCompartmentLeft = new ScriptInvoker();
		return m_OnCompartmentLeft;
	}

	//------------------------------------------------------------------------------------------------
	OnPlayerEnterCompartment GetOnPlayerCompartmentEnter(bool createNew = true)
	{
		if (!m_OnPlayerEnterCompartment && createNew)
			m_OnPlayerEnterCompartment = new OnPlayerEnterCompartment();

		return m_OnPlayerEnterCompartment;
	}

	//------------------------------------------------------------------------------------------------
	OnPlayerExitCompartment GetOnPlayerCompartmentExit(bool createNew = true)
	{
		if (!m_OnPlayerExitCompartment && createNew)
			m_OnPlayerExitCompartment = new OnPlayerExitCompartment();

		return m_OnPlayerExitCompartment;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnCompartmentEntered(IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		if (m_OnCompartmentEntered)
			m_OnCompartmentEntered.Invoke( targetEntity, manager, mgrID, slotID, move );
		
		PlayerManager playerManager = GetGame().GetPlayerManager();
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());

		if (!playerManager || !character)
			return;
		
		//--- Check if the character is a player
		int playerId = playerManager.GetPlayerIdFromControlledEntity(character);
		if (playerId == 0)
			return;

		if (m_OnPlayerEnterCompartment)
			m_OnPlayerEnterCompartment.Invoke(character, targetEntity);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnCompartmentLeft(IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		if (m_OnCompartmentLeft)
		m_OnCompartmentLeft.Invoke( targetEntity, manager, mgrID, slotID, move );

		PlayerManager playerManager = GetGame().GetPlayerManager();
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());

		if (!playerManager || !character)
			return;

		//--- Check if the character is a player
		int playerId = playerManager.GetPlayerIdFromControlledEntity(character);
		if (playerId == 0)
			return;

		if (m_OnPlayerExitCompartment)
			m_OnPlayerExitCompartment.Invoke(character, targetEntity);
	}
	
	/*!
	Find a compartment that is marked as accessible and not occupied by anyone.
	\param vehicle Target vehicle
	\param compartmentType Type of compartment
	\return Compartment, null if none was found
	*/
	BaseCompartmentSlot FindFreeAndAccessibleCompartment(IEntity vehicle, ECompartmentType compartmentType, BaseCompartmentSlot customSlot = null)
	{
		BaseCompartmentManagerComponent compartmentManager = BaseCompartmentManagerComponent.Cast(vehicle.FindComponent(BaseCompartmentManagerComponent));
		if (!compartmentManager)
			return null;
		
		array<BaseCompartmentSlot> compartments = {};
		compartmentManager.GetCompartments(compartments);
		
		if (customSlot)
		{
			if (!customSlot.IsCompartmentAccessible())
				return null;
			if (customSlot.IsGetInLockedFor(GetOwner()))
				return null;
			if (customSlot.GetOccupant())
				return null;
			if (compartmentType != GetCompartmentType(customSlot))
				return null;
			
			return customSlot;
		}

		foreach (BaseCompartmentSlot compartment: compartments)
		{
			if (!compartment.IsCompartmentAccessible())
				continue;
			if (compartment.IsGetInLockedFor(GetOwner()))
				continue;
			if (compartment.GetOccupant())
				continue;
			if (compartmentType != GetCompartmentType(compartment))
				continue;
			
			return compartment;
		}
		
		//--- Scan compartments in child entities (ToDo: Remove once GetCompartments is recursive)
		BaseCompartmentSlot childSlot;
		IEntity child = vehicle.GetChildren();
		while (child)
		{
			childSlot = FindFreeAndAccessibleCompartment(child, compartmentType);
			if (childSlot)
				return childSlot;
			
			child = child.GetSibling();
		}
		
		return null;
	}
	
	/*!
	Move in vehicle to specified slot type.
	When executed on server while the character is owned by a client, this function will send the request there.
	\param vehicle Target vehicle
	\param compartmentType Type of compartment
	\return True if the request was successful
	*/
	bool MoveInVehicle(IEntity vehicle, ECompartmentType compartmentType, BaseCompartmentSlot customSlot = null)
	{
		if (!vehicle)
			return false;
		
		//--- Find suitable slot
		BaseCompartmentSlot slot = FindFreeAndAccessibleCompartment(vehicle, compartmentType, customSlot);
		if (!slot)
			return false;

		//--- Move character in the slot (must be called where character is local)		
		IEntity slotEntity = slot.GetOwner();
		RplComponent slotRplComponent = RplComponent.Cast(slotEntity.FindComponent(RplComponent));
		RplId slotEntityID = slotRplComponent.Id();
		int slotID = slot.GetCompartmentSlotID();
		Rpc(MoveInVehicleOwner, slotEntityID, slotID);
		
		//--- Lock the slot for a frame, so any other MoveInVehicle calls in this frame will skip it
		slot.SetCompartmentAccessible(false);
		GetGame().GetCallqueue().CallLater(slot.SetCompartmentAccessible, 1, false, true);
		return true;
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void MoveInVehicleOwner(RplId slotEntityID, int slotID)
	{
		RplComponent slotRplComponent = RplComponent.Cast(Replication.FindItem(slotEntityID));
		if (!slotRplComponent)
		{
			if (s_iWaitForVehicleTries >= WAIT_FOR_VEHICLE_MAX_TRIES)
			{
				// Time out, abort
				s_iWaitForVehicleTries = 0;
				return;
			}
			
			GetGame().GetCallqueue().CallLater(MoveInVehicleOwner, 1000, false, slotEntityID, slotID);
			s_iWaitForVehicleTries++;
			return;
		}
		
		s_iWaitForVehicleTries = 0;
		
		IEntity slotEntity = slotRplComponent.GetEntity();
		
		BaseCompartmentManagerComponent slotCompartmentManager = BaseCompartmentManagerComponent.Cast(slotEntity.FindComponent(BaseCompartmentManagerComponent));
		BaseCompartmentSlot slot = slotCompartmentManager.FindCompartment(slotID);
		
		//--- Use slot's owner, not the vehicle - the slot may be in vehicle's child entity
		MoveInVehicle(slotEntity, slot);
	}
	
	/*!
	Move in vehicle to any available slot.
	When executed on server while the character is owned by a client, this function will send the request there.
	\param vehicle Target vehicle
	\return True if the request was successful
	*/
	bool MoveInVehicleAny(IEntity vehicle)
	{
		if (!vehicle)
			return false;
		
		if (MoveInVehicle(vehicle, ECompartmentType.Pilot))
			return true;
		
		if (MoveInVehicle(vehicle, ECompartmentType.Turret))
			return true;
		
		if (MoveInVehicle(vehicle, ECompartmentType.Cargo))
			return true;
		
		return false;
	}
	
	/*!
	Get vehicle this entity is in.
	Works even when the entity is not directly in the vehicle, but in one of its child entities (e.g., turrets)
	Use this function when you specifically need pointer to the vehicle. If you want to simply check if a character is in a vehicle, use IsInCompartment()!
	\return Vehicle, null when not in any compartment
	*/
	IEntity GetVehicle()
	{
		if (GetCompartment())
			return GetCompartment().GetVehicle();
		else
			return null;
	}
	IEntity GetVehicle(out int compartmentID)
	{
		if (GetCompartment())
			return GetCompartment().GetVehicle(compartmentID);
		else
			return null;
	}
	
	/*!
	Get type of a compartment.
	\param compartment Queried compartment
	\return Compartment type, or -1 if invalid
	*/
	static ECompartmentType GetCompartmentType(notnull BaseCompartmentSlot compartment)
	{
		return compartment.GetType();		
	}
};
