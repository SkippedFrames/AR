//! Morphine effect
[BaseContainerProps()]
class SCR_ConsumableMorphine : SCR_ConsumableEffectHealthItems
{
	[Attribute("10", UIWidgets.EditBox, "Regeneration speed of related hitzone when consuming this item", category: "Regeneration")]
	protected float m_fItemRegenerationSpeed;
	
	[Attribute("10", UIWidgets.EditBox, "Regeneration duration of related hitzone when consuming this item in seconds", category: "Regeneration")]
	protected float m_fItemRegenerationDuration;	
	
	[Attribute("0", UIWidgets.EditBox, "Total amount of regeneration that will be applied to the related hitzone. Will be ignored if m_fItemRegenerationDuration > 0", category: "Regeneration")]
	protected float m_fItemAbsoluteRegenerationAmount;
	
	//------------------------------------------------------------------------------------------------
	override void ApplyEffect(notnull IEntity target, notnull IEntity user, IEntity item, SCR_ConsumableEffectAnimationParameters animParams)
	{
		super.ApplyEffect(target, user, item, animParams);

		ChimeraCharacter char = ChimeraCharacter.Cast(target);
		if (!char)
			return;
		
		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(char.GetDamageManager());
		if (!damageMgr)
			return;
		
		array<HitZone> physicalHitZones = {};
		damageMgr.GetPhysicalHitZones(physicalHitZones);
		foreach (HitZone physicalHitZone: physicalHitZones)
		{
			ScriptedHitZone scriptedHitZone = ScriptedHitZone.Cast(physicalHitZone);
			if (!scriptedHitZone)
				continue;

			scriptedHitZone.CustomRegeneration(target, m_fItemRegenerationDuration, m_fItemRegenerationSpeed, m_fItemAbsoluteRegenerationAmount);
		}		
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanApplyEffect(notnull IEntity target, notnull IEntity user, out SCR_EConsumableFailReason failReason)
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(target);
		if (!char)
			return false;
		
		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(char.GetDamageManager());
		if (!damageMgr)
			return false;
		
		array<HitZone> hitzones = {};
		damageMgr.GetPhysicalHitZones(hitzones);

		foreach (HitZone hitzone : hitzones)
		{
			if (hitzone.GetDamageOverTime(EDamageType.HEALING) < 0)
			{
				failReason = SCR_EConsumableFailReason.ALREADY_APPLIED;
				return false;
			}
			
			if (hitzone.GetDamageState() != EDamageState.UNDAMAGED)
				return true;
		}

		failReason = SCR_EConsumableFailReason.UNDAMAGED;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanApplyEffectToHZ(notnull IEntity target, notnull IEntity user, ECharacterHitZoneGroup group, out SCR_EConsumableFailReason failReason = SCR_EConsumableFailReason.NONE)
	{
		return CanApplyEffect(target, user, failReason);
	}
		
	//------------------------------------------------------------------------------------------------
	void SCR_ConsumableMorphine()
	{
		m_eConsumableType = SCR_EConsumableType.MORPHINE;
	}
}
