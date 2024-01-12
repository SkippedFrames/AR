//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_BuildingDestructionStruct : SCR_JsonApiStruct
{
	protected ref array<int> m_aDestroyedBuildings = {};
	
	//------------------------------------------------------------------------------------------------
	override void Log()
	{
		Print("--- SCR_BuildingDestructionStruct ------------------------");
		for (int i = 0, count = m_aDestroyedBuildings.Count(); i < count; i++)
		{
			Print("Destroyed building ID: " + m_aDestroyedBuildings[i]);
		}
		Print("---------------------------------------------");
	}
	
	//------------------------------------------------------------------------------------------------
	override bool Serialize()
	{
		m_aDestroyedBuildings = {};
		
		array<SCR_DestructibleBuildingComponent> destroyedBuildings = {};
		int count = GetGame().GetBuildingDestructionManager().GetDestroyedBuildings(destroyedBuildings);
		
		for (int i = 0; i < count; i++)
		{
			m_aDestroyedBuildings.Insert(destroyedBuildings[i].GetBuildingId());
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool Deserialize()
	{
		for (int i = 0, count = m_aDestroyedBuildings.Count(); i < count; i++)
		{
			SCR_DestructibleBuildingComponent destructibleBuilding = SCR_BuildingDestructionManagerComponent.GetDestructibleBuilding(m_aDestroyedBuildings[i]);
			destructibleBuilding.GoToDestroyedStateLoad();
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_BuildingDestructionStruct()
	{
		RegV("m_aDestroyedBuildings");
	}
}
