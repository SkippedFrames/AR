//------------------------------------------------------------------------------------------------
//! Bandage effect
[BaseContainerProps()]
class SCR_ConsumableBandage : SCR_ConsumableEffectHealthItems
{
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

		array<HitZone> hitZones = {};
		ECharacterHitZoneGroup hzGroup;
		damageMgr.GetBandageAnimHitzones(animParams.m_intParam, hitZones);
		if (hitZones.IsEmpty())
		{
			hzGroup = damageMgr.GetCharMostDOTHitzoneGroup(EDamageType.BLEEDING);
		}
		else
		{
			HitZone targetHitZone = hitZones.Get(0);
			if (!targetHitZone)
				return;
				
			SCR_CharacterHitZone charHitZone = SCR_CharacterHitZone.Cast(targetHitZone);
			if (!charHitZone)
				return;
			
			hzGroup = charHitZone.GetHitZoneGroup();
		}

		if (hzGroup == ECharacterHitZoneGroup.VIRTUAL)
			return;
		
		damageMgr.RemoveGroupBleeding(hzGroup);
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

		return damageMgr.IsDamagedOverTime(EDamageType.BLEEDING);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanApplyEffectToHZ(notnull IEntity target, notnull IEntity user, ECharacterHitZoneGroup group, out SCR_EConsumableFailReason failReason = SCR_EConsumableFailReason.NONE)
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(target);
		if (!char)
			return false;
		
		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(char.GetDamageManager());
		return damageMgr && damageMgr.GetGroupDamageOverTime(group, EDamageType.BLEEDING) > 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Optional param for specific hitzone, still will ensure external users don't have to create their own local animParams
	override SCR_ConsumableEffectAnimationParameters GetAnimationParameters(notnull IEntity target, ECharacterHitZoneGroup group = ECharacterHitZoneGroup.VIRTUAL)
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(target);
		if (!char)
			return null;
		
		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(char.GetDamageManager());
		if (!damageMgr)
			return null;

		EBandagingAnimationBodyParts bodyPartToBandage = EBandagingAnimationBodyParts.Invalid;
		
		if (group != ECharacterHitZoneGroup.VIRTUAL)
		{
			if (!damageMgr.GetGroupIsBeingHealed(group))
				bodyPartToBandage = damageMgr.FindAssociatedBandagingBodyPart(group);
		}
		else
		{
			group = damageMgr.GetCharMostDOTHitzoneGroup(EDamageType.BLEEDING, ignoreIfBeingTreated: true);
			array<HitZone> hitzones = {};
			damageMgr.GetHitZonesOfGroup(group, hitzones);
			if (!hitzones || hitzones.IsEmpty())
				return null;
				
			SCR_CharacterHitZone hitzone = SCR_CharacterHitZone.Cast(hitzones[0]);
			if (!hitzone)
				return null;
	
			bodyPartToBandage = hitzone.GetBodyPartToHeal();
		}
		
		if (bodyPartToBandage == EBandagingAnimationBodyParts.Invalid)
			return null;
			
		return new SCR_ConsumableEffectAnimationParameters(GetApplyToSelfAnimCmnd(target), 1, 0.0, m_fApplyToSelfDuration, bodyPartToBandage, 0, false);
	}

	//------------------------------------------------------------------------------------------------
	void SCR_ConsumableBandage()
	{
		m_eConsumableType = SCR_EConsumableType.BANDAGE;
	}		
};
