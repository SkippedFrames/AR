class SCR_AIActivityBase : SCR_AIActionBase
{
	SCR_AIGroupUtilityComponent m_Utility;
	ref SCR_BTParam<bool> m_bIsWaypointRelated = new SCR_BTParam<bool>(SCR_AIActionTask.WAYPOINT_RELATED_PORT);
	AIWaypoint m_RelatedWaypoint = null;
	bool m_bAllowFireteamRebalance = true;	// Allows rebalancing of fireteams while this activity exists
	
	//---------------------------------------------------------------------------------------------------------------------------------
	void InitParameters(AIWaypoint relatedWaypoint)
	{
		bool isWaypointRelated = relatedWaypoint != null;
		m_bIsWaypointRelated.Init(this, isWaypointRelated);
		m_RelatedWaypoint = relatedWaypoint;
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------	
	void SCR_AIActivityBase(SCR_AIGroupUtilityComponent utility, AIWaypoint relatedWaypoint)
	{
		m_Utility = utility;
		InitParameters(relatedWaypoint);
	}
	
	//------------------------------------------------------------------------------------
	protected void SendCancelMessagesToAllAgents()
	{
		AICommunicationComponent comms = m_Utility.m_Owner.GetCommunicationComponent();
		if (!comms)
			return;
		
		// Send to all agents
		array<AIAgent> agents = {};
		m_Utility.m_Owner.GetAgents(agents);
		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;
			
			SCR_AIMessage_Cancel msg = SCR_AIMessage_Cancel.Create(this);
			
			msg.SetReceiver(agent);
			comms.RequestBroadcast(msg, agent);
		}
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	#ifdef AI_DEBUG
	override void AddDebugMessage(string str)
	{
		SCR_AIInfoBaseComponent infoComp = SCR_AIInfoBaseComponent.Cast(m_Utility.GetOwner().FindComponent(SCR_AIInfoBaseComponent));
		infoComp.AddDebugMessage(string.Format("%1: %2", this, str), msgType: EAIDebugMsgType.ACTION);
	}
	#endif
};

class SCR_AIIdleActivity : SCR_AIActivityBase
{
	//---------------------------------------------------------------------------------------------------------------------------------
	void SCR_AIIdleActivity(SCR_AIGroupUtilityComponent utility, AIWaypoint relatedWaypoint)
	{
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Group/ActivityIdle.bt";
		SetPriority(1.0);
	}
};
