[ComponentEditorProps(category: "GameScripted/AI", description: "Component for utility AI system for groups")]
class SCR_AIGroupUtilityComponentClass: SCR_AIBaseUtilityComponentClass
{
};

class SCR_AIGroupUtilityComponent : SCR_AIBaseUtilityComponent
{
	SCR_AIGroup m_Owner;
	SCR_AIConfigComponent m_ConfigComponent;
	SCR_AIGroupInfoComponent m_GroupInfo;
	SCR_MailboxComponent m_Mailbox;
	ref array<SCR_AIInfoComponent> m_aInfoComponents = new ref array<SCR_AIInfoComponent>;
	
	protected float m_fLastUpdateTime = -1.0;
	protected float m_fPerceptionUpdateTimer_ms;
	
	// Update interval of group perception and target clusters and their processing
	protected const float PERCEPTION_UPDATE_TIMER_MS = 2000.0;
	
	protected bool m_bNewGroupMemberAdded;
	protected ref SCR_AIActionBase m_CurrentActivity;
	
	// Waypoint state
	protected ref SCR_AIWaypointState m_WaypointState;
	
	// Group perception and clusters
	ref SCR_AIGroupPerception m_Perception;
	
	protected ref SCR_AIGroupTargetClusterProcessor m_TargetClusterProcessor;
	
	// Fireteams
	ref SCR_AIGroupFireteamManager m_FireteamMgr;
	
	// Used by SCR_AIGetMemberByGoal nodes
	int m_iGetMemberByGoalNextIndex = 0;
	
	//---------------------------------------------------------------------------------------------------
	SCR_AIActionBase EvaluateActivity(out bool restartActivity)
	{
		ref SCR_AIActionBase activity;
		restartActivity = false;
		
		if (!m_ConfigComponent)
			return null;
		
		float currentTime = GetGame().GetWorld().GetWorldTime();
		float deltaTime_ms = 0;
		if (m_fLastUpdateTime != -1.0)
			deltaTime_ms = currentTime - m_fLastUpdateTime;
		
		#ifdef AI_DEBUG
		AddDebugMessage("EvaluateActivity START");
		if (m_bEvaluationBreakpoint)
		{
			Print("EvaluateActivity breakpoint triggered");
			debug;
			m_bEvaluationBreakpoint = false;
		}
		#endif
		
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
					
					bool overrideReaction = CallActionsOnMessage(msgInfo);
			
					if (!overrideReaction)
					{
						#ifdef AI_DEBUG
						AddDebugMessage(string.Format("PerformInfoReaction: %1, from BT: %2", msgInfo, msgInfo.m_sSentFromBt));
						#endif
						
						m_ConfigComponent.PerformInfoReaction(this, msgInfo);
					}
					#ifdef AI_DEBUG
					else
					{
						#ifdef AI_DEBUG
						AddDebugMessage(string.Format("InfoMessage consumed by action: %1, from BT: %2", msgInfo, msgInfo.m_sSentFromBt));
						#endif
					}
					#endif
				}
			}
		}
			
		RemoveObsoleteActions();
		
		activity = SCR_AIActionBase.Cast(EvaluateActions());
		#ifdef AI_DEBUG
		DiagIncreaseCounter();
		DebugLogActionsPriority();
		#endif
		
		if (activity && (!m_CurrentActivity || (m_CurrentActivity != activity && m_CurrentActivity.IsActionInterruptable())))
			restartActivity = true;
		else if (m_bNewGroupMemberAdded && activity)
			restartActivity = true;
			
		if (restartActivity)
		{
			SetCurrentAction(activity);
			UpdateGroupControlMode(activity);
			m_CurrentActivity = activity;
			
#ifdef WORKBENCH
			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_PRINT_ACTIVITY))
				PrintFormat("Agent %1 activity %2",m_Owner,m_CurrentActivity.GetActionDebugInfo());
#endif
		}
		
		#ifdef AI_DEBUG
		AddDebugMessage("EvaluateActivity END\n");
		#endif
		
		// Rebalance fireteams if needed
		if (m_FireteamMgr.m_bRebalanceFireteams)
		{
			if (CanRebalanceFireteams()) // In some cases we can't rebalance fireteams yet
			{
				m_FireteamMgr.RebalanceFireteams();
			}
		}
		
		// Perception and clusters
		m_fPerceptionUpdateTimer_ms += deltaTime_ms;
		if (m_fPerceptionUpdateTimer_ms > PERCEPTION_UPDATE_TIMER_MS)
		{
			m_Perception.Update();
			if (!m_Perception.m_aTargetClusters.IsEmpty())
				UpdateClustersState(m_fPerceptionUpdateTimer_ms);
			
			m_fPerceptionUpdateTimer_ms -= PERCEPTION_UPDATE_TIMER_MS;
		}
			
		m_fLastUpdateTime = currentTime;
		m_bNewGroupMemberAdded = false; // resetting reaction on group member added
		
		return m_CurrentActivity;
	}
	

	//---------------------------------------------------------------------------------------------------
	// Updates info of group members to planner - should be called when adding or removing group member 	
	void OnAgentAdded(AIAgent agent)
	{
		// Add to array of AIInfo
		SCR_ChimeraAIAgent chimeraAgent = SCR_ChimeraAIAgent.Cast(agent);
		if (!chimeraAgent)
			return;

		SCR_AIInfoComponent info = chimeraAgent.m_InfoComponent;
		
		if (!info)
			return;
		
		m_aInfoComponents.Insert(info);	
		
		m_FireteamMgr.OnAgentAdded(agent, info);
		
		m_bNewGroupMemberAdded = true;
		
		return;
	}
	
	//---------------------------------------------------------------------------------------------------
	void OnAgentRemoved(SCR_AIGroup group, AIAgent agent)
	{
		// Remove from array of AIInfo
		for (int i = m_aInfoComponents.Count() - 1; i >= 0; i--)
		{
			if (!m_aInfoComponents[i])
			{
				Debug.Error("Null AI info occured"); // investigate when this happens!
				m_aInfoComponents.RemoveOrdered(i);
			}
			else if (m_aInfoComponents[i].IsOwnerAgent(agent))
			{
				m_aInfoComponents.RemoveOrdered(i);
				break;
			}
		}
		
		m_FireteamMgr.OnAgentRemoved(agent);
	}
	
	//---------------------------------------------------------------------------------------------------
	void OnWaypointCompleted(AIWaypoint waypoint)
	{
		if (m_WaypointState && waypoint)
			m_WaypointState.OnDeselected();
		
		m_WaypointState = null;
	}
	
	//---------------------------------------------------------------------------------------------------
	void OnWaypointRemoved(AIWaypoint waypoint, bool isCurrentWaypoint)
	{
		// Remove old wp state, if it existed
		if (isCurrentWaypoint)
		{
			if (waypoint && m_WaypointState)
				m_WaypointState.OnDeselected();
			
			m_WaypointState = null;
		}
	}
	
	//---------------------------------------------------------------------------------------------------
	void OnCurrentWaypointChanged(AIWaypoint currentWp, AIWaypoint prevWp)
	{
		// Remove old wp state, if it existed
		if (m_WaypointState && prevWp)
			m_WaypointState.OnDeselected();
		
		m_WaypointState = null;
		
		// Create new wp state
		if (currentWp)
		{
			SCR_AIWaypoint scrCurrentWp = SCR_AIWaypoint.Cast(currentWp);
			if (scrCurrentWp)
			{
				SCR_AIWaypointState wpState = scrCurrentWp.CreateWaypointState(this);
				if (wpState)
				{
					m_WaypointState = wpState;
					m_WaypointState.OnSelected();
				}
			}
		}
	}
		
	//---------------------------------------------------------------------------------------------------
	void OnExecuteWaypointTree()
	{
		if (m_WaypointState)
			m_WaypointState.OnExecuteWaypointTree();
	}
	
	//---------------------------------------------------------------------------------------------------
	//! Called from m_Perception
	void OnEnemyDetectedFiltered(SCR_AIGroup group, SCR_AITargetInfo target, AIAgent reporter)
	{
		SCR_ChimeraAIAgent agent = SCR_ChimeraAIAgent.Cast(reporter);
		if (!agent)
			return;
		
		SCR_AICommsHandler commsHandler = agent.m_UtilityComponent.m_CommsHandler;
		
		// Ignore if the talk request can be optimized out
		if (commsHandler.CanBypass())
			return;
		
		SCR_AITalkRequest rq = new SCR_AITalkRequest(ECommunicationType.REPORT_CONTACT, target.m_Entity, target.m_vWorldPos,
			enumSignal: 0, transmitIfNoReceivers: true, preset: SCR_EAITalkRequestPreset.MEDIUM);
		commsHandler.AddRequest(rq);
	}
	
	//---------------------------------------------------------------------------------------------------
	void OnTargetClusterStateChanged(SCR_AITargetClusterState state, EAITargetClusterState prevState, EAITargetClusterState newState)
	{
		// If we've lost enemies at some place, make the assigned fireteams report that
		if (newState == EAITargetClusterState.LOST &&
			(prevState == EAITargetClusterState.INVESTIGATING || prevState == EAITargetClusterState.ATTACKING) &&
			state.m_Activity)
		{
			SCR_AIFireteamsActivity ftActivity = SCR_AIFireteamsActivity.Cast(state.m_Activity);
			if (ftActivity)
			{
				TFireteamLockRefArray fireteamLocks = {};
				ftActivity.GetAssignedFireteams(fireteamLocks);
				foreach (SCR_AIGroupFireteamLock ftLock : fireteamLocks)
				{
					AIAgent reporterAgent = ftLock.GetFireteam().GetMember(0);
					if (!reporterAgent)
						continue;
					SCR_AICommsHandler commsHandler = SCR_AISoundHandling.FindCommsHandler(reporterAgent);
					if (!commsHandler)
						continue;
					if (commsHandler.CanBypass())
						continue;
					
					SCR_AITalkRequest rq = new SCR_AITalkRequest(ECommunicationType.REPORT_CLEAR, null, vector.Zero, 0, 0, SCR_EAITalkRequestPreset.MEDIUM);
					commsHandler.AddRequest(rq);
				}
			}
		}
	}
	
	//---------------------------------------------------------------------------------------------------
	void CancelActivitiesRelatedToWaypoint(notnull AIWaypoint waypoint, typename activityType = typename.Empty)
	{
		array<ref AIActionBase> actions = {};
		GetActions(actions);
		bool checkType = activityType != typename.Empty;
		foreach (AIActionBase action : actions)
		{
			SCR_AIActivityBase activity = SCR_AIActivityBase.Cast(action);
			
			if (!activity || (checkType && !activity.IsInherited(activityType)))
				continue;
			
			if (activity.m_RelatedWaypoint == waypoint)
				activity.Fail();
		}
	}
	
	//---------------------------------------------------------------------------------------------------
	//! Determines when we can rebalance fireteams. We don't want to do that when fighting for example.
	protected bool CanRebalanceFireteams()
	{
		// Can't rebalance fireteams if there is any fireteams-related activity
		array<ref AIActionBase> allActions = {};
		GetActions(allActions);
		
		array<AIActionBase> subactions = {};
		foreach (AIActionBase action : allActions)
		{
			if (SCR_AIFireteamsActivity.Cast(action))
				return false;
			SCR_AICompositeActionParallel parallel = SCR_AICompositeActionParallel.Cast(action);
			if (parallel)
			{
				parallel.GetSubactions(subactions);
				foreach (AIActionBase subaction : subactions)
				{
					if (SCR_AIFireteamsActivity.Cast(subaction))
						return false;
				}
			}
		}
		
		return true;
	}
	

	//---------------------------------------------------------------------------------------------------	
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		m_Owner = SCR_AIGroup.Cast(owner);
		if (!m_Owner)
			return;	
		
		m_ConfigComponent = SCR_AIConfigComponent.Cast(m_Owner.FindComponent(SCR_AIConfigComponent));
		
		m_ConfigComponent.AddDefaultActivities(this);
		//AddAction(new SCR_AIIdleActivity(this))
		
		m_Owner.GetOnAgentAdded().Insert(OnAgentAdded);
		m_Owner.GetOnAgentRemoved().Insert(OnAgentRemoved);
		m_Owner.GetOnWaypointCompleted().Insert(OnWaypointCompleted);
		m_Owner.GetOnWaypointRemoved().Insert(OnWaypointRemoved);
		m_Owner.GetOnCurrentWaypointChanged().Insert(OnCurrentWaypointChanged);
		
		m_GroupInfo = SCR_AIGroupInfoComponent.Cast(m_Owner.FindComponent(SCR_AIGroupInfoComponent));
		
		m_TargetClusterProcessor = new SCR_AIGroupTargetClusterProcessor(this);
		m_TargetClusterProcessor.m_OnClusterStateChanged.Insert(OnTargetClusterStateChanged);
		
		m_FireteamMgr = new SCR_AIGroupFireteamManager(m_Owner);
		
		m_Perception = new SCR_AIGroupPerception(this, m_Owner);
		m_Perception.GetOnEnemyDetectedFiltered().Insert(OnEnemyDetectedFiltered);
		
		m_Mailbox = SCR_MailboxComponent.Cast(m_Owner.FindComponent(SCR_MailboxComponent));
		
		m_fPerceptionUpdateTimer_ms = Math.RandomFloat(0, PERCEPTION_UPDATE_TIMER_MS);
	}
	
	//---------------------------------------------------------------------------------------------------
	override void EOnDiag(IEntity owner, float timeSlice)
	{
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_TARGET_CLUSTERS))
			m_Perception.DiagDrawClusters();
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_FIRETEAMS))
			m_FireteamMgr.DiagDrawFireteams();
	}
	
	//---------------------------------------------------------------------------------------------------
	void UpdateGroupControlMode(SCR_AIActionBase currentAction)
	{
		#ifdef AI_DEBUG
		AddDebugMessage("UpdateGroupControlMode");
		#endif
		
		if (m_GroupInfo)
		{
			SCR_AIActivityBase currentActivity = SCR_AIActivityBase.Cast(currentAction);
			
			if (currentActivity && currentActivity.m_bIsWaypointRelated.m_Value)
				m_GroupInfo.SetGroupControlMode(EGroupControlMode.FOLLOWING_WAYPOINT);
			else if (SCR_AIIdleActivity.Cast(currentActivity))
				m_GroupInfo.SetGroupControlMode(EGroupControlMode.IDLE);
			else
				m_GroupInfo.SetGroupControlMode(EGroupControlMode.AUTONOMOUS);
		}
	}
	
	//---------------------------------------------------------------------------
	// Iterates all clusters and updates their state
	void UpdateClustersState(float deltaTime_ms)
	{
		foreach (SCR_AIGroupTargetCluster cluster : m_Perception.m_aTargetClusters)
		{
			m_TargetClusterProcessor.UpdateCluster(cluster, cluster.m_State, deltaTime_ms);
		}
	}
	
	//---------------------------------------------------------------------------
	// Decides whether we are allowed to go to this position
	bool IsPositionAllowed(vector pos)
	{		
		AIWaypoint wp = m_Owner.GetCurrentWaypoint();
		
		// If there is no waypoint, we can go anywhere
		if (!wp)
			return true;
		
		// If we have a defend wp, we can go only inside of it
		SCR_DefendWaypoint defendWp = SCR_DefendWaypoint.Cast(wp);
		if (defendWp)
			return vector.Distance(defendWp.GetOrigin(), pos) < defendWp.GetCompletionRadius();
		
		return true;
	}
	
	//---------------------------------------------------------------------------
	// Diagnostics and debugging
	
	
};
