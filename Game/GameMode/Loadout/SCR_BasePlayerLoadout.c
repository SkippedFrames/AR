//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_BasePlayerLoadout
{	
	//------------------------------------------------------------------------------------------------
	ResourceName GetLoadoutResource()
	{
		return ResourceName.Empty;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetLoadoutName()
	{
		return string.Empty;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetLoadoutImageResource()
	{
		return ResourceName.Empty;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsLoadoutAvailable(int playerId)
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsLoadoutAvailableClient()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnLoadoutSpawned(GenericEntity pOwner, int playerId)
	{
	}
};
