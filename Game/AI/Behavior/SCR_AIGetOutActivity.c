class SCR_AIGetOutActivity : SCR_AIActivityBase
{
	ref SCR_BTParam<IEntity> m_Vehicle = new SCR_BTParam<IEntity>(SCR_AIActionTask.ENTITY_PORT);
	
	void InitParameters(IEntity vehicle, float priorityLevel)
	{
		m_Vehicle.Init(this, vehicle);
		m_fPriorityLevel.Init(this, priorityLevel);
	}
	
	void SCR_AIGetOutActivity(SCR_AIGroupUtilityComponent utility, AIWaypoint relatedWaypoint, IEntity vehicle, float priority = PRIORITY_ACTIVITY_GET_OUT, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		InitParameters(vehicle, priorityLevel);
		
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Group/ActivityGetOut.bt";
		SetPriority(priority);
	}
	
	override string GetActionDebugInfo()
	{
		return this.ToString() + " leaving " + m_Vehicle.ValueToString();
	}
};

