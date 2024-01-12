class SCR_DrowningScreenEffect : SCR_BaseScreenEffect
{
	// Play Animation of drowningVignette
	protected const float DROWNINGEFFECT_OPACITY_FADEOUT_1_DURATION 	= 0.2;
	protected const float DROWNINGEFFECT_PROGRESSION_FADEOUT_1_DURATION = 0.2;
	
	// Fade out animation of drowningVignette
	protected const float DROWNINGEFFECT_OPACITY_FADEIN_1_DURATION		= 1;
	protected const float DROWNINGEFFECT_PROGRESSION_FADEIN_1_DURATION  = 4.5;
	
	// Play Animation of BlackoutEffect()
	protected const float BLACKOUT_OPACITY_MULTIPLIER					= 0.70;
	
	// Adaptive opacity widgets 
	protected Widget								m_wAdaptiveOpacityDrowning;
	protected ImageWidget							m_wBlackOut;

	
	// Widgets
	protected ImageWidget 							m_wDrowningEffect;
	
	// Character
	protected ChimeraCharacter						m_pCharacterEntity;

	
	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		m_wDrowningEffect = ImageWidget.Cast(m_wRoot.FindAnyWidget("DrowningVignette"));
		m_wBlackOut = ImageWidget.Cast(m_wRoot.FindAnyWidget("BlackOut"));
	}	
	
	//------------------------------------------------------------------------------------------------
 	override void DisplayControlledEntityChanged(IEntity from, IEntity to)
	{
		ClearEffects();
		RemoveInvokers(from);
		
		m_pCharacterEntity = ChimeraCharacter.Cast(to);
		if (!m_pCharacterEntity)
			return;
		
		SCR_CharacterControllerComponent m_CharController = SCR_CharacterControllerComponent.Cast(m_pCharacterEntity.GetCharacterController());
		if (!m_CharController)
			return;
		
		if (m_CharController.m_OnPlayerDrowning)
			m_CharController.m_OnPlayerDrowning.Insert(CreateEffect);			
		if (m_CharController.m_OnPlayerStopDrowning)
			m_CharController.m_OnPlayerStopDrowning.Insert(ClearEffect);
				
		SCR_CharacterDamageManagerComponent damageMan = SCR_CharacterDamageManagerComponent.Cast(m_pCharacterEntity.GetDamageManager());
		if (damageMan)
			damageMan.GetOnDamageStateChanged().Insert(OnDeath);
		
		// In case player started drowning before invokers were established, check if already drowning
		if (m_CharController.IsCharacterDrowning())
			CreateEffect(10, 4);
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateOpacity(float opacity, float sceneBrightness, float sceneBrightnessRaw)
	{
		if (!m_wAdaptiveOpacityDrowning)
			m_wAdaptiveOpacityDrowning = m_wRoot.FindAnyWidget("AdaptiveOpacity-Drowning");

		//PrintFormat("<Adaptive Opacity - m_wAdaptiveOpacityDrowning> %1 | Updating opacity %2 -> %3", this, m_wAdaptiveOpacityDrowning.GetOpacity(), opacity);
		
		m_wAdaptiveOpacityDrowning.SetOpacity(opacity);
	}
	
	//------------------------------------------------------------------------------------------------
	void CreateEffect(float maxDrowningDuration, float drowningTimeStartFX)
	{
		if (!m_wDrowningEffect)
			return;
		
		float effectStrength = 1;
		
		// Add a couple seconds to drowningDuration so the widget still animates when death occurs
		float drowningDuration = (maxDrowningDuration - drowningTimeStartFX) + 2;
		if (drowningDuration <= 0)
			drowningDuration = 1;
	
		m_wDrowningEffect.SetSaturation(1);
		m_wDrowningEffect.SetMaskTransitionWidth(0.8);

		AnimateWidget.Opacity(m_wDrowningEffect, effectStrength, 1 / drowningDuration);
		AnimateWidget.AlphaMask(m_wDrowningEffect, effectStrength * 0.6, 1 / drowningDuration);

		BlackoutEffect(effectStrength, drowningDuration);
	}

	//------------------------------------------------------------------------------------------------
	void ClearEffect()
	{
		AnimateWidget.Opacity(m_wDrowningEffect, 0, DROWNINGEFFECT_PROGRESSION_FADEOUT_1_DURATION);
		AnimateWidget.AlphaMask(m_wDrowningEffect, 0, DROWNINGEFFECT_OPACITY_FADEOUT_1_DURATION);
		
		BlackoutEffect(0, 0);
	}
	
	//------------------------------------------------------------------------------------------------
	void BlackoutEffect(float effectStrength, float drowningDuration)
	{
		if (!m_wBlackOut)
			return;
		
		if (effectStrength <= 0 || drowningDuration <= 0 )
		{
			AnimateWidget.AlphaMask(m_wBlackOut, 0, DROWNINGEFFECT_OPACITY_FADEOUT_1_DURATION);
			return;
		}
		
		m_wBlackOut.SetOpacity(1);
		effectStrength *= BLACKOUT_OPACITY_MULTIPLIER;
		AnimateWidget.AlphaMask(m_wBlackOut, effectStrength, 1 / drowningDuration);
	}
			
	//------------------------------------------------------------------------------------------------
	void OnDeath(EDamageState state)
	{
		if (state != EDamageState.DESTROYED)
			return;

		ClearEffect();
	}

	//------------------------------------------------------------------------------------------------
	protected override void ClearEffects()
	{
		if (m_wDrowningEffect)
		{
			AnimateWidget.StopAllAnimations(m_wDrowningEffect);
			m_wDrowningEffect.SetOpacity(0);
			m_wDrowningEffect.SetMaskProgress(0);
		}

		if (m_wBlackOut)
		{
			AnimateWidget.StopAllAnimations(m_wBlackOut);
			m_wBlackOut.SetOpacity(0);
			m_wBlackOut.SetMaskProgress(0);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RemoveInvokers(IEntity prevEntity)
	{		
		if (!prevEntity)
			return;
		
		m_pCharacterEntity = ChimeraCharacter.Cast(prevEntity);
		if (!m_pCharacterEntity)
			return;
		
		SCR_CharacterControllerComponent m_CharController = SCR_CharacterControllerComponent.Cast(m_pCharacterEntity.GetCharacterController());
		if (!m_CharController)
			return;
		
		if (m_CharController.m_OnPlayerDrowning)
			m_CharController.m_OnPlayerDrowning.Remove(CreateEffect);			
		if (m_CharController.m_OnPlayerStopDrowning)
			m_CharController.m_OnPlayerStopDrowning.Remove(ClearEffect);
	}

	//------------------------------------------------------------------------------------------------
	override event void DisplayStopDraw(IEntity owner)
	{
		if (GetGame().GetHUDManager())
			GetGame().GetHUDManager().GetSceneBrightnessChangedInvoker().Remove(UpdateOpacity);
	}
};
