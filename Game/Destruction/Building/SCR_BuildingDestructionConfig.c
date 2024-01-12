[BaseContainerProps(configRoot: true)]
class SCR_BuildingDestructionConfig
{
	[Attribute(desc: "Add typenames inheriting from IEntity to exclude them from entity query during final destruction phase")]
	ref array<string> m_aExcludedEntityQueryTypes;
}
