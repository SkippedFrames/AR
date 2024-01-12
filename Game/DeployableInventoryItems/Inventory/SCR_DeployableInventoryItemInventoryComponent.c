[EntityEditorProps(category: "GameScripted/Components", description: "Mine inventory item component.")]
class SCR_DeployableInventoryItemInventoryComponentClass : InventoryItemComponentClass
{
	
};

//------------------------------------------------------------------------------------------------
class SCR_DeployableInventoryItemInventoryComponent : InventoryItemComponent
{
	//------------------------------------------------------------------------------------------------
	override bool ShouldHideInVicinity()
	{
		SCR_BaseDeployableInventoryItemComponent deployableItemComponent = SCR_BaseDeployableInventoryItemComponent.Cast(GetOwner().FindComponent(SCR_BaseDeployableInventoryItemComponent));
		if (!deployableItemComponent)
			return false;
		
		return deployableItemComponent.IsDeployed();
	}
};
