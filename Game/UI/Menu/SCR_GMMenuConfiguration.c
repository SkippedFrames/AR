[BaseContainerProps(configRoot: true)]
class SCR_GMMenuConfiguration : Managed
{
	[Attribute("", UIWidgets.ResourceNamePicker, "Tutorial mission", "conf")]
	ResourceName m_TutorialScenario;

	[Attribute("", UIWidgets.ResourceAssignArray, "GM scenarios headers", "conf")]
	ref array<ResourceName> m_aGameMasterScenarios;
};
