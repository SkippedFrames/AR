class SCR_ScriptedCommandsStaticTable
{
	void SCR_ScriptedCommandsStaticTable() 
	{
	}
	
	bool Bind(CharacterAnimationComponent animationComponent)
	{
		m_CommandGesture = animationComponent.BindCommand("CMD_Gestures");
		
		m_IsLoiteringTag = GameAnimationUtils.RegisterAnimationTag("IsLoitering");
		m_IsLoiteringStartingTag = GameAnimationUtils.RegisterAnimationTag("IsLoiteringStarting");
		m_IsChangingWeaponTag = GameAnimationUtils.RegisterAnimationTag("TagChangingWeapon");
		
		m_bIsBound = m_CommandGesture != -1 && m_IsLoiteringTag != -1 && m_IsLoiteringStartingTag != -1 && m_IsChangingWeaponTag != -1;
		return m_bIsBound;
	}
	
	bool IsBound()
	{
		return m_bIsBound;
	}
	
	// Loiter
	TAnimGraphCommand m_CommandGesture = -1;
	TAnimGraphTag m_IsLoiteringTag = -1;
	TAnimGraphTag m_IsLoiteringStartingTag = -1;
	TAnimGraphTag m_IsChangingWeaponTag = -1;
	
	protected bool m_bIsBound = false;
};
