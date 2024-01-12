//------------------------------------------------------------------------------------------------
class SCR_ScenarioFrameworkTaskClass : SCR_BaseTaskClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_ScenarioFrameworkTask : SCR_BaseTask
{
	protected SCR_ScenarioFrameworkArea 							m_Area;
	protected IEntity 												m_Asset;
	protected SCR_ScenarioFrameworkTaskSupportEntity		 		m_SupportEntity;
	protected SCR_ScenarioFrameworkLayerTask						m_Layer;
	protected SCR_ScenarioFrameworkSlotTask							m_SlotTask;
	protected string 												m_sTaskExecutionBriefing;
	
	//------------------------------------------------------------------------------------------------
	void SetTaskState(SCR_TaskState state)
	{
		SetState(state);
	}

	//------------------------------------------------------------------------------------------------
	void SetTaskLayer(SCR_ScenarioFrameworkLayerTask layer)
	{
		m_Layer = layer;
	}

	//------------------------------------------------------------------------------------------------
	SCR_ScenarioFrameworkLayerTask GetTaskLayer()
	{
		return m_Layer;
	}

	//------------------------------------------------------------------------------------------------
	//! An event called when the state of this task has been changed.
	override void OnStateChanged(SCR_TaskState previousState, SCR_TaskState newState)
	{
		if (!m_Layer)
			return;
		
		SCR_GameModeSFManager gameModeManager = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
		if (!gameModeManager || !gameModeManager.IsMaster())
			return;

		if (m_SlotTask)
		{
			m_SlotTask.OnTaskStateChanged(newState);
		}
		else
		{
			SCR_ScenarioFrameworkSlotTask slotTask = m_Layer.GetTaskSubject();
			if (slotTask)
				slotTask.OnTaskStateChanged(newState);
			else
				Print("ScenarioFramework: Task Subject not found for task", LogLevel.ERROR);
		}
		
		m_Layer.OnTaskStateChanged(previousState, newState);
				
		if (newState == SCR_TaskState.FINISHED)
			gameModeManager.PopUpMessage(GetTitle(), "#AR-Tasks_StatusFinished-UC", m_TargetFaction.GetFactionKey());

		if (newState == SCR_TaskState.CANCELLED)
			gameModeManager.PopUpMessage(GetTitle(), "#AR-Tasks_StatusFailed-UC", m_TargetFaction.GetFactionKey());
	}
	
	//------------------------------------------------------------------------------------------------
	override void Finish(bool showMsg = true)
	{
		showMsg = SCR_FactionManager.SGetLocalPlayerFaction() == m_TargetFaction;
		super.Finish(showMsg);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetTaskSubject(IEntity object)
	{
		m_Asset = object;
	}
	
	//------------------------------------------------------------------------------------------------
	void RehookTaskSubject(IEntity object)
	{
		m_Asset = object;
		
		if (m_SupportEntity)
			m_SupportEntity.SetTaskEntity(m_Asset);
	}

	//------------------------------------------------------------------------------------------------
	IEntity GetTaskSubject()
	{
		return m_Asset;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSlotTask(SCR_ScenarioFrameworkSlotTask slotTask)
	{
		m_SlotTask = slotTask;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ScenarioFrameworkSlotTask GetSlotTask()
	{
		return m_SlotTask;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ScenarioFrameworkTaskSupportEntity GetSupportEntity()
	{
		return m_SupportEntity;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetTaskExecutionBriefing(string text)
	{
		m_sTaskExecutionBriefing = text;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetTaskExecutionBriefing() 
	{ 
		return m_sTaskExecutionBriefing;
	}

	//------------------------------------------------------------------------------------------------
	protected bool SetSupportEntity()
	{
		m_SupportEntity = SCR_ScenarioFrameworkTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_ScenarioFrameworkTaskSupportEntity));

		if (!m_SupportEntity)
		{
			Print("ScenarioFramework: Default Task support entity not found in the world, task won't be created!", LogLevel.ERROR);
			return false;
		}

		return m_SupportEntity != null;
	}

	//------------------------------------------------------------------------------------------------
	void Init()
	{
		if (SCR_Global.IsEditMode(this))
			return;

		SetSupportEntity();
		if (!m_SupportEntity)
			return;

		m_Asset = m_SupportEntity.GetTaskEntity();
		if (!m_Asset)
		{
			if (m_SlotTask)
				m_Asset = m_SlotTask.GetSpawnedEntity();
			
			if (!m_Asset)
			{
				m_SupportEntity.CancelTask(this.GetTaskID());
				Print("ScenarioFramework: Task subject not found!", LogLevel.ERROR);
				return;
			}
			
			m_SupportEntity.SetTaskEntity(m_Asset);
		}

		SCR_GameModeSFManager gameModeManager = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
		if (!gameModeManager)
			return;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (!GetTaskManager() || GetTaskManager().IsProxy())
			return;

		Init();
	}
}
