[BaseContainerProps()]
class SCR_AIDangerReaction_ProjectileHit : SCR_AIDangerReaction
{
	override bool PerformReaction(notnull SCR_AIUtilityComponent utility, notnull SCR_AIThreatSystem threatSystem, AIDangerEvent dangerEvent)
	{
		float distanceSq = vector.DistanceSq(utility.GetOrigin(), dangerEvent.GetPosition());
		if (distanceSq > BULLET_IMPACT_DISTANCE_SQ_MAX)
			return false;
		
		IEntity shooter = dangerEvent.GetObject();
		vector shooterPos;
		if (shooter)
			shooterPos = shooter.GetOrigin();
		float distanceToShooter = vector.Distance(utility.GetOrigin(), shooterPos);
		
		if (utility.m_CombatComponent.GetCurrentTarget() == null && distanceToShooter > LONG_RANGE_FIRE_DISTANCE && shooter)
		{
			utility.AddAction(new SCR_AIMoveFromUnknownFire(utility, null, shooterPos, shooter));
			//TODO: change combat type from SILENT
		}

		threatSystem.ThreatBulletImpact(dangerEvent.GetCount());
		
		return true;
	}	
};
