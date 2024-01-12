//------------------------------------------------------------------------------------------------
//! Tourniquet effect
[BaseContainerProps()]
class SCR_ConsumableTourniquet: SCR_ConsumableEffectHealthItems
{
	//Cached to remember which bodypart to remove tourniquet from
	ECharacterHitZoneGroup m_eTargetHZGroup;
	
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
		
		m_eTargetHZGroup = damageMgr.FindAssociatedHitZoneGroup(animParams.m_intParam);

		SCR_TourniquetStorageComponent tqStorageComp = SCR_TourniquetStorageComponent.Cast(user.FindComponent(SCR_TourniquetStorageComponent));
		if (!tqStorageComp)
		{
			Print("SCR_TourniquetStorageComponent missing", LogLevel.ERROR);
			return;
		}
		
		tqStorageComp.AddTourniquetToSlot(target, m_eTargetHZGroup, item);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanApplyEffect(notnull IEntity target, notnull IEntity user, out SCR_EConsumableFailReason failReason = SCR_EConsumableFailReason.NONE)
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(target);
		if (!char)
			return false;
		
		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(char.GetDamageManager());
		if (!damageMgr || !damageMgr.IsDamagedOverTime(EDamageType.BLEEDING))
			return false;
		
		return damageMgr.GetCharMostDOTHitzoneGroup(EDamageType.BLEEDING, true, true);
	}	
	
	//------------------------------------------------------------------------------------------------
	override bool CanApplyEffectToHZ(notnull IEntity target, notnull IEntity user, ECharacterHitZoneGroup group, out SCR_EConsumableFailReason failReason = SCR_EConsumableFailReason.NONE)
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(target);
		if (!char)
			return false;
		
		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(char.GetDamageManager());
		if (!damageMgr)
			return false;
		
		array<ECharacterHitZoneGroup> limbs = {};
		damageMgr.GetAllExtremities(limbs);
		
		if (!limbs.Contains(group))
			return false;
		
		if (damageMgr.GetGroupTourniquetted(group))
		{
			failReason = SCR_EConsumableFailReason.ALREADY_APPLIED;
			return false;
		}
		
		if (damageMgr.GetGroupDamageOverTime(group, EDamageType.BLEEDING) > 0)
			return true;
		
		failReason = SCR_EConsumableFailReason.NOT_BLEEDING;
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
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
			group = damageMgr.GetCharMostDOTHitzoneGroup(EDamageType.BLEEDING, true, true, true);
			if (!group)
				return null;
			
			array<HitZone> groupHitZones = {};
			damageMgr.GetHitZonesOfGroup(group, groupHitZones);
			
			SCR_CharacterHitZone charHitZone = SCR_CharacterHitZone.Cast(groupHitZones[0]);
			if (!charHitZone)
				return null;
			
			bodyPartToBandage = charHitZone.GetBodyPartToHeal();
		}
		
		if (bodyPartToBandage == EBandagingAnimationBodyParts.Invalid)
				return null;
		
		return new SCR_ConsumableEffectAnimationParameters(GetApplyToSelfAnimCmnd(target), 1, 0, m_fApplyToSelfDuration, bodyPartToBandage, 0.0, false);	
	}
	
	//------------------------------------------------------------------------------------------------
	ECharacterHitZoneGroup GetTargetHitZoneGroup()
	{
		return m_eTargetHZGroup;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_ConsumableTourniquet()
	{
		m_eConsumableType = SCR_EConsumableType.TOURNIQUET;
	}
	
};
