[BaseContainerProps()]
class SCR_ResourceGeneratorActionStore : SCR_ResourceGeneratorActionBase
{
	//------------------------------------------------------------------------------------------------
	override void PerformAction(SCR_ResourceContainer containerTo, inout float resourceValue)
	{
		if (!containerTo)
			return;
		
		SCR_ResourceComponent containerComponent = containerTo.GetComponent();
		float usedResources	= Math.Min(containerTo.ComputeResourceDifference(), resourceValue);
		resourceValue		-= usedResources;
		
		SCR_ResourceEncapsulator encapsulator = containerComponent.GetEncapsulator(containerTo.GetResourceType());
		
		if (encapsulator)
			encapsulator.RequestGeneration(usedResources);
		else
			containerTo.IncreaseResourceValue(usedResources);
		
		containerComponent.Replicate();
	}
}
