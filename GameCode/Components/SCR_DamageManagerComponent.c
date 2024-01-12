class SCR_DamageManagerComponentClass : DamageManagerComponentClass
{
	[Attribute()]
	protected ref SCR_SecondaryExplosions m_SecondaryExplosions;

	[Attribute()]
	protected ref SCR_SecondaryExplosions m_SecondaryFires;

	//------------------------------------------------------------------------------------------------
	SCR_SecondaryExplosions GetSecondaryExplosions()
	{
		return m_SecondaryExplosions;
	}

	//------------------------------------------------------------------------------------------------
	SCR_SecondaryExplosions GetSecondaryFires()
	{
		return m_SecondaryFires;
	}
}

class SCR_DamageManagerComponent : DamageManagerComponent
{
	//------------------------------------------------------------------------------------------------
	/*!
	Get damage manager from given owner
	\param owner Entity to get damage manager from
	\return Damage Manager if any is found
	*/
	static SCR_DamageManagerComponent GetDamageManager(notnull IEntity owner)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(owner);
		if (character)
			return character.GetDamageManager();

		BaseVehicle vehicle = BaseVehicle.Cast(owner);
		if (vehicle)
			return vehicle.GetDamageManager();

		return SCR_DamageManagerComponent.Cast(owner.FindComponent(SCR_DamageManagerComponent));
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get a list of all hitzones of specific hitzone group
	\param hitZoneGroup Hitzone group to get hitzones of
	\param[out] groupHitZones List of group hitzones found
	\return Count of found group hitzones
	*/
	int GetHitZonesOfGroup(EHitZoneGroup hitZoneGroup, out notnull array<HitZone> groupHitZones, bool clearArray = true)
	{
		if (clearArray)
			groupHitZones.Clear();

		array<HitZone> allHitZones = {};
		GetAllHitZones(allHitZones);
		ScriptedHitZone scrHitZone;

		foreach (HitZone hitzone : allHitZones)
		{
			scrHitZone = ScriptedHitZone.Cast(hitzone);
			if (scrHitZone && scrHitZone.GetHitZoneGroup() == hitZoneGroup)
				groupHitZones.Insert(hitzone);
		}

		return groupHitZones.Count();
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get a list of all hitzones of specific hitzone group
	\param hitZoneGroups List of Hitzone groups to get hitzones of
	\param[out] groupHitZones List of group hitzones found
	\return Count of found group hitzones
	*/
	int GetHitZonesOfGroups(notnull array<EHitZoneGroup> hitZoneGroups, out notnull array<HitZone> groupHitZones)
	{
		groupHitZones.Clear();

		if (hitZoneGroups.IsEmpty())
			return 0;

		array<HitZone> allHitZones = {};
		GetAllHitZones(allHitZones);
		ScriptedHitZone scrHitZone;

		foreach (HitZone hitzone : allHitZones)
		{
			scrHitZone = ScriptedHitZone.Cast(hitzone);
			if (scrHitZone && hitZoneGroups.Contains(scrHitZone.GetHitZoneGroup()))
				groupHitZones.Insert(hitzone);
		}

		return groupHitZones.Count();
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get total damage over time of given hitzone group
	\param hitZoneGroup Hitzone group to get damage over time from
	\param damageType Damage type to get damage over time from
	\return Damage over time value
	*/
	float GetGroupDamageOverTime(ECharacterHitZoneGroup hitZoneGroup, EDamageType damageType)
	{
		float totalGroupDamage;
		array<HitZone> groupedHitZones = {};
		GetHitZonesOfGroup(hitZoneGroup, groupedHitZones);

		foreach (HitZone hitzone : groupedHitZones)
		{
			totalGroupDamage += hitzone.GetDamageOverTime(damageType);
		}

		return totalGroupDamage;
	}

	//------------------------------------------------------------------------------------------------
	/*! Neutralize the entity with a specific damage type, registering the killer entity.
	\param instigator Source of the damage
	*/
	void Kill(notnull Instigator instigator)
	{
		IEntity owner = GetOwner();
		if (!owner)
			return;

		HitZone hitZone = GetDefaultHitZone();
		if (!hitZone)
			return;

		vector hitPosDirNorm[3];
		HandleDamage(EDamageType.TRUE, hitZone.GetMaxHealth(), hitPosDirNorm, owner, hitZone, instigator, null, -1, -1);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Damage random physical hitzones up to damage amount
	\param damage Total damage to be applied
	\param type Type of damage
	\param instigator Entity that fired the projectile or otherwise caused the damage
	\param onlyPhysical Whether only physical hitzones should be damaged
	\param outMat [hitPosition, hitDirection, hitNormal]
	\param damageDefault Whether to damage default hitzone as well
	*/
	void DamageRandomHitZones(float damage, EDamageType type, notnull Instigator instigator, bool onlyPhysical = true, vector outMat[3] = {}, bool damageDefault = true)
	{
		array<HitZone> hitZones = {};

		if (onlyPhysical)
			GetPhysicalHitZones(hitZones);
		else
			GetAllHitZones(hitZones);

		if (!damageDefault)
			hitZones.RemoveItem(GetDefaultHitZone());

		while (damage > 0 && !hitZones.IsEmpty())
		{
			HitZone hitZone = hitZones.GetRandomElement();
			if (!hitZone)
			{
				hitZones.RemoveItem(hitZone);
				continue;
			}

			float hitZoneDamage = Math.Min(damage, hitZone.GetHealth() + hitZone.GetDamageReduction());
			damage -= hitZoneDamage;

			if (hitZoneDamage <= 0)
				continue;

			HandleDamage(type, hitZoneDamage, outMat, GetOwner(), hitZone, instigator, null, -1, -1);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Return true if there is damage that can be repaired
	bool CanBeHealed()
	{
		// Any damage to default hitzone
		if (GetDefaultHitZone().GetHealthScaled() < 1)
			return true;

		// Check if has damage over time
		array<EDamageType> damageTypes = {};
		SCR_Enum.GetEnumValues(EDamageType, damageTypes);

		foreach (EDamageType type : damageTypes)
		{
			if (IsDamagedOverTime(type))
				return true;
		}

		// Check if any hitzone is damaged
		array<HitZone> hitZones = {};
		GetAllHitZones(hitZones);
		foreach (HitZone hitZone : hitZones)
		{
			if (hitZone && hitZone.GetHealthScaled() < 1)
				return true;

			// Flammable hitzone may be smoking
			SCR_FlammableHitZone flammableHitZone = SCR_FlammableHitZone.Cast(hitZone);
			if (flammableHitZone && flammableHitZone.GetFireState() != EFireState.NONE)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Fix all the damage
	void FullHeal(bool ignoreHealingDOT = true)
	{
		array<HitZone> hitZones = {};
		GetAllHitZones(hitZones);

		// Extinguish flammable hitzones
		SCR_FlammableHitZone flammableHitZone;
		foreach (HitZone hitZone : hitZones)
		{
			// Put out fire and smoke
			flammableHitZone = SCR_FlammableHitZone.Cast(hitZone);
			if (flammableHitZone)
				flammableHitZone.SetFireRate(0);
		}

		// Heal remaining damage over time
		array<EDamageType> damageTypes = {};
		SCR_Enum.GetEnumValues(EDamageType, damageTypes);

		foreach (EDamageType type : damageTypes)
		{
			// Prevent removing of healing DOTs so morphine and saline effects aren't stopped by healing action
			if (ignoreHealingDOT && (type == EDamageType.REGENERATION || type == EDamageType.HEALING))
				continue;

			if (IsDamagedOverTime(type))
				RemoveDamageOverTime(type);
		}

		// Fix hitzones
		foreach (HitZone hitZone : hitZones)
		{
			if (hitZone && hitZone.GetDamageState() != EDamageState.UNDAMAGED)
				hitZone.SetHealthScaled(1);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Partially heal the hitZones of an entity
	//! \param healthToDistribute The total value to be distributed among the hitzones
	//! \param sequential If true, apply health sequentially, if false apply health in parallel
	//! \param maxHealThresholdScaled When set, hitzone is healed only up to this threshold. Must be between 0 and 1.
	//! \param alternativeHitZones When filled, function will use these hitzones instead of the default, physicalHitZones
	//! \return Returns float of health that was not distributed. Value above 0 means not all health was distributed
	float HealHitZones(float healthToDistribute, bool sequential = false, float maxHealThresholdScaled = 1, array<HitZone> alternativeHitZones = null)
	{
		if (healthToDistribute <= 0)
			return healthToDistribute;

		if (maxHealThresholdScaled < 0 || maxHealThresholdScaled > 1)
			maxHealThresholdScaled = 1;

		array<HitZone> targetHitZones = {};
		if (alternativeHitZones)
			targetHitZones.Copy(alternativeHitZones);
		else
			GetPhysicalHitZones(targetHitZones);

		if (sequential)
			healthToDistribute = HealHitZonesInSequence(healthToDistribute, maxHealThresholdScaled, targetHitZones);
		else // if in Parallel
			healthToDistribute = HealHitZonesInParallel(healthToDistribute, maxHealThresholdScaled, targetHitZones);

		return healthToDistribute;
	}

	//------------------------------------------------------------------------------------------------
	protected float HealHitZonesInSequence(float healthToDistribute, float maxHealThresholdScaled, array<HitZone> targetHitZones)
	{
		foreach (HitZone hitZone : targetHitZones)
		{
			if (healthToDistribute <= 0)
				break;

			// If hitzone is undamaged, go onto the next one
			float healthToAdd = (hitZone.GetMaxHealth() * maxHealThresholdScaled) - hitZone.GetHealth();
			if (healthToAdd <= 0)
				continue;

			// If health to distribute runs out, heal hitzone with whats left and exit
			if (healthToAdd > healthToDistribute)
			{
				hitZone.HandleDamage(-healthToDistribute, EDamageType.HEALING, null);
				healthToDistribute = 0;
				break;
			}

			// Heal hitZone and substract distributable health value
			hitZone.HandleDamage(-healthToAdd, EDamageType.HEALING, null);
			healthToDistribute -= healthToAdd;
		}

		return healthToDistribute;
	}

	//------------------------------------------------------------------------------------------------
	protected float HealHitZonesInParallel(float healthToDistribute, float maxHealThresholdScaled, array<HitZone> targetHitZones)
	{
		array<HitZone> damagedHitZones = {};

		while (healthToDistribute > 0)
		{
			foreach (HitZone hitZone : targetHitZones)
			{
				if (hitZone.GetHealth() < (hitZone.GetMaxHealth() * maxHealThresholdScaled))
					damagedHitZones.Insert(hitZone);
			}

			if (damagedHitZones.IsEmpty())
				break;

			float healthToDistributeHitZone = healthToDistribute / damagedHitZones.Count();
			foreach (HitZone hitZone : damagedHitZones)
			{
				if (healthToDistribute <= 0)
					break;

				// If hitzone is undamaged, go onto the next one
				float healthToAdd = (hitZone.GetMaxHealth() * maxHealThresholdScaled) - hitZone.GetHealth();
				if (healthToAdd <= 0)
					continue;

				// If health to distribute is more than needed, apply what's needed and save whats' left
				if (healthToDistributeHitZone > healthToAdd)
				{
					hitZone.HandleDamage(-healthToAdd, EDamageType.HEALING, null);
					healthToDistribute -= healthToAdd;
					continue;
				}
				else// If health to distribute is less than needed, reduce healthToDistribute by health applied and continue
				{
					hitZone.HandleDamage(-healthToDistributeHitZone, EDamageType.HEALING, null);
					healthToDistribute -= healthToDistributeHitZone;
					continue;
				}
			}

			damagedHitZones.Clear();
		}

		return healthToDistribute;
	}

	//------------------------------------------------------------------------------------------------
	//! Get all damage dealt to hitzones. Maxhealth minus current health is damage.
	//! \param untilThresholdScaled When set damage until this scaled threshold is returned.
	//! \param alternativeHitZones When filled, function will use these hitzones instead of the default, physicalHitZones
	//! \return Returns float of all health missing from checked hitZones
	float GetHitZonesDamage(float untilThresholdScaled = 1, array<HitZone> alternativeHitZones = null)
	{
		array<HitZone> hitZones = {};
		if (!alternativeHitZones || alternativeHitZones.IsEmpty())
			GetPhysicalHitZones(hitZones);
		else
			hitZones.Copy(alternativeHitZones);

		if (!hitZones)
			return 0;
		
		float totalDamage, addedDamage;

		foreach (HitZone hitZone : hitZones)
		{
			//~ Ignore undamaged
			if (hitZone.GetDamageState() == EDamageState.UNDAMAGED)
				continue;
			
			addedDamage = (hitZone.GetMaxHealth() * untilThresholdScaled) - hitZone.GetHealth();
			if (addedDamage > 0)
				totalDamage += addedDamage;
		}

		return totalDamage;
	}

	//------------------------------------------------------------------------------------------------
	//! Get Health scaled of all hitzones
	//! \param alternativeHitZones When filled, function will use these hitzones instead of the default, physicalHitZones
	//! \return Returns float of all hitzones scaled devided by the amount of hitzones
	float GetHitZonesHealthScaled(array<HitZone> alternativeHitZones = null)
	{
		array<HitZone> hitZones = {};
		if (!alternativeHitZones || alternativeHitZones.IsEmpty())
			GetPhysicalHitZones(hitZones);
		else
			hitZones.Copy(alternativeHitZones);

		//~ No hitzones so health is full
		if (!hitZones || hitZones.IsEmpty())
			return 1;

		float totalHealthScaled;

		foreach (HitZone hitZone : hitZones)
		{
			if (hitZone.GetDamageState() == EDamageState.UNDAMAGED)
			{
				totalHealthScaled += 1;
				continue;
			}
				
			totalHealthScaled += hitZone.GetHealthScaled();
		}

		return totalHealthScaled / hitZones.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! Get single Health scaled from hitzones. Will return lowest or highest value
	//! \param alternativeHitZones When filled, function will use these hitzones instead of the default, physicalHitZones
	//! \param getLowestHealth If true it will return the hitzone health that is lowest, else it will return the hitzone health that is highest
	//! \return Returns float of all hitzones scaled devided by the amount of hitzones
	float GetSingleHitZonesHealthScaled(array<HitZone> alternativeHitZones = null, bool getLowestHealth = true)
	{
		array<HitZone> hitZones = {};
		if (!alternativeHitZones || alternativeHitZones.IsEmpty())
			GetPhysicalHitZones(hitZones);
		else
			hitZones.Copy(alternativeHitZones);

		//~ No hitzones so health is full
		if (!hitZones || hitZones.IsEmpty())
			return 1;

		float returnValue = -1;
		float healthScaled;

		foreach (HitZone hitZone : hitZones)
		{
			if (returnValue < 0)
			{
				if (hitZone.GetDamageState() == EDamageState.UNDAMAGED)
					returnValue = 1;
				else 
					returnValue = hitZone.GetHealthScaled();
				
				continue;
			}
			
			if (hitZone.GetDamageState() == EDamageState.UNDAMAGED)
				healthScaled = 1;
			else 
				healthScaled = hitZone.GetHealthScaled();

			if ((getLowestHealth && returnValue > healthScaled) || (!getLowestHealth && returnValue < healthScaled))
				returnValue = healthScaled;
		}

		return returnValue;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Determine secondary explosion prefab based on explosion value, type and resource type if defined
	ResourceName GetSecondaryExplosion(float value, SCR_ESecondaryExplosionType explosionType, EResourceType resourceType = EResourceType.SUPPLIES, bool fire = false)
	{
		SCR_DamageManagerComponentClass prefabData = SCR_DamageManagerComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData)
			return ResourceName.Empty;

		SCR_SecondaryExplosions secondaries;
		if (fire)
			secondaries = prefabData.GetSecondaryFires();
		else
			secondaries = prefabData.GetSecondaryExplosions();

		if (!secondaries)
			return ResourceName.Empty;

		return secondaries.GetExplosionPrefab(value, explosionType, resourceType);
	}

	//------------------------------------------------------------------------------------------------
	//! Determine secondary explosion prefab based on explosion scale, type and resource type if defined
	SCR_SecondaryExplosion GetSecondaryExplosionForScale(SCR_ESecondaryExplosionScale scale, SCR_ESecondaryExplosionType explosionType, EResourceType resourceType = EResourceType.SUPPLIES)
	{
		SCR_DamageManagerComponentClass prefabData = SCR_DamageManagerComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData)
			return null;

		SCR_SecondaryExplosions secondaries = prefabData.GetSecondaryFires();
		if (!secondaries)
			return null;

		return secondaries.GetSecondaryExplosionForScale(scale, explosionType, resourceType);
	}

	//------------------------------------------------------------------------------------------------
	//! Determine secondary explosion prefab based on explosion value, type and resource type if defined
	SCR_ESecondaryExplosionScale GetSecondaryExplosionScale(float value, SCR_ESecondaryExplosionType explosionType, EResourceType resourceType = EResourceType.SUPPLIES)
	{
		SCR_DamageManagerComponentClass prefabData = SCR_DamageManagerComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData)
			return SCR_ESecondaryExplosionScale.NONE;

		SCR_SecondaryExplosions secondaries = prefabData.GetSecondaryFires();
		if (!secondaries)
			return SCR_ESecondaryExplosionScale.NONE;

		return secondaries.GetSecondaryExplosionScale(value, explosionType, resourceType);
	}

	//------------------------------------------------------------------------------------------------
	//! Determine secondary explosion prefab based on explosion value, type and resource type if defined
	ResourceName GetSecondaryFireParticle(float value, SCR_ESecondaryExplosionType explosionType, EResourceType resourceType = EResourceType.SUPPLIES)
	{
		SCR_DamageManagerComponentClass prefabData = SCR_DamageManagerComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData)
			return ResourceName.Empty;

		SCR_SecondaryExplosions secondaries = prefabData.GetSecondaryFires();
		if (!secondaries)
			return ResourceName.Empty;

		return secondaries.GetFireParticles(value, explosionType, resourceType);
	}

	//------------------------------------------------------------------------------------------------
	//! Spawn secondary explosion
	void SecondaryExplosion(ResourceName prefabName, notnull Instigator instigator, notnull EntitySpawnParams spawnParams)
	{
		if (GetDefaultHitZone() && GetDefaultHitZone().IsProxy())
			return;

		Resource secondaryResource = Resource.Load(prefabName);
		if (!secondaryResource.IsValid())
			return;

		if (!spawnParams.Parent)
			spawnParams.Parent = GetOwner();

		IEntity explosion = GetGame().SpawnEntityPrefab(secondaryResource, spawnParams.Parent.GetWorld(), spawnParams);
		if (!explosion)
			return;

		if (instigator.GetInstigatorType() != InstigatorType.INSTIGATOR_PLAYER)
			return;

		// Set instigator on trigger component
		BaseTriggerComponent trigger = BaseTriggerComponent.Cast(explosion.FindComponent(BaseTriggerComponent));
		if (trigger)
			trigger.GetInstigator().SetInstigatorByPlayerID(instigator.GetInstigatorPlayerID());
	}

	//------------------------------------------------------------------------------------------------
	SCR_ResourceEncapsulator GetResourceEncapsulator(EResourceType suppliesType = EResourceType.SUPPLIES)
	{
		IEntity owner = GetOwner();
		if (!owner)
			return null;

		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.FindResourceComponent(owner);
		if (!resourceComponent)
			return null;

		SCR_ResourceContainer container = resourceComponent.GetContainer(suppliesType);
		if (!container)
			return null;

		return container.GetResourceEncapsulator();
	}

	//------------------------------------------------------------------------------------------------
	//! Spawn supply secondary explosion when vehicle becomes destroyed
	void SupplySecondaryExplosion(notnull Instigator instigator)
	{
		SCR_ResourceEncapsulator encapsulator = GetResourceEncapsulator();
		if (!encapsulator)
			return;

		// First check, if there are any supplies at all
		// If there is no explosion, then do not spend time trying to figure out where it should be originated from
		float resourceValue = encapsulator.GetAggregatedResourceValue();
		if (resourceValue <= 0)
			return;

		IEntity owner = encapsulator.GetOwner();
		if (!owner)
			owner = GetOwner();

		if (!owner)
			return;

		ResourceName secondaryExplosionPrefab = GetSecondaryExplosion(resourceValue, SCR_ESecondaryExplosionType.RESOURCE);
		if (secondaryExplosionPrefab.IsEmpty())
			return;

		SCR_ResourceContainerQueueBase containerQueue = encapsulator.GetContainerQueue();
		int containerCount = encapsulator.GetContainerCount();
		SCR_ResourceContainer container;
		float weight;
		vector position;
		vector averagePosition = owner.CoordToLocal(encapsulator.GetOwnerOrigin());
		EntitySpawnParams spawnParams();
		spawnParams.Parent = owner;

		// Get the weighed average position of explosion relative to encapsulator
		for (int i; i < containerCount; i++)
		{
			container = containerQueue.GetContainerAt(i);
			if (!container)
				continue;

			// Determine secondary explosion position
			weight = container.GetResourceValue() / resourceValue;
			if (weight <= 0)
				continue;

			position = owner.CoordToLocal(container.GetOwnerOrigin());
			averagePosition += position * weight;
		}

		spawnParams.Transform[3] = averagePosition;

		// Destroy the resources and deny further use
		for (int i; i < containerCount; i++)
		{
			container = containerQueue.GetContainerAt(i);
			if (!container)
				continue;

			container.SetResourceValue(0);
			container.SetResourceRights(EResourceRights.NONE);
		}

		SecondaryExplosion(secondaryExplosionPrefab, instigator, spawnParams);
	}

	//------------------------------------------------------------------------------------------------
	//! Get weighed average position of explosion for hitzones of specified type
	vector GetSecondaryExplosionPosition(typename hitZoneType, out float totalWeight = 0)
	{
		IEntity owner = GetOwner();
		if (!owner)
			return vector.Zero;

		array<HitZone> hitZones = {};
		GetAllHitZones(hitZones);
		SCR_DestructibleHitzone destructibleHitZone;
		PointInfo explosionPoint;
		vector position;
		vector weighedAveragePosition;
		float weight;

		// Get weighed average position of fuel tanks if defined in their hitzones
		foreach (HitZone hitZone : hitZones)
		{
			destructibleHitZone = SCR_DestructibleHitzone.Cast(hitZone);
			if (!destructibleHitZone || !destructibleHitZone.Type().IsInherited(hitZoneType))
				continue;

			weight = destructibleHitZone.GetSecondaryExplosionScale();
			if (weight < 0 || float.AlmostEqual(weight, 0))
				continue;

			explosionPoint = destructibleHitZone.GetSecondaryExplosionPoint();
			if (!explosionPoint && destructibleHitZone != GetDefaultHitZone())
				continue;

			if (explosionPoint)
				position = owner.CoordToLocal(explosionPoint.GetWorldTransformAxis(3));
			else if (owner.GetPhysics())
				position = owner.GetPhysics().GetCenterOfMass();

			weighedAveragePosition += position * weight;
			totalWeight += weight;
		}

		if (totalWeight != 0)
			weighedAveragePosition /= totalWeight;

		return weighedAveragePosition;
	}

	//------------------------------------------------------------------------------------------------
	//! Spawn fuel secondary explosion when vehicle becomes destroyed
	void FuelSecondaryExplosion(notnull Instigator instigator)
	{
		float totalFuel;
		vector explosionPosition = GetSecondaryExplosionPosition(SCR_FuelHitZone, totalFuel);
		EntitySpawnParams spawnParams();
		spawnParams.Transform[3] = explosionPosition;
		ResourceName secondaryExplosionPrefab = GetSecondaryExplosion(totalFuel, SCR_ESecondaryExplosionType.FUEL);
		SecondaryExplosion(secondaryExplosionPrefab, instigator, spawnParams);
	}

	//------------------------------------------------------------------------------------------------
	void UpdateFireDamage(float timeSlice);

	//------------------------------------------------------------------------------------------------
	/*! Update visual effects and store result in particles variable
	\param position origin of particle effect entity in owner local space
	\param particles variable to store ParticleEffectEntity
	\param fireType type of fire particles to be created
	\param resourceType type of resource if fireType is RESOURCE, by default SUPPLIES
	*/
	void UpdateFireParticles(vector position, out ParticleEffectEntity particles, SCR_ESecondaryExplosionScale state, SCR_ESecondaryExplosionType fireType, EResourceType resourceType = EResourceType.SUPPLIES)
	{
		if (particles)
			particles.StopEmission();

		SCR_SecondaryExplosion secondaryFire = GetSecondaryExplosionForScale(state, fireType, resourceType);
		if (!secondaryFire)
			return;

		ResourceName fireParticles = secondaryFire.m_sSecondaryFireParticles;
		if (fireParticles.IsEmpty())
			return;

		ParticleEffectEntitySpawnParams params();
		params.Parent = GetOwner();
		params.Transform[3] = position;
		params.PlayOnSpawn = true;
		params.DeleteWhenStopped = true;

		particles = ParticleEffectEntity.SpawnParticleEffect(fireParticles, params);
	}

	//------------------------------------------------------------------------------------------------
	protected void ConnectToFireDamageSystem()
	{
		World world = GetOwner().GetWorld();
		FireDamageSystem system = FireDamageSystem.Cast(world.FindSystem(FireDamageSystem));
		if (system)
			system.Register(this);
	}

	//------------------------------------------------------------------------------------------------
	protected void DisconnectFromFireDamageSystem()
	{
		World world = GetOwner().GetWorld();
		FireDamageSystem system = FireDamageSystem.Cast(world.FindSystem(FireDamageSystem));
		if (system)
			system.Unregister(this);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDamageStateChanged(EDamageState state)
	{
		super.OnDamageStateChanged(state);

		// Only main hitzone can explode supplies
		if (state == EDamageState.DESTROYED)
		{
			Instigator instigator = GetInstigator();
			SupplySecondaryExplosion(instigator);
			FuelSecondaryExplosion(instigator);
		}
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Invoked every time the DoT is added to certain hitzone.
	*/
	override void OnDamageOverTimeAdded(EDamageType dType, float dps, HitZone hz)
	{
		super.OnDamageOverTimeAdded(dType, dps, hz);

		RplComponent rpl = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (rpl && rpl.IsProxy())
			return;

		if (dType == EDamageType.FIRE)
			ConnectToFireDamageSystem();
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Invoked when provided damage type is removed from certain hitzone.
	*/
	override void OnDamageOverTimeRemoved(EDamageType dType, HitZone hz)
	{
		super.OnDamageOverTimeAdded(dType, dps, hz);

		RplComponent rpl = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (rpl && rpl.IsProxy())
			return;

		if (dType == EDamageType.FIRE && !IsDamagedOverTime(dType))
			DisconnectFromFireDamageSystem();
	}

#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	protected override void _WB_AfterWorldUpdate(IEntity owner, float timeSlice)
	{
		super._WB_AfterWorldUpdate(owner, timeSlice);

		GenericEntity entity = GenericEntity.Cast(owner);
		if (!entity)
			return;

		WorldEditorAPI api = entity._WB_GetEditorAPI();
		if (!api || !api.IsEntitySelected(owner))
			return;

		//! Hitzone DrawDebug
		array<HitZone> hitZones = {};
		GetAllHitZones(hitZones);

		ScriptedHitZone scriptedHitZone
		foreach (HitZone hitZone : hitZones)
		{
			scriptedHitZone = ScriptedHitZone.Cast(hitZone);
			if (scriptedHitZone)
				scriptedHitZone.DrawDebug();
		}
	}
#endif
}
