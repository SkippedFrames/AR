[EntityEditorProps(category: "GameScripted/Tasks", description: "A requested task.", color: "0 0 255 255")]
class SCR_RequestedTaskClass: SCR_BaseTaskClass
{
};

//THIS TASK TYPE IS NOT SYNCHRONIZED BY ITSELF YET. WHEREAS IT'S INHERITED (E.G.) SCR_EvacuateTask IS.
//------------------------------------------------------------------------------------------------
class SCR_RequestedTask : SCR_BaseTask
{
	[Attribute(SCR_EXPRewards.SUPPORT_EVAC.ToString(), uiwidget: UIWidgets.ComboBox, enums:ParamEnumArray.FromEnum(SCR_EXPRewards))]
	SCR_EXPRewards m_AssigneeXPReward;
	
	int m_iRequesterID = -1;
	SCR_BaseTaskExecutor m_Requester;
	
	static const string TASK_REQUEST_TRANSMITTED_TEXT = "#AR-Tasks_StatusTransmitted-UC";
	static const string TASK_SUPPORT_CANCELLED_TEXT = "#AR-Tasks_StatusCancelled-UC";
	protected static const string LOCAL_PLAYER_REQUEST = "#AR-Tasks_TitleRequest";
	
	//------------------------------------------------------------------------------------------------
	protected override void ShowAvailableTask(bool afterAssigneeRemoved = false)
	{
		SCR_BaseTaskExecutor localExecutor = SCR_BaseTaskExecutor.GetLocalExecutor();
		if (localExecutor == m_Requester)
		{
			if (!afterAssigneeRemoved)
				SCR_PopUpNotification.GetInstance().PopupMsg(TASK_REQUEST_TRANSMITTED_TEXT + " " + m_sName, prio: SCR_ECampaignPopupPriority.TASK_AVAILABLE);
			return;
		}
		
		Faction localPlayerFaction;
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (factionManager)
			localPlayerFaction = factionManager.GetLocalPlayerFaction();
		
		if (localPlayerFaction == GetTargetFaction())
			SCR_PopUpNotification.GetInstance().PopupMsg(TASK_AVAILABLE_TEXT + " " + m_sName, prio: SCR_ECampaignPopupPriority.TASK_AVAILABLE, text2: TASK_HINT_TEXT, text2param1: SCR_PopUpNotification.TASKS_KEY_IMAGE_FORMAT, sound: SCR_SoundEvent.TASK_CREATED);
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsLocallyRequestedTask()
	{
		if (!GetTaskManager())
			return false;
		
		SCR_RequestedTaskSupportEntity supportEntity = SCR_RequestedTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_RequestedTaskSupportEntity));
		if (!supportEntity)
			return false;
		
		return this == supportEntity.GetLocallyRequestedTask();
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetTitleWidgetText(notnull TextWidget textWidget, string taskText)
	{
		if (IsLocallyRequestedTask())
		{
			textWidget.SetTextFormat(GetTaskManager().m_sLocalRequestTitle);
			return;
		}
		if (m_Requester)
			textWidget.SetTextFormat(taskText, m_Requester.GetPlayerName());
	}

	//------------------------------------------------------------------------------------------------
	override string GetTitleText()
	{
		if (IsLocallyRequestedTask())
			return GetTaskManager().m_sLocalRequestTitle;

		if (m_Requester)
			return string.Format("%1 %2", m_sName, m_Requester.GetPlayerName());

		return string.Empty;
	}
	
	//------------------------------------------------------------------------------------------------
	override Widget GenerateTaskDescriptionUI(notnull Widget rootWidget, array<Widget> widgets)
	{
		Widget w = super.GenerateTaskDescriptionUI(rootWidget, widgets);
		
		if (IsLocallyRequestedTask())
		{
			Widget description = w.FindAnyWidget("TaskDescription");
		}
		
		return w;
	}
	
	//------------------------------------------------------------------------------------------------
	override void DoNotifyAssignment(int assigneeID)
	{
		SCR_CampaignNetworkComponent assigneeNetworkComponent = SCR_CampaignNetworkComponent.GetCampaignNetworkComponent(assigneeID);
		
		if (assigneeNetworkComponent)
			assigneeNetworkComponent.SendPlayerMessage(GetAssignMessage(), calledID: m_iRequesterID);
	}
	
	//------------------------------------------------------------------------------------------------
	override void DoNotifyUnassign(int assigneeID)
	{
		SCR_CampaignNetworkComponent assigneeNetworkComponent = SCR_CampaignNetworkComponent.GetCampaignNetworkComponent(assigneeID);
		
		if (assigneeNetworkComponent)
			assigneeNetworkComponent.SendPlayerMessage(GetUnassignMessage(), calledID: m_iRequesterID);
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetDescriptionWidgetText(notnull TextWidget textWidget, string taskText)
	{
		textWidget.SetTextFormat(taskText, GetRequesterName());
	}
	
	//------------------------------------------------------------------------------------------------
	override string GetTaskListTaskText()
	{
		if (IsLocallyRequestedTask())
			return LOCAL_PLAYER_REQUEST;
		
		return super.GetTaskListTaskText();
	}
	
	//------------------------------------------------------------------------------------------------
	override void Cancel(bool showMsg = true)
	{
		SCR_BaseTaskExecutor localExecutor = SCR_BaseTaskExecutor.GetLocalExecutor();
		if (localExecutor == m_Requester || localExecutor == GetAssignee())
		{
			SCR_PopUpNotification.GetInstance().PopupMsg(TASK_CANCELLED_TEXT + " " + m_sName, prio: SCR_ECampaignPopupPriority.TASK_DONE, sound: SCR_SoundEvent.TASK_CANCELED);
		}
		
		super.Cancel();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Fails the task.
	override void Fail(bool showMsg = true)
	{
		SCR_BaseTaskExecutor localExecutor = SCR_BaseTaskExecutor.GetLocalExecutor();

		if (localExecutor == GetAssignee())
			SCR_PopUpNotification.GetInstance().PopupMsg(TASK_FAILED_TEXT + " " + m_sName, prio: SCR_ECampaignPopupPriority.TASK_DONE, sound: SCR_SoundEvent.TASK_FAILED);
		
		if (!m_Requester)
		{
			if (localExecutor.GetTaskExecutorID(localExecutor) == m_iRequesterID)
				SCR_PopUpNotification.GetInstance().PopupMsg(TASK_SUPPORT_CANCELLED_TEXT + " " + m_sName, prio: SCR_ECampaignPopupPriority.TASK_DONE, sound: SCR_SoundEvent.TASK_CANCELED);
		}
		else
		{
			if (localExecutor == m_Requester)
				SCR_PopUpNotification.GetInstance().PopupMsg(TASK_SUPPORT_CANCELLED_TEXT + " " + m_sName, prio: SCR_ECampaignPopupPriority.TASK_DONE, sound: SCR_SoundEvent.TASK_CANCELED);
		}
		
		super.Fail(showMsg);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Marks the task as finished.
	override void Finish(bool showMsg = true)
	{
		SCR_BaseTaskManager taskManager = GetTaskManager();
		SCR_BaseTaskExecutor assignee = GetAssignee();
		
		if (GetTaskManager().IsProxy())
		{
			SCR_BaseTaskExecutor localExecutor = SCR_BaseTaskExecutor.GetLocalExecutor();
			if (localExecutor && (localExecutor == GetRequester() || localExecutor == assignee))
				SCR_PopUpNotification.GetInstance().PopupMsg(TASK_COMPLETED_TEXT + " " + m_sName, prio: SCR_ECampaignPopupPriority.TASK_DONE, sound: SCR_SoundEvent.TASK_SUCCEED, text2: SCR_BaseTask.TASK_HINT_TEXT, text2param1: SCR_PopUpNotification.TASKS_KEY_IMAGE_FORMAT);
			return;
		}
		
		if (!assignee)
			return;
		
		// Award XP to the volunteer
		SCR_XPHandlerComponent comp = SCR_XPHandlerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_XPHandlerComponent));
		
		if (comp)
			comp.AwardXP(SCR_BaseTaskExecutor.GetTaskExecutorID(assignee), m_AssigneeXPReward);
		
		super.Finish(showMsg);
	}
	
	//------------------------------------------------------------------------------------------------
	void RequesterDied()
	{
		SCR_BaseTaskManager taskManager = GetTaskManager();
		
		if (!taskManager || taskManager.IsProxy())
			return;
		
		SCR_BaseTaskSupportEntity supportEntity = SCR_BaseTaskSupportEntity.Cast(taskManager.FindSupportEntity(SCR_BaseTaskSupportEntity));
		if (!supportEntity)
			return;
		
		supportEntity.FailTask(this);
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_BaseTaskExecutor GetRequester()
	{
		return m_Requester;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetRequesterName()
	{
		return GetGame().GetPlayerManager().GetPlayerName(m_iRequesterID);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnEntityDeath(IEntity killedEntity)
	{
		if (!m_Requester)
			return;
		if (killedEntity == m_Requester.GetControlledEntity())
			Fail(); //TODO
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRequesterID(int requesterID)
	{
		m_iRequesterID = requesterID;
		SetEventMask(EntityEvent.FRAME);
	}
	
	//------------------------------------------------------------------------------------------------
	bool AutoSetRequester()
	{
		if (m_iRequesterID == -1)
			return false;
		
		SCR_BaseTaskExecutor requester = SCR_BaseTaskExecutor.GetTaskExecutorByID(m_iRequesterID);
		
		if (requester)
			SetRequester(requester);
		else
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns whether this task can be assigned to the player with the given player ID
	//! If playerID == -1, the local player is used as the target
	override bool CanBeAssigned(out SCR_ECannotAssignReasons reason, int playerID = -1)
	{
		SCR_BaseTaskExecutor localExecutor = SCR_BaseTaskExecutor.GetLocalExecutor();
		if (!localExecutor)
			return false;
		
		if (localExecutor == m_Requester)
		{
			reason = SCR_ECannotAssignReasons.IS_TASK_REQUESTER;
			return false;
		}
		
		return super.CanBeAssigned(reason, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRequester(SCR_BaseTaskExecutor requester)
	{
		if (!GetTaskManager())
			return;
		
		SCR_RequestedTaskSupportEntity supportEntity = SCR_RequestedTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_RequestedTaskSupportEntity));
		if (!supportEntity)
			return;
		
		if (!requester)
			return;
		
		m_Requester = requester;
		m_iRequesterID = SCR_BaseTaskExecutor.GetTaskExecutorID(requester);
		
		IEntity requesterEntity = requester.GetControlledEntity();
		if (!requesterEntity)
			return;
		
		SCR_CharacterControllerComponent characterControllerComponent = SCR_CharacterControllerComponent.Cast(requesterEntity.FindComponent(SCR_CharacterControllerComponent));
		if (!characterControllerComponent)
			return;
		
		characterControllerComponent.GetOnPlayerDeath().Insert(RequesterDied);
		
		SCR_BaseTaskExecutor localTaskExecutor = SCR_BaseTaskExecutor.GetLocalExecutor();
		if (localTaskExecutor == requester)
			supportEntity.SetLocallyRequestedTask(this);
		
		FactionAffiliationComponent factionAffiliationComponent = FactionAffiliationComponent.Cast(requesterEntity.FindComponent(FactionAffiliationComponent));
		if (factionAffiliationComponent)
			m_TargetFaction = factionAffiliationComponent.GetAffiliatedFaction();
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (!m_Requester)
			AutoSetRequester();
	}
	
	//------------------------------------------------------------------------------------------------
	override void Deserialize(ScriptBitReader reader)
	{
		super.Deserialize(reader);
		
		int requesterID;
		reader.ReadInt(requesterID);
		SetRequesterID(requesterID);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Serialize(ScriptBitWriter writer)
	{
		super.Serialize(writer);
		
		writer.WriteInt(m_iRequesterID);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_RequestedTask(IEntitySource src, IEntity parent)
	{
		if (SCR_Global.IsEditMode(this))
			return;
		
		SetEventMask(EntityEvent.FRAME);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_RequestedTask()
	{
	}

};
