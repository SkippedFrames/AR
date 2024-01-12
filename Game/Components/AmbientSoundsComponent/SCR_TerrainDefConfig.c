/*
Array has to have the same size as ESoundType count
*/
[BaseContainerProps(configRoot: true)]
class SCR_TerrainDefConfig
{
	[Attribute("", UIWidgets.Object, "")]
	ref array<ref SCR_TerrainDef> m_aTerrainDef;
};
