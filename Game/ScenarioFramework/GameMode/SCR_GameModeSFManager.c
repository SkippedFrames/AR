[EntityEditorProps(category: "GameScripted/GameMode", description: "")]
class SCR_GameModeSFManagerClass : SCR_BaseGameModeComponentClass
{
	// prefab properties here
};

//------------------------------------------------------------------------------------------------
enum SCR_ScenarioFrameworkEActivationType
{
	SAME_AS_PARENT = 0,
	ON_TRIGGER_ACTIVATION,
	ON_AREA_TRIGGER_ACTIVATION,
	ON_INIT,						//when the game mode is initiated
	ON_TASKS_INIT,					//when the  game mode starts creating tasks
	CUSTOM1,						//won't spawn until something will try to spawn the object with CUSTOM as parameter
	CUSTOM2,
	CUSTOM3,
	CUSTOM4,
};

enum SCR_ESFTaskType
{
	NONE,
	DELIVER,
	DESTROY,
	DEFEND,
	KILL,
	CLEAR_AREA,
	LAST,
	EXTRACTION,
	DEFAULT
};

class SCR_GameModeSFManager : SCR_BaseGameModeComponent
{	
	[Attribute("Available Tasks for the Scenario", category: "Tasks")];
	protected ref array<ref SCR_ScenarioFrameworkTaskType> m_aTaskTypesAvailable;
	
	[Attribute( defvalue: "3", desc: "Maximal number of tasks that can be generated", category: "Tasks" )];
	protected int m_iMaxNumberOfTasks;
	
	[Attribute(UIWidgets.Auto, desc: "Actions that will be activated after tasks are initialized", category: "Tasks")];
	protected ref array<ref SCR_ScenarioFrameworkActionBase> m_aAfterTasksInitActions;
	
	[Attribute(desc: "Name of the Area which will be only one to spawn", category: "Debug")];
	protected string m_sForcedArea;
	
	[Attribute(desc: "Name of the task layer to be only one to create", category: "Debug")];
	protected string m_sForcedTaskLayer;
	
	[Attribute(desc: "Should the dynamic Spawn/Despawn based on distance from observer cameras be enabled for the whole GameMode?", category: "Dynamic Spawn/Despawn")];
	protected bool m_bDynamicDespawn;
	
	[Attribute(defvalue: "4", UIWidgets.Slider, params: "0 600 1", desc: "How frequently is dynamic spawn/despawn being checked in seconds", category: "Dynamic Spawn/Despawn")]
	protected int m_iUpdateRate;
	
	protected bool m_bMatchOver;
	protected int m_iCurrentlySpawnedLayerTasks;
	
	protected ref ScriptInvoker m_OnAllAreasInitiated;
	protected ref ScriptInvoker m_OnTaskStateChanged;
	
	protected SCR_ScenarioFrameworkLayerBase m_LastFinishedTaskLayer;
	protected SCR_BaseTask m_LastFinishedTask;
	protected EGameOverTypes m_eGameOverType = EGameOverTypes.COMBATPATROL_DRAW;
	
	protected ref array<SCR_ScenarioFrameworkArea> m_aAreas = {};
	protected ref array<SCR_ScenarioFrameworkArea> m_aSelectedAreas = {};
	protected ref array<SCR_ScenarioFrameworkLayerTask> m_aLayerTasksToBeInitialized = {};
	protected ref array<SCR_ScenarioFrameworkLayerTask> m_aLayerTasksForRandomization = {};
	protected ref array<string> m_aAreasTasksToSpawn = {};
	protected ref array<string> m_aLayersTaskToSpawn = {};
	protected ref array<SCR_ESFTaskType> m_aESFTaskTypesAvailable = {};
	protected ref array<SCR_ESFTaskType> m_aESFTaskTypeForRandomization = {};
	
	protected ref array<ref Tuple3<SCR_ScenarioFrameworkArea, vector, int>> m_aSpawnedAreas = {};
	protected ref array<ref Tuple3<SCR_ScenarioFrameworkArea, vector, int>> m_aDespawnedAreas = {};
	protected ref array<vector> m_aObservers = {};
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_PlaySoundOnEntity(EntityID pEntID, string sSndName)
	{
		if (!pEntID)
			return;	

		IEntity entity = GetGame().GetWorld().FindEntityByID(pEntID);
		if (!entity)
			return;

		SCR_CommunicationSoundComponent pSndComp = SCR_CommunicationSoundComponent.Cast(entity.FindComponent(SCR_CommunicationSoundComponent));
		if (!pSndComp)
			return;

		pSndComp.PlayStr(sSndName);
	}
	
	//------------------------------------------------------------------------------------------------
	void PlaySoundOnEntity(IEntity entity, string sSndName)
	{
		if (!entity)
			entity = GetOwner();		//play it on game mode if any entity is passed

		if (!entity)
			return;

		if (IsMaster())
			Rpc(RpcDo_PlaySoundOnEntity, entity.GetID(), sSndName);
		
		RpcDo_PlaySoundOnEntity(entity.GetID(), sSndName);		
	}
	
	//------------------------------------------------------------------------------------------------
	void PlaySoundOnPlayer(string sSndName)
	{
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		
		if (!pc)
			return;
		IEntity player = pc.GetMainEntity();
		
		if (!player)
			return;
		
		PlaySoundOnEntity(player, sSndName);
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_BaseTask GetLastFinishedTask()
	{
		return m_LastFinishedTask;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ScenarioFrameworkLayerBase GetLastFinishedTaskLayer()
	{
		return m_LastFinishedTaskLayer;
	}
	
	//------------------------------------------------------------------------------------------------
	array<SCR_ScenarioFrameworkArea> GetAreas()
	{
		return m_aAreas;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnTaskCreated(SCR_BaseTask task)
	{
		Faction faction =  task.GetTargetFaction();
		if (faction)
			PopUpMessage(task.GetTitle(), "#AR-CampaignTasks_NewObjectivesAvailable-UC", faction.GetFactionKey());
		else
			PopUpMessage(task.GetTitle(), "#AR-CampaignTasks_NewObjectivesAvailable-UC");
	}
	
	//------------------------------------------------------------------------------------------------
	void OnTaskUpdate(SCR_BaseTask task, SCR_ETaskEventMask mask)
	{
		if (!task) 
			return;

		Faction faction =  task.GetTargetFaction();
		
		if (task.GetTaskState() == SCR_TaskState.FINISHED)
		{
			m_LastFinishedTaskLayer = SCR_ScenarioFrameworkTask.Cast(task).GetTaskLayer(); 
			m_LastFinishedTask = task;
		}
		
		if (mask & SCR_ETaskEventMask.TASK_PROPERTY_CHANGED && !(mask & SCR_ETaskEventMask.TASK_CREATED) && !(mask & SCR_ETaskEventMask.TASK_FINISHED) && !(mask & SCR_ETaskEventMask.TASK_ASSIGNEE_CHANGED))
		{
			if (faction)
				PopUpMessage(task.GetTitle(), "#AR-Workshop_ButtonUpdate", faction.GetFactionKey());
			else
				PopUpMessage(task.GetTitle(), "#AR-Workshop_ButtonUpdate");
			
			SCR_ScenarioFrameworkLayerTask taskLayer = SCR_ScenarioFrameworkTask.Cast(task).GetTaskLayer();
			SCR_ScenarioFrameworkSlotTask subject = taskLayer.GetTaskSubject();
			if (subject)
				subject.OnTaskStateChanged(SCR_TaskState.UPDATED);
		}

		GetOnTaskStateChanged().Invoke(task, mask);
	}
	
	void SetMissionEndScreen(EGameOverTypes GameOverType)
	{
		m_eGameOverType = GameOverType;
	}
	
	//------------------------------------------------------------------------------------------------
	void Finish()
	{
		SCR_GameModeEndData endData = SCR_GameModeEndData.CreateSimple(m_eGameOverType, 0,0);
		
		m_bMatchOver = true;
		
		SCR_BaseGameMode.Cast(GetOwner()).EndGameMode(endData);
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetIsMatchOver()
	{
		return m_bMatchOver;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnAllAreasInitiated()
	{
		if (!m_OnAllAreasInitiated)
			m_OnAllAreasInitiated = new ScriptInvoker();
		
		return m_OnAllAreasInitiated;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnTaskStateChanged()
	{
		if (!m_OnTaskStateChanged)
			m_OnTaskStateChanged = new ScriptInvoker();
		
		return m_OnTaskStateChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ScenarioFrameworkArea SelectNearestAreaByTaskType(SCR_ESFTaskType eTaskType)
	{
		if (m_aAreas.IsEmpty())
			return null;

		SCR_ScenarioFrameworkArea selectedArea = null;
		if (!m_LastFinishedTask)
			return null;

		vector vTaskPos = m_LastFinishedTask.GetOrigin();
		float fMinDistance = float.MAX;
		float fDistance = 0;
		for (int i = 0, count = m_aAreas.Count(); i < count; i++)
		{
			if (!m_aAreas[i].GetIsTaskSuitableForArea(eTaskType))
				continue;
			
			fDistance = vector.Distance(vTaskPos, m_aAreas[i].GetOwner().GetOrigin());
			if (fDistance < fMinDistance)
			{
				fMinDistance = fDistance;
				selectedArea = m_aAreas[i];
			}
		}
		return selectedArea;
	}

	
	//------------------------------------------------------------------------------------------------	
	bool IsMaster()// IsServer
	{
		RplComponent comp = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (!comp)
			return false;			//by purpose - debug

		return !comp.IsProxy();
	}
	
	//------------------------------------------------------------------------------------------------
	void RegisterArea(SCR_ScenarioFrameworkArea area)
	{
		if (m_aAreas.Find(area) == -1)
			m_aAreas.Insert(area);
	}
	
	//------------------------------------------------------------------------------------------------
	void StoreAreaStates(out notnull array<ref SCR_ScenarioFrameworkAreaStruct> outEntries)
	{
		if (!m_aAreas)
			return;
		
		for (int i = m_aAreas.Count() - 1; i >= 0; i--)
		{
			SCR_ScenarioFrameworkAreaStruct struct = new SCR_ScenarioFrameworkAreaStruct();
			m_aAreas[i].StoreState(struct);
			outEntries.Insert(struct);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void LoadAreaStates(notnull array<ref SCR_ScenarioFrameworkAreaStruct> loadedAreaStruct)
	{
		if (!m_aAreas)
			return;
		
		IEntity entity;
		SCR_ScenarioFrameworkArea area;
		foreach (SCR_ScenarioFrameworkAreaStruct areaStruct : loadedAreaStruct)
		{
			entity = GetGame().GetWorld().FindEntityByName(areaStruct.GetName());
			if (!entity)
				continue;

			area = SCR_ScenarioFrameworkArea.Cast(entity.FindComponent(SCR_ScenarioFrameworkArea));
			if (!area)
				continue;
			
			LoadAreaStructs(area, areaStruct);
			LoadNestedAreaStructs(areaStruct);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void LoadAreaStructs(SCR_ScenarioFrameworkArea area, SCR_ScenarioFrameworkAreaStruct areaStruct)
	{
		if (areaStruct.GetAreaSelected())
		{
			m_aAreasTasksToSpawn.Insert(areaStruct.GetName());
			m_aLayersTaskToSpawn.Insert(areaStruct.GetLayerTaskname());
		}
		
		if (areaStruct.GetDeliveryPointNameForItem())
			area.StoreDeliveryPoint(areaStruct.GetDeliveryPointNameForItem());
			
		if (areaStruct.GetRandomlySpawnedChildren())
			area.SetRandomlySpawnedChildren(areaStruct.GetRandomlySpawnedChildren());
			
		LoadRepeatedSpawnAreaStructs(area, areaStruct);
		
	}
	
	//------------------------------------------------------------------------------------------------
	protected void LoadRepeatedSpawnAreaStructs(SCR_ScenarioFrameworkArea area, SCR_ScenarioFrameworkAreaStruct areaStruct)
	{
		if (areaStruct.GetEnableRepeatedSpawn())
		{
			area.SetEnableRepeatedSpawn(areaStruct.GetEnableRepeatedSpawn());
			if (areaStruct.GetRepeatedSpawnNumber())
				area.SetRepeatedSpawnNumber(areaStruct.GetRepeatedSpawnNumber());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void LoadNestedAreaStructs(SCR_ScenarioFrameworkAreaStruct areaStruct)
	{
		if (areaStruct.GetLayerStructs())
			LoadLayer(areaStruct.GetLayerStructs());
			
		if (areaStruct.GetLogicStructs())
			LoadLogic(areaStruct.GetLogicStructs());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void LoadLayer(notnull array<ref SCR_ScenarioFrameworkLayerStruct> loadedLayerStruct)
	{
		IEntity entity;
		SCR_ScenarioFrameworkLayerBase layer;
		foreach (SCR_ScenarioFrameworkLayerStruct layerStruct : loadedLayerStruct)
		{
			entity = GetGame().GetWorld().FindEntityByName(layerStruct.GetName());
			if (!entity)
				continue;
			
			layer = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (!layer)
				continue;
			
			LoadLayerStructs(layer, layerStruct);
			LoadNestedLayerStructs(layerStruct);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void LoadLayerStructs(SCR_ScenarioFrameworkLayerBase layer, SCR_ScenarioFrameworkLayerStruct layerStruct)
	{
		if (layerStruct.GetIsTerminated())
		{
			layer.SetIsTerminated(layerStruct.GetIsTerminated());
			return;
		}
			
		LoadRepeatedSpawnLayerStructs(layer, layerStruct);
		LoadLayerStructSlots(layer, layerStruct);
			
		//Layer task handling
		SCR_ScenarioFrameworkLayerTask layerTask = SCR_ScenarioFrameworkLayerTask.Cast(layer);
		if (layerTask)
			layerTask.SetLayerTaskState(layerStruct.GetLayerTaskState());
			
		if (layerStruct.GetRandomlySpawnedChildren())
			layer.SetRandomlySpawnedChildren(layerStruct.GetRandomlySpawnedChildren());
			
		LoadNestedLayerStructs(layerStruct);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void LoadRepeatedSpawnLayerStructs(SCR_ScenarioFrameworkLayerBase layer, SCR_ScenarioFrameworkLayerStruct layerStruct)
	{
		if (layerStruct.GetEnableRepeatedSpawn())
		{
			layer.SetEnableRepeatedSpawn(layerStruct.GetEnableRepeatedSpawn());
			if (layerStruct.GetRepeatedSpawnNumber())
				layer.SetRepeatedSpawnNumber(layerStruct.GetRepeatedSpawnNumber());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void LoadLayerStructSlots(SCR_ScenarioFrameworkLayerBase layer, SCR_ScenarioFrameworkLayerStruct layerStruct)
	{
		SCR_ScenarioFrameworkSlotBase slot;
		SCR_ScenarioFrameworkSlotAI slotAI;
		slot = SCR_ScenarioFrameworkSlotBase.Cast(layer.FindComponent(SCR_ScenarioFrameworkSlotBase));
		if (slot)
		{
			slotAI = SCR_ScenarioFrameworkSlotAI.Cast(slot.FindComponent(SCR_ScenarioFrameworkSlotAI));
			if (slotAI)
			{
				if (layerStruct.GetAIPrefabsForRemoval())
					slotAI.SetAIPrefabsForRemoval(layerStruct.GetAIPrefabsForRemoval());
			}
				
			if (layerStruct.GetRandomlySpawnedObject())
				slot.SetRandomlySpawnedObject(layerStruct.GetRandomlySpawnedObject());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void LoadNestedLayerStructs(SCR_ScenarioFrameworkLayerStruct layerStruct)
	{
		if (layerStruct.GetLayerStructs())
			LoadLayer(layerStruct.GetLayerStructs());
			
		if (layerStruct.GetLogicStructs())
			LoadLogic(layerStruct.GetLogicStructs());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void LoadLogic(notnull array<ref SCR_ScenarioFrameworkLogicStruct> entries)
	{
		IEntity entity;
		SCR_ScenarioFrameworkLogic logic;
		SCR_ScenarioFrameworkLogicCounter logicCounter;
		foreach (SCR_ScenarioFrameworkLogicStruct logicInfo : entries)
		{
			entity = GetGame().GetWorld().FindEntityByName(logicInfo.GetName());
			if (!entity)
				continue;
			
			logic = SCR_ScenarioFrameworkLogic.Cast(entity.FindComponent(SCR_ScenarioFrameworkLogic));
			if (!logic)
				continue;
			
			if (logicInfo.GetIsTerminated())
			{
				logic.SetIsTerminated(logicInfo.GetIsTerminated());
				continue;
			}
			
			logicCounter = SCR_ScenarioFrameworkLogicCounter.Cast(logic.FindComponent(SCR_ScenarioFrameworkLogicCounter));
			if (!logicCounter)
				continue;
			
			if (logicInfo.GetCounterValue())
			{
				logicCounter.SetCounterValue(logicInfo.GetCounterValue());
				continue;
			}		
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void LoadHeaderSettings();
	
	//------------------------------------------------------------------------------------------------
	protected bool Init()
	{
		if (!IsMaster())
			return false;

		LoadHeaderSettings();
		
		// Spawn everything inside the Area except the task layers
		foreach(SCR_ScenarioFrameworkArea area : m_aAreas)
		{
			if (m_sForcedArea.IsEmpty())		//for debug purposes
			{
				if (area.GetDynamicDespawnEnabled())
					continue;
				
				area.Init();
			}
			else
			{
				if (area.GetOwner().GetName() == m_sForcedArea)
					area.Init(area);
			}
		}
		
		SCR_ScenarioFrameworkLayerTask.s_OnTaskSetup.Insert(OnTaskCreated);	
		SCR_BaseTaskManager.s_OnTaskUpdate.Insert(OnTaskUpdate);
		
		//if someone registered for the event, then call it
		if (m_OnAllAreasInitiated)
			m_OnAllAreasInitiated.Invoke();
		
		PostInit();
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void PostInit()
	{
		if (m_sForcedTaskLayer.IsEmpty())
			GenerateTasks();
	}
	
	//------------------------------------------------------------------------------------------------
	//! It allows you to spawn just one selected area out of all the areas you have in your scenario. 
	//! This is a helpful way for Scenario Creators to quickly test some desired area without having to disable everything else.
	void SpawnForcedArea(SCR_ScenarioFrameworkArea area)
	{
		IEntity entity = GetGame().GetWorld().FindEntityByName(m_sForcedArea);
		if (entity)
			area = SCR_ScenarioFrameworkArea.Cast(entity.FindComponent(SCR_ScenarioFrameworkArea));
	}
	
	//------------------------------------------------------------------------------------------------
	// Spawns random Task based on available tasks and Areas that haven't spawned any
	void SpawnRandomTask()
	{
		// In case of more max tasks to be spawned but with less task types, we need to refill it again
		if (m_aESFTaskTypeForRandomization.IsEmpty())
			m_aESFTaskTypeForRandomization.Copy(m_aESFTaskTypesAvailable);
		
		if (m_aLayerTasksForRandomization.IsEmpty())
			return;
		
		Math.Randomize(-1);
		SCR_ScenarioFrameworkLayerTask layerTask = m_aLayerTasksForRandomization.GetRandomElement();
		if (!layerTask)
			return;
		
		// This filters out task types that were already spawned
		SCR_ESFTaskType taskType = layerTask.GetTaskType();
		if (!m_aESFTaskTypeForRandomization.Contains(taskType))
		{
			for (int i = m_aLayerTasksForRandomization.Count() - 1; i >= 0; i--)
			{
			    if (m_aLayerTasksForRandomization[i].GetTaskType() == taskType)
			        m_aLayerTasksForRandomization.Remove(i);
			}
			
			if (m_aESFTaskTypesAvailable.Contains(taskType))
				SpawnRandomTask();
		}
		m_aESFTaskTypeForRandomization.RemoveItem(taskType);
		
		//Spawning and setting necessary states
		SCR_ScenarioFrameworkArea area = layerTask.GetParentArea();
		m_aLayerTasksToBeInitialized.Insert(layerTask);
		layerTask.SetParentLayer(layerTask.GetParentLayer());
		layerTask.Init(area, SCR_ScenarioFrameworkEActivationType.ON_TASKS_INIT);
		area.SetLayerTask(layerTask);
		area.SetAreaSelected(true);
		m_aSelectedAreas.Insert(area);
		
		// Removing all Layer Tasks that have the same Task Type
		for (int i = m_aLayerTasksForRandomization.Count() - 1; i >= 0; i--)
		{
		    if (m_aLayerTasksForRandomization[i].GetTaskType() == taskType)
		        m_aLayerTasksForRandomization.Remove(i);
		}
		
		// Removing all Layer Tasks that are from the same Area of randomly selected Layer Task
		for (int i = m_aLayerTasksForRandomization.Count() - 1; i >= 0; i--)
		{
		    if (m_aLayerTasksForRandomization[i].GetParentArea() == area)
		        m_aLayerTasksForRandomization.Remove(i);
		}
		
		Print(string.Format("ScenarioFramework: Creating area %1 with Layer Task %2", area.GetOwner().GetName(), layerTask.GetOwner().GetName()), LogLevel.NORMAL);
	}
	
	//------------------------------------------------------------------------------------------------
	// Main function responsible for selecting available tasks and spawning the areas related to the tasks
	void GenerateTasks()
	{
		if (m_aTaskTypesAvailable.IsEmpty())
		{
			Print("ScenarioFramework: Available tasks are empty, no new tasks will be generated.", LogLevel.NORMAL);
			return;
		}
		
		if (m_aAreas.IsEmpty())
		{
			Print("ScenarioFramework: There are no Areas to generate tasks from", LogLevel.NORMAL);
			return;
		}
		
		Print("ScenarioFramework: ---------------------- Generating tasks -------------------", LogLevel.NORMAL);
		
		if (!m_aLayersTaskToSpawn.IsEmpty())
		{
			for (int i = 0; i < m_aLayersTaskToSpawn.Count(); i++)
			{
				GenerateSingleTask(i);
			}
			
			PrepareLayerTasksAfterInit();
			Print("ScenarioFramework: ---------------------- Generation of tasks completed -------------------", LogLevel.NORMAL);
			
			return;
		}
		
		//Fetching all Layer Tasks from Areas
		array<SCR_ScenarioFrameworkLayerTask> layerTasksToRandomize = {};
		foreach (SCR_ScenarioFrameworkArea area : m_aAreas)
		{
			if (!area)
				continue;
			
			array<SCR_ScenarioFrameworkLayerTask> layerTasks = {};
			area.GetAllLayerTasks(layerTasks);
			layerTasksToRandomize.InsertAll(layerTasks);
		}
		
		//Fetching available Task Types for generation based on type
		foreach (SCR_ScenarioFrameworkTaskType taskTypeClass : m_aTaskTypesAvailable)
		{
			m_aESFTaskTypesAvailable.Insert(taskTypeClass.GetTaskType());
		}
		
		//Removing Layer Tasks that don't have Task Type set in Available Task Types
		for (int i = layerTasksToRandomize.Count() - 1; i >= 0; i--)
		{
			if (!m_aESFTaskTypesAvailable.Contains(layerTasksToRandomize[i].GetTaskType()))
		        layerTasksToRandomize.Remove(i);
		}
		
		//Removing Layer Tasks that don't have activation type set to ON_TASKS_INIT
		for (int i = layerTasksToRandomize.Count() - 1; i >= 0; i--)
		{
		    if (layerTasksToRandomize[i].GetActivationType() != SCR_ScenarioFrameworkEActivationType.ON_TASKS_INIT)
		        layerTasksToRandomize.Remove(i);
		}
		
		//Creating a copy so we can work with these in loops
		m_aESFTaskTypeForRandomization.Copy(m_aESFTaskTypesAvailable);
		m_aLayerTasksForRandomization.Copy(layerTasksToRandomize);
		
		//Spawning desired number of tasks
		for (int i = 0; i < m_iMaxNumberOfTasks; i++)
		{
			//Layers Tasks are shrinked down and after certain number of generations, we need to refill it
			if (m_aLayerTasksForRandomization.IsEmpty())
			{
				m_aLayerTasksForRandomization.Copy(layerTasksToRandomize);
			
				foreach (SCR_ScenarioFrameworkArea selectedArea : m_aSelectedAreas)
				{
					for (int j = m_aLayerTasksForRandomization.Count() - 1; j >= 0; j--)
					{
					    if (m_aLayerTasksForRandomization[j].GetParentArea() == selectedArea)
						{
							m_aLayerTasksForRandomization.Remove(j);
							continue;
						}
					}
				}
			}
			
			SpawnRandomTask();
		}

		if (m_aLayerTasksToBeInitialized.Count() < m_iMaxNumberOfTasks)
			Print(string.Format("ScenarioFramework: Available areas do not have any other tasks to generate. Only %1 out of %2 was generated", m_aLayerTasksToBeInitialized, m_iMaxNumberOfTasks), LogLevel.NORMAL);

		PrepareLayerTasksAfterInit();
		Print("ScenarioFramework: ---------------------- Generation of tasks completed -------------------", LogLevel.NORMAL);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Prepares Layer Tasks that were selected by ON_TASK_INIT activation for invoking AfterTasksInitActions
	protected void PrepareLayerTasksAfterInit()
	{
		foreach (SCR_ScenarioFrameworkLayerTask layerTask : m_aLayerTasksToBeInitialized)
		{
			if (!layerTask)
				continue;
			
			layerTask.GetOnAllChildrenSpawned().Insert(CheckLayerTasksAfterInit);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Checks if all Layer Tasks that were selected by ON_TASK_INIT activation for invoking AfterTasksInitActions are finished with spawning
	protected void CheckLayerTasksAfterInit()
	{
		m_iCurrentlySpawnedLayerTasks++;
		if (m_iCurrentlySpawnedLayerTasks == m_aLayerTasksToBeInitialized.Count())
			//Due to how Task System sometimes works, not everything is initialized right after the Layer Task so we need to wait a bit
			GetGame().GetCallqueue().CallLater(AfterLayerTasksInit, 1000);
	
	}
	
	//------------------------------------------------------------------------------------------------
	//! Executes AfterTasksInitActions after all Layer Tasks are finished spawning
	protected void AfterLayerTasksInit()
	{
		foreach (SCR_ScenarioFrameworkLayerTask layerTask : m_aLayerTasksToBeInitialized)
		{
			if (!layerTask)
				continue;
			
			layerTask.GetOnAllChildrenSpawned().Remove(CheckLayerTasksAfterInit);
			if (m_bDynamicDespawn && !layerTask.GetDynamicDespawnExcluded())
				layerTask.DynamicDespawn();
		}
		
		foreach(SCR_ScenarioFrameworkActionBase afterTasksInitActions : m_aAfterTasksInitActions)
		{
			afterTasksInitActions.Init(m_aAreas.GetRandomElement().GetOwner());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void GenerateSingleTask(int index)
	{
		string targetLayer = m_aLayersTaskToSpawn[index];
		string targetArea = m_aAreasTasksToSpawn[index];
		
		IEntity layerEntity = GetGame().GetWorld().FindEntityByName(targetLayer);
		if (!layerEntity)
			return;
		
		SCR_ScenarioFrameworkLayerTask taskComponent = SCR_ScenarioFrameworkLayerTask.Cast(layerEntity.FindComponent(SCR_ScenarioFrameworkLayerTask));
		if (!taskComponent)
			return;
		
		m_aLayerTasksToBeInitialized.Insert(taskComponent);
		
		IEntity areaEntity = GetGame().GetWorld().FindEntityByName(targetArea);
		if (!areaEntity)
			areaEntity = layerEntity.GetParent();

		if (!areaEntity)
			return;
				
		SCR_ScenarioFrameworkArea area = SCR_ScenarioFrameworkArea.Cast(areaEntity.FindComponent(SCR_ScenarioFrameworkArea));
		if (!area)
			return;

		area.SetAreaSelected(true);
		taskComponent.SetActivationType(SCR_ScenarioFrameworkEActivationType.ON_TASKS_INIT);
		
		area.Create(taskComponent);
		Print(string.Format("ScenarioFramework: Creating area %1", area.GetOwner().GetName()), LogLevel.NORMAL);
		Print("ScenarioFramework: ---------------------------------------------------------------", LogLevel.NORMAL);
		m_iMaxNumberOfTasks--;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ScenarioFrameworkArea SelectRandomAreaByTaskType(SCR_ESFTaskType eTaskType)
	{
		if (m_aAreas.IsEmpty())
			return null;
		
		SCR_ScenarioFrameworkArea selectedArea;
		array<SCR_ScenarioFrameworkArea> aAreasCopy = {};
		aAreasCopy.Copy(m_aAreas);
		for (int i = 0, count = m_aAreas.Count(); i < count; i++)
		{
			Math.Randomize(-1);
			selectedArea = aAreasCopy.GetRandomElement();
			if (!selectedArea.GetIsAreaSelected() && selectedArea.GetIsTaskSuitableForArea(eTaskType))
			{
				selectedArea.SetAreaSelected(true);
				return selectedArea;
			}
			else
			{
				aAreasCopy.RemoveItem(selectedArea);
			}
		}
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get parent area the object is nested into
	SCR_ScenarioFrameworkArea GetParentArea(IEntity child) 
	{ 
		if (!child)
			return null;
			
		SCR_ScenarioFrameworkArea layer;
		IEntity entity = child.GetParent();
		while (entity)
		{
			layer = SCR_ScenarioFrameworkArea.Cast(entity.FindComponent(SCR_ScenarioFrameworkArea));
			if (layer)
				return layer;
			
			entity = entity.GetParent();
		}
		
		return layer;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Prepares dynamic spawn/despawn for specific area (Intended for runtime usage)
	void PrepareAreaSpecificDynamicDespawn(SCR_ScenarioFrameworkArea area, bool staySpawned = false)
	{
		int despawnRange = area.GetDynamicDespawnRange();
			
		//If this method is called with staySpawned = false, area will be added to m_aDespawnedAreas and gets despawned
		if (!staySpawned)
		{
			m_aDespawnedAreas.Insert(new Tuple3<SCR_ScenarioFrameworkArea, vector, int>(area, area.GetOwner().GetOrigin(), (despawnRange * despawnRange)));
			area.DynamicDespawn();
		}
		else
		{
			m_aSpawnedAreas.Insert(new Tuple3<SCR_ScenarioFrameworkArea, vector, int>(area, area.GetOwner().GetOrigin(), (despawnRange * despawnRange)));
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Removes dynamic spawn/despawn for specific area (Intended for runtime usage)
	void RemoveAreaSpecificDynamicDespawn(SCR_ScenarioFrameworkArea area, bool staySpawned = false)
	{
		int despawnRange = area.GetDynamicDespawnRange();
			
		//If this method is called with staySpawned = false, area will be despawned
		if (!staySpawned)
			area.DynamicDespawn();
		
		for (int i = m_aDespawnedAreas.Count() - 1; i >= 0; i--)
		{
			Tuple3<SCR_ScenarioFrameworkArea, vector, int> areaInfo = m_aDespawnedAreas[i];
			if (area == areaInfo.param1)
				m_aDespawnedAreas.Remove(i);
		}
		
		for (int i = m_aSpawnedAreas.Count() - 1; i >= 0; i--)
		{
			Tuple3<SCR_ScenarioFrameworkArea, vector, int> areaInfo = m_aSpawnedAreas[i];
			if (area == areaInfo.param1)
				m_aSpawnedAreas.Remove(i);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Prepares dynamic spawn/despawn
	protected void PrepareDynamicDespawn()
	{
		if (!m_bDynamicDespawn)
			return;
		
		GetOnAllAreasInitiated().Remove(PrepareDynamicDespawn);
		
		foreach (SCR_ScenarioFrameworkArea area : m_aAreas)
		{
			if (!area.GetDynamicDespawnEnabled())
				continue;
			
			int despawnRange = area.GetDynamicDespawnRange();
			m_aDespawnedAreas.Insert(new Tuple3<SCR_ScenarioFrameworkArea, vector, int>(area, area.GetOwner().GetOrigin(), (despawnRange * despawnRange)));
			
			area.DynamicDespawn();
		}
		
		GetGame().GetCallqueue().CallLater(CheckDistance, 1000 * m_iUpdateRate, true);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Refreshes array of camera observers and checks which areas should spawn/despawn
	protected void CheckDistance()
	{
		m_aObservers.Clear();
		array<int> playerIds = {};
		PlayerManager playerManager = GetGame().GetPlayerManager();
		IEntity player;
		SCR_DamageManagerComponent damageManager;
		playerManager.GetPlayers(playerIds);

		foreach (int playerId : playerIds)
		{
			player = playerManager.GetPlayerControlledEntity(playerId);
			if (!player)
				continue;
			
			damageManager = SCR_DamageManagerComponent.GetDamageManager(player);
			if (damageManager && damageManager.GetState() != EDamageState.DESTROYED)
				m_aObservers.Insert(player.GetOrigin());
		}
		
		DynamicSpawn();
		DynamicDespawn();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Goes over despawned areas and checks whether or not said area should spawn
	protected void DynamicSpawn()
	{
		for (int i = m_aDespawnedAreas.Count() - 1; i >= 0; i--)
		{
			Tuple3<SCR_ScenarioFrameworkArea, vector, int> areaInfo = m_aDespawnedAreas[i];
			foreach (vector observerPos : m_aObservers)
			{
				if (vector.DistanceSqXZ(observerPos, areaInfo.param2) < areaInfo.param3)
				{
					areaInfo.param1.DynamicReinit();
					m_aSpawnedAreas.Insert(areaInfo);
					m_aDespawnedAreas.Remove(i);
					break;
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Goes over spawned areas and checks whether or not said area should despawn
	protected void DynamicDespawn()
	{
		for (int i = m_aSpawnedAreas.Count() - 1; i >= 0; i--)
		{
			Tuple3<SCR_ScenarioFrameworkArea, vector, int> areaInfo = m_aSpawnedAreas[i];
			bool observerInRange;
			foreach (vector observerPos : m_aObservers)
			{
				if (vector.DistanceSqXZ(observerPos, areaInfo.param2) < areaInfo.param3)
				{
					observerInRange = true;
					break;
				}
			}

			if (!observerInRange)
			{
				areaInfo.param1.DynamicDespawn();
				m_aDespawnedAreas.Insert(areaInfo);
				m_aSpawnedAreas.Remove(i);
			}
		}
	}
		
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		GetOnAllAreasInitiated().Insert(PrepareDynamicDespawn);
		GetGame().GetCallqueue().CallLater(Init,1000,false); //TODO: make the init order properly (the init should start after all Areas are registered)
	}
	
	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void PopUpMessage(string sTitle, string sSubtitle, FactionKey factionKey = "")
	{
		Rpc(RpcDo_PopUpMessage, sTitle, sSubtitle, factionKey);
		
		if (factionKey != "")
			if (SCR_FactionManager.SGetLocalPlayerFaction() != GetGame().GetFactionManager().GetFactionByKey(factionKey))
				return;
		
		SCR_PopUpNotification.GetInstance().PopupMsg(sTitle, text2: sSubtitle);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_PopUpMessage(string sTitle, string sSubtitle, FactionKey factionKey)
	{
		if (factionKey != "")
			if (SCR_FactionManager.SGetLocalPlayerFaction() != GetGame().GetFactionManager().GetFactionByKey(factionKey))
				return;
		
		SCR_PopUpNotification.GetInstance().PopupMsg(sTitle, text2: sSubtitle);
	}
}
