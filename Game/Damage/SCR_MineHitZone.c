class SCR_MineHitZone : ScriptedHitZone
{
	//------------------------------------------------------------------------------------------------
	override float ComputeEffectiveDamage(EDamageType damageType, float rawDamage, IEntity hitEntity, HitZone struckHitZone, IEntity damageSource, notnull Instigator instigator, const GameMaterial hitMaterial, int colliderID, inout vector hitTransform[3], const vector impactVelocity, int nodeID, bool isDOT)
	{
		if (damageType != EDamageType.MELEE)
			return super.ComputeEffectiveDamage(damageType, rawDamage, hitEntity, struckHitZone, damageSource, instigator, hitMaterial, colliderID, hitTransform, impactVelocity, nodeID, isDOT);
		
		// Inactive mines don't take melee damage, activated mines can be destroyed using melee and explode
		SCR_PressureTriggerComponent triggerComponent = SCR_PressureTriggerComponent.Cast(GetOwner().FindComponent(SCR_PressureTriggerComponent));
		if (!triggerComponent || triggerComponent.IsActivated())
			return super.ComputeEffectiveDamage(damageType, rawDamage, hitEntity, struckHitZone, damageSource, instigator, hitMaterial, colliderID, hitTransform, impactVelocity, nodeID, isDOT);
		
		return 0;
	}
}
