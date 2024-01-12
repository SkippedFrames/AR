[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class SCR_ScenarioFrameworkLayerTaskDeliverClass : SCR_ScenarioFrameworkLayerTaskClass
{
	// prefab properties here
};

class SCR_ScenarioFrameworkLayerTaskDeliver : SCR_ScenarioFrameworkLayerTask
{	
	[Attribute(desc: "Name of the task in list of tasks after item is possessed.", category: "Task")]		
	protected string 		m_sTaskTitleUpdated;
	
	[Attribute(desc: "Description of the task after item is possessed", category: "Task")]			//TODO: make config, memory
	protected string 		m_sTaskDescriptionUpdated;
	
	[Attribute(defvalue: "30", desc: "Delay in seconds for Intel map marker when it is dropped on the ground", params: "0 6000 1", category: "Task")]
	protected int 			m_iIntelMapMarkerUpdateDelay;
	
	protected IEntity		m_DeliverPoint;
	
	//------------------------------------------------------------------------------------------------
	override void SetEntity(IEntity entity)
	{
		m_Entity = entity;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetDeliveryPointEntity(IEntity entity)
	{
		m_DeliverPoint = entity;
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity GetDeliveryPointEntity()
	{
		return m_DeliverPoint;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetIntelMapMarkerUpdateDelay()
	{
		return m_iIntelMapMarkerUpdateDelay;
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetupTask()
	{
		super.SetupTask();
		
		if (!m_Task || !m_DeliverPoint)
			return;
		
		SCR_TaskDeliver.Cast(m_Task).SetDeliveryTrigger(SCR_BaseTriggerEntity.Cast(m_DeliverPoint));
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateTaskTitleAndDescription()
	{
		if (!m_sTaskTitleUpdated.IsEmpty()) 
			m_Task.SetTitle(m_sTaskTitleUpdated);
		if (!m_sTaskDescriptionUpdated.IsEmpty()) 
			m_Task.SetDescription(m_sTaskDescriptionUpdated);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool SetSupportEntity()
	{
		if (!GetTaskManager().FindSupportEntity(SCR_ScenarioFrameworkTaskDeliverSupportEntity))
		{
			Print("ScenarioFramework: Task Deliver support entity not found in the world, task won't be created!", LogLevel.ERROR);
			return false;
		}
		m_SupportEntity = SCR_ScenarioFrameworkTaskDeliverSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_ScenarioFrameworkTaskDeliverSupportEntity));
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_ScenarioFrameworkLayerTaskDeliver(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_eTypeOfTask = SCR_ESFTaskType.DELIVER;
	}
};
