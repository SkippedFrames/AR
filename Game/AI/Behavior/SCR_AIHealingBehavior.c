// AI healing behaviour
// TODO: You have to handle situation, in which movement can be disabled (in AIConfig component)

class SCR_AIHealBehavior : SCR_AIBehaviorBase
{
	IEntity m_EntityToHeal;
	
	SCR_AIInfoComponent m_AIInfo;
	
	ScriptedDamageManagerComponent m_DamageManager;
	
	protected const float PRIORITY_DELAY_MIN_MS = 800.0; // Delay until action returns its max priority
	protected const float PRIORITY_DELAY_MAX_MS = 1200.0;
	
	protected float m_fTimeCreated_ms;
	protected float m_fPriorityDelay_ms;
	
	// Max threat value under which we will consider healing someone
	protected const float MAX_THREAT_THRESHOLD = 0.002;
	
	//-------------------------------------------------------------------------------------------------------------------------------
	//! priority should be between normal Move and AttackMove and replaced by enum later, maybe evaluated dynamicly
	void SCR_AIHealBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, IEntity entityToHeal, bool allowHealMove, float priority = PRIORITY_BEHAVIOR_HEAL, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/Heal.bt";
		m_EntityToHeal = entityToHeal;
		SetPriority(priority);
		m_fPriorityLevel.m_Value = priorityLevel;
		m_fTimeCreated_ms = GetGame().GetWorld().GetWorldTime();
		m_fPriorityDelay_ms = Math.RandomFloat(PRIORITY_DELAY_MIN_MS, PRIORITY_DELAY_MAX_MS);
		
		SCR_AIUtilityComponent aiUtilComp = SCR_AIUtilityComponent.Cast(utility);
		if (!aiUtilComp)
			return;
		IEntity owner =	aiUtilComp.m_OwnerEntity;
		if (owner)
		{
			m_DamageManager = ScriptedDamageManagerComponent.Cast(owner.FindComponent(ScriptedDamageManagerComponent));
		}
	}
	
	override void OnActionCompleted()
	{
		super.OnActionCompleted();
#ifdef WORKBENCH
		SCR_AIDebugVisualization.VisualizeMessage(m_Utility.m_OwnerEntity, "Unit healed", EAIDebugCategory.INFO, 5);
#endif
	}
	
	override float CustomEvaluate()
	{
		if ((GetGame().GetWorld().GetWorldTime() - m_fTimeCreated_ms > m_fPriorityDelay_ms) &&
			(m_Utility.m_ThreatSystem.GetThreatMeasureWithoutInjuryFactor() < MAX_THREAT_THRESHOLD))
			return GetPriority();
		else
			return 0;
	}
};
