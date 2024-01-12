[ComponentEditorProps(category: "GameScripted/AI", description: "Component for utility AI system calculations")]
class SCR_AIUtilityComponentClass: SCR_AIBaseUtilityComponentClass
{	
};

//------------------------------------------------------------------------------------------------
class SCR_AIUtilityComponent : SCR_AIBaseUtilityComponent
{
	GenericEntity m_OwnerEntity;
	protected SCR_CharacterControllerComponent m_OwnerController;
	SCR_AIConfigComponent m_ConfigComponent;
	SCR_AIInfoComponent m_AIInfo;
	SCR_AICombatComponent m_CombatComponent;
	PerceptionComponent m_PerceptionComponent;
	SCR_MailboxComponent m_Mailbox;
	
	ref SCR_AIThreatSystem m_ThreatSystem;
	ref SCR_AILookAction m_LookAction;
	ref SCR_AICommsHandler m_CommsHandler;
	ref SCR_AIBehaviorBase m_CurrentBehavior; // Used for avoiding constant casting, outside of this class use GetCurrentBehavior()

	protected ref BaseTarget m_UnknownTarget;
	protected float m_fReactionUnknownTargetTime_ms; // WorldTime timestamp

	
	protected float m_fLastUpdateTime;
	
	
	protected static const float DISTANCE_HYSTERESIS_FACTOR = 0.45; 	// how bigger must be old distance to new in IsInvestigationRelevant()
	protected static const float NEARBY_DISTANCE_SQ = 2500; 			// what is the minimal distance of new vs old in IsInvestigationRelevant()
	protected static const float REACTION_TO_SAME_UNKNOWN_TARGET_INTERVAL_MS = 2500; // How often to react to same unknown target if it didn't change
		
	//------------------------------------------------------------------------------------------------
	SCR_AIBehaviorBase EvaluateBehavior(BaseTarget unknownTarget)
	{
		if (!m_OwnerController || !m_ConfigComponent || !m_OwnerEntity)
			return null;
		
		#ifdef AI_DEBUG
		AddDebugMessage("EvaluateBehavior START");
		if (m_bEvaluationBreakpoint)
		{
			Print("EvaluateBehavior breakpoint triggered");
			debug;
			m_bEvaluationBreakpoint = false;
		}
		#endif
		
		// Update delta time and players's position
		float time = m_OwnerEntity.GetWorld().GetWorldTime();
		float deltaTime = time - m_fLastUpdateTime;
		m_fLastUpdateTime = time;

		// Create events from commands, danger events, new targets
		m_ThreatSystem.Update(this, deltaTime);
		m_CombatComponent.UpdatePerceptionFactor(m_PerceptionComponent, m_ThreatSystem);

		// Read messages
		AIMessage msgBase = m_Mailbox.ReadMessage(true);
		if (msgBase)
		{
			SCR_AIMessageGoal msgGoal = SCR_AIMessageGoal.Cast(msgBase);
			if (msgGoal)
			{
				// Process goal message
				#ifdef AI_DEBUG
				AddDebugMessage(string.Format("PerformGoalReaction: %1, from BT: %2", msgGoal, msgGoal.m_sSentFromBt));
				#endif
				m_ConfigComponent.PerformGoalReaction(this, msgGoal);
			}
			else
			{
				SCR_AIMessageInfo msgInfo = SCR_AIMessageInfo.Cast(msgBase);
				if (msgInfo)
				{
					// Process info message
					
					// Try to notify actions about the message
					bool overrideReaction = CallActionsOnMessage(msgInfo);
		
					#ifdef AI_DEBUG
					if (overrideReaction)
					{
						AddDebugMessage(string.Format("InfoMessage consumed by action: %1, from BT: %2", msgInfo, msgInfo.m_sSentFromBt));
					}
					#endif
					
					// If message was not consumed by action, process it
					if (!overrideReaction)
					{
						#ifdef AI_DEBUG
						AddDebugMessage(string.Format("PerformInfoReaction: %1, from BT: %2", msgInfo, msgInfo.m_sSentFromBt));
						#endif
						m_ConfigComponent.PerformInfoReaction(this, msgInfo);
					}
				}
			}
		}
		
		bool reactToUnknownTarget = false;
		if (unknownTarget)
		{
			if (unknownTarget == m_UnknownTarget)
			{	// Same target
				if (GetGame().GetWorld().GetWorldTime() - m_fReactionUnknownTargetTime_ms > REACTION_TO_SAME_UNKNOWN_TARGET_INTERVAL_MS)
					reactToUnknownTarget = true;
			}
			else
			{	// Different target
				reactToUnknownTarget = true;
			}
		}
		if (reactToUnknownTarget && m_ConfigComponent.m_Reaction_UnknownTarget)
		{
			#ifdef AI_DEBUG
			AddDebugMessage(string.Format("PerformReaction: Unknown Target: %1", unknownTarget));
			#endif
			
			m_ConfigComponent.m_Reaction_UnknownTarget.PerformReaction(this, m_ThreatSystem, unknownTarget, unknownTarget.GetLastSeenPosition());
			m_fReactionUnknownTargetTime_ms = GetGame().GetWorld().GetWorldTime();
		}
		m_UnknownTarget = unknownTarget;
		
		
		// Evaluate current weapon and target
		bool weaponEvent;
		bool selectedTargetChanged;
		bool retreatTargetChanged;
		bool compartmentChanged;
		m_CombatComponent.EvaluateWeaponAndTarget(weaponEvent, selectedTargetChanged, retreatTargetChanged, compartmentChanged);
		
		BaseTarget selectedTarget = m_CombatComponent.GetCurrentTarget();
		if (selectedTargetChanged || compartmentChanged)
		{
			// If we have some target valid for attack, we attack it
			// Otherwise if there is a target we can't attack, we retreat
			if (selectedTarget)
			{
				if (m_ConfigComponent.m_Reaction_EnemyTarget)
				{
					#ifdef AI_DEBUG
					AddDebugMessage(string.Format("PerformReaction: Selected Target: %1", selectedTarget));
					#endif
					m_ConfigComponent.m_Reaction_EnemyTarget.PerformReaction(this, m_ThreatSystem, selectedTarget, selectedTarget.GetLastSeenPosition());
				}
			}
		}
		if (!selectedTarget)
		{
			BaseTarget retreatTarget = m_CombatComponent.GetRetreatTarget();
			if (retreatTarget)
			{
				if (m_ConfigComponent.m_Reaction_RetreatFromTarget)
				{
					// We can't attack anything. Shall we retreat?
					#ifdef AI_DEBUG
					AddDebugMessage(string.Format("PerformReaction: Retreat From Target: %1", retreatTarget));
					#endif
					m_ConfigComponent.m_Reaction_RetreatFromTarget.PerformReaction(this, m_ThreatSystem, retreatTarget, retreatTarget.GetLastSeenPosition());
				}
			}
		}
		
		// Update combat component
		m_CombatComponent.Update(deltaTime);
		
		// Evaluation: Remove completed behaviors, evaluate, set new behavior
		RemoveObsoleteActions();
		AIActionBase selectedAction = EvaluateActions();
		#ifdef AI_DEBUG
		DiagIncreaseCounter();
		DebugLogActionsPriority();
		#endif
		
		if (selectedAction && selectedAction != m_CurrentBehavior && (!m_CurrentBehavior || m_CurrentBehavior.IsActionInterruptable()))
		{
			SetCurrentAction(selectedAction);
			m_CurrentBehavior = SCR_AIBehaviorBase.Cast(selectedAction);
#ifdef WORKBENCH
			SCR_AIDebugVisualization.VisualizeMessage(m_OwnerEntity, SCR_AIDebug.GetBehaviorName(m_CurrentBehavior), EAIDebugCategory.BEHAVIOR, 5);
#endif
		}
		
		m_CurrentBehavior.OnActionExecuted();
		
		// Update comms handler
		if (m_CommsHandler.m_bNeedUpdate)
			m_CommsHandler.Update(deltaTime);
		
		#ifdef AI_DEBUG
		AddDebugMessage("EvaluateBehavior END\n");
		#endif
		
		return m_CurrentBehavior;
	}
	
	// Independent attacking with movement is allowed only when group combat move is not present or unit state is EUnitState.IN_TURRET
	//------------------------------------------------------------------------------------------------
	bool CanIndependentlyMove()
	{
		if (m_AIInfo && m_AIInfo.HasUnitState(EUnitState.IN_TURRET))
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_AIBehaviorBase GetCurrentBehavior()
	{
		return m_CurrentBehavior;
	}
	
	//------------------------------------------------------------------------------------------------
	void WrapBehaviorOutsideOfVehicle(SCR_AIActionBase action)
	{
		
	 	CompartmentAccessComponent compartmentAccess = CompartmentAccessComponent.Cast(m_OwnerEntity.FindComponent(CompartmentAccessComponent));
		if (!compartmentAccess)
			return;
			 
		if (!compartmentAccess.IsInCompartment())
			return;	
		
		if (m_AIInfo && m_AIInfo.HasUnitState(EUnitState.IN_TURRET))
			return;
		
		SCR_AIActivityBase relatedActivity;
		SCR_AIBehaviorBase behavior = SCR_AIBehaviorBase.Cast(action);
		if (behavior)
			relatedActivity = SCR_AIActivityBase.Cast(behavior.GetRelatedGroupActivity());
		
		float score = action.Evaluate();
		float priorityLevel = action.EvaluatePriorityLevel();
		IEntity vehicle = compartmentAccess.GetCompartment().GetOwner();
		ECompartmentType compartmentType = SCR_AICompartmentHandling.CompartmentClassToType(compartmentAccess.GetCompartment().Type());
		AddAction(new SCR_AIGetOutVehicle(this, relatedActivity, vehicle, priority: score + 100, priorityLevel: priorityLevel));
		AddAction(new SCR_AIGetInVehicle(this, relatedActivity, vehicle, compartmentType, priorityLevel: priorityLevel));
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		AIAgent agent = AIAgent.Cast(GetOwner());
		if (!agent)
			return;	
		
		m_ConfigComponent = SCR_AIConfigComponent.Cast(agent.FindComponent(SCR_AIConfigComponent));

		m_OwnerEntity = GenericEntity.Cast(agent.GetControlledEntity());
		if (!m_OwnerEntity)
			return;

		m_AIInfo = SCR_AIInfoComponent.Cast(agent.FindComponent(SCR_AIInfoComponent));
		m_CombatComponent = SCR_AICombatComponent.Cast(m_OwnerEntity.FindComponent(SCR_AICombatComponent));
		m_PerceptionComponent = PerceptionComponent.Cast(m_OwnerEntity.FindComponent(PerceptionComponent));
		m_OwnerController = SCR_CharacterControllerComponent.Cast(m_OwnerEntity.FindComponent(SCR_CharacterControllerComponent));
		m_ThreatSystem = new ref SCR_AIThreatSystem(this);
		m_AIInfo.InitThreatSystem(m_ThreatSystem); // let the AIInfo know about the threat system - move along with creating threat system instance!
		m_LookAction = new ref SCR_AILookAction(this, false); // LookAction is not regular behavior and is evaluated separately
		m_ConfigComponent.AddDefaultBehaviors(this);
		m_Mailbox = SCR_MailboxComponent.Cast(owner.FindComponent(SCR_MailboxComponent));
		m_CommsHandler = new SCR_AICommsHandler(m_OwnerEntity, agent);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnDiag(IEntity owner, float timeSlice)
	{
		m_CommsHandler.EOnDiag(timeSlice);
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetOrigin()
	{
		return m_OwnerEntity.GetOrigin();
	}
	
	//----------------------------------------------------------------------------------------------------
	bool ShouldAttackEnd(out BaseTarget enemyTarget, out bool shouldInvestigateFurther = false, out string context = string.Empty)
	{
		SCR_AIAttackBehavior attackBehavior;
		// do I have currently run attack behavior?
		attackBehavior = SCR_AIAttackBehavior.Cast(m_CurrentBehavior);
		if (attackBehavior)
		{
			enemyTarget = attackBehavior.m_Target.m_Value;
			if (m_CombatComponent.ShouldAttackEndForTarget(enemyTarget,shouldInvestigateFurther,context))
				return true;
			else 
			{
#ifdef AI_DEBUG
				context = "Attack in progress.";
#endif
				return false;
			}
		};
		
		
		attackBehavior = SCR_AIAttackBehavior.Cast(FindActionOfInheritedType(SCR_AIAttackBehavior));
		if (attackBehavior)
		{
			enemyTarget = attackBehavior.m_Target.m_Value;
			if (m_CombatComponent.ShouldAttackEndForTarget(enemyTarget,shouldInvestigateFurther,context))
				return true;
		}
				
#ifdef AI_DEBUG
		context = "No attack.";
#endif
		return false;
	}
	
	
	//------------------------------------------------------------------------------------------------
	bool FinishAttackForTarget(BaseTarget target, ENodeResult behaviorResult)
	{
		if (!target)
			return false;
		
		bool returnValue = false;
		
		auto attack = SCR_AIAttackBehavior.Cast(FindActionOfInheritedType(SCR_AIAttackBehavior)); // also find Attack_Static behavior
		if (attack && attack.GetActionState() != EAIActionState.COMPLETED && attack.GetActionState() != EAIActionState.FAILED)
		{
			if (attack.m_Target.m_Value == target)
			{
				if (behaviorResult == ENodeResult.SUCCESS)
				{
					attack.Complete();
				}
				else
				{
					attack.Fail();
				}
				
				returnValue = true; 
			}
		}
		
		if(!attack)
			return false;
		
		return returnValue;
	}
};

