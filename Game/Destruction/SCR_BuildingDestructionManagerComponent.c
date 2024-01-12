[EntityEditorProps(category: "GameScripted/Destruction", description: "Building destruction manager, stores data for destroyed buildings.")]
class SCR_BuildingDestructionManagerComponentClass : ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_BuildingDestructionData
{
	int m_iNextFreeIndex = -1;
	vector m_vTargetOrigin;
	vector m_vStartAngles;
	vector m_vStartMatrix[4];
	vector m_vTargetAngles;
	int m_iRotatedTimes;
	int m_iRotationTime;
	int m_iRotationMultiplier = -1;
	int m_iChecksThisFrame;
	int m_iPauseTime;
	float m_fRotationSpeedMultiplier = 1;
	int m_iRotationStart;
	float m_fSpeedMultiplier = 1;
	ref RandomGenerator m_RandomGenerator = new RandomGenerator();
	ref array<IEntity> m_aQueriedEntities;
	ref set<int> m_aExecutedEffectIndices;
	ref array<IEntity> m_aExcludeList;
	ref array<ref Tuple2<vector, vector>> m_aNavmeshAreas;
	ref array<bool> m_aRedoRoads;
	ref SCR_BuildingDestructionCameraShakeProgress m_CameraShake = new SCR_BuildingDestructionCameraShakeProgress();
	SCR_AudioSource m_AudioSource;
	float m_fBuildingVolume;
	float m_fSizeMultiplier;
	
	//------------------------------------------------------------------------------------------------
	//! Sets variables to default values when freed to be reused by another object on demand
	void Reset()
	{
		m_vTargetOrigin = vector.Zero;
		m_vStartAngles = vector.Zero;
		Math3D.MatrixIdentity4(m_vStartMatrix);
		m_vTargetAngles = vector.Zero;
		m_iRotatedTimes = 0;
		m_iRotationTime = 0;
		m_iRotationMultiplier = -1;
		m_iChecksThisFrame = 0;
		m_iPauseTime = 0;
		m_fRotationSpeedMultiplier = 1;
		m_iRotationStart = 0;
		m_fSpeedMultiplier = 1;
		m_RandomGenerator = new RandomGenerator();
		m_aQueriedEntities = null;
		m_aExecutedEffectIndices = null;
		m_aExcludeList = null;
		m_aNavmeshAreas = null;
		m_aRedoRoads = null;
		m_CameraShake = new SCR_BuildingDestructionCameraShakeProgress();
		m_AudioSource = null;
		m_fBuildingVolume = 0;
	}
};

//------------------------------------------------------------------------------------------------
class SCR_BuildingDestructionManagerComponent : ScriptComponent
{			
	[Attribute()]
	protected ref SCR_BuildingDestructionConfig m_BuildingDestructionConfig;
	
	private int m_iFirstFreeData = -1;
	private ref array<ref SCR_BuildingDestructionData> m_aBuildingDestructionData = {};
	
	protected ref array<SCR_DestructibleBuildingComponent> m_aDestroyedBuildings = {};
	protected static int s_iHighestId;
	protected static ref map<int, SCR_DestructibleBuildingComponent> s_mBuildingIds = new map<int, SCR_DestructibleBuildingComponent>();
	
	//------------------------------------------------------------------------------------------------
	//! Returns data stored at provided index
	notnull SCR_BuildingDestructionData GetData(inout int index)
	{
		if (index == -1)
			index = AllocateData();
		
		return m_aBuildingDestructionData[index];
	}
	
	//------------------------------------------------------------------------------------------------
	//! Allocates data for the objects storing it here
	//! Returns index of the stored data
	private int AllocateData()
	{
		if (m_iFirstFreeData == -1)
			return m_aBuildingDestructionData.Insert(new SCR_BuildingDestructionData());
		
		int returnIndex = m_iFirstFreeData;
		SCR_BuildingDestructionData data = m_aBuildingDestructionData[returnIndex];
		m_iFirstFreeData = data.m_iNextFreeIndex;
		data.m_iNextFreeIndex = -1;
		return returnIndex;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Public to be used by objects storing data here
	void FreeData(int index)
	{
		// If you got a VME here, please report it only if you didn't touch the array/index
		// Otherwise no guarantees
		m_aBuildingDestructionData[index].Reset();
		m_aBuildingDestructionData[index].m_iNextFreeIndex = m_iFirstFreeData;
		m_iFirstFreeData = index;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get array of typenames to be excluded from entity query
	array<string> GetExcludedQueryTypes()
	{
		if (!m_BuildingDestructionConfig)
			return null;
		
		return m_BuildingDestructionConfig.m_aExcludedEntityQueryTypes;
	}
	
	//------------------------------------------------------------------------------------------------
	// Returns true if id is taken by other building than "building"
	static bool IsIdTaken(int id, SCR_DestructibleBuildingComponent building)
	{
		SCR_DestructibleBuildingComponent idBuilding = s_mBuildingIds.Get(id);
		return idBuilding && building != idBuilding;
	}
	
	//------------------------------------------------------------------------------------------------
	static int GetNewId()
	{
		s_iHighestId++;
		return s_iHighestId;
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_DestructibleBuildingComponent GetDestructibleBuilding(int id)
	{
		return s_mBuildingIds.Get(id);
	}
	
	//------------------------------------------------------------------------------------------------
	static void UnregisterBuildingId(notnull SCR_DestructibleBuildingComponent component)
	{
		int id = s_mBuildingIds.GetKeyByValue(component);
		if (id <= 0 || id > s_iHighestId)
			return;
		
		s_mBuildingIds.Remove(id);
		
		if (id == s_iHighestId)
		{
			while (!s_mBuildingIds.Contains(s_iHighestId) && s_iHighestId > 0)
			{
				s_iHighestId--;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	static void RegisterBuildingId(notnull SCR_DestructibleBuildingComponent component, int id)
	{
		if (s_mBuildingIds.Contains(id))
		{
			Print("Id taken!", LogLevel.WARNING);
			return;
		}
		
		if (id >= s_iHighestId)
			s_iHighestId = id;
		
		s_mBuildingIds.Insert(id, component);
	}
	
	//------------------------------------------------------------------------------------------------
	int GetDestroyedBuildings(notnull array<SCR_DestructibleBuildingComponent> destroyedBuildings)
	{
		return destroyedBuildings.Copy(m_aDestroyedBuildings);
	}
	
	//------------------------------------------------------------------------------------------------
	void RegisterDestroyedBuilding(SCR_DestructibleBuildingComponent building)
	{
		if (m_aDestroyedBuildings.Contains(building))
			return;
		
		m_aDestroyedBuildings.Insert(building);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_BuildingDestructionManagerComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		GetGame().RegisterBuildingDestructionManager(this);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_BuildingDestructionManagerComponent()
	{
		GetGame().UnregisterBuildingDestructionManager(this);
	}
};
