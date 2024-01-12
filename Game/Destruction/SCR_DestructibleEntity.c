//------------------------------------------------------------------------------------------------
[EntityEditorProps(category: "GameScripted/Destructibles", visible: false, dynamicBox: true)]
class SCR_DestructibleEntityClass: DestructibleEntityClass
{
	[Attribute("0.05", UIWidgets.Slider, "Contact momentum to damage multiplier", "0.01 10 0.01", category: "Destruction Setup")]
	float m_fMomentumToDamageScale;
	
	[Attribute("", UIWidgets.Object, "List of objects (particles, debris, etc) to spawn on destruction of the object", category: "Destruction FX")]
	ref array<ref SCR_BaseSpawnable> m_aDestroySpawnObjects;
	
	[Attribute("0", uiwidget: UIWidgets.ComboBox, "Type of material for destruction sound", "", ParamEnumArray.FromEnum(SCR_EMaterialSoundTypeBreak))]
	SCR_EMaterialSoundTypeBreak m_eMaterialSoundType;
};

//------------------------------------------------------------------------------------------------
class SCR_DestructionData : BaseDestructibleData
{
	const int MAX_PHASES_BIT_SIZE = 4; // Max 16 phases are supported now = 4 bits
	float m_fHitDamage;
	EDamageType m_eDamageType;
	vector m_vHitPosition;
	vector m_vHitDirection; //Normalized
	vector m_vHitNormal; //Normalized
	bool m_bTotalDestruction;
	int m_iPreviousPhase;
	
	//------------------------------------------------------------------------------------------------
	void SaveNormalizedVector(ScriptBitWriter w, vector norm)
	{
		w.WriteHalf(norm[0]);
		w.WriteHalf(norm[1]);
		w.WriteHalf(norm[2]);
	}
	
	//------------------------------------------------------------------------------------------------
	void LoadNormalizedVector(ScriptBitReader r, out vector norm)
	{
		float x;
		float y;
		float z;
		
		r.ReadHalf(x);
		r.ReadHalf(y);
		r.ReadHalf(z);
		
		norm = {x,y,z};
	}

	//------------------------------------------------------------------------------------------------
	bool Save(ScriptBitWriter w)
	{
		SaveNormalizedVector(w, m_vHitDirection);
		w.WriteFloat(m_fHitDamage);
		SaveNormalizedVector(w, m_vHitNormal);
		w.WriteInt(m_eDamageType);
		
		w.WriteVector(m_vHitPosition); // Maybe we should make this relative to the entity, so it's smaller numbers and we can also write it as half
		w.WriteBool(m_bTotalDestruction);
		w.WriteInt(m_iPreviousPhase);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	bool Load(ScriptBitReader r)
	{
		LoadNormalizedVector(r, m_vHitDirection);
		r.ReadFloat(m_fHitDamage);
		LoadNormalizedVector(r, m_vHitNormal);
		r.ReadInt(m_eDamageType);
		r.ReadVector(m_vHitPosition);
		r.ReadBool(m_bTotalDestruction);
		r.ReadInt(m_iPreviousPhase);
		
		return true;
	}
}

//------------------------------------------------------------------------------------------------
class SCR_DestructibleEntity: DestructibleEntity
{
	const float SIMULATION_IMPRECISION_MULTIPLIER = 1.1;
	protected static const int MIN_MOMENTUM_RESPONSE_INDEX = 1;
	protected static const int MAX_MOMENTUM_RESPONSE_INDEX = 5;
	protected static const int MIN_DESTRUCTION_RESPONSE_INDEX = 6;
	protected static const string DAMAGE_PHASE_SIGNAL_NAME = "DamagePhase";
	static const int MAX_DESTRUCTION_RESPONSE_INDEX = 10;
	static const string MAX_DESTRUCTION_RESPONSE_INDEX_NAME = "HugeDestructible";
	static const int TOTAL_DESTRUCTION_MAX_HEALTH_MULTIPLIER = 10;
	
	//------------------------------------------------------------------------------------------------
	float GetDamageMultiplier(EDamageType type)
	{
		SCR_DestructibleEntityClass prefabData = SCR_DestructibleEntityClass.Cast(GetPrefabData());
		if (prefabData)
			return prefabData.GetDamageMultiplier(type);
		
		return 0;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetDamageReduction()
	{
		SCR_DestructibleEntityClass prefabData = SCR_DestructibleEntityClass.Cast(GetPrefabData());
		if (prefabData)
			return prefabData.GetDamageReduction();
		
		return 0;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetDamageThreshold()
	{
		DestructibleEntityClass prefabData = DestructibleEntityClass.Cast(GetPrefabData());
		if (prefabData)
			return prefabData.GetDamageThreshold();
		
		return 0;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetMaxHealth()
	{
		DestructibleEntityClass prefabData = DestructibleEntityClass.Cast(GetPrefabData());
		if (!prefabData)
			return 0;
		
		return prefabData.GetMaxHealth();
	}
	
	//------------------------------------------------------------------------------------------------
	void RegenerateNavmeshDelayed()
	{
		if (Replication.IsClient())
			return;
		
		SCR_AIWorld aiWorld = SCR_AIWorld.Cast(GetGame().GetAIWorld());
		
		if (!aiWorld)
			return;
		
		array<ref Tuple2<vector, vector>> areas = {};
		array<bool> redoAreas = {};
		aiWorld.GetNavmeshRebuildAreas(this, areas, redoAreas); // Get area with current phase
		GetGame().GetCallqueue().CallLater(aiWorld.RequestNavmeshRebuildAreas, 1000, false, areas, redoAreas); // Rebuild later with new phase/when this object is destroyed
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDamage(int previousState, int newState, EDamageType type, float damageTaken, float currentHealth, inout vector hitTransform[3], ScriptBitWriter frameData)
	{
		UpdateResponseIndex(currentHealth);
		if (!frameData)
			return;
		
		// Handling double damage in one frame:
		// FrameData exists from previous damage - the object was damaged twice in a frame, the second damage didn't actually change the phase,
		// therefore not writing it into the destructionData.
		if (previousState == newState)
			return;
		
		float maxHealth = GetMaxHealth();
		
		SCR_DestructionData destructionData = new SCR_DestructionData();
		destructionData.m_vHitPosition = hitTransform[0];
		destructionData.m_vHitDirection = hitTransform[1].Normalized();
		destructionData.m_vHitNormal = hitTransform[2].Normalized();
		destructionData.m_fHitDamage = damageTaken;
		destructionData.m_eDamageType = type;
		destructionData.m_bTotalDestruction = damageTaken > maxHealth * TOTAL_DESTRUCTION_MAX_HEALTH_MULTIPLIER;
		destructionData.m_iPreviousPhase = previousState;
		destructionData.Save(frameData);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the model of the object
	void SetModel(ResourceName model)
	{
		Resource resource = Resource.Load(model);
		VObject asset;
		if (resource)
		{
			BaseResourceObject resourceObject = resource.GetResource();
			if (resourceObject)
				asset = resourceObject.ToVObject();
			else
				asset = null;
		}
		else
			asset = null;
		
		SetObject(asset, "");
		Update();
		
		Physics phys = GetPhysics();
		if (!phys)
			return;
		
		// Update physics geometries after mesh change
		if (!phys.UpdateGeometries())
			phys.Destroy(); // No geoms found, destroy physics
	}
	
	//------------------------------------------------------------------------------------------------
	//! Spawns objects that are meant to be created when the object is destroyed (particles, debris, etc)
	protected void SpawnPhaseObjects(SCR_BaseDestructionPhase phase, SCR_DestructionData destructionData)
	{
		Physics physics = GetPhysics();
		
		array<ref SCR_BaseSpawnable> destroySpawnObjects = phase.m_aPhaseDestroySpawnObjects;
		int numSpawnOnDestroy = destroySpawnObjects.Count();
		for (int i = 0; i < numSpawnOnDestroy; i++)
		{
			SCR_BaseSpawnable spawnObject = destroySpawnObjects[i];
			SCR_DestructionHitInfo hitInfo = SCR_DestructionHitInfo.FromDestructionData(destructionData);
			spawnObject.Spawn(this, physics, hitInfo);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Only call from OnStateChanged, otherwise you have HUGE desync
	protected void GoToDamagePhase(int damagePhaseIndex, int previousDamagePhaseIndex, SCR_DestructionData destructionData, bool streamed)
	{
		DestructibleEntityClass prefabData = DestructibleEntityClass.Cast(GetPrefabData());
		if (!prefabData)
			return;
		
		SCR_BaseDestructionPhase phase = SCR_BaseDestructionPhase.Cast(prefabData.GetDestructionPhase(damagePhaseIndex));
		if (!phase)
			return; // Should never happen, unless some memory issue happens
		
		RegenerateNavmeshDelayed();
		if (streamed)
		{
			// When streamed, we don't care about sounds or effects, we just change the model and quit
			SetModel(phase.m_sPhaseModel);
			SetDamagePhaseSignal(damagePhaseIndex);
			return;
		}
		
		bool destroyAtNoHealth;
		prefabData.GetPrefab().Get("DestroyAtNoHealth", destroyAtNoHealth);
		PlaySound(damagePhaseIndex);
		SetDamagePhaseSignal(damagePhaseIndex);
		SetModel(phase.m_sPhaseModel);
		
		if (damagePhaseIndex == prefabData.GetNumDestructionPhases() - 1 && destroyAtNoHealth) // is last phase and gets deleted
		{
			SetModel(phase.m_sPhaseModel);
			SpawnPhaseObjects(phase, destructionData);
			return;
		}
		
		SCR_BaseDestructionPhase previousPhase = SCR_BaseDestructionPhase.Cast(prefabData.GetDestructionPhase(previousDamagePhaseIndex));
		if (!previousPhase)
			return; // Should never happen, unless some memory issue happens
		
		SpawnPhaseObjects(previousPhase, destructionData);
		
		//GetPhaseData(destructibleState);
		//Change model to new phase
		//Spawn previous phases effects
	}
	
	//------------------------------------------------------------------------------------------------
	float GetDestructibleSize()
	{
		vector mins, maxs
		GetBounds(mins, maxs);
		return (maxs[0] - mins[0]) * (maxs[1] - mins[1]);
	}
	
	//------------------------------------------------------------------------------------------------
	void PlaySound(int damagePhaseIndex)
	{
		SCR_SoundManagerEntity soundManagerEntity = GetGame().GetSoundManagerEntity();
		if (!soundManagerEntity)
			return;
		
		SCR_DestructibleEntityClass prefabData = SCR_DestructibleEntityClass.Cast(GetPrefabData());
		if (!prefabData || prefabData.m_eMaterialSoundType == 0)
			return;
		
		SCR_MPDestructionManager destructionManager = SCR_MPDestructionManager.GetInstance();
		if (!destructionManager)
			return;
		
		SCR_AudioSourceConfiguration audioSourceConfiguration = destructionManager.GetAudioSourceConfiguration();
		if (!audioSourceConfiguration)
			return;
		
		// Get AudioSource
		audioSourceConfiguration.m_sSoundEventName = SCR_SoundEvent.SOUND_MPD_ + typename.EnumToString(SCR_EMaterialSoundTypeBreak, prefabData.m_eMaterialSoundType);
		
		SCR_AudioSource audioSource = soundManagerEntity.CreateAudioSource(this, audioSourceConfiguration);
		if (!audioSource)
			return;
		
		// Set signals
		audioSource.SetSignalValue(SCR_AudioSource.PHASES_TO_DESTROYED_PHASE_SIGNAL_NAME, prefabData.GetNumDestructionPhases() - damagePhaseIndex - 1);
		audioSource.SetSignalValue(SCR_AudioSource.ENTITY_SIZE_SIGNAL_NAME, GetDestructibleSize());
		
		// Set position
		vector mat[4];
		
		// Get center of bounding box
		vector mins;
		vector maxs;
		GetWorldBounds(mins, maxs);
		mat[3] = vector.Lerp(mins, maxs, 0.5);
			
		// Play sound
		soundManagerEntity.PlayAudioSource(audioSource, mat);
	}
	
	//------------------------------------------------------------------------------------------------
	// Update DamagePhase signal on entity that has SignalsManagerComponent
	// Used for triggering looped sounds
	protected void SetDamagePhaseSignal(int damagePhaseIndex = 0)
	{
		SignalsManagerComponent signalsManagerComponent = SignalsManagerComponent.Cast(FindComponent(SignalsManagerComponent));
		if (!signalsManagerComponent)
			return;
		
		// Pristine = 0, Destoyed > 0
		signalsManagerComponent.SetSignalValue(signalsManagerComponent.AddOrFindSignal(DAMAGE_PHASE_SIGNAL_NAME), damagePhaseIndex);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnStateChanged(int destructibleState, ScriptBitReader frameData, bool JIP)
	{
		SCR_DestructionData destructionData = new SCR_DestructionData();
		if (frameData)
			destructionData.Load(frameData);
		
		GoToDamagePhase(destructibleState, destructionData.m_iPreviousPhase, destructionData, JIP);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnBeforeDestroyed()
	{
		RegenerateNavmeshDelayed();
	}
	
	//------------------------------------------------------------------------------------------------
	float CalculateMomentum(Contact contact, float ownerMass, float otherMass)
	{
		float DotMultiplier = vector.Dot(contact.VelocityAfter1.Normalized(), contact.VelocityBefore1.Normalized());
		float MomentumBefore = ownerMass * contact.VelocityBefore1.Length() * SIMULATION_IMPRECISION_MULTIPLIER;
		float MomentumAfter = ownerMass * contact.VelocityAfter1.Length() * DotMultiplier;
		float momentumA = Math.AbsFloat(MomentumBefore - MomentumAfter);
		
		DotMultiplier = vector.Dot(contact.VelocityAfter2.Normalized(), contact.VelocityBefore2.Normalized());
		MomentumBefore = otherMass * contact.VelocityBefore2.Length() * SIMULATION_IMPRECISION_MULTIPLIER;
		MomentumAfter = otherMass * contact.VelocityAfter2.Length() * DotMultiplier;
		float momentumB = Math.AbsFloat(MomentumBefore - MomentumAfter);
		return momentumA + momentumB;
	}
	
	//------------------------------------------------------------------------------------------------
	//Called from OnDamage()
	void UpdateResponseIndex(float currentHealth)
	{
		Physics physics = GetPhysics();
		if (!physics)
			return;
		
		int responseIndex = physics.GetResponseIndex();
		if (responseIndex <= MIN_DESTRUCTION_RESPONSE_INDEX)
			return; // Cannot go lower
		
		float healthPercentage = currentHealth / GetMaxHealth();
		int minMaxDiff = MAX_DESTRUCTION_RESPONSE_INDEX - MIN_DESTRUCTION_RESPONSE_INDEX + 1;
		
		responseIndex = Math.ClampInt(MIN_DESTRUCTION_RESPONSE_INDEX - 1 + Math.Ceil(minMaxDiff * healthPercentage), MIN_DESTRUCTION_RESPONSE_INDEX, responseIndex);
		physics.SetResponseIndex(responseIndex);
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsProxy()
	{
		// GetDestructionManager();
		// GetItsRplComponent();
		// Return its Proxy check
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Contact
	override void EOnContact(IEntity owner, IEntity other, Contact contact)
	{
		if (other && other.IsInherited(SCR_DebrisSmallEntity)) // Ignore impacts from debris
 			return;

		SCR_DestructibleEntityClass prefabData = SCR_DestructibleEntityClass.Cast(GetPrefabData());
		if (!prefabData)
			return;
		
		// Get the physics of the dynamic object (if owner is static, then we use the other object instead)
		Physics physics = contact.Physics1;
		
		int responseIndex = physics.GetResponseIndex();
		float ownerMass = physics.GetMass();
		float otherMass;
		int ownerReponseIndex = physics.GetResponseIndex();
		int otherResponseIndex;
		if (!physics.IsDynamic())
		{
			physics = contact.Physics2;
			if (!physics)
				return; // This only happens with ragdolls, other objects do have physics here, as collision only happens between physical objects
			
			otherMass = physics.GetMass();
			otherResponseIndex = physics.GetResponseIndex();
		}
		else
		{
			Physics otherPhysics = other.GetPhysics();
			if (!otherPhysics)
				return; // This only happens with ragdolls, other objects do have physics here, as collision only happens between physical objects
			
			otherMass = otherPhysics.GetMass();
			otherResponseIndex = otherPhysics.GetResponseIndex();
		}
		
		float momentum = CalculateMomentum(contact, ownerMass, otherMass);
		
		// Now get the relative force, which is the impulse divided by the mass of the dynamic object
		float relativeForce = 0;
		if (physics.IsDynamic())
		{
			relativeForce = contact.Impulse / physics.GetMass();
			//We divide the relative force by 20, because for some reason, objects with this response index receive ~20x bigger impulse
			//TODO: investigate @matousvoj1
			if (responseIndex >= MIN_DESTRUCTION_RESPONSE_INDEX && responseIndex <= MAX_DESTRUCTION_RESPONSE_INDEX)
				relativeForce *= 0.05;
		}
		
		/*if (relativeForce < componentData.m_fRelativeContactForceThresholdMinimum) // Below minimum threshold, ignore
			return;*/
		
		vector outMat[3];
		vector relVel = contact.VelocityBefore2 - contact.VelocityBefore1;
		outMat[0] = contact.Position; // Hit position
		outMat[1] = relVel.Normalized(); // Hit direction
		outMat[2] = contact.Normal; // Hit normal
		
		//If vehicle response index is the same or higher -> automatically destroy
		if (otherResponseIndex - MIN_MOMENTUM_RESPONSE_INDEX >= ownerReponseIndex - MIN_DESTRUCTION_RESPONSE_INDEX)
		{
			HandleDamage(EDamageType.TRUE, prefabData.GetMaxHealth(), outMat); //Immediately destroy, otherwise the other object goes right through
		}
		else
		{
			//Vehicle response index is smaller -> just deal damage, vehicle gets stopped
			// Send damage to damage handling
			HandleDamage(EDamageType.COLLISION, momentum * prefabData.m_fMomentumToDamageScale, outMat);
		}

		return;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_DestructibleEntity(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.CONTACT);
	}
};
