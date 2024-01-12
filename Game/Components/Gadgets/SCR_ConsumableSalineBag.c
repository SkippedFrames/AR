class LoadoutSalineBagArea : LoadoutAreaType
{
};

//------------------------------------------------------------------------------------------------
//! Saline bag effect
[BaseContainerProps()]
class SCR_ConsumableSalineBag : SCR_ConsumableEffectHealthItems
{
	[Attribute("10", UIWidgets.EditBox, "Regeneration duration of related hitzone when consuming this item in seconds", category: "Consumable")]
	protected float m_fItemRegenerationDuration;	
	
	[Attribute("0", UIWidgets.EditBox, "Total amount of regeneration that will be applied to the related hitzone", category: "Consumable")]
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
		
		SCR_CharacterBloodHitZone bloodHitZone = SCR_CharacterBloodHitZone.Cast(damageMgr.GetBloodHitZone());
		if (!bloodHitZone)
			return;
		
		SCR_InventoryStorageManagerComponent inventoryStorageComp = SCR_InventoryStorageManagerComponent.Cast(user.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inventoryStorageComp)
			return;

		SCR_SalineStorageComponent salineStorageComp = SCR_SalineStorageComponent.Cast(user.FindComponent(SCR_SalineStorageComponent));
		if (!salineStorageComp)
			return;
		
		if (salineStorageComp.AddSalineBagToSlot(target, ECharacterHitZoneGroup.RIGHTARM, item, m_fItemRegenerationDuration))
			bloodHitZone.CustomRegeneration(target, m_fItemRegenerationDuration, 0, m_fItemAbsoluteRegenerationAmount);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanApplyEffect(notnull IEntity target, notnull IEntity user,out SCR_EConsumableFailReason failReason = SCR_EConsumableFailReason.NONE)
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(target);
		if (!char)
			return false;
		
		//Can only apply when SCR_CharacterBloodHitZone is damaged
		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(char.GetDamageManager());
		if (!damageMgr || damageMgr.GetBloodHitZone().GetDamageState() == EDamageState.UNDAMAGED)
		{
			failReason = SCR_EConsumableFailReason.UNDAMAGED;
			return false;
		}

		//Character must have slot for the salinebag to move to
		SCR_SalineStorageComponent salineStorageMan = SCR_SalineStorageComponent.Cast(target.FindComponent(SCR_SalineStorageComponent));
		if (!salineStorageMan)
			return false;

		array<IEntity> items = {};
		salineStorageMan.GetAll(items);
		if (!items.IsEmpty())
		{
			failReason = SCR_EConsumableFailReason.ALREADY_APPLIED;
			return false;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanApplyEffectToHZ(notnull IEntity target, notnull IEntity user, ECharacterHitZoneGroup group, out SCR_EConsumableFailReason failReason = SCR_EConsumableFailReason.NONE)
	{
		return CanApplyEffect(target, user, failReason);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_ConsumableSalineBag()
	{
		m_eConsumableType = SCR_EConsumableType.SALINE;
	}
	
};
