class SCR_AIGetRadialAngleForAgent : AITaskScripted
{
	static const string PORT_AGENT = "AgentIn";
	static const string PORT_ATTACK_LOCATION = "AttackLocationIn";
	static const string PORT_DIRECTION_ANGLE = "AgentDiractionOut";
	static const string PORT_ANGULAR_RANGE = "AngularRangeOut";
	
	protected SCR_AIGroupUtilityComponent m_groupUtilityComponent;
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_AGENT,
		PORT_ATTACK_LOCATION
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
		
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_DIRECTION_ANGLE,PORT_ANGULAR_RANGE 
	};
	override TStringArray GetVariablesOut()
    {
        return s_aVarsOut;
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
		
		int index = defendActivity.FindRadialCoverAgent(defender);
		int count = defendActivity.GetRadialCoverAgentsCount();
		if (index < 0 || count == 0)
		{
			ClearVariable(PORT_DIRECTION_ANGLE);
			ClearVariable(PORT_ANGULAR_RANGE);
			return ENodeResult.FAIL;
		};	
		
		vector attackLocation, originOfLocalSpace = defendActivity.m_Waypoint.m_Value.GetOrigin();
		if (!GetVariableIn(PORT_ATTACK_LOCATION, attackLocation))
			return NodeError(this, owner, "No attack direction provided!");
		
		vector directionToDefend = attackLocation - originOfLocalSpace;
		float angleToDefend = directionToDefend.ToYaw();
		float sector = 360 / count;
		float length = directionToDefend.Length();
		int directionAngle = Math.Round(angleToDefend + sector * index);
		directionAngle = directionAngle % 360;
		directionToDefend[0] = Math.Sin(directionAngle * Math.DEG2RAD) * length;
		directionToDefend[2] = Math.Cos(directionAngle * Math.DEG2RAD) * length;		
		SetVariableOut(PORT_DIRECTION_ANGLE, directionToDefend + originOfLocalSpace);
		SetVariableOut(PORT_ANGULAR_RANGE, sector/2);		
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
		return "Returns direction of defending and angle of sector covarage centered around the direction. If provided agent is not registered for sector coverage it FAILS";
	}

};
