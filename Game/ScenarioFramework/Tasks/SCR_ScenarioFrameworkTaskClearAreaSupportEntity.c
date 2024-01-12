//------------------------------------------------------------------------------------------------
[EntityEditorProps(category: "GameScripted/Tasks", description: "Transport task support entity.", color: "0 0 255 255")]
class SCR_ScenarioFrameworkTaskClearAreaSupportEntityClass: SCR_ScenarioFrameworkTaskSupportEntityClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_ScenarioFrameworkTaskClearAreaSupportEntity : SCR_ScenarioFrameworkTaskSupportEntity
{
	//------------------------------------------------------------------------------------------------
	override SCR_BaseTask CreateTask(SCR_ScenarioFrameworkLayerTask layer)
	{
		m_Entity = layer.GetSpawnedEntity();
		return super.CreateTask();
	}
}
