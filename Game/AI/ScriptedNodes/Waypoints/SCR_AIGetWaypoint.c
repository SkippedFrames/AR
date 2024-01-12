class SCR_AIGetWaypoint : AITaskScripted
{
	static const string WAYPOINT_PORT = "WaypointIn";
	static const string RADIUS_PORT = "RadiusOut";
		
	protected AIWaypoint m_Waypoint;
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		WAYPOINT_PORT
	};
	
	//------------------------------------------------------------------------------------------------
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		RADIUS_PORT
	};
	
	//------------------------------------------------------------------------------------------------
	override TStringArray GetVariablesOut()
    {
        return s_aVarsOut;
    }
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIGroup group = AIGroup.Cast(owner);
		if (!group)
		{
			SCR_AgentMustBeAIGroup(this, owner);
			return ENodeResult.FAIL;
		}
		
		if (!GetVariableIn(WAYPOINT_PORT,m_Waypoint))
			m_Waypoint = group.GetCurrentWaypoint();
		if (!m_Waypoint)
		{
			ClearVariable(RADIUS_PORT);
			return ENodeResult.FAIL;
		}
		
		SetVariableOut(RADIUS_PORT, m_Waypoint.GetCompletionRadius());		
		return ENodeResult.SUCCESS;
	}

	//------------------------------------------------------------------------------------------------
	protected override bool VisibleInPalette()
	{
		return false;
	}
};
