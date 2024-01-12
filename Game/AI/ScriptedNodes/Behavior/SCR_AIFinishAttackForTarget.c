class SCR_AIFinishAttackForTarget : AITaskScripted
{
	[Attribute("1", UIWidgets.ComboBox, "Behavior result", "", ParamEnumArray.FromEnum(ENodeResult) )]
	private ENodeResult m_eBehaviorResult;
	
	static const string PORT_TARGET = "BaseTargetIn";

	protected SCR_AIUtilityComponent m_UtilityComponent;
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_UtilityComponent)
		{
			m_UtilityComponent = SCR_AIUtilityComponent.Cast(owner.FindComponent(SCR_AIUtilityComponent));
			if (!m_UtilityComponent)
				return ENodeResult.FAIL;
		};
		BaseTarget target;
		if (!GetVariableIn(PORT_TARGET,target))
		{
			NodeError(this,owner, "No base target provided!");
			return ENodeResult.FAIL;
		}		
		if (m_UtilityComponent.FinishAttackForTarget(target, m_eBehaviorResult))
			return ENodeResult.SUCCESS;
		return ENodeResult.FAIL;
	}

	//------------------------------------------------------------------------------------------------
	protected override bool VisibleInPalette()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_TARGET
	};
	
	//------------------------------------------------------------------------------------------------
    override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
	
	//------------------------------------------------------------------------------------------------
	override protected string GetNodeMiddleText()
	{
		string result;
		if (m_eBehaviorResult == ENodeResult.SUCCESS)
			result = "COMPLETE";
		else
			result = "FAIL";
		return "Attack behavior will: " + result;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override string GetOnHoverDescription()
	{
		return "Completes or Fails attack of current enemy given by provided base target, node fails if attack couldnt be finished.";
	}
};
