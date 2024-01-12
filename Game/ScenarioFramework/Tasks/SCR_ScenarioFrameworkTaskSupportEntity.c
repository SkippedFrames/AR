//------------------------------------------------------------------------------------------------
[EntityEditorProps(category: "GameScripted/Tasks", description: "Transport task support entity.", color: "0 0 255 255")]
class SCR_ScenarioFrameworkTaskSupportEntityClass: SCR_BaseTaskSupportEntityClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_ScenarioFrameworkTaskSupportEntity : SCR_BaseTaskSupportEntity
{
	protected IEntity 	m_Entity;
	
	//------------------------------------------------------------------------------------------------
	override void FinishTask(notnull SCR_BaseTask task)
	{
		super.FinishTask(task);
	}
	
	
	//------------------------------------------------------------------------------------------------
	IEntity GetTaskEntity()
	{
		return m_Entity;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetTaskEntity(IEntity entity)
	{
		m_Entity = entity;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_BaseTask CreateTask(SCR_ScenarioFrameworkLayerTask layer)
	{
		m_Entity = layer.GetSpawnedEntity();
		return super.CreateTask();
	}

	//------------------------------------------------------------------------------------------------
	void SetTaskPrefab(ResourceName sTaskPrefab)
	{
		m_sTaskPrefab = sTaskPrefab;
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_Faction GetCharacterFaction(IEntity unit)
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(unit);
		if (!character)
			return null;

		return SCR_Faction.Cast(character.GetFaction());
	}
	
	//------------------------------------------------------------------------------------------------
	//This should only be called on the server!
	void SetTaskExecutionBriefing(notnull SCR_ScenarioFrameworkTask task, string description)
	{
		int taskID = task.GetTaskID();
		RPC_TaskExecutionBriefing(taskID, description);
		Rpc(RPC_TaskExecutionBriefing, taskID, description);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_TaskExecutionBriefing(int taskID, string description)
	{
		if (!GetTaskManager())
			return;
		
		SCR_ScenarioFrameworkTask task = SCR_ScenarioFrameworkTask.Cast(GetTaskManager().GetTask(taskID));
		if (!task)
			return;
		
		task.SetTaskExecutionBriefing(description);
	}
}
