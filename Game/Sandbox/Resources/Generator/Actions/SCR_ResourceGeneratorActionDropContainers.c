[BaseContainerProps()]
class SCR_ResourceGeneratorActionDropContainers : SCR_ResourceGeneratorActionBase
{
	//------------------------------------------------------------------------------------------------
	override void PerformAction(SCR_ResourceContainer containerTo, inout float resourceValue)
	{
		if (!containerTo)
			return;
		
		SCR_ResourceComponent containerComponent = containerTo.GetComponent();
		float usedResources	= Math.Min(containerTo.ComputeResourceDifference(), resourceValue);
		resourceValue		-= usedResources;
		
		containerTo.IncreaseResourceValue(usedResources);
		containerComponent.Replicate();
	}
}
