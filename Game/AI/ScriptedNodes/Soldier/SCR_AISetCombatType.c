class SCR_AISetCombatType : AITaskScripted
{
	[Attribute("", UIWidgets.ComboBox, "Combat type to set:", "", ParamEnumArray.FromEnum(EAICombatType) )]
	private EAICombatType m_eCombatType	
	
	protected SCR_AICombatComponent m_CombatComponent;
	
	static const string PORT_COMBAT_TYPE = "ActionIn";	
		
	override void OnInit(AIAgent owner)
	{
		IEntity entity = owner.GetControlledEntity();
		if (!entity)
		{
			NodeError(this, owner, "Can't find Controlled entity.");
			return;
		}	
		m_CombatComponent = SCR_AICombatComponent.Cast(entity.FindComponent(SCR_AICombatComponent));
		if (!m_CombatComponent)
			NodeError(this, owner, "Can't find Combat component.");		
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_CombatComponent)
			return ENodeResult.FAIL;
		
		EAICombatType combatType;
					
		if (!GetVariableIn(PORT_COMBAT_TYPE,combatType))
			combatType = m_eCombatType;
					
		m_CombatComponent.SetCombatType(combatType);		

		return ENodeResult.SUCCESS;
	}
	//------------------------------------------------------------------------------------------------
    override bool VisibleInPalette() 
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_COMBAT_TYPE
	};
    override array<string> GetVariablesIn()
    {
        return s_aVarsIn;
    }
	//------------------------------------------------------------------------------------------------	
	
	override string GetNodeMiddleText() 
	{ 
		return "Combat type: " + typename.EnumToString(EAICombatType,m_eCombatType);	
	};
	
	//------------------------------------------------------------------------------------------------
	protected override string GetOnHoverDescription()
	{
		return "Allows or disallows a combat action on a CombatComponent of a character";
	}	
};
