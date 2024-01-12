//------------------------------------------------------------------------------------------------
enum EVehicleType
{
	VEHICLE 		= 0,
	CAR 			= 2,
	TRUCK 			= 4,
	APC 			= 8,
	FUEL_TRUCK 		= 16,
	COMM_TRUCK 		= 32,
	SUPPLY_TRUCK 	= 64,
	MORTAR			= 128,
};

//------------------------------------------------------------------------------------------------
class VehicleClass: BaseVehicleClass
{
};

//------------------------------------------------------------------------------------------------
class Vehicle : BaseVehicle
{
	// Can be removed when EOnPhysicsActive is on components as well
	protected ref ScriptInvoker<IEntity, bool> m_OnPhysicsActive;
	
	[Attribute("0", UIWidgets.ComboBox, "Vehicle type:", "", ParamEnumArray.FromEnum(EVehicleType) )]
	EVehicleType m_eVehicleType;
	
	SCR_VehicleFactionAffiliationComponent m_pFactionComponent;
	protected SCR_WaterPhysicsComponent m_WaterPhysics;
	protected BaseControllerComponent m_VehicleController;
	protected SCR_ResourceComponent m_ResourceComponent;
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnPhysicsActive()
	{
		if (!m_OnPhysicsActive)
			m_OnPhysicsActive = new ScriptInvoker();
		
		return m_OnPhysicsActive;
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnPhysicsActive(IEntity owner, bool activeState)
	{
		if (m_OnPhysicsActive)
			m_OnPhysicsActive.Invoke(owner, activeState);
	}
	
	//------------------------------------------------------------------------------------------------
	Faction GetFaction()
	{
		if (m_pFactionComponent)
			return m_pFactionComponent.GetAffiliatedFaction();
		
		return null;
	}	
	
	//------------------------------------------------------------------------------------------------
	SCR_VehicleFactionAffiliationComponent GetFactionAffiliation()
	{
		return m_pFactionComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_WaterPhysicsComponent GetWaterPhysicsComponent()
	{
		return m_WaterPhysics;
	}
	
	//------------------------------------------------------------------------------------------------
	Faction GetDefaultFaction()
	{
		if (m_pFactionComponent)
			return m_pFactionComponent.GetDefaultAffiliatedFaction();
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsOccupied()
	{	
		if (m_pFactionComponent)
			return m_pFactionComponent.IsVehicleOccupied();

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get vehicle controller, stored as BaseControllerComponent
	BaseControllerComponent GetVehicleController()
	{
		return m_VehicleController;
	}

	//------------------------------------------------------------------------------------------------
	//! Get primary pilot compartment slot
	PilotCompartmentSlot GetPilotCompartmentSlot()
	{
		if (GetGame().GetIsClientAuthority())
		{
			VehicleControllerComponent controllerCA = VehicleControllerComponent.Cast(m_VehicleController);
			if (controllerCA)
				return controllerCA.GetPilotCompartmentSlot();
		}
		else
		{
			VehicleControllerComponent_SA controller = VehicleControllerComponent_SA.Cast(m_VehicleController);
			if (controller)
				return controller.GetPilotCompartmentSlot();
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Get pilot of the vehicle - occupant of the primary pilot compartment
	IEntity GetPilot()
	{
		PilotCompartmentSlot slot = GetPilotCompartmentSlot();
		if (slot)
			return slot.GetOccupant();

		return null;
	}

	//------------------------------------------------------------------------------------------------
	SCR_ResourceComponent GetResourceComponent()
	{
		return m_ResourceComponent;
	}

	//------------------------------------------------------------------------------------------------
	void UpdateResourceComponent()
	{
		m_ResourceComponent = SCR_ResourceComponent.FindResourceComponent(this);
	}

	//------------------------------------------------------------------------------------------------
	void Vehicle(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.PHYSICSACTIVE);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		if (GetGame().GetIsClientAuthority())
			m_VehicleController = BaseControllerComponent.Cast(FindComponent(VehicleControllerComponent));
		else
			m_VehicleController = BaseControllerComponent.Cast(FindComponent(VehicleControllerComponent_SA));

		m_pFactionComponent = SCR_VehicleFactionAffiliationComponent.Cast(FindComponent(SCR_VehicleFactionAffiliationComponent));
		m_WaterPhysics = SCR_WaterPhysicsComponent.Cast(FindComponent(SCR_WaterPhysicsComponent));
		UpdateResourceComponent();
	}
};
