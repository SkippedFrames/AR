class SCR_AIClearAgentsForRadialCover : AITaskScripted
{
	protected SCR_AIGroupUtilityComponent m_groupUtilityComponent;
	
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
			
		SCR_AIDefendActivity defendActivity = SCR_AIDefendActivity.Cast(m_groupUtilityComponent.GetCurrentAction());
		if (!defendActivity)
			return NodeError(this, owner, "Not running defend activity!");
		
		defendActivity.ClearRadialCoverAgents();	
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
		return "Clears the array of radial cover agents";
	}		

};
