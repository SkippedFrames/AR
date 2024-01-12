class SCR_AIDefendActivity : SCR_AIActivityBase
{
	ref SCR_BTParam<AIWaypoint> m_Waypoint = new ref SCR_BTParam<AIWaypoint>(SCR_AIActionTask.WAYPOINT_PORT);
	ref SCR_BTParam<vector> m_vAttackLocation = new ref SCR_BTParam<vector>(SCR_AIActionTask.ATTACK_LOCATION_PORT);	
	
	protected ref array<AIAgent> m_aRadialCoverAgents = {};
	//-------------------------------------------------------------------------------------------------
	void InitParameters(AIWaypoint waypoint, float priorityLevel)
	{
		m_Waypoint.Init(this, waypoint);
		m_vAttackLocation.Init(this, vector.Zero);
		m_fPriorityLevel.Init(this, priorityLevel);
	}
	
	//-------------------------------------------------------------------------------------------------
	void SCR_AIDefendActivity(SCR_AIGroupUtilityComponent utility, AIWaypoint relatedWaypoint, AIWaypoint waypoint, vector attackLocation, float priority = PRIORITY_ACTIVITY_DEFEND, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		InitParameters(waypoint, priorityLevel);
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Group/ActivityDefend.bt";
		SetPriority(priority);
		if (!waypoint) 
		{
			m_vAttackLocation.m_Value = attackLocation;
			return;
		}
		else
		{
			float waypointRadius = waypoint.GetCompletionRadius();
			vector waypointPositionWorld = waypoint.GetOrigin(); 
			if (attackLocation == vector.Zero) // use orientation of the waypoint
			{
				float rotationAngle = waypoint.GetAngles()[1] * Math.DEG2RAD;
				vector attackPositionWorld;
				attackPositionWorld[0] = waypointPositionWorld[0] + Math.Sin(rotationAngle) * waypointRadius;
				attackPositionWorld[2] = waypointPositionWorld[2] + Math.Cos(rotationAngle) * waypointRadius;
				attackPositionWorld[1] = waypointPositionWorld[1];
				m_vAttackLocation.m_Value = attackPositionWorld; // attack direction is a point on the circumference of waypoint given by the orientation of the waypoint
			}
			else
			{
				m_vAttackLocation.m_Value =  (attackLocation - waypointPositionWorld).Normalized() * waypointRadius + waypointPositionWorld;
			}
		}
	}
	
	//-------------------------------------------------------------------------------------------------
	void AddAgentToRadialCover (AIAgent agent)
	{
		if (m_aRadialCoverAgents.Find(agent) == -1)
			m_aRadialCoverAgents.Insert(agent);	
	}
	
	//-------------------------------------------------------------------------------------------------
	void ClearRadialCoverAgents ()
	{
		m_aRadialCoverAgents.Clear();
	}
	
	//-------------------------------------------------------------------------------------------------
	int FindRadialCoverAgent (AIAgent agent)
	{
		return m_aRadialCoverAgents.Find(agent);
	}
	
	//-------------------------------------------------------------------------------------------------
	int GetRadialCoverAgentsCount ()
	{
		return m_aRadialCoverAgents.Count();
	}
	
	//-------------------------------------------------------------------------------------------------
	override string GetActionDebugInfo()
	{
		return this.ToString() + " defending area of " + m_Waypoint.ToString();
	}
	
	//-------------------------------------------------------------------------------------------------
	override void OnActionDeselected()
	{
		SCR_AIGroup group = SCR_AIGroup.Cast(m_Utility.GetAIAgent());
		if (group)
			group.ReleaseCompartments();
		SendCancelMessagesToAllAgents();
	}
	
	//-------------------------------------------------------------------------------------------------
	override void OnActionFailed()
	{
		SCR_AIGroup group = SCR_AIGroup.Cast(m_Utility.GetAIAgent());
		if (group)
			group.ReleaseCompartments();
		SendCancelMessagesToAllAgents();
	}
	
	//-------------------------------------------------------------------------------------------------
	override void OnActionCompleted()
	{
		SendCancelMessagesToAllAgents();
	}
};
