class SCR_BleedingScreenEffect : SCR_BaseScreenEffect
{
	// Play Animation of CreateEffectOverTime()
	protected const int   BLEEDING_REPEAT_DELAY 						= 2500;
	protected const float BLEEDINGEFFECT_OPACITY_FADEOUT_1_DURATION 	= 0.2;
	protected const float BLEEDINGEFFECT_PROGRESSION_FADEOUT_1_DURATION = 0.2;
	protected const float BLEEDINGEFFECT_OPACITY_FADEOUT_2_DURATION 	= 0.3;
	protected const float BLEEDINGEFFECT_PROGRESSION_FADEOUT_2_DURATION = 3;
	protected const float BLEEDINGEFFECT_OPACITY_FADEIN_1_DURATION		= 1;
	protected const float BLEEDINGEFFECT_PROGRESSION_FADEIN_1_DURATION  = 4.5;
	
	// Play Animation of BlackoutEffect()
	protected const float BLACKOUT_OPACITY_MULTIPLIER					= 0.20;
	
	//Saturation
	private static float s_fSaturation;
	
	// Adaptive opacity widgets 
	protected Widget								m_wAdaptiveOpacityBlood;
	
	// Widgets
	protected ImageWidget 							m_wBloodEffect1;
	protected ImageWidget 							m_wBloodEffect2;
	protected ImageWidget							m_wBlackOut;
	private int m_iEffectNo 						= 1;
	
	// Character
	protected SCR_CharacterDamageManagerComponent	m_pDamageManager;
	protected SCR_CharacterBloodHitZone				m_pBloodHZ;
	protected ChimeraCharacter						m_pCharacterEntity;

	protected bool m_bBleedingEffect;
	protected bool m_bIsBleeding;

	
	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		m_wBloodEffect1 = ImageWidget.Cast(m_wRoot.FindAnyWidget("BloodVignette1"));
		m_wBloodEffect2 = ImageWidget.Cast(m_wRoot.FindAnyWidget("BloodVignette2"));
		m_wBlackOut = ImageWidget.Cast(m_wRoot.FindAnyWidget("BlackOut"));
	}	
	
	//------------------------------------------------------------------------------------------------
 	override void DisplayControlledEntityChanged(IEntity from, IEntity to)
	{
		ClearEffects();
		
		m_pCharacterEntity = ChimeraCharacter.Cast(to);
		if (!m_pCharacterEntity)
			return;
		
		m_pDamageManager = SCR_CharacterDamageManagerComponent.Cast(m_pCharacterEntity.GetDamageManager());
		if (!m_pDamageManager)
			return;

		// define hitzones for later getting
		m_pBloodHZ = SCR_CharacterBloodHitZone.Cast(m_pDamageManager.GetBloodHitZone());
		
		// Invoker for momentary damage events and DOT damage events
		m_pDamageManager.GetOnDamageOverTimeAdded().Insert(OnDamageOverTimeAdded);
		m_pDamageManager.GetOnDamageOverTimeRemoved().Insert(OnDamageOverTimeRemoved);
		
		// In case player started bleeding before invokers were established, check if already bleeding
		if (m_pDamageManager.IsDamagedOverTime(EDamageType.BLEEDING))
			OnDamageOverTimeAdded(EDamageType.BLEEDING, m_pDamageManager.GetDamageOverTime(EDamageType.BLEEDING));
	}
	
	//------------------------------------------------------------------------------------------------	
	void OnDamageOverTimeAdded(EDamageType dType, float dps, HitZone hz = null)
	{
		m_bIsBleeding = m_pDamageManager.IsDamagedOverTime(EDamageType.BLEEDING);
		if (!m_bBleedingEffect && m_bIsBleeding && m_pDamageManager.GetState() != EDamageState.DESTROYED)
			CreateEffectOverTime(true);
		
		m_bBleedingEffect = m_bIsBleeding;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnDamageOverTimeRemoved(EDamageType dType, HitZone hz = null)
	{
		m_bIsBleeding = m_pDamageManager.IsDamagedOverTime(EDamageType.BLEEDING);
		if (m_bIsBleeding)
			return;

		ClearEffectOverTime(false);
		GetGame().GetCallqueue().Remove(CreateEffectOverTime);
		GetGame().GetCallqueue().Remove(ClearEffectOverTime);

		m_bBleedingEffect = false;
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateOpacity(float opacity, float sceneBrightness, float sceneBrightnessRaw)
	{
		if (!m_wAdaptiveOpacityBlood)
			m_wAdaptiveOpacityBlood = m_wRoot.FindAnyWidget("AdaptiveOpacity-Blood");

		//PrintFormat("<Adaptive Opacity - m_wAdaptiveOpacityBlood> %1 | Updating opacity %2 -> %3", this, m_wAdaptiveOpacityBlood.GetOpacity(), opacity);
		
		m_wAdaptiveOpacityBlood.SetOpacity(opacity);
	}
	
	//------------------------------------------------------------------------------------------------
	void CreateEffectOverTime(bool repeat)
	{
		if (!m_wBloodEffect1 || !m_wBloodEffect2 || !m_pBloodHZ)
			return;
		
		float effectStrength = 1;
		const float REDUCEDSTRENGTH = 0.7;
		bool playHeartBeat = true; 
		
		if (m_pBloodHZ.GetDamageOverTime(EDamageType.BLEEDING) < 5)
		{
			effectStrength *= REDUCEDSTRENGTH;
			playHeartBeat = false;
		}
	
		m_wBloodEffect1.SetSaturation(1);
		m_wBloodEffect2.SetSaturation(1);
		
		if (m_iEffectNo == 1)
		{
			AnimateWidget.Opacity(m_wBloodEffect1, effectStrength, BLEEDINGEFFECT_OPACITY_FADEIN_1_DURATION);
			AnimateWidget.AlphaMask(m_wBloodEffect1, effectStrength * 0.5, BLEEDINGEFFECT_PROGRESSION_FADEIN_1_DURATION);
			AnimateWidget.Opacity(m_wBloodEffect2, 0, BLEEDINGEFFECT_OPACITY_FADEOUT_2_DURATION);
			AnimateWidget.AlphaMask(m_wBloodEffect2, 0, BLEEDINGEFFECT_PROGRESSION_FADEOUT_2_DURATION);
		}
		else if (m_iEffectNo == 2)
		{
			AnimateWidget.Opacity(m_wBloodEffect1, 0, BLEEDINGEFFECT_OPACITY_FADEOUT_2_DURATION);
			AnimateWidget.AlphaMask(m_wBloodEffect1, 0, BLEEDINGEFFECT_PROGRESSION_FADEOUT_2_DURATION);
			AnimateWidget.Opacity(m_wBloodEffect2, effectStrength, BLEEDINGEFFECT_OPACITY_FADEIN_1_DURATION);
			AnimateWidget.AlphaMask(m_wBloodEffect2, effectStrength * 0.5, BLEEDINGEFFECT_PROGRESSION_FADEIN_1_DURATION);
		}

		BlackoutEffect(effectStrength);
		
		// Play heartbeat sound
		if (playHeartBeat && m_pDamageManager.GetDefaultHitZone().GetDamageState() != EDamageState.DESTROYED)
		{
			SCR_UISoundEntity.SetSignalValueStr("BloodLoss", 1 - m_pBloodHZ.GetHealthScaled());
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INJURED_PLAYERCHARACTER);
		}

		GetGame().GetCallqueue().CallLater(ClearEffectOverTime, 1000, false, repeat, m_iEffectNo);
	}

	//------------------------------------------------------------------------------------------------
	void ClearEffectOverTime(bool repeat)
	{
		if (m_iEffectNo == 1)
			m_iEffectNo = 2;
		else
			m_iEffectNo = 1;

		AnimateWidget.Opacity(m_wBloodEffect1, 0, BLEEDINGEFFECT_PROGRESSION_FADEOUT_1_DURATION);
		AnimateWidget.AlphaMask(m_wBloodEffect1, 0, BLEEDINGEFFECT_OPACITY_FADEOUT_1_DURATION);
		AnimateWidget.Opacity(m_wBloodEffect2, 0, BLEEDINGEFFECT_PROGRESSION_FADEOUT_1_DURATION);
		AnimateWidget.AlphaMask(m_wBloodEffect2, 0, BLEEDINGEFFECT_OPACITY_FADEOUT_1_DURATION);
		
		BlackoutEffect(0);

		if (repeat && m_bBleedingEffect)
			GetGame().GetCallqueue().CallLater(CreateEffectOverTime, BLEEDING_REPEAT_DELAY, false, repeat);
	}
	
	//------------------------------------------------------------------------------------------------
	void BlackoutEffect(float effectStrength)
	{
		if (!m_wBlackOut)
			return;
		
		if (effectStrength > 0)
		{
			m_wBlackOut.SetOpacity(1);
			effectStrength *= BLACKOUT_OPACITY_MULTIPLIER;
			AnimateWidget.AlphaMask(m_wBlackOut, effectStrength, BLEEDINGEFFECT_PROGRESSION_FADEIN_1_DURATION);
		}
		else
		{
			AnimateWidget.AlphaMask(m_wBlackOut, 0, BLEEDINGEFFECT_OPACITY_FADEOUT_1_DURATION);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override void ClearEffects()
	{
		m_bBleedingEffect = 0;
		
		if (m_wBloodEffect1)
		{
			AnimateWidget.StopAllAnimations(m_wBloodEffect1);
			m_wBloodEffect1.SetOpacity(0);
			m_wBloodEffect1.SetMaskProgress(0);
		}

		if (m_wBloodEffect2)
		{
			AnimateWidget.StopAllAnimations(m_wBloodEffect2);
			m_wBloodEffect2.SetOpacity(0);
			m_wBloodEffect2.SetMaskProgress(0);
		}
		
		if (m_wBlackOut)
		{
			AnimateWidget.StopAllAnimations(m_wBlackOut);
			m_wBlackOut.SetOpacity(0);
			m_wBlackOut.SetMaskProgress(0);
		}
		
		GetGame().GetCallqueue().Remove(CreateEffectOverTime);
		GetGame().GetCallqueue().Remove(ClearEffectOverTime);
		
		if (!m_pDamageManager)
			return;

		m_pDamageManager.GetOnDamageOverTimeAdded().Remove(OnDamageOverTimeAdded);
		m_pDamageManager.GetOnDamageOverTimeRemoved().Remove(OnDamageOverTimeRemoved);
	}
	
	//------------------------------------------------------------------------------------------------
	override event void DisplayStopDraw(IEntity owner)
	{
		if (GetGame().GetHUDManager())
			GetGame().GetHUDManager().GetSceneBrightnessChangedInvoker().Remove(UpdateOpacity);
	}
};
