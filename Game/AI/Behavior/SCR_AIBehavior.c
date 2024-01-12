class SCR_AIBehaviorBase : SCR_AIActionBase
{
	SCR_AIUtilityComponent m_Utility;
	
	float m_fThreat = 0.0; // This threat value will be added to the calculated threat level in threat system
	bool m_bAllowLook = true;
	bool m_bResetLook = false;
	
	//---------------------------------------------------------------------------------------------------------------------------------
	void SCR_AIBehaviorBase(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity)
	{
		m_Utility = utility;
		SetRelatedGroupActivity(groupActivity);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	SCR_AIActivityBase GetGroupActivityContext()
	{
		return SCR_AIActivityBase.Cast(GetRelatedGroupActivity());
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	override void OnActionSelected()
	{
		super.OnActionSelected();
		if (m_bResetLook)
			m_Utility.m_LookAction.Cancel();
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	#ifdef AI_DEBUG
    override protected void AddDebugMessage(string str)
	{
		SCR_AIInfoBaseComponent infoComp = SCR_AIInfoBaseComponent.Cast(m_Utility.GetOwner().FindComponent(SCR_AIInfoBaseComponent));
		infoComp.AddDebugMessage(string.Format("%1: %2", this, str), msgType: EAIDebugMsgType.ACTION);
	}
	#endif
};

class SCR_AIWaitBehavior : SCR_AIBehaviorBase
{	
	void SCR_AIWaitBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity)
	{
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/Wait.bt";
		SetPriority(10);
    }
};

class SCR_AIIdleBehavior : SCR_AIBehaviorBase
{
	void SCR_AIIdleBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity)
	{
		if (!utility)
			return;
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/Idle.bt";
		SetPriority(1.0);
	}
	
	override void OnActionSelected()
	{
		super.OnActionSelected();
		
		// Temporary solution to make AI select some generic weapon suitable against infantry, like a rifle.
		m_Utility.m_CombatComponent.SetExpectedEnemyType(EAIUnitType.UnitType_Infantry);
	}
};

