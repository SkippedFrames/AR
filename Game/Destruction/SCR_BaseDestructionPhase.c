class SCR_BaseDestructionPhase: BaseDestructionPhase
{
	[Attribute(ResourceName.Empty, UIWidgets.ResourcePickerThumbnail, "Model to use for the damage phase", "xob")]
	ResourceName m_sPhaseModel;
	
	[Attribute("", UIWidgets.Object, "List of objects (particles, debris, etc) to spawn on destruction of the phase")]
	ref array<ref SCR_BaseSpawnable> m_aPhaseDestroySpawnObjects;
}
