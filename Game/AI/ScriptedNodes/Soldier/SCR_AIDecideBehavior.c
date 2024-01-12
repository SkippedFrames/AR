class SCR_AIDecideBehavior: AITaskScripted
{
	SCR_AIBehaviorBase		m_PreviousBehavior;
	SCR_AIBehaviorBase		m_CurrentBehavior;	
	SCR_AIUtilityComponent	m_UtilityComponent;	
	
	//------------------------------------------------------------------------------------------------
	override bool VisibleInPalette() {return true;}
	
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_UtilityComponent)
			return ENodeResult.FAIL;
		
		BaseTarget unknownTarget;		
		GetVariableIn("UnknownTarget", unknownTarget);

		m_CurrentBehavior = m_UtilityComponent.EvaluateBehavior(unknownTarget);
		if (!m_CurrentBehavior || m_CurrentBehavior.m_sBehaviorTree == ResourceName.Empty)
		{
			Print("AI: Missing behavior tree in " + m_CurrentBehavior.ToString(), LogLevel.WARNING);
			return ENodeResult.FAIL;
		}

		if (m_PreviousBehavior != m_CurrentBehavior)
		{
			SetVariableOut("BehaviorTree", m_CurrentBehavior.m_sBehaviorTree);
			SetVariableOut("UpdateBehavior", true);
		}		
				
		m_PreviousBehavior = m_CurrentBehavior;
		return ENodeResult.SUCCESS;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		"BehaviorTree",
		"UpdateBehavior"
	};
	override TStringArray GetVariablesOut()
    {
        return s_aVarsOut;
    }
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		"UnknownTarget"
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		m_UtilityComponent = SCR_AIUtilityComponent.Cast(owner.FindComponent(SCR_AIUtilityComponent));
		if (!m_UtilityComponent)
		{
			NodeError(this, owner, "Can't find utility component.");
		}
	}
};
