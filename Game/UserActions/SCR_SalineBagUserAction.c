class SCR_SalineBagUserAction : SCR_HealingUserAction
{
	
	//------------------------------------------------------------------------------------------------
	//! Method called when the action is interrupted/canceled.
	//! \param pUserEntity The entity that was performing this action prior to interruption
	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return;
		
		SCR_CharacterDamageManagerComponent damageMan = SCR_CharacterDamageManagerComponent.Cast(character.GetDamageManager());
		if (damageMan) 
		{
			if (damageMan.GetIsUnconscious() || damageMan.GetHealth() <= 0)
				return;
		}
		
		SCR_ConsumableItemComponent consumableComponent = GetConsumableComponent(character);
		if (consumableComponent)
			consumableComponent.SetAlternativeModel(false);
	}
			
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		// Target character
		ChimeraCharacter targetCharacter = ChimeraCharacter.Cast(GetOwner());
		if (!targetCharacter)
			return false;
		
		// Medic character
		ChimeraCharacter userCharacter = ChimeraCharacter.Cast(user);
		if (!userCharacter)
			return false;

		SCR_ConsumableItemComponent consumableComponent = GetConsumableComponent(userCharacter);
		if (!consumableComponent)
			return false;
		
		SCR_ConsumableSalineBag consumableSaline = SCR_ConsumableSalineBag.Cast(consumableComponent.GetConsumableEffect());
		if (!consumableSaline)
			return false;
		
		int reason;
		if (consumableSaline.CanApplyEffectToHZ(targetCharacter, userCharacter, m_eHitZoneGroup, reason))
			return true;
		
		if (reason == SCR_EConsumableFailReason.UNDAMAGED)
			SetCannotPerformReason(m_sNoBloodLoss);

		if (reason == SCR_EConsumableFailReason.ALREADY_APPLIED)
			SetCannotPerformReason(m_sAlreadyApplied);

		return false;
	}
};
