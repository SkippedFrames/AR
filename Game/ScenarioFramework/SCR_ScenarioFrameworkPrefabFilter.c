[BaseContainerProps()]
class SCR_ScenarioFrameworkPrefabFilter
{
	[Attribute(desc: "If SPECIFIC_PREFAB_NAME is selected, fill the class name here.", category: "Trigger")]
	ResourceName m_sSpecificPrefabName;
	
	[Attribute(defvalue: "0", UIWidgets.CheckBox, desc: "Activate the trigger once or everytime the activation condition is true?", category: "Trigger")]
	bool m_bIncludeChildren;
	
	BaseContainer m_PrefabContainer;
};

[BaseContainerProps()]
class SCR_ScenarioFrameworkPrefabFilterCountNoInheritance
{
	[Attribute(desc: "Prefab name of the entity")]
	ResourceName m_sPrefabName;
	
	[Attribute(defvalue: "1", desc: "Count of how many of these prefab instances will be there", params: "0 100000 1")]
	int m_iPrefabCount;
};

[BaseContainerProps()]
class SCR_ScenarioFrameworkPrefabFilterCount
{
	[Attribute(desc: "If SPECIFIC_PREFAB_NAME is selected, fill the class name here.", category: "Trigger")]
	ResourceName m_sSpecificPrefabName;
	
	[Attribute(defvalue: "0", UIWidgets.CheckBox, desc: "Activate the trigger once or everytime the activation condition is true?", category: "Trigger")]
	bool m_bIncludeChildren;
	
	[Attribute(defvalue: "1", desc: "How many entities of specific prefab are requiered to be inside the trigger", params: "0 100000 1", category: "Trigger")]
	int m_iPrefabCount;
	
	BaseContainer m_PrefabContainer;
};
