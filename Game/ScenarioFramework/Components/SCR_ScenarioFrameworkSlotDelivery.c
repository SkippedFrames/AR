[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class SCR_ScenarioFrameworkSlotDeliveryClass : SCR_ScenarioFrameworkSlotTaskClass
{
	// prefab properties here
};

//------------------------------------------------------------------------------------------------
/*!
	Class generated via ScriptWizard.
*/
class SCR_ScenarioFrameworkSlotDelivery : SCR_ScenarioFrameworkSlotTask
{
	
	[Attribute(desc: "Name of the task layer associated with this deliver point, Doesn't need to be set if nested under task layer Deliver.", category: "Task")];
	protected ref array<string>						m_aAssociatedTaskLayers;
	
	/*
	[Attribute(defvalue: "", desc: "Name of the entity to be delivered")];
	protected string					m_sIDToDeliver;
	*/	
		
	//------------------------------------------------------------------------------------------------
	override void StoreTaskSubjectToParentTaskLayer()
	{
		if (m_aAssociatedTaskLayers.IsEmpty())
		{
			//the Associated layer task attribute is empty, lets check if the parent layer of this delivery slot is Task Deliver
			//if yes, the user didn't need to fill the mentioned attribute since it's nested the layer
			//Not every time is the delivery slot nested under the Layer task Deliver since it can be created later.
			m_TaskLayer = GetParentTaskLayer();
			if (m_TaskLayer && SCR_ScenarioFrameworkLayerTaskDeliver.Cast(m_TaskLayer))
				SCR_ScenarioFrameworkLayerTaskDeliver.Cast(m_TaskLayer).SetDeliveryPointEntity(m_Entity);
			else
				PrintFormat("ScenarioFramework: ->Task->Delivery point %1 doesn't have associated layer attribute set (and is nested outside of its layer task delivery)", GetOwner().GetName(), LogLevel.ERROR);
		}
		else
		{
			IEntity entity;
			foreach (string sLayerName : m_aAssociatedTaskLayers)
			{
				entity = GetGame().GetWorld().FindEntityByName(sLayerName);
				if (entity)
					m_TaskLayer = SCR_ScenarioFrameworkLayerTaskDeliver.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerTaskDeliver));
						
				if (m_TaskLayer)
				{
					if (m_Entity)
						SCR_ScenarioFrameworkLayerTaskDeliver.Cast(m_TaskLayer).SetDeliveryPointEntity(m_Entity);
				}
				else
				{
					PrintFormat("ScenarioFramework: ->Task->Delivery point: Task Layer %1 doesn't exist.", sLayerName, LogLevel.ERROR);
				}
			}
		}
	}
	
	
	//------------------------------------------------------------------------------------------------
	override void Init(SCR_ScenarioFrameworkArea area = null, SCR_ScenarioFrameworkEActivationType activation = SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT)
	{
		if (!m_bDynamicallyDespawned && activation != m_eActivationType)
			return;
		
		super.Init(area, activation);
		
		if (!m_TaskLayer)
			return;
		
		SCR_BaseTriggerEntity trigger = SCR_BaseTriggerEntity.Cast(m_Entity);
		if (!trigger)
			return;
		
		SCR_TaskDeliver task = SCR_TaskDeliver.Cast(m_TaskLayer.GetTask());
		if (task)
		{
			task.SetTaskLayer(m_TaskLayer);
			task.SetDeliveryTrigger(trigger);
			task.UpdateTaskTitleAndDescription();
		}
		else
		{
			if (m_aAssociatedTaskLayers.IsEmpty())
			{
				PrintFormat("ScenarioFramework: ->Task->Delivery point: Associated Task Layers are empty", LogLevel.ERROR);
				return;
			}
					
			IEntity entity;
			foreach (string sLayerName : m_aAssociatedTaskLayers)
			{
				entity = GetGame().GetWorld().FindEntityByName(sLayerName);
				if (entity)
					m_TaskLayer = SCR_ScenarioFrameworkLayerTaskDeliver.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerTaskDeliver));
								
				if (m_TaskLayer)
				{
					task = SCR_TaskDeliver.Cast(m_TaskLayer.GetTask());
					if (task)
					{
						task.SetTaskLayer(m_TaskLayer);
						task.SetDeliveryTrigger(trigger);
						task.UpdateTaskTitleAndDescription();
					}
				}
				else
				{
					PrintFormat("ScenarioFramework: ->Task->Delivery point: Task Delivery does not exist for Layer %1", sLayerName, LogLevel.ERROR);
				}
			}
		}
		
		GetOnAllChildrenSpawned().Insert(AfterAllChildrenSpawnedDelivery);
		InvokeAllChildrenSpawned();
	}
	
	//------------------------------------------------------------------------------------------------
	override void AfterAllChildrenSpawned()
	{
	}
	
	//------------------------------------------------------------------------------------------------
	void AfterAllChildrenSpawnedDelivery()
	{
		m_bInitiated = true;
		
		foreach (SCR_ScenarioFrameworkPlugin plugin : m_aPlugins)
		{
			plugin.Init(this);
		}

		if (m_ParentLayer)
			m_ParentLayer.CheckAllChildrenSpawned(this);

		GetOnAllChildrenSpawned().Remove(AfterAllChildrenSpawnedDelivery);
	}
};
