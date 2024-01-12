//------------------------------------------------------------------------------------------------
// GOAL REACTION BASE
//------------------------------------------------------------------------------------------------

[BaseContainerProps()]
class SCR_AIGoalReaction : SCR_AIReactionBase
{
	[Attribute("0", UIWidgets.ComboBox, "Type of event activating the reaction", "", ParamEnumArray.FromEnum(EMessageType_Goal) )]
	EMessageType_Goal m_eType;
	
	void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message) {}
	void PerformReaction(notnull SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message) {}
};


//------------------------------------------------------------------------------------------------
// GOAL REACTIONS - Reactions on different orders
//------------------------------------------------------------------------------------------------

[BaseContainerProps()]
class SCR_AIGoalReaction_Attack : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		SCR_AIMessage_Attack msg = SCR_AIMessage_Attack.Cast(message); // new approach: m_Target can be null!
		if (!msg)
			return;
		
		utility.m_CombatComponent.UpdateLastSeenPosition(msg.m_TargetInfo.m_Entity, msg.m_TargetInfo);
		utility.m_CombatComponent.SetAssignedTargets({msg.m_TargetInfo.m_Entity}, null);
	}
	
	override void PerformReaction(notnull SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message)
	{

	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_AttackCluster : SCR_AIGoalReaction
{
	// Duration of investigation if target is not seen
	const float INVESTIGATION_DURATION_S = 120.0;
	
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		SCR_AIMessage_AttackCluster msg = SCR_AIMessage_AttackCluster.Cast(message);
		if (!msg)
			return;
		
		// Does it still exist?
		if (!msg.m_TargetClusterState || !msg.m_TargetClusterState.m_Cluster)
			return;
		
		// ------ Set assigned targets
		utility.m_CombatComponent.SetAssignedTargets(null, msg.m_TargetClusterState);
			
		
		// ------ Should we investigate there?
		
		// Check if we have identified at least one target
		if (msg.m_bAllowInvestigate)
		{
			bool targetIdentified = false;
			foreach (IEntity targetEntity : msg.m_TargetClusterState.m_Cluster.m_aEntities)
			{
				if (utility.m_PerceptionComponent.GetTargetPerceptionObject(targetEntity, ETargetCategory.ENEMY))
				{
					targetIdentified = true;	// Found in 'Enemy' category - we have direct sight on it
					break;
				}
				else
					continue;
			}
			
			// If we haven't identified any target, go there
			if (!targetIdentified && msg.m_bAllowInvestigate)
			{
				// Cancel all other investigations
				utility.SetStateAllActionsOfType(SCR_AIMoveAndInvestigateBehavior, EAIActionState.FAILED);
				
				vector investigatePos;
				float investigateRadius;
				SCR_AIInvestigateClusterActivity.CalculateInvestigationArea(msg.m_TargetClusterState, investigatePos, investigateRadius);
				
				auto investigateBehavior = new SCR_AIMoveAndInvestigateBehavior(utility, msg.m_RelatedGroupActivity,
					investigatePos, radius: investigateRadius,
					priorityLevel: msg.m_fPriorityLevel, isDangerous: true,
					duration: INVESTIGATION_DURATION_S);
				utility.AddAction(investigateBehavior);
			}
		}
		else
		{
			// Report 'Covering' if we are not supposed to investigate there
			SCR_AICommsHandler commsHandler = utility.m_CommsHandler;
			if (!commsHandler.CanBypass())
			{
				SCR_AITalkRequest rq = new SCR_AITalkRequest(ECommunicationType.REPORT_COVERING, null, vector.Zero, 0, false, SCR_EAITalkRequestPreset.MEDIUM);
				commsHandler.AddRequest(rq);
			}
		}
	}
}

[BaseContainerProps()]
class SCR_AIGoalReaction_AttackClusterDone : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		SCR_AIMessage_AttackClusterDone msg = SCR_AIMessage_AttackClusterDone.Cast(message);
		if (!msg)
			return;
		
		utility.m_CombatComponent.ResetAssignedTargets();
	}
}

[BaseContainerProps()]
class SCR_AIGoalReaction_CoverCluster : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		// Now we don't have a behavior to cover those who are attacking/ivnestigating a cluster yet
		// We just use it for voice line
		SCR_AICommsHandler commsHandler = utility.m_CommsHandler;
		if (!commsHandler.CanBypass())
		{
			SCR_AITalkRequest rq = new SCR_AITalkRequest(ECommunicationType.REPORT_COVERING, null, vector.Zero, 0, false, SCR_EAITalkRequestPreset.MEDIUM);
			commsHandler.AddRequest(rq);
		}
	}
}


[BaseContainerProps()]
class SCR_AIGoalReaction_AttackStatic : SCR_AIGoalReaction_Attack
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		SCR_AIMessage_AttackStatic msg = SCR_AIMessage_AttackStatic.Cast(message); // new approach: m_Target can be null!
		if (!msg)
			return;
		
		BaseTarget baseTarget = utility.m_CombatComponent.FindTargetByEntity(msg.m_TargetInfo.m_Entity);
		if (!baseTarget)
			return;
		
		utility.m_CombatComponent.UpdateLastSeenPosition(baseTarget, msg.m_TargetInfo);
		SCR_AIAttackStaticBehavior behavior = new SCR_AIAttackStaticBehavior(utility, msg.m_RelatedGroupActivity, baseTarget, msg.m_TargetInfo.m_vWorldPos, msg.m_fPriorityLevel);
		if (m_OverrideBehaviorTree != string.Empty)
			behavior.m_sBehaviorTree = m_OverrideBehaviorTree;

		utility.AddAction(behavior);
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_AttackStaticDone : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		// only relevant for static attacks, others may receive it for the sake of simplicity of trees
		if (utility.m_AIInfo.HasUnitState(EUnitState.IN_TURRET))
		{
			utility.SetStateAllActionsOfType(SCR_AIAttackStaticBehavior, EAIActionState.COMPLETED);
		}
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_Move : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_Move.Cast(message);
		if (!msg)
			return;
		
		auto behavior = new SCR_AIMoveIndividuallyBehavior(utility, msg.m_RelatedGroupActivity,
			msg.m_MovePosition, SCR_AIActionBase.PRIORITY_BEHAVIOR_MOVE_INDIVIDUALLY, msg.m_fPriorityLevel, msg.m_FollowEntity);
		if (m_OverrideBehaviorTree != string.Empty)
			behavior.m_sBehaviorTree = m_OverrideBehaviorTree;

		utility.WrapBehaviorOutsideOfVehicle(behavior);
		utility.AddAction(behavior);
	}
	
	override void PerformReaction(notnull SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message)
	{		
		auto msg = SCR_AIMessage_Move.Cast(message);
		if (!msg)
			return;
		
		auto activity = new SCR_AIMoveActivity(utility, msg.m_RelatedWaypoint, msg.m_MovePosition,
			msg.m_FollowEntity, msg.m_eMovementType, msg.m_bUseVehicles, SCR_AIActionBase.PRIORITY_ACTIVITY_MOVE, priorityLevel: msg.m_fPriorityLevel);
		
		utility.SetStateAllActionsOfType(SCR_AISeekAndDestroyActivity,EAIActionState.FAILED); // move fails seek and destroy
		utility.AddAction(activity);
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_Follow : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_Follow.Cast(message);
		if (!msg)
			return;
		
		auto behavior = new SCR_AIMoveInFormationBehavior(utility, msg.m_RelatedGroupActivity, vector.Zero, SCR_AIActionBase.PRIORITY_BEHAVIOR_MOVE_IN_FORMATION, priorityLevel: msg.m_fPriorityLevel);

		utility.AddAction(behavior);
	}
	
	override void PerformReaction(notnull SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message)
	{		
		auto msg = SCR_AIMessage_Follow.Cast(message);
		if (!msg)
			return;
		
		//SCR_AIBaseUtilityComponent utility, bool prioritize, bool isWaypointRelated, vector pos, float priority = PRIORITY_ACTIVITY_FOLLOW, IEntity ent = null, bool useVehicles = false, float distance = 1.0
		auto activity = new SCR_AIFollowActivity(utility, msg.m_RelatedWaypoint, vector.Zero, msg.m_FollowEntity, 
			msg.m_eMovementType, false, SCR_AIActionBase.PRIORITY_ACTIVITY_FOLLOW, priorityLevel: msg.m_fPriorityLevel, distance: msg.m_fDistance);
		utility.SetStateAllActionsOfType(SCR_AISeekAndDestroyActivity, EAIActionState.FAILED); // move fails seek and destroy
		utility.AddAction(activity);
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_Investigate : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_Investigate.Cast(message);
		if (!msg)
			return;
		
		// Cancel previous investigations
		utility.SetStateAllActionsOfType(SCR_AIMoveAndInvestigateBehavior, EAIActionState.FAILED);
			
		auto behavior = new SCR_AIMoveAndInvestigateBehavior(utility, msg.m_RelatedGroupActivity, msg.m_vMovePosition,
			SCR_AIActionBase.PRIORITY_BEHAVIOR_MOVE_AND_INVESTIGATE, msg.m_fPriorityLevel, isDangerous: msg.m_bIsDangerous, targetUnitType: msg.m_eTargetUnitType, duration: msg.m_fDuration);
		utility.AddAction(behavior);
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_MoveInFormation : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		SCR_AIMessageGoal msg = SCR_AIMessageGoal.Cast(message);
		
		auto behavior = new SCR_AIMoveInFormationBehavior(utility, msg.m_RelatedGroupActivity, vector.Zero, 
			SCR_AIActionBase.PRIORITY_BEHAVIOR_MOVE_IN_FORMATION, msg.m_fPriorityLevel);
		
		utility.AddAction(behavior);
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_GetInVehicle : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_GetIn.Cast(message);
		if (!msg)
			return;
			
		auto behavior = new SCR_AIGetInVehicle(utility, msg.m_RelatedGroupActivity, msg.m_Vehicle, msg.m_eRoleInVehicle, SCR_AIActionBase.PRIORITY_BEHAVIOR_VEHICLE, msg.m_fPriorityLevel);
		if (m_OverrideBehaviorTree != string.Empty)
			behavior.m_sBehaviorTree = m_OverrideBehaviorTree;

		utility.AddAction(behavior);
	}
	
	override void PerformReaction(notnull SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_GetIn.Cast(message);
		if (!msg)
			return;
			
		auto activity = new SCR_AIGetInActivity(utility, msg.m_RelatedWaypoint, msg.m_Vehicle, msg.m_eRoleInVehicle, priorityLevel: msg.m_fPriorityLevel);
		utility.AddAction(activity);
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_GetOutVehicle : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_GetOut.Cast(message);
		if (!msg)
			return;

		auto behavior = new SCR_AIGetOutVehicle(utility, msg.m_RelatedGroupActivity, msg.m_Vehicle, SCR_AIActionBase.PRIORITY_BEHAVIOR_GET_OUT_VEHICLE, priorityLevel: msg.m_fPriorityLevel);
		if (m_OverrideBehaviorTree != string.Empty)
			behavior.m_sBehaviorTree = m_OverrideBehaviorTree;

		utility.AddAction(behavior);
	}
	
	override void PerformReaction(notnull SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_GetOut.Cast(message);
		if (!msg)
			return;
		
		auto activity = new SCR_AIGetOutActivity(utility, msg.m_RelatedWaypoint, msg.m_Vehicle, priorityLevel: msg.m_fPriorityLevel);
		utility.AddAction(activity);
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_SeekAndDestroy : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message)
	{
  		auto msg = SCR_AIMessage_SeekAndDestroy.Cast(message);
		if (!msg)
			return;
		
		auto activity = new SCR_AISeekAndDestroyActivity(utility, msg.m_RelatedWaypoint, msg.m_MovePosition, ent: msg.m_FollowEntity, useVehicles: msg.m_bUseVehicles, priorityLevel: msg.m_fPriorityLevel);
		
		utility.SetStateAllActionsOfType(SCR_AIMoveActivity,EAIActionState.FAILED);
		utility.AddAction(activity);
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_Heal : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_Heal.Cast(message);
		if (!msg)
			return;

		auto behavior = new SCR_AIMedicHealBehavior(utility, msg.m_RelatedGroupActivity, msg.m_EntityToHeal, false, priorityLevel: msg.m_fPriorityLevel);
		if (m_OverrideBehaviorTree != string.Empty)
			behavior.m_sBehaviorTree = m_OverrideBehaviorTree;

		utility.WrapBehaviorOutsideOfVehicle(behavior);
		utility.AddAction(behavior);
	}
	
	override void PerformReaction(notnull SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_Heal.Cast(message);
		if (!msg)
			return;
		
		SCR_AIActionBase currentAction = SCR_AIActionBase.Cast(utility.GetCurrentAction());
		if (!currentAction)
			return;
		
		float priorityLevelClamped = currentAction.GetRestrictedPriorityLevel(msg.m_fPriorityLevel);		

		// Ignore message if we already have an activity to heal this soldier
		SCR_AIHealActivity healActivity = SCR_AIHealActivity.Cast(utility.FindActionOfType(SCR_AIHealActivity));
		if (!healActivity)
			return;
		
		// Return if we are already healing this soldier
		if (healActivity.m_EntityToHeal.m_Value == msg.m_EntityToHeal)
		{
			healActivity.SetPriorityLevel(priorityLevelClamped);
			return;
		}
		
		auto activity = new SCR_AIHealActivity(utility, msg.m_RelatedWaypoint, msg.m_EntityToHeal, priorityLevel: priorityLevelClamped);

		utility.AddAction(activity);
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_HealWait : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_HealWait.Cast(message);
		if (!msg)
			return;
		
		auto behavior = new SCR_AIHealWaitBehavior(utility, msg.m_RelatedGroupActivity, msg.m_HealProvider, msg.m_fPriorityLevel);
		
		utility.WrapBehaviorOutsideOfVehicle(behavior); // Should we dismount to get healed?
		utility.AddAction(behavior);
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_Defend : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_Defend.Cast(message);
		if (!msg)
			return;
		
		auto behavior = new SCR_AIDefendBehavior(utility, msg.m_RelatedGroupActivity, msg.m_RelatedWaypoint,
			msg.m_vDefendLocation, msg.m_fDefendAngularRange, priorityLevel: msg.m_fPriorityLevel);
		if (m_OverrideBehaviorTree != string.Empty)
			behavior.m_sBehaviorTree = m_OverrideBehaviorTree;

		utility.AddAction(behavior);
	}
	
	override void PerformReaction(notnull SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_Defend.Cast(message);
		if (!msg)
			return;
		
		auto activity = new SCR_AIDefendActivity(utility, msg.m_RelatedWaypoint, msg.m_RelatedWaypoint, msg.m_vDefendLocation,
			priorityLevel: msg.m_fPriorityLevel);
		if (m_OverrideBehaviorTree != string.Empty)
			activity.m_sBehaviorTree = m_OverrideBehaviorTree;
		
		utility.AddAction(activity);
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_PerformAction : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_PerformAction.Cast(message);
		if (!msg)
			return;
		
		auto behavior = new SCR_AIPerformActionBehavior(utility, msg.m_RelatedGroupActivity, msg.m_SmartActionComponent, priorityLevel: msg.m_fPriorityLevel);
		if (m_OverrideBehaviorTree != string.Empty)
			behavior.m_sBehaviorTree = m_OverrideBehaviorTree;
		
		utility.AddAction(behavior);
	}
	
	override void PerformReaction(notnull SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_PerformAction.Cast(message);
		if (!msg)
			return;
		
		auto activity = new SCR_AIPerformActionActivity(utility, msg.m_RelatedWaypoint, msg.m_SmartActionEntity, msg.m_SmartActionTag, priorityLevel: msg.m_fPriorityLevel);
		if (m_OverrideBehaviorTree != string.Empty)
			activity.m_sBehaviorTree = m_OverrideBehaviorTree;
		
		utility.AddAction(activity);
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_Cancel : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		SCR_AIMessage_Cancel msg = SCR_AIMessage_Cancel.Cast(message);
		if (!msg || !msg.m_RelatedGroupActivity)
			return;
//		utility.FailBehaviorsOfActivity(msg.m_RelatedGroupActivity);
		utility.SetStateOfRelatedAction(msg.m_RelatedGroupActivity, EAIActionState.FAILED);
	}	
	
	override void PerformReaction(notnull SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message)
	{
		// This is wrong and unsafe, must be redone
		/*
		SCR_AIActivityBase activity = SCR_AIActivityBase.Cast(utility.GetCurrentAction());
		AIGroup group = utility.m_Owner;
		array<AIAgent> agents = {};
		group.GetAgents(agents);
		foreach (AIAgent agent : agents)
		{
			SCR_AISendCancelMsg(utility, activity, group, agent);
		}
		activity.Fail();
		*/
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_Retreat : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_ThrowGrenadeTo : SCR_AIGoalReaction
{
	private static float THROW_PREDICTION_DISTANCE = 5.0;

	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		SCR_AIMessage_ThrowGrenadeTo msg = SCR_AIMessage_ThrowGrenadeTo.Cast(message);
		if (!msg)
			return;
		
		SCR_AIThrowGrenadeToBehavior behavior = new SCR_AIThrowGrenadeToBehavior(utility, msg.m_RelatedGroupActivity, msg.m_vTargetPosition, msg.m_fPriorityLevel);
		
		if (m_OverrideBehaviorTree != string.Empty)
			behavior.m_sBehaviorTree = m_OverrideBehaviorTree;
		
		if (msg.m_TargetEntity)
		{
			bool found = behavior.TraceForGrenadeThrow(utility.GetOrigin(), msg.m_TargetEntity.GetOrigin(), THROW_PREDICTION_DISTANCE, utility.m_OwnerEntity, msg.m_TargetEntity);
			if (found)
				behavior.UpdatePositionInfo(msg.m_TargetEntity.GetOrigin());
			
			utility.AddAction(behavior);
		}
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_ProvideAmmo : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		SCR_AIMessage_ProvideAmmo msg = SCR_AIMessage_ProvideAmmo.Cast(message);
		if (!msg)
			return;
		
		SCR_AIProvideAmmoBehavior behavior = new SCR_AIProvideAmmoBehavior(utility, msg.m_RelatedGroupActivity,
			msg.m_AmmoConsumer, msg.m_MagazineWell, msg.m_fPriorityLevel);
		
		utility.AddAction(behavior);
	}
}

[BaseContainerProps()]
class SCR_AIGoalReaction_PickupInventoryItems : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		SCR_AIMessage_PickupInventoryItems msg = SCR_AIMessage_PickupInventoryItems.Cast(message);
		if (!msg)
			return;
		
		SCR_AIPickupInventoryItemsBehavior behavior = new SCR_AIPickupInventoryItemsBehavior(utility, msg.m_RelatedGroupActivity,
			msg.m_vPickupPosition, msg.m_MagazineWellType, msg.m_fPriorityLevel);
		
		utility.AddAction(behavior);
	}
}

//--------------------------------------------------------------------------------------------------------
// Helper methods used in above


static void SCR_AISendCancelMsg(SCR_AIGroupUtilityComponent utility, SCR_AIActivityBase relatedActivity, AIAgent sender, AIAgent receiver)
{
	if (!utility.m_Owner)
		return;
	AICommunicationComponent mailbox = utility.m_Owner.GetCommunicationComponent();
	if (mailbox)
	{
		SCR_AIMessage_Cancel msg = new SCR_AIMessage_Cancel;
		msg.m_RelatedGroupActivity = relatedActivity;
		msg.SetText("Cancelled activity");
		msg.SetReceiver(receiver);
		mailbox.RequestBroadcast(msg, receiver);
	}	
};

static void UpdateLastSeenPosition2(BaseTarget baseTarget, SCR_AITargetInfo newTargetInfo)
{
	if (baseTarget && baseTarget.GetTargetEntity() == newTargetInfo.m_Entity)
		baseTarget.UpdateLastSeenPosition(newTargetInfo.m_vWorldPos, newTargetInfo.m_fTimestamp);
}
