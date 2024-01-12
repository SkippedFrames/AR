[BaseContainerProps()]
class SCR_ConsumableEffectHealthItems : SCR_ConsumableEffectBase
{

	//------------------------------------------------------------------------------------------------	
	override bool ActivateEffect(IEntity target, IEntity user, IEntity item, SCR_ConsumableEffectAnimationParameters animParams = null)
	{
		SCR_ConsumableEffectAnimationParameters localAnimParams = animParams;
		if (!localAnimParams)
		{
			//user-held healthitem needs to get data from target to perform anim
			localAnimParams = GetAnimationParameters(target);
		}
		
		if (!localAnimParams)
			return false;
		
		if (!super.ActivateEffect(target, user, item, localAnimParams))
			return false;		

		ChimeraCharacter character = ChimeraCharacter.Cast(target);
		if (!character)
			return false;
		
		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return false;
			
		if (localAnimParams.m_intParam == EBandagingAnimationBodyParts.LeftLeg || localAnimParams.m_intParam == EBandagingAnimationBodyParts.RightLeg)
		{
			if (controller.GetStance() == ECharacterStance.STAND)
				controller.SetStanceChange(ECharacterStanceChange.STANCECHANGE_TOCROUCH);
		}
					
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void ApplyEffect(notnull IEntity target, notnull IEntity user, IEntity item, SCR_ConsumableEffectAnimationParameters animParams)
	{
		InventoryItemComponent itemComp = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
 	 	if (itemComp)
 	 		itemComp.RequestUserLock(user, false);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool UpdateAnimationCommands(IEntity user)
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(user);
		if (!char)
			return false;
		
		CharacterAnimationComponent animationComponent = char.GetAnimationComponent();
		if (!animationComponent)
			return false;		
		
		m_iPlayerApplyToSelfCmdId = animationComponent.BindCommand("CMD_HealSelf");
		m_iPlayerApplyToOtherCmdId = animationComponent.BindCommand("CMD_HealOther");
		m_iPlayerReviveCmdId = animationComponent.BindCommand("CMD_Revive");
		if (m_iPlayerApplyToSelfCmdId < 0 || m_iPlayerApplyToOtherCmdId < 0)
		{
			Print("One or both healing animationCommands have incorrect ID's!!", LogLevel.ERROR);
			return false;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Condition whther this effect can be applied to the specific hitzone
	//! /param target is the character who is having the effect applied
	//! /param EHitZoneGroup is the hitzonegroup which is having the effect applied
	bool CanApplyEffectToHZ(notnull IEntity target, notnull IEntity user, ECharacterHitZoneGroup group, out SCR_EConsumableFailReason failReason = SCR_EConsumableFailReason.NONE)
	{}
	
	TAnimGraphCommand GetReviveAnimCmnd(IEntity user)
	{
		UpdateAnimationCommands(user);
		return m_iPlayerReviveCmdId;
	}
	
};

