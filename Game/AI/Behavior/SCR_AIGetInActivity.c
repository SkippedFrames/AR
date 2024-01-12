class SCR_AIGetInActivity : SCR_AIActivityBase
{
	ref SCR_BTParam<IEntity> m_Vehicle = new SCR_BTParam<IEntity>(SCR_AIActionTask.ENTITY_PORT);
	ref SCR_BTParam<ECompartmentType> m_eRoleInVehicle = new SCR_BTParam<ECompartmentType>(SCR_AIActionTask.ROLEINVEHICLE_PORT);	
	protected SCR_AIGroup m_OwnerGroup;
	
	void InitParameters(IEntity vehicle, ECompartmentType role, float priorityLevel)
	{
		m_Vehicle.Init(this, vehicle);
		m_eRoleInVehicle.Init(this, role);
		m_fPriorityLevel.Init(this, priorityLevel);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_AIGetInActivity(SCR_AIGroupUtilityComponent utility, AIWaypoint relatedWaypoint, IEntity vehicle, ECompartmentType role = ECompartmentType.Cargo, float priority = PRIORITY_ACTIVITY_GET_IN, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		InitParameters(vehicle, role, priorityLevel);
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Group/ActivityGetIn.bt";
		SetPriority(priority);
		if (utility)
			m_OwnerGroup = SCR_AIGroup.Cast(utility.GetAIAgent());
	}
	
	//------------------------------------------------------------------------------------------------
	override string GetActionDebugInfo()
	{
		return this.ToString() + " moving to " + m_Vehicle.ValueToString() + " as " + m_eRoleInVehicle.ValueToString();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionDeselected()
	{
		super.OnActionDeselected();
		m_OwnerGroup.ReleaseCompartments();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionCompleted()
	{
		super.OnActionCompleted();
		m_OwnerGroup.ReleaseCompartments();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionFailed()
	{
		super.OnActionFailed();
		m_OwnerGroup.ReleaseCompartments();
		m_OwnerGroup.RemoveUsableVehicle(m_Vehicle.m_Value);
	}
};
