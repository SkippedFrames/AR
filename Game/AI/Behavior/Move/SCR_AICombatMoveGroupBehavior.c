class SCR_AICombatMoveGroupBehavior : SCR_AIMoveBehaviorBase
{
	ref SCR_BTParam<vector> m_vNextCoverPos = new ref SCR_BTParam<vector>(SCR_AIActionTask.NEXTCOVERPOSITION_PORT);
	ref SCR_BTParamRef<BaseTarget> m_Target = new ref SCR_BTParamRef<BaseTarget>(SCR_AIActionTask.TARGET_PORT); // needed to check if one should look around when in combat move	
	
	//--------------------------------------------------------------------------------------------------------------
	override void OnComplete()
	{
		super.OnComplete();
		if (!m_Utility.m_AIInfo)
			return;
		m_Utility.m_AIInfo.SetAIState(EUnitAIState.AVAILABLE);
	}
	
	//--------------------------------------------------------------------------------------------------------------
	override void OnFail()
	{
		super.OnFail();
		if (!m_Utility.m_AIInfo)
			return;
		m_Utility.m_AIInfo.SetAIState(EUnitAIState.AVAILABLE);
	}
	
	//--------------------------------------------------------------------------------------------------------------
	void InitParameters(vector nextCoverPosition, BaseTarget target)
	{
		m_Target.Init(this, target);
		m_vNextCoverPos.Init(this, nextCoverPosition);
	}

	//--------------------------------------------------------------------------------------------------------------
	void SCR_AICombatMoveGroupBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, vector pos, float priority = PRIORITY_BEHAVIOR_COMBAT_MOVE_GROUP, float priorityLevel = PRIORITY_LEVEL_NORMAL, IEntity target = null)
	{
		BaseTarget _tgt;
		if (!utility)
		{
			InitParameters(vector.Zero, _tgt);
			return;
		}
		else if (target)
		{
			_tgt = utility.m_CombatComponent.FindTargetByEntity(target);
		}
		
		InitParameters(vector.Zero, _tgt);
		
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/CombatMove.bt";
		SetPriority(priority);
		m_fPriorityLevel.m_Value = priorityLevel;
		SetIsUniqueInActionQueue(false);
		m_Utility.m_AIInfo.SetAIState(EUnitAIState.BUSY);
	}
};
