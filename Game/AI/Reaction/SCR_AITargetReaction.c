//------------------------------------------------------------------------------------------------
// TARGET REACTION BASE
//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_AITargetReactionBase : SCR_AIReactionBase
{
	void PerformReaction(notnull SCR_AIUtilityComponent utility, notnull SCR_AIThreatSystem threatSystem, BaseTarget baseTarget, vector lastSeenPosition) {}
};

//------------------------------------------------------------------------------------------------
// TARGET REACTIONS: These are not danger events, although they should be in some time
//------------------------------------------------------------------------------------------------


[BaseContainerProps()]
class SCR_AITargetReaction_Unknown : SCR_AITargetReactionBase
{
	// @TODO: Is it possible to send enemy target and unknown target as Target class, somehow? Can I overload danger event to gimme target?
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, notnull SCR_AIThreatSystem threatSystem, BaseTarget baseTarget, vector lastSeenPosition)
	{
		IEntity targetEntity = baseTarget.GetTargetEntity();
		if (targetEntity)
			utility.m_LookAction.LookAt(targetEntity, SCR_AILookAction.PRIO_UNKNOWN_TARGET, 2.0);
	}
};

[BaseContainerProps()]
class SCR_AITargetReaction_Enemy : SCR_AITargetReactionBase
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, notnull SCR_AIThreatSystem threatSystem, BaseTarget baseTarget, vector lastSeenPosition)
	{		
		if (!baseTarget)
			return;
		IEntity target = baseTarget.GetTargetEntity();
		
		if (!SCR_AIDamageHandling.IsAlive(target))
			return;
		
		if (!target)
			return;
		
		// Terminate retreat action if we had one
		AIActionBase retreatAction = utility.FindActionOfType(SCR_AIRetreatFromTargetBehavior);
		if (retreatAction)
			retreatAction.Complete();
		
		// Terminate all attack actions on same target if we had any
		ENodeResult finishAttackResult;
		utility.FinishAttackForTarget(baseTarget, finishAttackResult);
		
		//this shouldn't be dependent on Info component since Info component should provide state of agent to outside
		if (!utility.m_AIInfo.HasUnitState(EUnitState.IN_TURRET))
		{
			auto behavior = new SCR_AIAttackBehavior(utility, null, baseTarget, lastSeenPosition);
			behavior.InitWaitTime(utility);
			
			// AddAction must be used here, not AddActionIfMissing!
			// When target switches, we must delete the old behavior.
			utility.AddAction(behavior);
			utility.WrapBehaviorOutsideOfVehicle(behavior);
			utility.m_LookAction.LookAt(target, SCR_AILookAction.PRIO_ENEMY_TARGET);
		}
		else
		{
			auto behavior = new SCR_AIAttackStaticBehavior(utility, null, baseTarget, lastSeenPosition);
			behavior.InitWaitTime(utility);
			
			utility.AddAction(behavior);
			utility.m_LookAction.LookAt(target, SCR_AILookAction.PRIO_ENEMY_TARGET);
		}
	}
};

[BaseContainerProps()]
class SCR_AITargetReaction_RetreatFromEnemy : SCR_AITargetReactionBase
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, notnull SCR_AIThreatSystem threatSystem, BaseTarget baseTarget, vector lastSeenPosition)
	{
		if (!baseTarget)
			return;
		
		// Find if we already have a retreat action from that target
		SCR_AIRetreatFromTargetBehavior prevRetreatBehavior = SCR_AIRetreatFromTargetBehavior.Cast(utility.FindActionOfType(SCR_AIRetreatFromTargetBehavior));
		if (prevRetreatBehavior)
		{
			if (prevRetreatBehavior.m_RetreatFromTarget.m_Value == baseTarget)
				return;
			else
				prevRetreatBehavior.Complete();
		}
		
		SCR_AIRetreatFromTargetBehavior behavior = new SCR_AIRetreatFromTargetBehavior(utility, null, baseTarget);
		utility.AddAction(behavior);
	}
}
