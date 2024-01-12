class SCR_AIGetPriorityFromWaypoint : AITaskScripted
{
	static const string PORT_PRIORITY_LEVEL = "PriorityLevel";
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_PRIORITY_LEVEL
	};
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
		
		SCR_AIWaypoint wp = SCR_AIWaypoint.Cast(group.GetCurrentWaypoint());
		if (!wp)
		{
			return ENodeResult.FAIL;
		}
		
		SetVariableOut(PORT_PRIORITY_LEVEL, wp.GetPriorityLevel());
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
		return "Returns if waypoint is priority, i.e. autonomous behavior of AI will be ignored.";
	}
};
