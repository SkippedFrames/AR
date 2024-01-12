//! Handles looped ambient effects around specific entities
[BaseContainerProps(configRoot: true)]
class SCR_ObjectPositionalInsects : SCR_AmbientInsectsEffect
{
	[Attribute(defvalue: "100", UIWidgets.Slider, params: "0 1000 1", desc: "How far the player has to move (squared distance) in order to refresh insects around")]
	protected int m_iMinimumMoveDistanceSq;

	[Attribute(defvalue: "30", UIWidgets.Slider, params: "0 100 1", desc: "How far it will seek the objects in range for the Insects to spawn around")]
	protected float m_fSearchDistance;

	[Attribute(desc: "Enables setting up which effects are to be spawned around which entitites")]
	protected ref array<ref SCR_PositionalInsectType> 	m_aPositionalInsectType;

	protected ref array<ref SCR_InsectSpawnDef> m_aSpawnDef = {};
	protected vector m_vCameraPosLooped;

	protected ref array<float> m_aDayTimeCurve = {};
	protected ref array<float> m_aRainIntensityCurve = {};

	//------------------------------------------------------------------------------------------------
	//! Called by SCR_AmbientInsectsComponent.Update()
	override void Update(float worldTime, vector cameraPos, float timeOfDay, float rainIntensity)
	{
		// Limit processing by time and moved distance
		if (vector.DistanceSqXZ(m_vCameraPosLooped, cameraPos) < m_iMinimumMoveDistanceSq)
			return;

		m_vCameraPosLooped = cameraPos;

		// First we get closest entities coresponding to each InsectType
		GetClosestsEntities(cameraPos);

		// Then we trigger update for each InsectType
		foreach (SCR_PositionalInsectType type : m_aPositionalInsectType)
		{
			type.Update(cameraPos);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Finds Entity for each InsectType and adds it to the array of closest entities
	protected void ProcessEntity(notnull IEntity entity)
	{
		EntityPrefabData prefabData = entity.GetPrefabData();
		if (!prefabData)
			return;

		BaseContainer prefabContainer = prefabData.GetPrefab();
		if (!prefabContainer)
			return;

		float dayTimeCurve;
		float rainIntensityCurve;
		
		// According to the prefab, if time and weather conditions are valid, it is added to the respective Insect type 
		foreach (int i, SCR_PositionalInsectType type : m_aPositionalInsectType)
		{
			SCR_PositionalAmbientLeafParticles leaves = SCR_PositionalAmbientLeafParticles.Cast(type);
			if (leaves)
				continue;
			
			if (m_aDayTimeCurve[i] <= 0)
				continue;

			if (m_aRainIntensityCurve[i] <= 0)
				continue;

			BaseContainer prefabContainerToTest = prefabContainer;
			while (prefabContainerToTest)
			{
				if (type.GetPrefabContainerSet().Contains(prefabContainerToTest))
				{
					type.AddClosestEntity(entity);
					return;
				}
	
				prefabContainerToTest = prefabContainerToTest.GetAncestor();
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Clears ClosestEntities array for each InsectType and then runs new querry to fill it again
	void GetClosestsEntities(vector cameraPos)
	{
		// Clear closest entities for all types and retrieve their curve values for time and weather
		foreach (int i, SCR_PositionalInsectType type : m_aPositionalInsectType)
		{
			type.ClearClosestEntities();

			m_aDayTimeCurve[i] = SCR_AmbientSoundsComponent.GetPoint(m_AmbientInsectsComponent.GetTimeOfDay(), m_aSpawnDef[i].m_TimeModifier);
			m_aRainIntensityCurve[i] = SCR_AmbientSoundsComponent.GetPoint(m_AmbientInsectsComponent.GetRainIntensity(), m_aSpawnDef[i].m_RainModifier);
		}
		
		ChimeraWorld world = GetGame().GetWorld();
		if (!world)
			return;

		// Using TagManager we get array of closest entities where Insects can spawn
		TagManager tagManager = world.GetTagManager();
		if (!tagManager)
		{
			Print("AMBIENT LIFE: SCR_AmbientInsectsComponent: Missing TagManager in the world", LogLevel.WARNING);
			m_AmbientInsectsComponent.RemoveInsectEffect(this);
			
			return;
		}

		array<IEntity> closestEntities = {};
		tagManager.GetTagsInRange(closestEntities, cameraPos, m_fSearchDistance, ETagCategory.Insect);

		// Process each entity to add it to the respective Insect type
		foreach (IEntity entity : closestEntities)
		{
			ProcessEntity(entity);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Called by SCR_AmbientInsectsComponent in EOnPostInit()
	override void OnPostInit(SCR_AmbientSoundsComponent ambientSoundsComponent, SCR_AmbientInsectsComponent ambientInsectsComponent, SignalsManagerComponent signalsManagerComponent)
	{
		super.OnPostInit(ambientSoundsComponent, ambientInsectsComponent, signalsManagerComponent);

		SCR_InsectSpawnDef spawnDef = null;
		foreach (SCR_PositionalInsectType type : m_aPositionalInsectType)
		{
			type.Init(ambientSoundsComponent);

			if (!type.m_sSpawnDef.IsEmpty())
			{
				Resource holder = BaseContainerTools.LoadContainer(type.m_sSpawnDef);
				if (holder)
					spawnDef = SCR_InsectSpawnDef.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));
			}

			if (!spawnDef)
			{
				Print("AMBIENT LIFE: SCR_AmbientInsectsComponent: Missing Spawn Definition Config", LogLevel.WARNING);
				m_AmbientInsectsComponent.RemoveInsectEffect(this);
				
				return;
			}

			// Preparing curves and SpawnDefs
			m_aDayTimeCurve.Insert(1);
			m_aRainIntensityCurve.Insert(1);
			m_aSpawnDef.Insert(spawnDef);
		}
	}
}
