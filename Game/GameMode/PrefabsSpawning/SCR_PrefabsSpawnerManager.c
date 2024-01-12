enum EPrefabSpawnType
{
	Generic,
//	AppleTrees,
	CivilVehicles,
	MilitaryVehicles,
	MilitaryHeliVehicles,
};

//------------------------------------------------------------------------------------------------
class SCR_PrefabsSpawnerManagerClass: GenericEntityClass
{
};

//------------------------------------------------------------------------------------------------
//! Manager for spawning prefabs in the mission.
class SCR_PrefabsSpawnerManager : GenericEntity
{
	[Attribute("", UIWidgets.Object, "Prefabs spawners, each for different type of SpawnPoints")]
	protected ref array<ref SCR_PrefabsSpawner> m_aPrefabsSpawners;
	
	//! Static lists of all SCR_PrefabSpawnPoints
	protected static ref map<EPrefabSpawnType, ref array<SCR_PrefabSpawnPoint>> s_aPrefabSpawnPoints = new ref map<EPrefabSpawnType, ref array<SCR_PrefabSpawnPoint>>();
	
	//------------------------------------------------------------------------------------------------
	static void RegisterPrefabSpawnPoint(SCR_PrefabSpawnPoint prefabSpawnPoint)
	{
		if (GetGame().GetWorldEntity() && s_aPrefabSpawnPoints)
		{
			array<SCR_PrefabSpawnPoint> spawnPoints = s_aPrefabSpawnPoints[prefabSpawnPoint.GetType()];
			
			if (!spawnPoints)
			{
				spawnPoints = new ref array<SCR_PrefabSpawnPoint>();
				s_aPrefabSpawnPoints.Insert(prefabSpawnPoint.GetType(), spawnPoints);
			}
			
			spawnPoints.Insert(prefabSpawnPoint);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	static void UnregisterPrefabSpawnPoint(SCR_PrefabSpawnPoint prefabSpawnPoint)
	{
		if (GetGame().GetWorldEntity() && s_aPrefabSpawnPoints)
		{
			array<SCR_PrefabSpawnPoint> spawnPoints = s_aPrefabSpawnPoints[prefabSpawnPoint.GetType()];
			if (spawnPoints)
				spawnPoints.RemoveItem(prefabSpawnPoint);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called only once. All SCR_PrefabSpawnPoints are already registered.
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (!GetGame().GetWorldEntity() || RplSession.Mode() == RplMode.Client)
		{
			ClearEventMask(EntityEvent.FRAME);
			return;
		}
		
		foreach (auto prefabsSpawner : m_aPrefabsSpawners)
		{
			prefabsSpawner.Spawn(s_aPrefabSpawnPoints[prefabsSpawner.GetType()]);
		}
		
		ClearEventMask(EntityEvent.FRAME);
	}
		
	//------------------------------------------------------------------------------------------------
	void SCR_PrefabsSpawnerManager(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.FRAME);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_PrefabsSpawnerManager()
	{
		s_aPrefabSpawnPoints = new ref map<EPrefabSpawnType, ref array<SCR_PrefabSpawnPoint>>();
	}
};
