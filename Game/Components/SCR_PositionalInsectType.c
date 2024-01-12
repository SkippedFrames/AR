//! Base class that handles different insects based on their type
[BaseContainerProps(configRoot: true)]
class SCR_PositionalInsectType
{
	protected const int INVALID = -1;
	protected SCR_AmbientSoundsComponent m_AmbientSoundsComponent;
	protected SignalsManagerComponent m_LocalSignalsManager;

	protected ref array<IEntity> m_aClosestEntities = {};
	protected ref array<IEntity> m_aActiveEntities = {};
	protected ref array<ref SCR_InsectParticle> m_aParticles = {};

	protected ref set<BaseContainer> m_PrefabContainerSet = new ref set<BaseContainer>();

	[Attribute()]
	protected ref array<ref SCR_InsectDef> m_aInsectDef;

	[Attribute("", UIWidgets.ResourceNamePicker, desc: "", params: "conf")]
	ResourceName m_sSpawnDef;

	[Attribute(defvalue: "25", UIWidgets.Slider, params: "0 100 1", desc: "When suitable entity is found, with what chance is Insect spawned there")]
	protected int m_iSpawnChance;

	[Attribute(defvalue: "0 0 0", desc: "Offset from parent entity")];
	protected vector 	m_vOffset;

	//------------------------------------------------------------------------------------------------
	//! Initializes necessary things for Insect type
	void Init(SCR_AmbientSoundsComponent ambientSoundsComponent)
	{
		m_AmbientSoundsComponent = ambientSoundsComponent;
	}

	//------------------------------------------------------------------------------------------------
	//! Performs updates to the Insect type
	void Update(vector cameraPos);
	
	//------------------------------------------------------------------------------------------------
	//! Randomizes animation for Insect
	protected void RandomizeAnimation(IEntity entity)
	{
		AnimationPlayerComponent animComponent = AnimationPlayerComponent.Cast(entity.FindComponent(AnimationPlayerComponent));
		if (!animComponent)
			return;
		
		SCR_AnimationResourceHolderComponent animResourceHolder = SCR_AnimationResourceHolderComponent.Cast(entity.FindComponent(SCR_AnimationResourceHolderComponent));
		if (!animResourceHolder)
			return;
			
		animComponent.Prepare(animResourceHolder.m_sAnimation, Math.RandomFloatInclusive(1, 5), 1, true);
		animComponent.Play();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Spawns particle with sound based on spawnParams and randomized chance
	protected void SpawnParticle(ParticleEffectEntitySpawnParams spawnParams, int chance = 100)
	{
		SCR_InsectDef insectDef = m_aInsectDef.GetRandomElement();
		SCR_InsectParticle particle = new SCR_InsectParticle();
		if (Math.RandomIntInclusive(0, 100) <= chance)
		{
			string soundName = insectDef.m_sSoundName;
			if (!soundName.IsEmpty())
				particle.m_AudioHandle = m_AmbientSoundsComponent.SoundEventLooped(soundName, spawnParams.Transform);

			ResourceName prefabName = insectDef.m_sPrefabName;
			if (!prefabName.IsEmpty())
			{
				Resource resource = Resource.Load(prefabName);
				if (resource)
				{
					EntitySpawnParams entitySpawnParams = new EntitySpawnParams;
					entitySpawnParams.Transform = spawnParams.Transform;
					entitySpawnParams.TransformMode = spawnParams.TransformMode;
					
					particle.m_InsectEntity = GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), entitySpawnParams);
					if (particle.m_InsectEntity)
					{
						particle.m_InsectEntity.SetVComponentFlags(VCFlags.DYNAMICBBOX);
						RandomizeAnimation(particle.m_InsectEntity);
					}
				}
			}
			else if (!insectDef.m_sParticleEffect.IsEmpty())
			{
				particle.m_ParticleEffect = ParticleEffectEntity.SpawnParticleEffect(insectDef.m_sParticleEffect, spawnParams);
			}

#ifdef ENABLE_DIAG
			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AMBIENT_LIFE))
			{
				if (!soundName.IsEmpty())
					particle.m_DebugShape = Shape.CreateSphere(COLOR_GREEN, ShapeFlags.NOZWRITE | ShapeFlags.WIREFRAME, spawnParams.Transform[3], 0.2);
				else
					particle.m_DebugShape = Shape.CreateSphere(COLOR_BLUE, ShapeFlags.NOZWRITE | ShapeFlags.WIREFRAME, spawnParams.Transform[3], 0.2);
			}
#endif
		}

		m_aParticles.Insert(particle);
	}

	//------------------------------------------------------------------------------------------------
	//! Deletes Insects that are too far away
	protected void RemoveDistantInsects()
	{
		for (int i = m_aActiveEntities.Count() - 1; i >= 0; i--)
		{
			int index = m_aClosestEntities.Find(m_aActiveEntities[i]);
			if (index == INVALID)
			{
				if (m_aParticles[i].m_AudioHandle)
					m_AmbientSoundsComponent.TerminateLooped(m_aParticles[i].m_AudioHandle);
				
				m_aActiveEntities.Remove(i);
				m_aParticles.Remove(i);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get prefab container set
	\return prefab container set
	*/
	set<BaseContainer> GetPrefabContainerSet()
	{
		return m_PrefabContainerSet;
	}

	//------------------------------------------------------------------------------------------------
	//! Adds entity to the closest entities array for selected insect type
	void AddClosestEntity(IEntity entity)
	{
		m_aClosestEntities.Insert(entity);
	}

	//------------------------------------------------------------------------------------------------
	//! Clears closest entities array for selected insect type
	void ClearClosestEntities()
	{
		m_aClosestEntities.Clear();
	}
}

//! Handles insects that are supposed to be spawned around light sources defined in prefab names array
[BaseContainerProps(configRoot: true)]
class SCR_PositionalLightSourceInsect : SCR_PositionalInsectType
{
	[Attribute(desc: "Prefab names of entities that correspond with this insect type")]
	protected ref array<ResourceName> 	m_aPrefabNames;

	//------------------------------------------------------------------------------------------------
	//! Initializes necessary things for Insect type
	override void Init(SCR_AmbientSoundsComponent ambientSoundsComponent)
	{
		super.Init(ambientSoundsComponent);

		array<BaseContainer> BaseContainerArray = {};
		BaseContainerArray = SCR_BaseContainerTools.GetBaseContainerArray(m_aPrefabNames);
		
		foreach(BaseContainer baseContainer : BaseContainerArray)
		{
			m_PrefabContainerSet.Insert(baseContainer);
		}

		delete m_aPrefabNames;
	}

	//------------------------------------------------------------------------------------------------
	//! Performs updates to the Insect type
	override void Update(vector cameraPos)
	{
		// Removes insects that are no longer among closest entities
		RemoveDistantInsects();

		// Play sounds on entities, that become closest
		ProcessLightsSources();
	}

	//------------------------------------------------------------------------------------------------
	//! Processeses closest light sources and spawns Insects on correct positions
	protected void ProcessLightsSources()
	{
		foreach (IEntity entity : m_aClosestEntities)
		{
			if (m_aActiveEntities.Contains(entity))
				continue;

			ParticleEffectEntitySpawnParams spawnParams();
			vector mat[4];
			entity.GetWorldTransform(mat);
			vector position = mat[3];
			
			//There is one lamp that does not have the light as child entity
			//but using a StreetLampComponent that has 5.65m offset on Y axis
			position[1] = position[1] + 5.65;
			position += m_vOffset;
			
			//All the other lamps have lights as their children
			IEntity childEntity = entity.GetChildren();
			if (childEntity)
				position = 	childEntity.CoordToParent(m_vOffset);
			
			mat[3] = position;
			
			spawnParams.TransformMode = ETransformMode.WORLD;
			spawnParams.Transform = mat;
			
			//Apply rotation
			vector angles = Math3D.MatrixToAngles(spawnParams.Transform);
			Math3D.AnglesToMatrix(angles, spawnParams.Transform);

			m_aActiveEntities.Insert(entity);
			SpawnParticle(spawnParams, m_iSpawnChance);
		}
	}
}

//! Handles insects that are supposed to be spawned trees
[BaseContainerProps(configRoot: true)]
class SCR_PositionalAmbientLeafParticles : SCR_PositionalInsectType
{
	//------------------------------------------------------------------------------------------------
	//! Performs updates to the Insect type
	override void Update(vector cameraPos)
	{
		//Get closest entities array
		m_AmbientSoundsComponent.GetClosestEntities(EQueryType.TreeLeafy, 10, m_aClosestEntities);

		// Removes insects that are no longer among closest entities
		RemoveDistantInsects();

		// Play sounds on entities, that become closest
		ProcessTrees();
	}

	//------------------------------------------------------------------------------------------------
	//! Processeses closest trees and spawns Insects on correct positions
	protected void ProcessTrees()
	{
		foreach (IEntity entity : m_aClosestEntities)
		{
			if (m_aActiveEntities.Contains(entity))
				continue;

			//Get tree entity
			Tree tree = Tree.Cast(entity);
			if (!tree)
				continue;

			//Get prefab data
			TreeClass treeClass = TreeClass.Cast(tree.GetPrefabData());
			if (!treeClass)
				continue;

			int foliageHeight = treeClass.m_iFoliageHeight * entity.GetScale();

			ParticleEffectEntitySpawnParams spawnParams();
			vector mat[4];
			entity.GetWorldTransform(mat);
			
			vector mins, maxs;
			entity.GetWorldBounds(mins, maxs);
			float minY = mins[1] + foliageHeight;
			float maxY = maxs[1];
			
			vector position = mat[3];
			position[1] = (minY + maxY) * 0.5;
			position += m_vOffset;
			mat[3] = position;
			
			spawnParams.TransformMode = ETransformMode.WORLD;
			spawnParams.Transform = mat;
			
			//Apply rotation
			vector angles = Math3D.MatrixToAngles(spawnParams.Transform);
			Math3D.AnglesToMatrix(angles, spawnParams.Transform);

			m_aActiveEntities.Insert(entity);
			SpawnParticle(spawnParams, m_iSpawnChance);
		}
	}
}

//! Handles insects that are supposed to be spawned around selected prefabs defined in prefab names array
[BaseContainerProps(configRoot: true)]
class SCR_PositionalGenericInsect : SCR_PositionalInsectType
{
	[Attribute(desc: "Prefab names of entities that correspond with this insect type")]
	protected ref array<ResourceName> 	m_aPrefabNames;

	//------------------------------------------------------------------------------------------------
	//! Initializes necessary things for Insect type
	override void Init(SCR_AmbientSoundsComponent ambientSoundsComponent)
	{
		super.Init(ambientSoundsComponent);

		array<BaseContainer> BaseContainerArray = {};
		BaseContainerArray = SCR_BaseContainerTools.GetBaseContainerArray(m_aPrefabNames);
		
		foreach(BaseContainer baseContainer : BaseContainerArray)
		{
			m_PrefabContainerSet.Insert(baseContainer);
		}
		
		delete m_aPrefabNames;
	}

	//------------------------------------------------------------------------------------------------
	//! Performs updates to the Insect type
	override void Update(vector cameraPos)
	{
		// Removes insects that are no longer among closest entities
		RemoveDistantInsects();

		// Play sounds on entities, that become closest
		ProcessEntitySources();
	}

	//------------------------------------------------------------------------------------------------
	//! Processeses closest entities and spawns Insects on correct positions
	protected void ProcessEntitySources()
	{
		foreach (IEntity entity : m_aClosestEntities)
		{
			if (m_aActiveEntities.Contains(entity))
				continue;
			
			ParticleEffectEntitySpawnParams spawnParams();
			vector mat[4];
			entity.GetWorldTransform(mat);
			
			vector position = mat[3];
			position += m_vOffset;
			mat[3] = position;
			
			spawnParams.TransformMode = ETransformMode.WORLD;
			spawnParams.Transform = mat;
			
			//Apply rotation
			vector angles = Math3D.MatrixToAngles(spawnParams.Transform);
			Math3D.AnglesToMatrix(angles, spawnParams.Transform);

			m_aActiveEntities.Insert(entity);
			SpawnParticle(spawnParams, m_iSpawnChance);
		}
	}
}
