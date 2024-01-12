class SCR_AIGetLookParameters : AITaskScripted
{
	static const string PORT_LOOK = "bLook";
	static const string PORT_CANCEL = "bCancel";
	static const string PORT_RESTART = "bRestart";
	static const string PORT_POSITION = "vPosition";
	static const string PORT_DURATION = "vDuration";
	
	SCR_AILookAction m_LookAction;
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_LookAction)
			return ENodeResult.FAIL;

		return m_LookAction.GetLookParametersToNode(this);
	}

	protected override bool VisibleInPalette() {return true;}
	protected override string GetOnHoverDescription() {return "Gets parameters of a lookAction";}
	
	override void OnInit(AIAgent owner)
	{
		SCR_AIUtilityComponent utility = SCR_AIUtilityComponent.Cast(owner.FindComponent(SCR_AIUtilityComponent));
		if (utility)
			m_LookAction = utility.m_LookAction;
	}

	protected static ref TStringArray s_aVarsOut = {
		PORT_LOOK,
		PORT_CANCEL,
		PORT_RESTART,
		PORT_POSITION,
		PORT_DURATION
	};
	override TStringArray GetVariablesOut()
	{
		return s_aVarsOut;
	}
};
