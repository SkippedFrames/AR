class SCR_AIGetHoldingTime : AITaskScripted
{
	
	protected static ref TStringArray s_aVarsOut = {
		"HoldingTimeOut"
	};
	override TStringArray GetVariablesOut()
    {
        return s_aVarsOut;
    }
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIGroup group = AIGroup.Cast(owner);
		if (!group)
		{
			SCR_AgentMustBeAIGroup(this, owner);
			return ENodeResult.FAIL;
		}

		SCR_TimedWaypoint wp = SCR_TimedWaypoint.Cast(group.GetCurrentWaypoint());
		if ( wp )
		{
			SetVariableOut("HoldingTimeOut",wp.GetHoldingTime());
			return ENodeResult.SUCCESS;
		}				
		
		return ENodeResult.FAIL;
	}

	protected override bool VisibleInPalette()
	{
		return true;
	}	
	
	protected override string GetOnHoverDescription()
	{
		return "Gets holding time set on current timed waypoint";
	}		

};
