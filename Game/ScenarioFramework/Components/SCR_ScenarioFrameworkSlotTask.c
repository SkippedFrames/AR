[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class SCR_ScenarioFrameworkSlotTaskClass : SCR_ScenarioFrameworkSlotBaseClass
{
	// prefab properties here
}

//------------------------------------------------------------------------------------------------
class SCR_ScenarioFrameworkSlotTask : SCR_ScenarioFrameworkSlotBase
{
	[Attribute(desc: "Name of the task in list of tasks", category: "Task")]		
	protected LocalizedString m_sTaskTitle;
	
	[Attribute(desc: "Description of the task", category: "Task")]
	protected LocalizedString m_sTaskDescription;
	
	[Attribute(desc: "Text for the Execution category in Briefing", category: "Task")]
	protected LocalizedString m_sTaskExecutionBriefing;
		
	[Attribute(defvalue: "1", desc: "What to do once task is finished", UIWidgets.Auto, category: "OnTaskFinish")];
	protected ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActionsOnFinished;
	
	[Attribute(defvalue: "1", desc: "What to do once task is created", UIWidgets.Auto, category: "OnTaskCreate")];
	protected ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActionsOnCreated;
	
	[Attribute(defvalue: "1", desc: "What to do once task is created", UIWidgets.Auto, category: "OnTaskFailed")];
	protected ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActionsOnFailed;
	
	[Attribute(defvalue: "1", desc: "What to do once task progressed", UIWidgets.Auto, category: "OnTaskProgress")];
	protected ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActionsOnProgress;
	
	[Attribute(defvalue: "1", desc: "What to do once task is updated", UIWidgets.Auto, category: "OnTaskUpdated")];
	protected ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActionsOnUpdated;
	
	
	
	protected SCR_ScenarioFrameworkLayerTask	m_TaskLayer;		//parent layer where the task is defined
	protected bool m_bTaskResolvedBeforeLoad;
	
	//------------------------------------------------------------------------------------------------
	void OnTaskStateChanged(SCR_TaskState newState)
	{
		if (newState == SCR_TaskState.OPENED)
		{
			foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnCreated)
			{
				action.OnActivate(null);
			}
		}			
		else if (newState == SCR_TaskState.FINISHED && !m_bTaskResolvedBeforeLoad)
		{
			foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnFinished)
			{
				action.OnActivate(null);
			}
		}
		else if (newState == SCR_TaskState.CANCELLED && !m_bTaskResolvedBeforeLoad)
		{
			foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnFailed)
			{
				action.OnActivate(null);
			}
		}
		else if (newState == SCR_TaskState.PROGRESSED && !m_bTaskResolvedBeforeLoad)
		{
			foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnProgress)
			{
				action.OnActivate(null);
			}
		}
		else
		{
			foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnUpdated)
			{
				action.OnActivate(null);
			};
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetTaskResolvedBeforeLoad(bool state)
	{
		m_bTaskResolvedBeforeLoad = state;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void StoreTaskSubjectToParentTaskLayer()
	{
		m_TaskLayer = GetParentTaskLayer();
		if (m_TaskLayer)
		{
			m_TaskLayer.SetTaskSubject(this);
			if (m_Entity)
				m_TaskLayer.SetEntity(m_Entity);
		}
		else
		{
			PrintFormat("ScenarioFramework: %1 could not be spawned and cannot bind it to task %2", m_sObjectToSpawn, m_TaskLayer.GetOwner().GetName(), LogLevel.ERROR);
		}
	}
		
	//------------------------------------------------------------------------------------------------
	LocalizedString GetTaskTitle(int iState = 0) 
	{ 
		return m_sTaskTitle;
	}
	
	//------------------------------------------------------------------------------------------------
	LocalizedString GetTaskExecutionBriefing() 
	{ 
		return m_sTaskExecutionBriefing;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetTaskDescription(int iState = 0)
	{ 
		if (!m_Entity)
			return m_sTaskDescription;
		
		if (!m_sOverrideObjectDisplayName.IsEmpty())
			return string.Format(WidgetManager.Translate(m_sTaskDescription, m_sOverrideObjectDisplayName));
		
		string entityDisplayName = GetSpawnedEntityDisplayName();
		if (entityDisplayName == string.Empty)
			return m_sTaskDescription;
		
		return string.Format(WidgetManager.Translate(m_sTaskDescription, entityDisplayName));
	}	
	
	//------------------------------------------------------------------------------------------------
	//! Get the Layer Task which is parent of this Slot 
	SCR_ScenarioFrameworkLayerTask GetParentTaskLayer() 
	{ 
		SCR_ScenarioFrameworkLayerTask layer;
		IEntity entity = GetOwner().GetParent();
		while (entity)
		{
			layer = SCR_ScenarioFrameworkLayerTask.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerTask));
			if (layer)
				return layer;
			
			entity = entity.GetParent();
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	override void DynamicReinit()
	{
		Init(null, SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT);
	}
	
	//------------------------------------------------------------------------------------------------
	override void DynamicDespawn()
	{
		GetOnAllChildrenSpawned().Remove(DynamicDespawn);
		if (!m_Entity && !SCR_StringHelper.IsEmptyOrWhiteSpace(m_sObjectToSpawn))
		{
			GetOnAllChildrenSpawned().Insert(DynamicDespawn);
			return;
		}
		
		if (!m_bInitiated || m_bExcludeFromDynamicDespawn)
			return;
		
		if (m_Entity)
			m_vPosition = m_Entity.GetOrigin();
		
		m_bInitiated = false;
		m_bDynamicallyDespawned = true;
		SCR_EntityHelper.DeleteEntityAndChildren(m_Entity);
	}	
	
	//------------------------------------------------------------------------------------------------
	override void Init(SCR_ScenarioFrameworkArea area = null, SCR_ScenarioFrameworkEActivationType activation = SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT)
	{
		if (m_bInitiated)
			return;
		
		if (!m_bDynamicallyDespawned && activation != m_eActivationType)
			return;
		
		bool tempTerminated = m_bIsTerminated;
		m_bIsTerminated = false;
		
		if (m_bIsTerminated)
			return;
		
		if (m_Entity && !m_bEnableRepeatedSpawn)
		{
			IEntity entity = GetOwner().GetParent();
			if (!entity)
				return;
				
			SCR_ScenarioFrameworkLayerBase layerBase = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (!layerBase)
				return;
				
			if (!layerBase.GetEnableRepeatedSpawn())
			{
				Print(string.Format("ScenarioFramework: Object %1 already exists and won't be spawned for %2, exiting...", m_Entity, GetOwner().GetName()), LogLevel.ERROR);
				return;
			}
		}
		
		// Handles inheritance of faction settings from parents
		if (m_sFactionKey.IsEmpty() && m_ParentLayer && !m_ParentLayer.GetFactionKey().IsEmpty())
			SetFactionKey(m_ParentLayer.GetFactionKey());
		
		if (!m_bUseExistingWorldAsset)
		{
			m_Entity = SpawnAsset();
		}
		else
		{
			QueryObjectsInRange();	//sets the m_Entity in subsequent callback
		}
		
		GetOnAllChildrenSpawned().Insert(AfterAllChildrenSpawned);
		
		if (!m_Entity)
		{
			InvokeAllChildrenSpawned();
			return;
		}
		
		if (!m_sID.IsEmpty())
			m_Entity.SetName(m_sID);	
		
		ScriptedDamageManagerComponent objectDmgManager = ScriptedDamageManagerComponent.Cast(m_Entity.FindComponent(ScriptedDamageManagerComponent));
		if (objectDmgManager)
			objectDmgManager.GetOnDamageStateChanged().Insert(OnObjectDamage);
		
		if (Vehicle.Cast(m_Entity))
		{
			EventHandlerManagerComponent ehManager = EventHandlerManagerComponent.Cast(m_Entity.FindComponent(EventHandlerManagerComponent));
			if (ehManager)
				ehManager.RegisterScriptHandler("OnCompartmentEntered", this, OnCompartmentEntered, false, true);
		}
			
		StoreTaskSubjectToParentTaskLayer();
		m_bIsTerminated = tempTerminated;
		
		InvokeAllChildrenSpawned();
	}
}
