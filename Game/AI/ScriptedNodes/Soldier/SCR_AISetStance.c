class SCR_AISetStance : SCR_AICharacterStats
{
	[Attribute("0", UIWidgets.ComboBox, "Desired character stance", "", ParamEnumArray.FromEnum(ECharacterStance) )]
	private ECharacterStance m_eStance;
	
	[Attribute("0", UIWidgets.CheckBox, "Should reset stance to Standing when node is aborted")]
	private bool m_bStandupOnAbort;

	static const string STANCE_PORT = "Stance";
	
	private CharacterControllerComponent m_CharacterController;

	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_AIInfo || !m_CharacterController)
			return ENodeResult.FAIL;
		
		ECharacterStance stance;
		
		if(!GetVariableIn(STANCE_PORT,stance))
			stance = m_eStance;
		
		m_AIInfo.SetStance(stance);
		m_CharacterController.SetStanceChange(ConvertStanceToInt(stance));

		return ENodeResult.SUCCESS;
	}

	//------------------------------------------------------------------------------------------------
	int ConvertStanceToInt(ECharacterStance stance)
	{
		switch (stance)
		{
			case ECharacterStance.STAND:
				return ECharacterStanceChange.STANCECHANGE_TOERECTED;
			case ECharacterStance.CROUCH:
				return ECharacterStanceChange.STANCECHANGE_TOCROUCH;
			case ECharacterStance.PRONE:
				return ECharacterStanceChange.STANCECHANGE_TOPRONE;
		}
		return 0;
	}

	//------------------------------------------------------------------------------------------------
    override bool VisibleInPalette() {return true;}

	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		STANCE_PORT
	};
    override array<string> GetVariablesIn()
    {
        return s_aVarsIn;
    }

	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		super.OnInit(owner);
		
		if(owner.GetControlledEntity())
		{
			m_CharacterController = CharacterControllerComponent.Cast(owner.GetControlledEntity().FindComponent(CharacterControllerComponent));
			if (!m_CharacterController)
				NodeError(this, owner, "Can't find CharacterControllerComponent.");
		}	
	}
	
	override void OnAbort(AIAgent owner, Node nodeCausingAbort)
	{
		if (!m_bStandupOnAbort)
		{
			super.OnAbort(owner, nodeCausingAbort);
			return;
		}
		
		if (m_AIInfo)
			m_AIInfo.SetStance(ECharacterStance.STAND);
		if (m_CharacterController)
			m_CharacterController.SetStanceChange(ConvertStanceToInt(ECharacterStance.STAND));		
		super.OnAbort(owner, nodeCausingAbort);
	}
};
