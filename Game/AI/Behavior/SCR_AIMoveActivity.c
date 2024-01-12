class SCR_AIMoveActivity : SCR_AIActivityBase
{
	protected static const string MOVEMENT_TYPE_PORT = "MovementType";
	
	ref SCR_BTParamAssignable<vector> m_vPosition = new SCR_BTParamAssignable<vector>(SCR_AIActionTask.POSITION_PORT);
	ref SCR_BTParam<IEntity> m_Entity = new SCR_BTParam<IEntity>(SCR_AIActionTask.ENTITY_PORT);
	ref SCR_BTParam<bool> m_bUseVehicles = new SCR_BTParam<bool>(SCR_AIActionTask.USE_VEHICLES_PORT);
	ref SCR_BTParam<EMovementType> m_eMovementType = new SCR_BTParam<EMovementType>(MOVEMENT_TYPE_PORT);
	
	//------------------------------------------------------------------------------------------------
	void InitParameters(vector position, IEntity entity, EMovementType movementType, bool useVehicles, float priorityLevel)
	{
		m_vPosition.Init(this, position);
		m_vPosition.m_AssignedOut = (position != vector.Zero);
		m_Entity.Init(this, entity);
		m_bUseVehicles.Init(this, useVehicles);
		m_eMovementType.Init(this, movementType);
		m_fPriorityLevel.Init(this, priorityLevel);	
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_AIMoveActivity(SCR_AIGroupUtilityComponent utility, AIWaypoint relatedWaypoint, vector pos, IEntity ent, EMovementType movementType = EMovementType.RUN, bool useVehicles = true, float priority = PRIORITY_ACTIVITY_MOVE, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		InitParameters(pos, ent, movementType, useVehicles, priorityLevel);
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Group/ActivityMove.bt";
		SetPriority(priority);
	}
	
	//------------------------------------------------------------------------------------------------
	override string GetActionDebugInfo()
	{
		return this.ToString() + " moving to " + m_vPosition.ValueToString();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionCompleted()
	{
		super.OnActionCompleted();
		if (m_bUseVehicles.m_Value)
		{
			SCR_AIGroup group = SCR_AIGroup.Cast(m_Utility.GetAIAgent());
			if (!group)
				return;
			group.ReleaseCompartments();
		}
		SendCancelMessagesToAllAgents();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionFailed()
	{
		super.OnActionFailed();
		if (m_bUseVehicles.m_Value)
		{
			SCR_AIGroup group = SCR_AIGroup.Cast(m_Utility.GetAIAgent());
			if (!group)
				return;
			group.ReleaseCompartments();
		}
		SendCancelMessagesToAllAgents();
	}
	
	override void OnActionDeselected()
	{
		super.OnActionDeselected();
		SendCancelMessagesToAllAgents();
	}
};

class SCR_AISeekAndDestroyActivity : SCR_AIMoveActivity
{
	void SCR_AISeekAndDestroyActivity(SCR_AIGroupUtilityComponent utility, AIWaypoint relatedWaypoint, vector pos, IEntity ent, EMovementType movementType = EMovementType.RUN, bool useVehicles = false, float priority = PRIORITY_ACTIVITY_SEEK_AND_DESTROY, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		//m_bRemoveOnCompletion = false; removed: after S&D move was not realized, completed S&D was not removed
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Group/ActivitySeekDestroy.bt";
	}
	
	override string GetActionDebugInfo()
	{
		return this.ToString() + " seek and destroy around" + m_Entity.ValueToString();
	}
};

class SCR_AIFollowActivity : SCR_AIMoveActivity
{
	ref SCR_BTParam<float> m_fCompletionDistance = new SCR_BTParam<float>(SCR_AIActionTask.DESIREDDISTANCE_PORT);
	
	// SCR_AIBaseUtilityComponent utility, bool isWaypointRelated, vector pos, float priority = PRIORITY_ACTIVITY_MOVE, IEntity ent = null, bool useVehicles = true
	void SCR_AIFollowActivity(SCR_AIGroupUtilityComponent utility, AIWaypoint relatedWaypoint, vector pos, IEntity ent, EMovementType movementType = EMovementType.RUN, bool useVehicles = false, float priority = PRIORITY_ACTIVITY_FOLLOW, float priorityLevel = PRIORITY_LEVEL_NORMAL, float distance = 1.0)
	{
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Group/ActivityFollow.bt";
		m_fCompletionDistance.Init(this, distance);
	}
	
	override string GetActionDebugInfo()
	{
		return this.ToString() + " following entity " + m_Entity.ValueToString();
	}
};
