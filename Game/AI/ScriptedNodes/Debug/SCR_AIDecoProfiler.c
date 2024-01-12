class SCR_AIDecoProfiler : DecoratorScripted
{
	override string GetOnHoverDescription() { return "Decorator used in external script profiler for profiling performance of trees."; }
	override bool VisibleInPalette() { return true; }

	override bool TestFunction(AIAgent owner)	
	{
		return true;	
	}	
}
