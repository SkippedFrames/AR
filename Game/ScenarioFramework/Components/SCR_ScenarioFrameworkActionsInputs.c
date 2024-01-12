//------------------------------------------------------------------------------------------------
//TODO: make this a generic action which can be used anywhere anytime (i.e. on task finished, etc)
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionInputBase
{
	protected SCR_ScenarioFrameworkLogicInput		m_Input;
	
	//------------------------------------------------------------------------------------------------
	void Init(SCR_ScenarioFrameworkLogicInput input)
	{
		m_Input = input;
	}
};	

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionInputOnTaskEventIncreaseCounter : SCR_ScenarioFrameworkActionInputBase
{
	[Attribute(desc: "Insert name of the task layer or leave empty for any task")];
	protected string			m_sTaskLayerName;
	
	[Attribute("1", UIWidgets.ComboBox, "Task state", "", ParamEnumArray.FromEnum(SCR_TaskState))];
	protected SCR_TaskState			m_eEventName;
	
	protected int 					m_iActionsInput;
	
	override void Init(SCR_ScenarioFrameworkLogicInput input)
	{	
		super.Init(input);
		SCR_GameModeSFManager gameModeComp = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
		if (!gameModeComp) 
			return;
			
		gameModeComp.GetOnTaskStateChanged().Insert(OnActivate);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnActivate(SCR_BaseTask task, SCR_ETaskEventMask mask)
	{
		if (task.GetTaskState() != m_eEventName || !m_Input)
			return;
			
		SCR_ScenarioFrameworkLayerTask taskLayer;
		string sTaskLayerName = "";
		if (SCR_ScenarioFrameworkTask.Cast(task))
		{
			taskLayer = SCR_ScenarioFrameworkTask.Cast(task).GetTaskLayer();
			if (taskLayer)
			{
				sTaskLayerName = taskLayer.GetOwner().GetName();
				if (taskLayer.GetLayerTaskResolvedBeforeLoad())
					return;
			}
		} 
		
		if (m_sTaskLayerName.IsEmpty() || m_sTaskLayerName == sTaskLayerName)
		{
			if (!(mask & SCR_ETaskEventMask.TASK_ASSIGNEE_CHANGED))
				m_Input.OnActivate(1, null);
				
			if (mask & SCR_ETaskEventMask.TASK_PROPERTY_CHANGED && !(mask & SCR_ETaskEventMask.TASK_CREATED) && !(mask & SCR_ETaskEventMask.TASK_FINISHED) && !(mask & SCR_ETaskEventMask.TASK_ASSIGNEE_CHANGED))
			{	
				if (taskLayer)
				{
					SCR_ScenarioFrameworkSlotTask subject = taskLayer.GetTaskSubject();
					if (subject)
						subject.OnTaskStateChanged(m_eEventName)
				}
			}	
		}
	}	
}

enum SCR_EScenarioFrameworkComparisonOperator
{
	LESS_THAN,
	LESS_OR_EQUAL,
	GREATER_THEN,
	GREATER_OR_EQUAL,
	EQUAL
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionInputCheckEntitiesInTrigger : SCR_ScenarioFrameworkActionInputBase
{
	[Attribute(desc: "Trigger")];
	protected ref SCR_ScenarioFrameworkGet					m_Getter;
	
	[Attribute("0", UIWidgets.ComboBox, "Operator", "", ParamEnumArray.FromEnum(SCR_EScenarioFrameworkComparisonOperator))]
	protected SCR_EScenarioFrameworkComparisonOperator			m_eComparisonOperator;
	
	[Attribute(desc: "Value")];
	protected int							m_iValue;
	
	protected SCR_CharacterTriggerEntity	m_Trigger;
		
	//------------------------------------------------------------------------------------------------
	override void Init(SCR_ScenarioFrameworkLogicInput input)
	{	
		super.Init(input);
		if (!m_Getter)
			return;

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper =  SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
		{
			PrintFormat("ScenarioFramework: Selected getter %1 is not suitable for this operation", m_Getter.ClassName(), LogLevel.ERROR);
			return;
		}
		
		IEntity entity = entityWrapper.GetValue();
		if (!entity)
		{
			PrintFormat("ScenarioFramework: Selected getter entity is null", m_Getter.ClassName(), LogLevel.ERROR);
			return;
		}
		
		SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
		if (!layer)
			return;

		SCR_CharacterTriggerEntity trigger = SCR_CharacterTriggerEntity.Cast(layer.GetSpawnedEntity());
		if (!trigger)
			return;		//TODO: add a universal method for informing user about errors

		m_Trigger = trigger;
		//We want to give trigger enough time to be properly set up and not to get OnChange called prematurely
		GetGame().GetCallqueue().CallLater(RegisterOnChange, 5000);
	}
	
	//------------------------------------------------------------------------------------------------
	void RegisterOnChange()
	{
		if (m_Trigger)
			m_Trigger.GetOnChange().Insert(OnActivate);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnActivate(SCR_ScenarioFrameworkParam<IEntity> param)
	{
		if (!m_Trigger)
			return;
		
		array<IEntity> aEntities = {};
		int iNrOfEnts = m_Trigger.GetCountEntitiesInside();
		
		if (
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.LESS_THAN) 			&& (iNrOfEnts < m_iValue)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.LESS_OR_EQUAL) 		&& (iNrOfEnts <= m_iValue)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.EQUAL) 				&& (iNrOfEnts == m_iValue)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.GREATER_OR_EQUAL) 	&& (iNrOfEnts >= m_iValue)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.GREATER_THEN) 		&& (iNrOfEnts > m_iValue)) 
		)
		{
			m_Input.OnActivate(true, m_Trigger);
		}
	}
	
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionInputCheckEntitiesInAreaTrigger : SCR_ScenarioFrameworkActionInputCheckEntitiesInTrigger
{
	//------------------------------------------------------------------------------------------------
	override void Init(SCR_ScenarioFrameworkLogicInput input)
	{	
		super.Init(input);
		if (!m_Getter)
			return;

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper =  SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
		{
			PrintFormat("ScenarioFramework: Selected getter %1 is not suitable for this operation", m_Getter.ClassName(), LogLevel.ERROR);
			return;
		}
		
		IEntity entity = entityWrapper.GetValue();
		if (!entity)
		{
			PrintFormat("ScenarioFramework: Selected getter entity is null", m_Getter.ClassName(), LogLevel.ERROR);
			return;
		}
		
		SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
		if (!layer)
			return;
		
		SCR_ScenarioFrameworkArea area = SCR_ScenarioFrameworkArea.Cast(layer);
		if (!area)
			return;

		SCR_CharacterTriggerEntity trigger = SCR_CharacterTriggerEntity.Cast(area.GetTrigger());
		if (!trigger)
			return;

		m_Trigger = trigger;
		//We want to give trigger enough time to be properly set up and not to get OnChange called prematurely
		GetGame().GetCallqueue().CallLater(RegisterOnChange, 5000);
	}
	
}	
