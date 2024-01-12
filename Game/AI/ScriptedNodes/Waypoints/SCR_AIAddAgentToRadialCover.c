class SCR_AIAddAgentToRadialCover : AITaskScripted
{
	static const string PORT_AGENT = "Agent";
	
	protected SCR_AIGroupUtilityComponent m_groupUtilityComponent;
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_AGENT
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		m_groupUtilityComponent = SCR_AIGroupUtilityComponent.Cast(owner.FindComponent(SCR_AIGroupUtilityComponent));			
	}	
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_groupUtilityComponent)
			return NodeError(this,owner, "AIAgent does not have group utility component!");
			
		AIAgent defender;
		if (!GetVariableIn(PORT_AGENT, defender))
		{
			return NodeError(this, owner, "No agent provided");
		};
		
		
		SCR_AIDefendActivity defendActivity = SCR_AIDefendActivity.Cast(m_groupUtilityComponent.GetCurrentAction());
		if (!defendActivity)
			return NodeError(this, owner, "Not running defend activity!");
		
		defendActivity.AddAgentToRadialCover(defender);		
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
		return "Adds unit to list of covering units of defend waypoint";
	}		

};
