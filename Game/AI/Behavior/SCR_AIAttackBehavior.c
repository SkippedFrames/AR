class SCR_AIAttackBehavior : SCR_AIBehaviorBase
{
	
#ifdef WORKBENCH
	ref Shape m_Shape;
#endif

	// Be careful, here we want to store a ref to BaseTarget
	ref SCR_BTParamRef<BaseTarget> m_Target = new SCR_BTParamRef<BaseTarget>(SCR_AIActionTask.TARGET_PORT);
	
	// Wait time before we start shooting
	ref SCR_BTParam<float> m_fWaitTime = new SCR_BTParam<float>("WaitTime");
	
    SCR_AIWorld m_AIWorld;
	SCR_AICombatComponent m_CombatComponent;
	
	bool m_bHasGrenades = true;
	bool m_bSelected = false;

	// This delay is executed before shooting starts
	protected static const float WAIT_TIME_UNEXPECTED = 0.25;
	protected static const float WAIT_TIME_OVERTHREATENED = 0.8;
	
	//----------------------------------------------------------------------------------
	void InitParameters(BaseTarget target, float waitTime)
	{
		m_Target.Init(this, target);
		m_fWaitTime.Init(this, waitTime);
	}
	
	//----------------------------------------------------------------------------------
	override void OnActionSelected()
	{
		super.OnActionSelected();
		m_bSelected = true;
		EnableFriendlyFireCheck(true);
	}
	
	//----------------------------------------------------------------------------------
	override void OnActionDeselected()
	{
		super.OnActionDeselected();
		EnableFriendlyFireCheck(false);
	}

	//----------------------------------------------------------------------------------
	override void OnActionCompleted()
	{
		super.OnActionCompleted();
		EnableFriendlyFireCheck(false);
	}
	
	//----------------------------------------------------------------------------------
	override void OnActionFailed()
	{
		super.OnActionFailed();
		EnableFriendlyFireCheck(false);
	}
	
	//----------------------------------------------------------------------------------
	override string GetActionDebugInfo()
	{
		return this.ToString() + " attacking " + m_Target.ValueToString();
	}
	
	//----------------------------------------------------------------------------------	
	void SCR_AIAttackBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, BaseTarget target, vector pos, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		InitParameters(target, 4);
		if (!utility)
			return;
		
		m_fPriorityLevel.m_Value = priorityLevel;
		SetPriority(PRIORITY_BEHAVIOR_ATTACK_NOT_SELECTED);
		m_fThreat = 1.01 * SCR_AIThreatSystem.VIGILANT_THRESHOLD;
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/Attack.bt";
		m_bAllowLook = false;
		m_bResetLook = true;
		SetIsUniqueInActionQueue(false);
		m_AIWorld = SCR_AIWorld.Cast(GetGame().GetAIWorld());
		m_CombatComponent = utility.m_CombatComponent;
	}
	
	//----------------------------------------------------------------------------------
	override float CustomEvaluate()
	{
		BaseTarget baseTarget = m_Target.m_Value;
		
		// Return 0 if it's not current target selected by weapon&target selector
		// Since weapon&target selector selects both weapon and target, if we chose to attack a different target,
		// it might happen that we use a wrong weapon
		if (baseTarget != m_CombatComponent.GetCurrentTarget())
			return 0;
		
		float targetScore = m_Utility.m_CombatComponent.m_WeaponTargetSelector.CalculateTargetScore(baseTarget);
		if (targetScore >= SCR_AICombatComponent.TARGET_SCORE_HIGH_PRIORITY_ATTACK ||
			baseTarget.IsEndangering())
			return PRIORITY_BEHAVIOR_ATTACK_HIGH_PRIORITY;
		else if (m_bSelected)
			return PRIORITY_BEHAVIOR_ATTACK_SELECTED;
		else
			return PRIORITY_BEHAVIOR_ATTACK_NOT_SELECTED;
	}
	
	//----------------------------------------------------------------------------------
	void SendMessageThrowGrenadeTo(IEntity target, vector position, string reason)
	{
		if (!m_bHasGrenades)
			return;
		
		AIAgent agent = AIAgent.Cast(m_Utility.GetOwner());
		if (!agent || !m_AIWorld)
			return;
		
		//Ensure there's at least 1 grenade in the inventory
		SCR_InventoryStorageManagerComponent inventoryManager = SCR_InventoryStorageManagerComponent.Cast(agent.GetControlledEntity().FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inventoryManager)
		{
			m_bHasGrenades = false;
			return;
		}	
	
		array<typename> components = {};
		components.Insert(WeaponComponent);
		components.Insert(GrenadeMoveComponent);
		IEntity grenade = inventoryManager.FindItemWithComponents(components, EStoragePurpose.PURPOSE_DEPOSIT);
		if (!grenade)
		{
			m_bHasGrenades = false;
			return;
		}	
		AICommunicationComponent mailbox = agent.GetCommunicationComponent();
		if (!mailbox)
			return;
		
		SCR_AIMessage_ThrowGrenadeTo msg = new SCR_AIMessage_ThrowGrenadeTo();
		if (!msg)
			Debug.Error("Unable to create valid message!");
		msg.SetText(reason);
		msg.m_MessageType = EMessageType_Goal.THROW_GRENADE_TO;
		msg.m_TargetEntity = target;
		msg.m_vTargetPosition = position;
		msg.SetReceiver(agent);
		mailbox.RequestBroadcast(msg, agent);
	}
	
	// Sets the delay until shooting starts.
	void InitWaitTime(SCR_AIUtilityComponent utility)
	{
		float threatMeasure = utility.m_ThreatSystem.GetThreatMeasure();
		
		// Delay depending on threat
		float threatDelay;
		if (threatMeasure < SCR_AIThreatSystem.ATTACK_DELAYED_THRESHOLD)
			threatDelay = WAIT_TIME_UNEXPECTED;
		else if (threatMeasure < SCR_AIThreatSystem.THREATENED_THRESHOLD)
			threatDelay = 0;
		else
			threatDelay = WAIT_TIME_OVERTHREATENED;
		
		// Delay depending on distance
		// 0m - 0ms
		// 100m - 340ms
		// 300m - 700ms
		// 500m - 870ms
		// 800m - 1018ms
		float distance = vector.Distance(m_Utility.m_OwnerEntity.GetOrigin(), m_Target.m_Value.GetLastSeenPosition());
		float distanceDelay = (1.4 * distance) / (300 + distance);
		
		m_fWaitTime.m_Value = threatDelay + distanceDelay;
	}
	
	void EnableFriendlyFireCheck(bool enable)
	{
		if (m_Utility && m_Utility.m_PerceptionComponent)
			m_Utility.m_PerceptionComponent.SetFriendlyFireCheck(enable);
	}
};
