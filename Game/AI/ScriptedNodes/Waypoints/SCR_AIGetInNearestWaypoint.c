class SCR_AIGetInNearestWaypoint : SCR_AIGetWaypoint
{
	static const string PORT_WAYPOINT_PARAMS = "WaypointParams";	
	static const string PORT_ORIGIN 	= "Origin";
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut2 = {
		RADIUS_PORT,
		PORT_ORIGIN,
		PORT_WAYPOINT_PARAMS
	};
	override TStringArray GetVariablesOut()
    {
        return s_aVarsOut2;
    }
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (ENodeResult.FAIL == super.EOnTaskSimulate(owner, dt))
			return ENodeResult.FAIL;
		
		SCR_BoardingWaypoint wp = SCR_BoardingWaypoint.Cast(m_Waypoint);
		if (!wp)
		{
			return NodeError(this, owner, "Wrong class of provided Waypoint!");
		}
		
		SetVariableOut(PORT_WAYPOINT_PARAMS,wp.GetAllowance());
		SetVariableOut(PORT_ORIGIN,wp.GetOrigin());
		
		return ENodeResult.SUCCESS;
	}

	//------------------------------------------------------------------------------------------------
	protected override bool VisibleInPalette()
	{
		return true;
	}	
	
	//------------------------------------------------------------------------------------------------
	protected override string GetOnHoverDescription()
	{
		return "Returns get in nearest waypoint parameters";
	}		

};
