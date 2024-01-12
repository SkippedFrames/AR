[BaseContainerProps()]
class SCR_AIDangerReaction_DamageTaken : SCR_AIDangerReaction
{
	override bool PerformReaction(notnull SCR_AIUtilityComponent utility, notnull SCR_AIThreatSystem threatSystem, AIDangerEvent dangerEvent)
	{
		if (dangerEvent.GetVictim() != utility.m_OwnerEntity)
			return false;
		// amount of dmg to threatsystem
		
		IEntity shooter = dangerEvent.GetObject();
		vector shooterPos;
		if (shooter)
			shooterPos = shooter.GetOrigin();
		
		float dist = vector.Distance(shooterPos, utility.GetOrigin());
		

		if (utility.m_CombatComponent.GetCurrentTarget() == null && dist > LONG_RANGE_FIRE_DISTANCE && shooter)
		{
			utility.AddAction(new SCR_AIMoveFromUnknownFire(utility, null, shooterPos, shooter));
		}
		
		return true;
	}
};
