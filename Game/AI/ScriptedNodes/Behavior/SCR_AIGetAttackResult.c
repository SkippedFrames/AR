class SCR_AIGetAttackResult : AITaskScripted
{
	static const string PORT_BASE_TARGET = "BaseTargetOut";
	static const string PORT_ENTITY = "EntityOut";
	static const string PORT_POSITION = "LastSeenPositionOut";
	static const string PORT_CONTEXT = "ContextOut";
	static const string PORT_SHOULD_INVESTIGATE = "ShouldInvestigateOut";
	
	protected SCR_AIUtilityComponent m_Utility;	
	
	//------------------------------------------------------------------------------------------------
	override void OnEnter(AIAgent owner)
	{
		m_Utility = SCR_AIUtilityComponent.Cast(owner.FindComponent(SCR_AIUtilityComponent));
		if (!m_Utility)
			NodeError(this, owner, "Can't find utility component on this agent.");
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		BaseTarget target;
		string context;
		bool shouldInvestigate;
		
		if (m_Utility && m_Utility.ShouldAttackEnd(target,shouldInvestigate,context))
		{
			SetVariableOut(PORT_BASE_TARGET, target);
			SetVariableOut(PORT_ENTITY, target.GetTargetEntity());
			SetVariableOut(PORT_POSITION, target.GetLastSeenPosition());
			SetVariableOut(PORT_CONTEXT, context);
			SetVariableOut(PORT_SHOULD_INVESTIGATE, shouldInvestigate);
			return ENodeResult.FAIL;
		};
		if (target)
		{
			SetVariableOut(PORT_BASE_TARGET, target);
			SetVariableOut(PORT_ENTITY, target.GetTargetEntity());
			SetVariableOut(PORT_POSITION, target.GetLastSeenPosition());
			ClearVariable(PORT_SHOULD_INVESTIGATE);
			SetVariableOut(PORT_CONTEXT, context);
			return ENodeResult.SUCCESS;
		}
		ClearVariables();
		SetVariableOut(PORT_CONTEXT, context);
		return ENodeResult.SUCCESS;
	};

	//------------------------------------------------------------------------------------------------
	void ClearVariables()
	{
		foreach (string var: s_aVarsOut)
			ClearVariable(var);
	};
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_BASE_TARGET,
		PORT_ENTITY,
		PORT_POSITION,
		PORT_SHOULD_INVESTIGATE,
		PORT_CONTEXT
	};
	
	//------------------------------------------------------------------------------------------------
    override TStringArray GetVariablesOut()
    {
        return s_aVarsOut;
    }
	
	
	//------------------------------------------------------------------------------------------------
	protected override bool VisibleInPalette()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected override string GetOnHoverDescription()
	{
		return "Returns FAIL when attack should fail for current enemy, returns SUCCESS when no attack in progress or attack should continue";
	}

};
