class SCR_SBGetServersParams : SBGetServersParams
{
	//------------------------------------------------------------------------------------------------
	void SetDefaultSBGetServersParams()
	{
		RowsPerPage			= 200;
		Page 				= 1;
		SortBy 				= SBSortBy.IDENTIFIER;
		SortOrder 			= SBSortOrder.ASCENDING;
		Platform 			= SBPlatform.ANY;
		Region 				= SBRegion.ANY;
		Filters 			= 0;
		FilterFlags			= 0;
		FilterName			= "";
		FilterDescription	= "";
		FilterGameType		= 0;
		FilterGameMode		= 0;
		FilterGameVersion	= "";
		FilterClientVersion	= "";
		FilterMaxPlayers	= 0;
		FilterMinFreeSlots	= 0;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_SBGetServersParams()
	{
		SetDefaultSBGetServersParams();
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_SBGetServersParams()
	{
	
	}
};
