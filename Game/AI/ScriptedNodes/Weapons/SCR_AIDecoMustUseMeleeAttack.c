class SCR_AIDecoMustUseMeleeAttack: DecoratorScripted
{
	protected static const string PORT_TARGET_POS = "TargetPos";
	
	protected const float MELEE_MAX_DISTANCE_SQ = 2*2;
	
	// This amount of updates will be skipped when TestFunction is called and it will return the previous value
	protected const int N_UPDATES_SKIP = 2;
	
	protected IEntity m_Controlled = null;
	protected SCR_CharacterControllerComponent m_Controller = null;
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		m_Controlled = owner.GetControlledEntity();
		m_Controller = SCR_CharacterControllerComponent.Cast(m_Controlled.FindComponent(SCR_CharacterControllerComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	int m_iCounter = 0;
	bool m_bPrevResult = false;
	override bool TestFunction(AIAgent owner)
	{
		// Skip actual tests, calculate every Nth update
		if (m_iCounter > 0)
		{
			m_iCounter--;
			return m_bPrevResult;
		}
		m_iCounter = N_UPDATES_SKIP;
		
		if (!m_Controller || !m_Controlled)
			return false;
		
		vector targetPos;
		GetVariableIn(PORT_TARGET_POS, targetPos);
		
		//Print(string.Format("dist: %1", vector.Distance(m_Controlled.GetOrigin(), targetPos)));
		
		m_bPrevResult = (vector.DistanceSq(m_Controlled.GetOrigin(), targetPos) < MELEE_MAX_DISTANCE_SQ) &&
				(!m_Controller.CanFire()) &&
				(m_Controller.GetStance() != ECharacterStance.PRONE);
		
		return m_bPrevResult;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool VisibleInPalette()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {PORT_TARGET_POS};
	override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	}

};
