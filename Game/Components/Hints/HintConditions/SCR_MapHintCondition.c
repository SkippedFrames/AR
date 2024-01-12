[BaseContainerProps(), SCR_BaseContainerHintCondition()]
class SCR_MapHintCondition: SCR_BaseEditorHintCondition
{
	override protected void OnInitCondition(Managed owner)
	{
		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();
		if (mapEntity)
		{
			mapEntity.GetOnMapOpen().Insert(OnMapOpen);
			mapEntity.GetOnMapClose().Insert(OnMapClosed);
		}
	}
	override protected void OnExitCondition(Managed owner)
	{
		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();
		if (mapEntity)
		{
			mapEntity.GetOnMapOpen().Remove(OnMapOpen);
			mapEntity.GetOnMapClose().Remove(OnMapClosed);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnMapOpen(MapConfiguration config)
	{
		Activate();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnMapClosed(MapConfiguration config)
	{
		Deactivate();
	}
};
