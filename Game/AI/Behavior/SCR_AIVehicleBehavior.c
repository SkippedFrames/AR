enum EAICompartmentType
{
	None = -1,
	Pilot,
	Turret,
	Cargo,
}

class SCR_AIVehicleBehavior : SCR_AIBehaviorBase
{
	ref SCR_BTParam<IEntity> m_Vehicle = new SCR_BTParam<IEntity>(SCR_AIActionTask.ENTITY_PORT);
	
	void SCR_AIVehicleBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, IEntity vehicleEntity)
	{
		m_Vehicle.Init(m_aParams, vehicleEntity);
	}
	
	override void OnActionSelected() 
	{
		super.OnActionSelected();
		m_Utility.m_AIInfo.SetAIState(EUnitAIState.BUSY);
	}	
	
	override void OnActionCompleted() 
	{
		m_Utility.m_AIInfo.SetAIState(EUnitAIState.AVAILABLE);
		super.OnActionCompleted();
	}
	
	override void OnActionFailed() 
	{
		m_Utility.m_AIInfo.SetAIState(EUnitAIState.AVAILABLE);
		super.OnActionFailed();
	}
};

class SCR_AIGetInVehicle : SCR_AIVehicleBehavior
{
	ref SCR_BTParam<EAICompartmentType> m_eRoleInVehicle = new SCR_BTParam<EAICompartmentType>(SCR_AIActionTask.ROLEINVEHICLE_PORT);
	ref SCR_BTParam<BaseCompartmentSlot> m_CompartmentToGetIn = new SCR_BTParam<BaseCompartmentSlot>(SCR_AIActionTask.COMPARTMENT_PORT);
	
	void InitParameters(ECompartmentType role, BaseCompartmentSlot slot)
	{
		m_eRoleInVehicle.Init(this, role);
		m_CompartmentToGetIn.Init(this, slot);
	}
	
	void SCR_AIGetInVehicle(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, IEntity vehicleEntity, EAICompartmentType role = ECompartmentType.Cargo, float priority = PRIORITY_BEHAVIOR_VEHICLE, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		BaseCompartmentSlot slot;
		InitParameters(role, slot);
		
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/GetInVehicle.bt";
		SetPriority(priority);
		m_fPriorityLevel.m_Value = priorityLevel;
		if (vehicleEntity)
		{
			BaseCompartmentManagerComponent compManager = BaseCompartmentManagerComponent.Cast(vehicleEntity.FindComponent(BaseCompartmentManagerComponent));
			if (!compManager)
			{
				Fail();
				return;
			};
			
			IEntity agentEntity = m_Utility.m_OwnerEntity;
			if (!agentEntity)
			{
				Fail();
				return;
			}
			
			array<BaseCompartmentSlot> compartments = {};
			compManager.GetCompartments(compartments);
			
			foreach (BaseCompartmentSlot comp: compartments)
			{
				if (SCR_AICompartmentHandling.CompartmentClassToType(comp.Type()) == m_eRoleInVehicle.m_Value)
				{
					IEntity occupant = comp.GetOccupant();
					if (!comp.IsReserved() && (!occupant || occupant == agentEntity))
					{
						comp.SetReserved(agentEntity);
						comp.SetCompartmentAccessible(true);
						m_CompartmentToGetIn.m_Value = comp;
						break;
					}
				}
			}
		}
	}
	
	override void OnActionCompleted() 
	{
		if (m_CompartmentToGetIn.m_Value)
		{
			m_CompartmentToGetIn.m_Value.SetCompartmentAccessible(true);
			m_CompartmentToGetIn.m_Value.SetReserved(null);
		};
		super.OnActionCompleted();
	}
	
	override void OnActionFailed() 
	{
		if (m_CompartmentToGetIn.m_Value)
		{
			m_CompartmentToGetIn.m_Value.SetCompartmentAccessible(true);
			m_CompartmentToGetIn.m_Value.SetReserved(null);
		};
		super.OnActionFailed();
	}
	
	override string GetActionDebugInfo()
	{
		return this.ToString() + " moving to " + m_Vehicle.ValueToString() + " as " + m_eRoleInVehicle.ValueToString();
	}
};

class SCR_AIGetOutVehicle : SCR_AIVehicleBehavior
{
	
	void SCR_AIGetOutVehicle(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, IEntity vehicleEntity, float priority = PRIORITY_BEHAVIOR_GET_OUT_VEHICLE, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/GetOutVehicle.bt";
		SetPriority(priority);
		m_fPriorityLevel.m_Value = priorityLevel;
	}
	
	override string GetActionDebugInfo()
	{
		return this.ToString() + " leaving " + m_Vehicle.ValueToString();
	}
};
