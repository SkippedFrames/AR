class MapDescriptorConfiguration
{
	EMapDescriptorType Type;
	
	void MapDescriptorConfiguration(EMapDescriptorType type)
	{
		Type = type;
	}
};

class LayerConfiguration
{
	ref array<ref MapDescriptorConfiguration> DescriptorConfigs = new array<ref MapDescriptorConfiguration>();
	ref SCR_LayerConfiguration LayerProps;
};

class MapConfiguration
{
	EMapEntityMode MapEntityMode;
	Widget RootWidgetRef;
	 
	int LayerCount;
	int DefaultLayerIndex = 0;
	ref SCR_MapDescriptorDefaults DescriptorDefsConfig;
	ref array<ref LayerConfiguration> LayerConfigs;
	ref array<ref SCR_MapPropsConfig> MapPropsConfigs;
	
	ref array<ref SCR_MapModuleBase> Modules = {}; 			// Must be child of SCR_MapModuleBase, NO constructor arguments
	ref array<ref SCR_MapUIBaseComponent> Components = {};	// Must be child of SCR_MapUIBaseComponent, NO constructor arguments
	EMapOtherComponents OtherComponents = 0; 				// Flags for components that are part of the map system, but not inherited from SCR_MapModuleBase or SCR_MapUIBaseComponent, such as MapLegend
};
