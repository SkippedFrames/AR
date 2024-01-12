class SCR_MissionHeaderCombatOps : SCR_MissionHeader
{
	[Attribute("-1", UIWidgets.EditBox, "Maximal number of tasks that can be generated (global override, -1 for default")]
	int m_iMaxNumberOfTasks;
	
	[Attribute()]
	ref array<ref SCR_ScenarioFrameworkTaskType> m_aTaskTypesAvailable;
};
