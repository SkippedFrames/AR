class SCR_AIGetDefendWaypoint : SCR_AIGetWaypoint
{
	static const string USE_TURRETS_PORT = "UseTurrets";
	static const string SEARCH_TAGS_PORT = "SearchTags";
	static const string FAST_INIT_PORT = "FastInit";
	
	ref array<string> m_tagsArray;
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut2 = {
		RADIUS_PORT,
		USE_TURRETS_PORT,
		SEARCH_TAGS_PORT,
		FAST_INIT_PORT
	};
	
	//------------------------------------------------------------------------------------------------
	override TStringArray GetVariablesOut()
    {
        return s_aVarsOut2;
    }
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (ENodeResult.FAIL == super.EOnTaskSimulate(owner, dt))
			return ENodeResult.FAIL;
		
		SCR_DefendWaypoint wp = SCR_DefendWaypoint.Cast(m_Waypoint);
		if (!wp)
		{
			return NodeError(this, owner, "Wrong class of provided Waypoint!");
		}
		
		m_tagsArray = wp.GetTagsForSearch();
		
		SetVariableOut(USE_TURRETS_PORT, wp.GetUseTurrets());
		SetVariableOut(SEARCH_TAGS_PORT, m_tagsArray);
		SetVariableOut(FAST_INIT_PORT, wp.GetFastInit());
		
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
		return "Returns defend waypoint parameters";
	}
};
