class SCR_NoiseFilterEffect : SCR_BaseScreenEffect
{
	//Character
	protected ChimeraCharacter 											m_pCharacterEntity;
	protected SCR_CharacterDamageManagerComponent 						m_pDamageManager;
	private bool m_bLocalPlayerOutsideCharacter							= true;
	bool m_bDisplaySuspended											= false;

	//------------------------------------------------------------------------------------------------
 	override void DisplayControlledEntityChanged(IEntity from, IEntity to)
	{
		ClearEffects();
		
		m_pCharacterEntity = ChimeraCharacter.Cast(to);
		
		if (!m_pCharacterEntity)
			return;
		
		m_pDamageManager = SCR_CharacterDamageManagerComponent.Cast(m_pCharacterEntity.GetDamageManager());
		
		if (m_pDamageManager)
			m_pDamageManager.GetOnDamageStateChanged().Insert(OnDamageStateChanged);
		
		// In case of unconsciousness starting outside of character, apply effect now
		if (m_pCharacterEntity.GetCharacterController() && m_pCharacterEntity.GetCharacterController().IsUnconscious())
			LowPassFilterEffect();	
	}

	//------------------------------------------------------------------------------------------------
	// Check whether to apply or unapply the filter effect in case of unconsciousness
	override void DisplayConsciousnessChanged(bool conscious, bool init = false)
	{
		if (m_bDisplaySuspended)
			return;
		
		LowPassFilterEffect();
	}	

	//------------------------------------------------------------------------------------------------
	override protected void DisplayOnSuspended()
	{
		m_bDisplaySuspended = true;
		LowPassFilterEffect(true);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void DisplayOnResumed()
	{
		m_bDisplaySuspended = false;
		LowPassFilterEffect(false);
	}	

	//------------------------------------------------------------------------------------------------
	// Check whether to apply or unapply the filter effect in case of death
	void OnDamageStateChanged()
	{
		if (m_bDisplaySuspended)
			return;
		
		LowPassFilterEffect();
	}	
	
	//------------------------------------------------------------------------------------------------
	private void LowPassFilterEffect(bool outsideCharacter = false)
	{
		ECharacterLifeState state = ECharacterLifeState.ALIVE;
		
		if (!outsideCharacter && m_pCharacterEntity)
		{
			SCR_CharacterControllerComponent scrCharController = SCR_CharacterControllerComponent.Cast(m_pCharacterEntity.GetCharacterController());
			state = scrCharController.GetLifeState();
		}
			
		AudioSystem.SetOutputStateSignal(
			AudioSystem.DefaultOutputState,
			AudioSystem.GetOutpuStateSignalIdx(AudioSystem.DefaultOutputState, "CharacterLifeState"),
			state);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnRespawnMenuOpen()
	{
		LowPassFilterEffect(true);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void DisplayInit(IEntity owner)
	{
		SCR_DeployMenuBase.SGetOnMenuOpen().Insert(OnRespawnMenuOpen);
	}	
	
	//------------------------------------------------------------------------------------------------
	protected override void ClearEffects()
	{
		
		if (m_pDamageManager)
			m_pDamageManager.GetOnDamageStateChanged().Remove(OnDamageStateChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_NoiseFilterEffect()
	{
		SCR_DeployMenuBase.SGetOnMenuOpen().Remove(OnRespawnMenuOpen);
	}
};
