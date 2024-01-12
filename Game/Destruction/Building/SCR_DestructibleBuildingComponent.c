//------------------------------------------------------------------------------------------------
class SCR_DestructibleBuildingComponentClass : ScriptedDamageManagerComponentClass
{
	[Attribute()]
	ref array<ref SCR_TimedEffect> m_aEffects;
	
	[Attribute("0", "Delay in seconds between damage and the beginning of the building transition")]
	float m_fDelay;
	
	[Attribute("1", "Meters per second")]
	float m_fSinkingSpeed;
	
	[Attribute("150", "This speeds up the sinking gradually % per second.", params: "0 10000 1")]
	float m_fSinkingSpeedGradualMultiplier;
	
	[Attribute("0.1", "Degrees per second")]
	float m_fRotationSpeed;
	
	[Attribute("0.8", "Time between rotation changes when building is collapsing in seconds")]
	float m_fRotationTime;
	
	[Attribute("50", "Rotation time randomizer in % - can both shorten/prolong the time", params: "0 10000 1")]
	float m_fRotationTimeRandom;
	
	[Attribute("1", "Max rotations count while sinking")]
	int m_iMaxRotations;
	
	[Attribute(vector.Zero.ToString(), "This vector defines offset for the final position after destruction.")]
	vector m_vSinkVector;
		
	[Attribute("", UIWidgets.Auto, desc: "Slow down event audio source configuration")]
	ref SCR_AudioSourceConfiguration m_AudioSourceConfiguration;
	
	[Attribute(uiwidget: UIWidgets.GraphDialog)]
	ref Curve m_CameraShakeCurve;
	
	[Attribute(uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(SCR_EDestructionRotationEnum))]
	SCR_EDestructionRotationEnum m_eAllowedRotations;
};

//------------------------------------------------------------------------------------------------
enum SCR_EDestructionRotationEnum
{
	NONE = 0,
	ROTATION_X = 1,
	ROTATION_Y = 2,
	ROTATION_Z = 4
}

//------------------------------------------------------------------------------------------------
class SCR_BuildingDestructionCameraShakeProgress : SCR_NoisyCameraShakeProgress
{
	protected const float MAX_MULTIPLIER = 1.5;
	protected float m_fMaxDistance = 50;
	protected float m_fSizeMultiplier = 1;
	
	protected ref Curve m_CameraShakeCurve;
	protected vector m_vStartOrigin;
	
	//------------------------------------------------------------------------------------------------
	override void Update(IEntity owner, float timeSlice)
	{
		if (!IsRunning())
			return;
		
		super.Update(owner, timeSlice);
		
		CameraBase camera = GetGame().GetCameraManager().CurrentCamera();
		if (!camera)
			return;
		
		float distanceSq = vector.DistanceSqXZ(m_vStartOrigin, camera.GetOrigin());
		float multiplier = 1 - Math.Clamp(distanceSq / (m_fMaxDistance * m_fMaxDistance * m_fSizeMultiplier), 0, 1);
		float curveMultiplier = Math3D.Curve(ECurveType.CatmullRom, 1 - multiplier, m_CameraShakeCurve)[1];
		
		m_vTranslation *= Math.Min(multiplier * m_fSizeMultiplier * curveMultiplier, MAX_MULTIPLIER);
		m_vRotation *= Math.Min(multiplier * m_fSizeMultiplier * curveMultiplier, MAX_MULTIPLIER);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetStartOrigin(vector startOrigin)
	{
		m_vStartOrigin = startOrigin
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCurve(Curve curve)
	{
		m_CameraShakeCurve = curve;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSizeMultiplier(float sizeMultiplier)
	{
		m_fSizeMultiplier = sizeMultiplier;
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class SCR_TimedDebris : SCR_TimedEffect
{
	[Attribute("0 0 0", UIWidgets.Coords, desc: "Positional offset (in local space to the destructible)")]
	protected vector m_vOffsetPosition;
	
	[Attribute("0 0 0", UIWidgets.Coords, desc: "Yaw, pitch & roll offset (in local space to the destructible)")]
	protected vector m_vOffsetRotation;
	
	[Attribute(ResourceName.Empty, UIWidgets.ResourcePickerThumbnail, "Debris model prefabs to spawn (spawns ALL of them)", "et xob")]
	ref array<ResourceName> m_ModelPrefabs;
	
	[Attribute("10", UIWidgets.Slider, "Mass of the debris", "0.01 1000 0.01")]
	float m_fMass;
	
	[Attribute("5", UIWidgets.Slider, "Minimum lifetime value for the debris (in s)", "0 3600 0.5")]
	float m_fLifetimeMin;
	
	[Attribute("10", UIWidgets.Slider, "Maximum lifetime value for the debris (in s)", "0 3600 0.5")]
	float m_fLifetimeMax;
	
	[Attribute("200", UIWidgets.Slider, "Maximum distance from camera above which the debris is not spawned (in m)", "0 3600 0.5")]
	float m_fDistanceMax;
	
	[Attribute("0", UIWidgets.Slider, "Higher priority overrides lower priority if at or over debris limit", "0 100 1")]
	int m_fPriority;
	
	[Attribute("0.1", UIWidgets.Slider, "Damage received to physics impulse (speed / mass) multiplier", "0 10000 0.01")]
	float m_fDamageToImpulse;
	
	[Attribute("2", UIWidgets.Slider, "Damage to speed multiplier, used when objects get too much damage to impulse", "0 10000 0.01")]
	float m_fMaxDamageToSpeedMultiplier;
	
	[Attribute("0.5", UIWidgets.Slider, "Random linear velocity multiplier (m/s)", "0 200 0.1")]
	float m_fRandomVelocityLinear;
	
	[Attribute("180", UIWidgets.Slider, "Random angular velocity multiplier (deg/s)", "0 3600 0.1")]
	float m_fRandomVelocityAngular;
	
	[Attribute("0", uiwidget: UIWidgets.ComboBox, "Type of material for debris sound", "", ParamEnumArray.FromEnum(SCR_EMaterialSoundTypeDebris))]
	SCR_EMaterialSoundTypeDebris m_eMaterialSoundType;
	
	//------------------------------------------------------------------------------------------------
	//! Calculates the spawn tranformation matrix for the object
	void GetSpawnTransform(IEntity owner, out vector outMat[4], bool localCoords = false)
	{
		if (localCoords)
		{
			Math3D.AnglesToMatrix(m_vOffsetRotation, outMat);
			// TODO: Remove hotfix for sleeping/static object
			if (m_vOffsetPosition == vector.Zero)
				outMat[3] = vector.Up * 0.001;
			else
				outMat[3] = m_vOffsetPosition;
		}
		else
		{
			vector localMat[4], parentMat[4];
			owner.GetWorldTransform(parentMat);
			Math3D.AnglesToMatrix(m_vOffsetRotation, localMat);
			localMat[3] = m_vOffsetPosition;
			
			Math3D.MatrixMultiply4(parentMat, localMat, outMat);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Spawns the object
	override void ExecuteEffect(IEntity owner, SCR_HitInfo hitInfo, inout notnull SCR_BuildingDestructionData data)
	{
		int numModelPrefabs = 0;
		if (m_ModelPrefabs)
			numModelPrefabs = m_ModelPrefabs.Count();
		
		for (int i = 0; i < numModelPrefabs; i++)
		{
			ResourceName prefabPath = m_ModelPrefabs[i];
			
			ResourceName modelPath;
			string remap;
			SCR_Global.GetModelAndRemapFromResource(prefabPath, modelPath, remap);
			
			if (modelPath == ResourceName.Empty)
				continue;
			
			vector spawnMat[4];
			GetSpawnTransform(owner, spawnMat);
			
			SCR_DestructionBaseComponent destructionComponent = SCR_DestructionBaseComponent.Cast(owner.FindComponent(SCR_DestructionBaseComponent));
			
			float dmgSpeed = Math.Clamp(hitInfo.m_HitDamage * m_fDamageToImpulse / m_fMass, 0, m_fMaxDamageToSpeedMultiplier);
			
			vector linearVelocity = hitInfo.m_HitDirection * Math.RandomFloat(0, 1);
			linearVelocity += Vector(Math.RandomFloat(-1, 1), Math.RandomFloat(-1, 1), Math.RandomFloat(-1, 1)) * m_fRandomVelocityLinear;
			linearVelocity *= dmgSpeed;
			vector angularVelocity = Vector(Math.RandomFloat(-1, 1), Math.RandomFloat(-1, 1), Math.RandomFloat(-1, 1)) * Math.RandomFloat(0.25, 4) * m_fRandomVelocityAngular;
			angularVelocity *= dmgSpeed;
			
			SCR_DebrisSmallEntity.SpawnDebris(owner.GetWorld(), spawnMat, modelPath, m_fMass, Math.RandomFloat(m_fLifetimeMin, m_fLifetimeMax), m_fDistanceMax, m_fPriority, linearVelocity, angularVelocity, remap, false, m_eMaterialSoundType);
		}
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class SCR_TimedEffect : Managed
{
	[Attribute("0", UIWidgets.Slider, "Set time in % of sinking the building. 0 = Immediately, can happen before the sinking starts if delay is used", "0 1 0.01")]
	float m_fSpawnTime;
	
	[Attribute()]
	bool m_bSnapToTerrain;
	
	[Attribute()]
	bool m_bAttachToParent;
	
	[Attribute("0", desc: "Does this effect remain after destruction? F. E. ruins.")]
	bool m_bPersistent;
	
	//------------------------------------------------------------------------------------------------
	//! Snaps the origin to terrain
	void SnapToTerrain(inout vector origin, IEntity owner)
	{
		if (!m_bSnapToTerrain)
			return;
		
		float y = SCR_TerrainHelper.GetTerrainY(origin, owner.GetWorld());
		origin[1] = y;
	}
	
	//------------------------------------------------------------------------------------------------
	//! To be overridden by inherited classes
	void ExecuteEffect(IEntity owner, SCR_HitInfo hitInfo, inout notnull SCR_BuildingDestructionData data)
	{
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class SCR_TimedSound : SCR_TimedEffect
{
	[Attribute("", UIWidgets.Auto, desc: "Audio Source Configuration")]
	protected ref SCR_AudioSourceConfiguration m_AudioSourceConfiguration;
	
	//------------------------------------------------------------------------------------------------
	//! Plays sound
	override void ExecuteEffect(IEntity owner, SCR_HitInfo hitInfo, inout notnull SCR_BuildingDestructionData data)
	{
		super.ExecuteEffect(owner, hitInfo, data);
		
		SCR_SoundManagerEntity soundManagerEntity = GetGame().GetSoundManagerEntity();
		if (!soundManagerEntity || !m_AudioSourceConfiguration || !m_AudioSourceConfiguration.IsValid())
			return;
				
		SCR_DestructibleBuildingComponent destructibleComponent = SCR_DestructibleBuildingComponent.Cast(owner.FindComponent(SCR_DestructibleBuildingComponent));
		if (!destructibleComponent)
			return;
		
		SCR_AudioSource audioSource = soundManagerEntity.CreateAudioSource(owner, m_AudioSourceConfiguration);
		if (!audioSource)
			return;
		
		destructibleComponent.SetAudioSource(audioSource);
		soundManagerEntity.PlayAudioSource(audioSource, data.m_vStartMatrix);
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class SCR_TimedPrefab : SCR_TimedEffect
{
	[Attribute("", "Defines what remains after the building is destroyed.", params: "et")]
	protected ResourceName m_sRuinsPrefab;
	
	//------------------------------------------------------------------------------------------------
	//! Spawns prefab
	override void ExecuteEffect(IEntity owner, SCR_HitInfo hitInfo, inout notnull SCR_BuildingDestructionData data)
	{
		super.ExecuteEffect(owner, hitInfo, data);
		
		Resource resource = Resource.Load(m_sRuinsPrefab);
		if (!resource || !resource.IsValid())
			return;
		
		vector mat[4];
		mat = data.m_vStartMatrix;
		
		if (m_bSnapToTerrain)
		{
			vector origin = mat[3];
			SnapToTerrain(origin, owner);
			mat[3] = origin;
		}
		
		EntitySpawnParams params = new EntitySpawnParams();
		params.Transform = mat;
		params.TransformMode = ETransformMode.WORLD;
		
		if (m_bAttachToParent)
			params.Parent = owner;
		
		IEntity spawnedEntity = GetGame().SpawnEntityPrefab(resource, owner.GetWorld(), params);
		if (!spawnedEntity)
		{
			Debug.Error("Could not spawn prefab in SCR_TimedPrefab.ExecuteEffect()");
			return;
		}
		
		data.m_aExcludeList.Insert(spawnedEntity);
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class SCR_TimedParticle : SCR_TimedEffect
{
	[Attribute()]
	protected ref SCR_ParticleSpawnable m_Particle;
	
	[Attribute("1")]
	protected float m_fParticlesMultiplier;
	
	//------------------------------------------------------------------------------------------------
	//! Plays particles
	override void ExecuteEffect(IEntity owner, SCR_HitInfo hitInfo, inout notnull SCR_BuildingDestructionData data)
	{
		super.ExecuteEffect(owner, hitInfo, data);
		
		ParticleEffectEntity emitter;
		if (m_bAttachToParent)
			emitter = m_Particle.SpawnAsChild(owner, hitInfo, m_bSnapToTerrain);
		else
			emitter = ParticleEffectEntity.Cast(m_Particle.Spawn(owner, owner.GetPhysics(), hitInfo, m_bSnapToTerrain));
		
		if (!emitter)
		{
			Debug.Error("No emitter was spawned in SCR_TimedParticle.ExecuteEffect()");
			return;
		}
		
		SetParticleParams(emitter, data);
		
		data.m_aExcludeList.Insert(emitter);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetParticleParams(ParticleEffectEntity emitter, inout notnull SCR_BuildingDestructionData data)
	{
		Particles particles = emitter.GetParticles();
		if (!particles)
			return;
		
		particles.MultParam(-1, EmitterParam.BIRTH_RATE, Math.Clamp(data.m_fSizeMultiplier, 0.5, 2) * m_fParticlesMultiplier);
		particles.MultParam(-1, EmitterParam.BIRTH_RATE_RND, Math.Clamp(data.m_fSizeMultiplier, 0.5, 2) * m_fParticlesMultiplier);
		particles.MultParam(-1, EmitterParam.SIZE, Math.Clamp(data.m_fSizeMultiplier, 0.5, 1));
		particles.MultParam(-1, EmitterParam.VELOCITY, Math.Clamp(data.m_fSizeMultiplier, 0.5, 2) * m_fParticlesMultiplier);
		particles.MultParam(-1, EmitterParam.VELOCITY_RND, Math.Clamp(data.m_fSizeMultiplier, 0.5, 2) * m_fParticlesMultiplier);
	}
};

//------------------------------------------------------------------------------------------------
class SCR_DestructibleBuildingComponent : ScriptedDamageManagerComponent
{
	// In project settings you can see the physics response indices & their interactions matrix
	protected const int NO_COLLISION_RESPONSE_INDEX = 11;
	protected const int MAX_CHECKS_PER_FRAME = 20;
	protected const float BUILDING_SIZE = 5000;
	protected const vector TRACE_DIRECTIONS[3] = {vector.Right, vector.Up, vector.Forward};
	
	private int m_iDataIndex = -1;
	
	protected bool m_bDestroyed = false;
	
	//------------------------------------------------------------------------------------------------
	//! Returns centrally stored data from building destruction manager
	protected SCR_BuildingDestructionData GetData()
	{
		SCR_BuildingDestructionManagerComponent manager = GetGame().GetBuildingDestructionManager();
		if (!manager)
		{
			Print("SCR_BuildingDestructionManagerComponent not found! Building destruction won't work.", LogLevel.ERROR);
			return null;
		}
		
		return manager.GetData(m_iDataIndex);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Frees the data stored in building destruction manager
	protected void FreeData()
	{
		SCR_BuildingDestructionManagerComponent manager = GetGame().GetBuildingDestructionManager();
		if (!manager)
		{
			Print("SCR_BuildingDestructionManagerComponent not found! Building destruction won't work.", LogLevel.ERROR);
			return;
		}
		
		manager.FreeData(m_iDataIndex);
		m_iDataIndex = -1;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns prefab data stored speed gradual multiplier
	protected float GetSpeedGradualMultiplier()
	{
		return SCR_DestructibleBuildingComponentClass.Cast(GetComponentData(GetOwner())).m_fSinkingSpeedGradualMultiplier;
	}
	
	//------------------------------------------------------------------------------------------------
	protected float GetRotationTimeRandomizer()
	{
		return SCR_DestructibleBuildingComponentClass.Cast(GetComponentData(GetOwner())).m_fRotationTimeRandom;
	}
	
	//------------------------------------------------------------------------------------------------
	protected float GetRotationSpeed()
	{
		return SCR_DestructibleBuildingComponentClass.Cast(GetComponentData(GetOwner())).m_fRotationSpeed;
	}
	
	//------------------------------------------------------------------------------------------------
	protected float GetRotationTime()
	{
		return SCR_DestructibleBuildingComponentClass.Cast(GetComponentData(GetOwner())).m_fRotationTime;
	}
	
	//------------------------------------------------------------------------------------------------
	protected int GetMaxRotations()
	{
		return SCR_DestructibleBuildingComponentClass.Cast(GetComponentData(GetOwner())).m_iMaxRotations;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns prefab data stored speed
	protected float GetSpeed()
	{
		return SCR_DestructibleBuildingComponentClass.Cast(GetComponentData(GetOwner())).m_fSinkingSpeed;
	}
	
	//------------------------------------------------------------------------------------------------
	protected vector GetSinkVector()
	{
		return SCR_DestructibleBuildingComponentClass.Cast(GetComponentData(GetOwner())).m_vSinkVector;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns prefab data stored delay
	protected float GetDelay()
	{
		return SCR_DestructibleBuildingComponentClass.Cast(GetComponentData(GetOwner())).m_fDelay;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool GetAllowRotationX()
	{
		return SCR_DestructibleBuildingComponentClass.Cast(GetComponentData(GetOwner())).m_eAllowedRotations & SCR_EDestructionRotationEnum.ROTATION_X;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool GetAllowRotationY()
	{
		return SCR_DestructibleBuildingComponentClass.Cast(GetComponentData(GetOwner())).m_eAllowedRotations & SCR_EDestructionRotationEnum.ROTATION_Y;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool GetAllowRotationZ()
	{
		return SCR_DestructibleBuildingComponentClass.Cast(GetComponentData(GetOwner())).m_eAllowedRotations & SCR_EDestructionRotationEnum.ROTATION_Z;
	}
	
	//------------------------------------------------------------------------------------------------
	protected Curve GetCameraShakeCurve()
	{
		return SCR_DestructibleBuildingComponentClass.Cast(GetComponentData(GetOwner())).m_CameraShakeCurve;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns pointer to prefab data stored array of effects, do not modify the array!
	protected array<ref SCR_TimedEffect> GetEffects()
	{
		return SCR_DestructibleBuildingComponentClass.Cast(GetComponentData(GetOwner())).m_aEffects;
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_AudioSourceConfiguration GetSlowDownAudioSourceConfiguration()
	{
		return SCR_DestructibleBuildingComponentClass.Cast(GetComponentData(GetOwner())).m_AudioSourceConfiguration;
	}
		
	//------------------------------------------------------------------------------------------------
	void SetAudioSource(SCR_AudioSource audioSource)
	{
		SCR_BuildingDestructionData data = GetData();
		if (!data)
			return;
		
		if (data.m_AudioSource)
		{
			// Kill previous sound
			SCR_SoundManagerEntity soundManagerEntity = GetGame().GetSoundManagerEntity();
			if (soundManagerEntity)
				soundManagerEntity.TerminateAudioSource(data.m_AudioSource);
		}
		
		data.m_AudioSource = audioSource;
		
		if (audioSource)
			data.m_AudioSource.SetSignalValue(SCR_AudioSource.ENTITY_SIZE_SIGNAL_NAME, data.m_fBuildingVolume);
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_AudioSource GetAudioSource()
	{
		SCR_BuildingDestructionData data = GetData();
		if (!data)
			return null;
		
		return data.m_AudioSource;
	}
		
	//------------------------------------------------------------------------------------------------
	protected void SetSeed(int seed)
	{
		SCR_BuildingDestructionData data = GetData();
		if (!data)
			return;
		
		data.m_RandomGenerator.SetSeed(seed);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when damage state is changed
	protected override void OnDamageStateChanged(EDamageState state)
	{
		super.OnDamageStateChanged(state);
		
		if (IsProxy())
			return;
		
		// Already destroyed
		if (m_bDestroyed)
			return;
		
		// Called only on the server
		if (state == EDamageState.DESTROYED)
		{
			Math.Randomize(-1);
			int seed = Math.RandomInt(int.MIN, int.MAX);
			StoreNavmeshData();
			RPC_GoToDestroyedState(seed);
			Rpc(RPC_GoToDestroyedState, seed);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Used to do runtime synchronization of state
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_GoToDestroyedState(int seed)
	{
		SetSeed(seed);
		DestroyInteriorInit(false);
		CalculateAndStoreVolume();
		SpawnEffects(0, GetOwner(), false);
		GetGame().GetCallqueue().CallLater(GoToDestroyedState, 1000 * GetDelay(), param1: false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Handles destruction of interior by gathering objects using AABB (will probably use OBB) and deleting them according to their type
	protected void DestroyInteriorInit(bool immediate)
	{
		SCR_BuildingDestructionData data = GetData();
		if (!data)
			return;
		
		//exclude spawned effects
		IEntity owner = GetOwner();
		BaseWorld world = owner.GetWorld();
		vector mins, maxs;
		
		owner.GetWorldBounds(mins, maxs);
		owner.GetTransform(data.m_vStartMatrix);
		
		data.m_aQueriedEntities = {};
		
		// Caches objects inside
		world.QueryEntitiesByAABB(mins, maxs, AddEntityCallback);
	}
	
	//------------------------------------------------------------------------------------------------
	// Uses data queried by DestroyInteriorInit method
	protected void DestroyInterior(bool immediate)
	{
		SCR_BuildingDestructionData data = GetData();
		if (!data)
			return;
		
		int count = data.m_aQueriedEntities.Count();
		int i = count - 1;
		SCR_DestructibleEntity destructibleEntity;
		SCR_DestructionBaseComponent destructionComponent;
		vector hitPosDirNorm[3];
		RplComponent rplComponent;
		IEntity childEntity;
		array<IEntity> handledEntities = {};
		while (count > 0)
		{
			if (i < 0)
				i = count - 1;
			
			if (!data.m_aQueriedEntities[i])
			{
				data.m_aQueriedEntities.Remove(i);
				i--;
				count--;
				continue;
			}
			
			//Has children, will be handled later
			childEntity = data.m_aQueriedEntities[i].GetChildren();
			if (childEntity)
			{
				// Child entity was not queried for some reason && has not been handled yet, add it to the list of queried entities
				if (!data.m_aQueriedEntities.Contains(childEntity) && !handledEntities.Contains(childEntity))
				{
					data.m_aQueriedEntities.Insert(childEntity);
					count++;
				}
				
				// If child was already handled, we can consider this entity a non-parent
				if (!handledEntities.Contains(childEntity))
				{
					i--;
					continue;
				}
			}
			
			// Following order of operations is crucial, so we don't end up with null pointers at some point!
			
			// Any entity that reaches this point is perceived as handled
			handledEntities.Insert(data.m_aQueriedEntities[i]);
			
			// Ignore entities outside the building
			data.m_iChecksThisFrame++;
			if (data.m_iChecksThisFrame >= MAX_CHECKS_PER_FRAME)
			{
				data.m_iChecksThisFrame = 0;
				// Callqueue used here, because it shouldn't rely on running EOnFrame
				GetGame().GetCallqueue().CallLater(DestroyInterior, param1: immediate);
				return;
			}
			
			// Interior check for the object
			// Disabled for now, this check now happens under ground, so it's pointless here
			/*if (!IsInside(data.m_aQueriedEntities[i]))
			{
				data.m_aQueriedEntities.Remove(i);
				i--;
				count--;
				
				continue;
			}*/
			
			destructibleEntity = SCR_DestructibleEntity.Cast(data.m_aQueriedEntities[i]);
			destructionComponent = SCR_DestructionBaseComponent.Cast(data.m_aQueriedEntities[i].FindComponent(SCR_DestructionBaseComponent));
			
			// Non-destructible object, just delete it later, so destruction happens on frame
			if (!destructibleEntity && !destructionComponent)
			{
				rplComponent = RplComponent.Cast(data.m_aQueriedEntities[i].FindComponent(RplComponent));
				// Don't delete replicated objects for now TODO: Solve this properly in the future
				if (!rplComponent)
					delete data.m_aQueriedEntities[i];
				/*if (rplComponent)
					RplComponent.DeleteRplEntity(data.m_aQueriedEntities[i], false);
				else
					delete data.m_aQueriedEntities[i];*/
			}
			
			// Is destructible entity
			if (destructibleEntity)
				destructibleEntity.HandleDamage(EDamageType.TRUE, destructibleEntity.GetCurrentHealth() * 11, hitPosDirNorm);
			
			// Uses destruction component
			if (destructionComponent)
				destructionComponent.DeleteDestructible();
			
			data.m_aQueriedEntities.Remove(i);
			i--;
			count--;
		}
		
		FinishDestruction();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Last method to be called after destruction happens, data is cleared
	protected void FinishDestruction()
	{
		SCR_BuildingDestructionData data = GetData();
		if (!data)
			return;
		
		//data.m_CameraShake.Stop();
		data.m_CameraShake.SetParams(0.15, 0.15, 0.01, 0.3, 0.24);
		data.m_CameraShake = null;
		data.m_aQueriedEntities = null;
		FreeData();
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool PerformTrace(notnull TraceParam param, vector start, vector direction, notnull BaseWorld world, float lengthMultiplier = 1)
	{
		param.Start = start - direction * lengthMultiplier;
		param.End = start + direction * lengthMultiplier;
		world.TraceMove(param, TraceFilter);
		
		return param.TraceEnt != null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Checks whether or not an entity is inside of the building, using a trace in each world axis
	protected bool IsInside(notnull IEntity entity)
	{
		IEntity owner = GetOwner();
		BaseWorld world = owner.GetWorld();
		vector start = entity.GetOrigin();
		
		TraceParam param = new TraceParam();
		param.Flags = TraceFlags.ENTS;
		param.LayerMask = EPhysicsLayerDefs.Projectile;
		param.Include = owner; // Include only the building for performance reasons
		
		bool result;
		for (int i = 0; i < 3; i++)
		{
			float lengthMultiplier = 1;
			if (i == 1)
				lengthMultiplier = 100; // Vertical traces can and must be long to detect roof, where there is no floor, also they are internally optimized
			
			result = PerformTrace(param, start, TRACE_DIRECTIONS[i], world, lengthMultiplier);
			
			if (result)
				return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Filters out unwanted entities
	protected bool TraceFilter(notnull IEntity e, vector start = "0 0 0", vector dir = "0 0 0")
	{
		return e == GetOwner();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Used by Query in DestroyInterior
	protected bool AddEntityCallback(IEntity e)
	{
		SCR_BuildingDestructionData data = GetData();
		if (!data)
			return true;
		
		if (e.FindComponent(SCR_DestructibleBuildingComponent))
			return true;
		
		IEntity owner = GetOwner();
		IEntity entityParent = e.GetParent();
		
		// Exclude the owner && children of other objects
		if (e == owner || (entityParent && entityParent != owner))
			return true;
		
		// Exclude entities in exclude list
		// Disabled for now, because it might not be necessary
		// Keeping it for later if we decide to use it in some other way than originally planned
		/*if (data.m_aExcludeList.Contains(e))
			return true;*/
		
		vector hitPosDirNorm[3];
		
		// Exclude chimera character, vehicles
		if (ChimeraCharacter.Cast(e) || Vehicle.Cast(e))
		{
			// The character is outside the building
			if (!IsInside(e))
				return true;
			
			DamageManagerComponent damageManager = DamageManagerComponent.Cast(e.FindComponent(DamageManagerComponent));
			if (damageManager)
				damageManager.HandleDamage(EDamageType.TRUE, 100000, hitPosDirNorm, e, null, null, null, -1, -1);
			
			return true;
		}
		
		// All children of chimera character
		IEntity mainParent = SCR_EntityHelper.GetMainParent(e);
		if (ChimeraCharacter.Cast(mainParent) || Vehicle.Cast(mainParent))
			return true;
		
		if (SCR_EntityHelper.GetMainParent(e) != owner)
			owner.AddChild(e, -1, EAddChildFlags.AUTO_TRANSFORM | EAddChildFlags.RECALC_LOCAL_TRANSFORM);
		
		data.m_aQueriedEntities.Insert(e);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void GoToDestroyedStateLoad()
	{
		StoreNavmeshData();
		GoToDestroyedState(true);
	}
	
	//------------------------------------------------------------------------------------------------
	void CalculateAndStoreVolume()
	{
		SCR_BuildingDestructionData data = GetData();
		if (!data)
			return;
		
		vector mins, maxs;
		GetOwner().GetBounds(mins, maxs);
		
		float x = Math.AbsFloat(mins[0]) + maxs[0];
		float y = Math.AbsFloat(mins[1]) + maxs[1];
		float z = Math.AbsFloat(mins[2]) + maxs[2];
		
		float buildingVolume = x * y * z;
		
		data.m_fBuildingVolume = buildingVolume;
		
		data.m_fSizeMultiplier = data.m_fBuildingVolume / BUILDING_SIZE; // BUILDING_SIZE constant is value for the average building size
	}
	
	//------------------------------------------------------------------------------------------------
	//! Destroys interior
	//! Starts position lerping (Enables frame, activates the entity)
	//! Or moves the building to target position immediately if it's JIP
	protected void GoToDestroyedState(bool immediate)
	{
		SCR_BuildingDestructionData data = GetData();
		if (!data)
			return;
		
		m_bDestroyed = true;
		GetGame().GetBuildingDestructionManager().RegisterDestroyedBuilding(this);
		
		IEntity owner = GetOwner();
		
		vector mins, maxs;
		owner.GetBounds(mins, maxs);
		
		maxs[0] = 0;
		maxs[2] = 0;
		
		vector sinkVector = GetSinkVector();
		if (sinkVector == vector.Zero)
			sinkVector = -maxs;
		
		data.m_vTargetOrigin = owner.GetOrigin() + sinkVector;
		data.m_vStartAngles = owner.GetAngles();
		
		StaticModelEntity.Cast(owner).DestroyOccluders();
		
		// Don't animate, JIP happened
		if (immediate)
		{
			DestroyInteriorInit(immediate);
			FinishLerp(owner, immediate);
		}
		else // Animate sinking, play particles, sounds etc...
		{
			owner.GetPhysics().SetResponseIndex(NO_COLLISION_RESPONSE_INDEX);
			data.m_CameraShake.SetParams(0.15, 0.15, 0.01, 400, 0.24);
			data.m_CameraShake.SetCurve(GetCameraShakeCurve());
			data.m_CameraShake.SetStartOrigin(data.m_vStartMatrix[3]);
			data.m_CameraShake.SetSizeMultiplier(data.m_fSizeMultiplier);
			SCR_CameraShakeManagerComponent.AddCameraShake(data.m_CameraShake);
			SetEventMask(owner, EntityEvent.FRAME);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called in Frame (while building is sinking)
	//! Spawns effects which are supposed to spawn at this time
	//! Immediate destruction = from JIP f. e., only spawn effects that remain after destruction - like ruins prefabs
	protected void SpawnEffects(float percentDone, IEntity owner, bool immediateDestruction)
	{
		SCR_BuildingDestructionData data = GetData();
		if (!data)
			return;
		
		if (!data.m_aExcludeList)
			data.m_aExcludeList = {};
		
		SCR_HitInfo hitInfo = new SCR_HitInfo();
		hitInfo.m_DamageType = EDamageType.KINETIC; // Todo properly store damage type
		
		array<ref SCR_TimedEffect> effects = GetEffects();
		SCR_TimedEffect currentEffect;
		for (int i = effects.Count() - 1; i >= 0; i--)
		{
			if (data.m_aExecutedEffectIndices && data.m_aExecutedEffectIndices.Contains(i))
				continue;
			
			currentEffect = effects[i]; // Store it, because each effects[i] is effects.Get(i) call internally
			
			if (immediateDestruction && !currentEffect.m_bPersistent)
				continue; // Skip because destruction happened immediately & effect isn't persistent
			
			if (currentEffect.m_fSpawnTime <= percentDone)
			{
				currentEffect.ExecuteEffect(owner, hitInfo, data);
				
				// Create the set if it doesn't exist yet
				if (!data.m_aExecutedEffectIndices)
				{
					data.m_aExecutedEffectIndices = new set<int>();
					// Max size of the set is known beforehand, because m_aEffects is in prefab data
					data.m_aExecutedEffectIndices.Reserve(effects.Count());
				}
				
				data.m_aExecutedEffectIndices.Insert(i);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Handles the end of building position lerp
	//! Disables frame, deactivates the entity
	//! Hides the mesh
	//! Plays final effects
	protected void FinishLerp(IEntity owner, bool immediate)
	{
		owner.SetObject(null, ""); // Hide the building
		ClearEventMask(owner, EntityEvent.FRAME);
		
		SpawnEffects(1, owner, immediate); // Ensure all effects get played
		
		SCR_BuildingDestructionData data = GetData();
		if (!data)
			return;
		
		owner.SetOrigin(data.m_vTargetOrigin);
		owner.Update();
		RegenerateNavmesh();
		DestroyInterior(immediate);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Regenerates navmesh using previously stored data
	protected void RegenerateNavmesh()
	{
		SCR_BuildingDestructionData data = GetData();
		if (!data)
			return;
		
		SCR_AIWorld aiWorld = SCR_AIWorld.Cast(GetGame().GetAIWorld());
		if (!aiWorld)
			return;
		
		aiWorld.RequestNavmeshRebuildAreas(data.m_aNavmeshAreas, data.m_aRedoRoads);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Stores navmesh data to regenerate navmesh later
	protected void StoreNavmeshData()
	{
		if (Replication.IsClient())
			return;
		
		SCR_BuildingDestructionData data = GetData();
		if (!data)
			return;
		
		SCR_AIWorld aiWorld = SCR_AIWorld.Cast(GetGame().GetAIWorld());
		if (!aiWorld)
			return;
		
		data.m_aNavmeshAreas = {};
		data.m_aRedoRoads = {};
		aiWorld.GetNavmeshRebuildAreas(GetOwner(), data.m_aNavmeshAreas, data.m_aRedoRoads); // Get area with current phase
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ClampVector(inout vector currentOrigin, vector startOrigin, vector endOrigin)
	{
		bool targetSmaller;
		for (int i = 0; i < 3; i++)
		{
			targetSmaller = endOrigin[i] < startOrigin[i];
			if (targetSmaller)
			{
				if (currentOrigin[i] < endOrigin[i])
					currentOrigin[i] = endOrigin[i];
			}
			else
			{
				if (currentOrigin[i] > endOrigin[i])
					currentOrigin[i] = endOrigin[i];
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void LerpRotation(IEntity owner, float timeSlice)
	{
		SCR_BuildingDestructionData data = GetData();
		if (!data)
			return;
		
		// When it's time to reset the target rotation
		if (data.m_iRotationStart + data.m_iRotationTime < owner.GetWorld().GetWorldTime() && data.m_iRotatedTimes < GetMaxRotations())
		{
			// Generate next rotation time offset
			float rotationTimeRandomizer = GetRotationTimeRandomizer() * 0.01; // * 0.01 to 0-1 range
			data.m_iRotationTime = GetRotationTime() * (1 + data.m_RandomGenerator.RandFloatXY(0, rotationTimeRandomizer)) * 1000; // * 1000 = to ms
			
			// Slow down the sinking to make it seem like it crashed into something
			data.m_fSpeedMultiplier *= 0.05;
			
			// Only call OnSlowDown if it's not initial rotation setting
			if (data.m_iRotatedTimes != 0)
				OnSlowDown();
			
			bool allowRotationX = GetAllowRotationX();
			bool allowRotationY = GetAllowRotationY();
			bool allowRotationZ = GetAllowRotationZ();
			
			// Generate new random angles
			vector newTargetAngles = Vector(data.m_RandomGenerator.RandFloatXY(5, 20) * allowRotationX * data.m_iRotationMultiplier + data.m_vStartAngles[0]
											, data.m_RandomGenerator.RandFloatXY(5, 20) * allowRotationY * data.m_iRotationMultiplier + data.m_vStartAngles[1]
											, data.m_RandomGenerator.RandFloatXY(5, 20) * allowRotationZ * data.m_iRotationMultiplier + data.m_vStartAngles[2]);
			
			// This ensures the rotation will always be going to the opposite side of the previous one
			data.m_iRotationMultiplier *= -1;
			
			// Reset the speed multiplier
			data.m_fRotationSpeedMultiplier = 0.5;
			
			// Save the new target angles
			data.m_vTargetAngles = newTargetAngles;
			
			// Cache world
			BaseWorld world = owner.GetWorld();
			
			int pauseTime = data.m_RandomGenerator.RandIntInclusive(0, 500);
			
			// Save the current timestamp as the rotation start
			// Add the pause time to it
			data.m_iRotationStart = world.GetWorldTime() + pauseTime;
			
			// Set pause time to stop the building for a while
			data.m_iPauseTime = world.GetWorldTime() + pauseTime;
			
			data.m_iRotatedTimes++;
		}
		
		// This is the actual lerp
		data.m_fRotationSpeedMultiplier += timeSlice;
		//vector newAngles = vector.Lerp(owner.GetAngles(), data.m_vTargetAngles, timeSlice * Math.Pow(data.m_fRotationSpeedMultiplier, 3));
		vector newAngles = LerpAngles(data.m_vStartAngles, owner.GetAngles(), data.m_vTargetAngles, GetRotationSpeed(), timeSlice, data);
		owner.SetAngles(newAngles);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void PlaySlowDownSound()
	{
		SCR_SoundManagerEntity soundManagerEntity = GetGame().GetSoundManagerEntity();
		if (!soundManagerEntity)
			return;
		
		SCR_AudioSourceConfiguration audioSourceConfiguration = GetSlowDownAudioSourceConfiguration();
		if (!audioSourceConfiguration || !audioSourceConfiguration.IsValid())
			return;
		
		SCR_AudioSource audioSource = soundManagerEntity.CreateAudioSource(GetOwner(), audioSourceConfiguration);
		if (!audioSource)
			return;
		
		SetAudioSource(audioSource);
		soundManagerEntity.PlayAudioSource(audioSource, GetData().m_vStartMatrix);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSlowDown()
	{
		PlaySlowDownSound();
	}
	
	//------------------------------------------------------------------------------------------------
	protected vector LerpAngles(vector start, vector current, vector target, float rotationSpeed, float timeSlice, notnull SCR_BuildingDestructionData data)
	{
		if (target == vector.Zero)
			return vector.Zero;
		
		float percent = timeSlice / (data.m_iRotationTime * 0.001);
		vector diff = target - start;
		vector nextRotation = current + percent * diff * rotationSpeed * data.m_fRotationSpeedMultiplier;
		
		ClampVector(nextRotation, start, target);
		return nextRotation;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Handles position lerping
	//! Handles calling SpawnEffects, calculates percentDone parameter
	protected void LerpPosition(IEntity owner, float timeSlice)
	{
		SCR_BuildingDestructionData data = GetData();
		if (!data)
			return;
		
		vector currentOrigin = owner.GetOrigin();
		vector direction = (data.m_vTargetOrigin - currentOrigin).Normalized();
		
		data.m_fSpeedMultiplier += (GetSpeedGradualMultiplier() * timeSlice) * 0.01;
		vector mat[4];
		owner.GetTransform(mat);
		vector newOrigin = currentOrigin + direction * GetSpeed() * timeSlice * data.m_fSpeedMultiplier;
		
		ClampVector(newOrigin, data.m_vStartMatrix[3], data.m_vTargetOrigin);
		
		float difY = data.m_vTargetOrigin[1] - data.m_vStartMatrix[3][1];
		float curY = newOrigin[1] - data.m_vStartMatrix[3][1];
		float percentDone = curY/difY;
		SpawnEffects(percentDone, owner, false);
		
		if (float.AlmostEqual(newOrigin[0], data.m_vTargetOrigin[0]) && float.AlmostEqual(newOrigin[1], data.m_vTargetOrigin[1]) && float.AlmostEqual(newOrigin[2], data.m_vTargetOrigin[2]))
			FinishLerp(owner, false);
		
		owner.SetOrigin(newOrigin);
		owner.Update();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Serializes state over network
	protected override event bool OnRplSave(ScriptBitWriter writer)
	{
		super.OnRplSave(writer);
		
		writer.WriteBool(m_bDestroyed);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Loads serialized state on client
	protected override event bool OnRplLoad(ScriptBitReader reader)
	{
		super.OnRplLoad(reader);
		
		reader.ReadBool(m_bDestroyed);
		
		if (m_bDestroyed)
		{
			// Need to remove it from callqueue in case it was queued by OnDamageStateChanged
			GetGame().GetCallqueue().Remove(GoToDestroyedState);
			GoToDestroyedState(true);
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Contact to deal damage
	override bool OnContact(IEntity owner, IEntity other, Contact contact)
	{
		if (IsProxy())
			return false;
		
		if (GetHealth() <= 0)
			return false;
		
		if (other && other.IsInherited(SCR_DebrisSmallEntity)) // Ignore impacts from debris
 			return false;
		
		// Get the physics of the dynamic object (if owner is static, then we use the other object instead)
		Physics physics = contact.Physics1;
		int responseIndex = physics.GetResponseIndex();
		float ownerMass = physics.GetMass();
		float otherMass;
		if (!physics.IsDynamic())
		{
			physics = contact.Physics2;
			if (!physics)
				return false; // This only happens with ragdolls, other objects do have physics here, as collision only happens between physical objects
			
			otherMass = physics.GetMass();
		}
		else
		{
			Physics otherPhysics = other.GetPhysics();
			if (!otherPhysics)
				return false; // This only happens with ragdolls, other objects do have physics here, as collision only happens between physical objects
			
			otherMass = otherPhysics.GetMass();
		}
		
		float momentum = CalculateMomentum(contact, ownerMass, otherMass);
		
		vector outMat[3];
		vector relVel = contact.VelocityBefore2 - contact.VelocityBefore1;
		outMat[0] = contact.Position; // Hit position
		outMat[1] = relVel.Normalized(); // Hit direction
		outMat[2] = contact.Normal; // Hit normal
		
		float damage = momentum * 0.05; // Todo replace with attribute
		
		// Send damage to damage handling
		HandleDamage(EDamageType.COLLISION, damage, outMat, GetOwner(), null, Instigator.CreateInstigator(other), null, -1, -1);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Handles per-frame operations, only enabled while the building is sinking
	protected override void OnFrame(IEntity owner, float timeSlice)
	{
		LerpPosition(owner, timeSlice);
		SCR_BuildingDestructionData data = GetData();
		
		if (!data || data.m_iRotatedTimes <= GetMaxRotations())
			LerpRotation(owner, timeSlice);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.CONTACT);
	}
	
#ifdef BUILDING_DESTRUCTION_SAVING
	//------------------------------------------------------------------------------------------------
	override event void _WB_OnDelete(IEntity owner, IEntitySource src)
	{
		SCR_BuildingDestructionManagerComponent.UnregisterBuildingId(this);
	}
	
	//------------------------------------------------------------------------------------------------
	override event void _WB_OnInit(IEntity owner, inout vector mat[4], IEntitySource src)
	{
		int id = GetBuildingId();
		if (id != 0 && !SCR_BuildingDestructionManagerComponent.IsIdTaken(id, this))
			return;
		
		id = SCR_BuildingDestructionManagerComponent.GetNewId();
		src.Set("m_iId", id);
		SCR_BuildingDestructionManagerComponent.RegisterBuildingId(this, id);
	}
#endif
	//------------------------------------------------------------------------------------------------
	int GetBuildingId()
	{
		SCR_DestructibleBuildingEntity ent = SCR_DestructibleBuildingEntity.Cast(GetOwner());
		if (ent)
			return ent.GetBuildingId();
		
		return 0;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_DestructibleBuildingComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
#ifdef BUILDING_DESTRUCTION_SAVING
		if (!SCR_DestructibleBuildingEntity.Cast(ent))
			Print("SCR_DestructibleBuildingComponent not attached to SCR_DestructibleBuildingEntity!", LogLevel.WARNING);
		else
		{
			int id = GetBuildingId();
			if (id != 0)
				SCR_BuildingDestructionManagerComponent.RegisterBuildingId(this, id);
		}
#endif
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_DestructibleBuildingComponent()
	{
#ifdef BUILDING_DESTRUCTION_SAVING
		SCR_BuildingDestructionManagerComponent.UnregisterBuildingId(this);
#endif
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns true if local instance is proxy (not the authority)
	protected bool IsProxy()
	{
		RplComponent rplComponent = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		return rplComponent && rplComponent.IsProxy();
	}
};
