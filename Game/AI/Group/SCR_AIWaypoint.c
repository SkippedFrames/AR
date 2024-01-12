class SCR_AIWaypointClass: AIWaypointClass
{
};

class SCR_AIWaypoint : AIWaypoint
{
	[Attribute("0", UIWidgets.SpinBox, "Waypoint priority level", "0 2000 1000")]
    private float m_fPriorityLevel;
	
	//------------------------------------------------------------------------------------------------
	float GetPriorityLevel()
	{
		return m_fPriorityLevel;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPriorityLevel(float priority)
	{
		m_fPriorityLevel = priority;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsWithinCompletionRadius(vector pos)
	{
		return vector.DistanceXZ(GetOrigin(), pos) < GetCompletionRadius();
	}
	
	//------------------------------------------------------------------------------------------------
	// Override this to create a custom waypoint state object.
	SCR_AIWaypointState CreateWaypointState(SCR_AIGroupUtilityComponent groupUtilityComp)
	{
		return new SCR_AIWaypointState(groupUtilityComp, this);
	}
};
