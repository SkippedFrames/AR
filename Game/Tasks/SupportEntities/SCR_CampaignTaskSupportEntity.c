//------------------------------------------------------------------------------------------------
[EntityEditorProps(category: "GameScripted/Tasks", description: "Campaign task support entity.", color: "0 0 255 255")]
class SCR_CampaignTaskSupportEntityClass: SCR_CampaignBaseTaskSupportEntityClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignTaskSupportEntity : SCR_CampaignBaseTaskSupportEntity
{
	protected bool m_bInitialTasksGenerated = false;
	protected ref array<SCR_CampaignMilitaryBaseComponent> m_aCheckedBases = {};
	
	//------------------------------------------------------------------------------------------------
	//! Returns the found task if it exists.
	//TODO: Remove this method after no longer needed.
	SCR_CampaignTask GetTask(SCR_CampaignMilitaryBaseComponent targetBase, Faction targetFaction, SCR_CampaignTaskType type)
	{
		if (!GetTaskManager())
			return null;
		
		array<SCR_BaseTask> tasks = {};
		int count = GetTaskManager().GetTasks(tasks);
		
		for (int i = count - 1; i >= 0; i--)
		{
			SCR_CampaignTask castTask = SCR_CampaignTask.Cast(tasks[i]);
			
			if (!castTask)
				continue;
			
			if (castTask.GetType() == type && castTask.GetTargetBase() == targetBase && castTask.GetTargetFaction() == targetFaction)
				return castTask;
		}
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Server side
	//! Called after a new base has been captured or a mobile HQ deployed
	//! Creates capture tasks for small, major and main bases in newly gained range
	void GenerateCaptureTasks(notnull IEntity entity)
	{
		if (!GetTaskManager())
			return;
		
		SCR_CampaignTaskType type = SCR_CampaignTaskType.CAPTURE;
		array<SCR_CampaignMilitaryBaseComponent> bases = new array<SCR_CampaignMilitaryBaseComponent>();
		SCR_CampaignFaction faction;
		SCR_CampaignMilitaryBaseComponent capturedBase = SCR_CampaignMilitaryBaseComponent.Cast(entity.FindComponent(SCR_CampaignMilitaryBaseComponent));
		
		if (capturedBase)
		{
			// It's a base
			m_aCheckedBases.Insert(capturedBase);
			GenerateRecaptureTask(capturedBase);
			faction = capturedBase.GetCampaignFaction();
			
			if (!faction)
				return;
			
			if (!capturedBase.IsHQRadioTrafficPossible(faction))
				return;
			
			capturedBase.GetBasesInRange(SCR_ECampaignBaseType.BASE | SCR_ECampaignBaseType.RELAY, bases);
			
			foreach (SCR_CampaignMilitaryBaseComponent base: bases)
			{
				if (base.IsHQ())
					continue;
				
				if (GetTask(base, faction, type))
					continue;
				
				if (base.GetFaction() == faction)
				{
					if (m_aCheckedBases.Contains(base))
						continue;
					
					GenerateCaptureTasks(base.GetOwner());
					continue;
				}
				
				CreateNewCampaignTask(type, base, faction, true);
			}
		}
		else
		{
			// It's a mobile HQ
			SCR_CampaignMobileAssemblyComponent comp = SCR_CampaignMobileAssemblyComponent.Cast(entity.FindComponent(SCR_CampaignMobileAssemblyComponent));
			
			if (!comp)
				return;
			
			faction = comp.GetParentFaction();
			comp.GetBasesInRange(bases);
			
			foreach (SCR_CampaignMilitaryBaseComponent base: bases)
			{
				if (GetTask(base, faction, type))
					continue;
				
				if (base.IsHQ())
					continue;
				
				if (base.GetFaction() == faction)
				{
					if (m_aCheckedBases.Contains(base))
						continue;
					
					GenerateCaptureTasks(base.GetOwner());
					continue;
				}
				
				CreateNewCampaignTask(type, base, faction, true);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void GenerateRecaptureTask(notnull SCR_CampaignMilitaryBaseComponent capturedBase)
	{
		if (capturedBase.IsHQ())
			return;
		
		if (!GetTaskManager())
			return;
		
		SCR_CampaignFactionManager factionManager = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return;
		
		SCR_CampaignFaction owningFaction = capturedBase.GetCampaignFaction();
		if (!owningFaction)
			return;
		
		SCR_CampaignFaction opposingFaction = factionManager.GetEnemyFaction(owningFaction);
		if (!opposingFaction)
			return;
		
		if (GetTask(capturedBase, opposingFaction, SCR_CampaignTaskType.CAPTURE))
			return;
		
		if (!capturedBase.IsHQRadioTrafficPossible(opposingFaction))
			return;
		
		CreateNewCampaignTask(SCR_CampaignTaskType.CAPTURE, capturedBase, opposingFaction, true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void GenerateInitialTasksForAllBases()
	{
		if (!GetTaskManager() || !GetTaskManager().Initialized() || m_bInitialTasksGenerated)
			return;
		
		m_bInitialTasksGenerated = true;
		
		//On proxy we don't create any tasks, we skip the creation
		if (GetTaskManager().IsProxy())
			return;
		
		//Get bases manager
		SCR_MilitaryBaseManager manager = SCR_MilitaryBaseManager.GetInstance();
		
		//Get all bases
		array<SCR_MilitaryBaseComponent> bases = {};
		manager.GetBases(bases);
		
		//Create start tasks for each base
		for (int i = bases.Count() - 1; i >= 0; i--)
		{
			SCR_CampaignMilitaryBaseComponent campaignBase = SCR_CampaignMilitaryBaseComponent.Cast(bases[i]);
			
			if (!campaignBase || !campaignBase.IsInitialized())
				continue;
			
			// Create tasks for this base
			if (campaignBase.GetFaction())
				GenerateNewTask(campaignBase);
		}
		
		//Let the game mode know, the tasks are ready
		SCR_BaseGameMode gamemode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gamemode)
			gamemode.HandleOnTasksInitialized();
	}
	
	//------------------------------------------------------------------------------------------------
	//!	This method is called when a new base has been captured.
	//! Creates recon task around a base + creates capture tasks for bases in the newly gained radio signal.
	void GenerateNewTask(SCR_CampaignMilitaryBaseComponent capturedBase)
	{
#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_EXECUTE_TASKS))
			return;
#endif
		
		GenerateCaptureTasks(capturedBase.GetOwner());
	}
	
	//------------------------------------------------------------------------------------------------
	//! An event called when a base has been captured.
	void OnBaseCaptured(notnull SCR_MilitaryBaseComponent capturedBase, Faction faction)
	{
#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_EXECUTE_TASKS))
			return;
#endif
		
		SCR_CampaignMilitaryBaseComponent campaignBase = SCR_CampaignMilitaryBaseComponent.Cast(capturedBase);
		
		if (!campaignBase)
			return;
		
		m_aCheckedBases.Clear();
		
		if (!GetTaskManager().IsProxy())
			GenerateNewTask(campaignBase);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnBaseSignalChanged(notnull SCR_CampaignMilitaryBaseComponent base)
	{
		OnBaseCaptured(base, null);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Creates a new conflict task.
	SCR_CampaignTask CreateNewCampaignTask(SCR_CampaignTaskType type, SCR_CampaignMilitaryBaseComponent base, Faction faction, bool newTask = false)
	{
		if (!GetTaskManager())
			return null;
		
		if (!base || !faction)
			return null;
		
		if (type == SCR_CampaignTaskType.CAPTURE && (base.IsHQ() || (base.CanBeHQ() && base.GetDisableWhenUnusedAsHQ())))
			return null;
		
		if (base.GetFaction() == faction && type == SCR_CampaignTaskType.CAPTURE)
			return null;
		
		SCR_CampaignTask task = SCR_CampaignTask.Cast(CreateTask());
		if (!task)
			return null;
		
		SetTargetFaction(task, faction); // Replicated internally
		SetTargetBase(task, base); // Replicated internally
		SetTaskType(task, type); // Replicated internally
		
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (campaign && newTask && SCR_FactionManager.SGetLocalPlayerFaction() == faction)
		{
			string text = task.TASK_AVAILABLE_TEXT + " " + task.GetTitle();
			string baseName;
			
			if (task.GetTargetBase())
				baseName = task.GetTargetBase().GetBaseNameUpperCase();

			SCR_PopUpNotification.GetInstance().PopupMsg(text, prio: SCR_ECampaignPopupPriority.TASK_AVAILABLE, param1: baseName, text2: task.TASK_HINT_TEXT, text2param1: SCR_PopUpNotification.TASKS_KEY_IMAGE_FORMAT, sound: SCR_SoundEvent.TASK_CREATED);
		}
		
		return task;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetTaskType(notnull SCR_CampaignTask task, SCR_CampaignTaskType type)
	{
		if (!GetTaskManager())
			return;
		
		int taskID = task.GetTaskID();
		
		Rpc(RPC_SetTaskType, taskID, type);
		RPC_SetTaskType(taskID, type);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_SetTaskType(int taskID, SCR_CampaignTaskType type)
	{
		if (!GetTaskManager())
			return;
		
		SCR_CampaignTask task = SCR_CampaignTask.Cast(GetTaskManager().GetTask(taskID));
		if (!task)
			return;
		
		task.SetType(type);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Initialize()
	{
		// Timeout added to allow for backend load process
		GetGame().GetCallqueue().CallLater(GenerateInitialTasksForAllBases, 2000);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_CampaignTaskSupportEntity(IEntitySource src, IEntity parent)
	{
		//Register to script invokers
		SCR_MilitaryBaseManager.GetInstance().GetOnBaseFactionChanged().Insert(OnBaseCaptured);
		
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (!campaign)
			return;
		
		campaign.GetBaseManager().GetOnSignalChanged().Insert(OnBaseSignalChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_CampaignTaskSupportEntity()
	{
		//Unregister from script invokers
		SCR_MilitaryBaseManager baseManager = SCR_MilitaryBaseManager.GetInstance(false);
		
		if (baseManager)
			baseManager.GetOnBaseFactionChanged().Remove(OnBaseCaptured);
		
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		
		if (!campaign)
			return;
		
		campaign.GetBaseManager().GetOnSignalChanged().Remove(OnBaseSignalChanged);
	}
};
