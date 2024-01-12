enum EFireState // Rename to SCR_EBurningState - we have conflict with gamecode
{
	NONE,
	SMOKING_LIGHT,
	SMOKING_HEAVY,
	SMOKING_IGNITING,
	BURNING
}

class SCR_FlammableHitZone : SCR_DestructibleHitzone
{
	protected static const float		FIRE_TERRAIN_HEIGHT_TOLERANCE = 2.2; // Prevents spawning of ground fire effect if the vehicle is too high (in meters)
	protected static const float 		LIGHT_EMISSIVITY_START = 5;

	protected Instigator				m_FireInstigator;
	protected EFireState				m_eFireState;
	protected float						m_fFireRate;
	protected float						m_fLightSmokeReductionRate;
	protected float						m_fHeavySmokeReductionRate;
	protected float						m_fIgnitingSmokeStokeRate;
	protected float						m_fFireDamageRateMin;
	protected float						m_fFireDamageRateMax;
	protected float						m_fUpdateFireTime;
	protected bool						m_bIsUpdatingFire;
	protected bool						m_bIsFireRateLocked;

	protected ref array<LightEntity> m_aLightEntities;

	[Attribute("", desc: "Data for flame lighting that is visible when vehicle burns", UIWidgets.Object, "", category: "Flammability")]
	protected ref array<ref SCR_BaseLightData> m_aLightData;

	// Burning configuration - amount of damage, burning rates etc.
	[Attribute(defvalue: "1", desc: "Fire damage multiplier\n[x * 100%]", params: "0 1000 0.01", category: "Flammability")]
	protected float m_fFireMultiplier;

	[Attribute(defvalue: "0", desc: "Fire damage applied to occupants of a burning vehicle each second (hp)", params: "0 100 0.01", category: "Flammability")]
	protected float m_fIncendiaryReduction;

	[Attribute(defvalue: "0", desc: "Minimum incendiary damage\n[hp]", params: "0 100 0.01", category: "Flammability")]
	protected float m_fIncendiaryThreshold;

	[Attribute(defvalue: "8", desc: "Fire damage applied to occupants of a burning vehicle each second\n[hp]", params: "0 100 0.01", category: "Flammability")]
	protected float m_fFireDamageOccupants;

	[Attribute(defvalue: "600", desc: "Maximum time for light smoke to stop\n[s]", params: "0 10000 0.1", category: "Flammability")]
	protected float m_fLightSmokeStopTime;

	[Attribute(defvalue: "1800", desc: "Maximum time for heavy smoke to become light smoke\n[s]", params: "0 10000 0.1", category: "Flammability")]
	protected float m_fHeavySmokeStopTime;

	[Attribute(defvalue: "180", desc: "Maximum time for igniting smoke to become fire\n[s]", params: "0 10000 0.1", category: "Flammability")]
	protected float m_fIgnitingSmokeStokeTime;

	[Attribute(defvalue: "60", desc: "Minimum time for burning hitzone down\nDetermines maximum fire rate\n[s]", params: "0 10000 0.1", category: "Flammability")]
	protected float m_fMinFireBurningTime;

	[Attribute(defvalue: "240", desc: "Maximum time for burning hitzone down\nDetermines minimum fire rate\n[s]", params: "0 10000 0.1", category: "Flammability")]
	protected float m_fMaxFireBurningTime;

	[Attribute(defvalue: "120", desc: "Burning time of the object after destruction\n[s]", params: "0 1000 1", category: "Effects")]
	protected float m_fBurningTime;

	[Attribute(defvalue: "0.1", uiwidget: UIWidgets.Slider, desc: "Light smoke effect threshold\n[x * minimum fire rate]", params: "0 1 0.01", category: "Flammability")]
	protected float m_fLightSmokeThreshold;

	[Attribute(defvalue: "0.5", uiwidget: UIWidgets.Slider, desc: "Heavy smoke effect threshold\n[x * minimum fire rate]", params: "0 1 0.01", category: "Flammability")]
	protected float m_fHeavySmokeThreshold;

	[Attribute(defvalue: "0.8", uiwidget: UIWidgets.Slider, desc: "Igniting smoke effect threshold\n[x * minimum fire rate]", params: "0 1 0.01", category: "Flammability")]
	protected float m_fIgnitingSmokeThreshold;

	// Particle effects
	[Attribute(desc: "Particle effect for light smoke", params: "ptc", category: "Effects")]
	protected ResourceName m_sDamagedParticle;

	[Attribute(desc: "Particle effect for heavy smoke", params: "ptc", category: "Effects")]
	protected ResourceName m_sDamagedParticleHeavy;

	[Attribute(desc: "Particle effect for fire", params: "ptc", category: "Effects")]
	protected ResourceName m_sBurningParticle;

	[Attribute(desc: "Particle effect for fire on ground under object after the wreck stops moving", params: "ptc", category: "Effects")]
	protected ResourceName m_sBurningGroundParticle;

	[Attribute(defvalue: "1", desc: "Minimum water depth to stop the fire\n[m]", params: "0 100 0.1", category: "Flammability")]
	protected float m_fWaterDepthThreshold;

	[Attribute(defvalue: "FireState", desc: "Fire state signal name", category: "Flammability")]
	protected string m_sFireStateSignal;

	// Repairs
	[Attribute(EVehicleHitZoneGroup.HULL.ToString(), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EVehicleHitZoneGroup))]
	protected EVehicleHitZoneGroup m_eHitZoneGroup;

	// Audio features
	protected SignalsManagerComponent	m_SignalsManager;
	protected int						m_iFireStateSignalIdx = -1;	// Damage particles

	// Fire particles
	protected ParticleEffectEntity		m_DamagedParticleLight; // Lighter damage particle emitter
	protected ParticleEffectEntity		m_DamagedParticleHeavy; // Darker damage particle emitter
	protected ParticleEffectEntity		m_BurningParticle; // Rapid fire damage particle emitter
	protected ParticleEffectEntity		m_BurningGroundParticle; // Burning fuel on ground particle

	//------------------------------------------------------------------------------------------------
	override void OnInit(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.OnInit(pOwnerEntity, pManagerComponent);

		InitFireRates();

		SCR_VehicleDamageManagerComponent damageManager = SCR_VehicleDamageManagerComponent.Cast(m_RootDamageManager);
		if (damageManager)
			damageManager.RegisterFlammableHitZone(this);

		m_SignalsManager = SignalsManagerComponent.Cast(pOwnerEntity.FindComponent(SignalsManagerComponent));
		if (m_SignalsManager && !m_sFireStateSignal.IsEmpty())
			m_iFireStateSignalIdx = m_SignalsManager.AddOrFindSignal(m_sFireStateSignal);
	}

	//------------------------------------------------------------------------------------------------
	override EHitZoneGroup GetHitZoneGroup()
	{
		return m_eHitZoneGroup;
	}

	//------------------------------------------------------------------------------------------------
	override void OnMaxHealthChanged()
	{
		super.OnMaxHealthChanged();

		InitFireRates();

		if (GetGame().GetWorld())
			UpdateFireRate(true);
	}

	//------------------------------------------------------------------------------------------------
	//! Compute smoke and  fire damage thresholds and rates
	void InitFireRates()
	{
		float maxDamage = GetMaxHealth();
		
		if (m_fFireMultiplier <= 0)
			return;

		if (m_fMinFireBurningTime > 0)
			m_fFireDamageRateMax = maxDamage / (m_fMinFireBurningTime * m_fFireMultiplier);

		if (m_fMaxFireBurningTime > 0)
			m_fFireDamageRateMin = maxDamage / (m_fMaxFireBurningTime * m_fFireMultiplier);

		if (m_fLightSmokeStopTime > 0)
			m_fLightSmokeReductionRate = m_fFireDamageRateMin * (m_fHeavySmokeThreshold - m_fLightSmokeThreshold) / m_fLightSmokeStopTime;

		if (m_fHeavySmokeStopTime > 0)
			m_fHeavySmokeReductionRate = m_fFireDamageRateMin * (m_fIgnitingSmokeThreshold - m_fHeavySmokeThreshold) / m_fHeavySmokeStopTime;

		if (m_fIgnitingSmokeStokeTime > 0)
			m_fIgnitingSmokeStokeRate = m_fFireDamageRateMin * (1 - m_fIgnitingSmokeThreshold) / m_fIgnitingSmokeStokeTime;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Calculates the amount of damage a hitzone will receive.
	\param damageType Damage type
	\param rawDamage Incoming damage, without any modifiers taken into account
	\param hitEntity Damaged entity
	\param struckHitZone Hitzone to be damaged
	\param damageSource Projectile
	\param instigator Instigator
	\param hitMaterial Surface physics material
	\param colliderID Collider ID if provided
	\param hitTransform Position, direction and normal
	\param impactVelocity Projectile velocity at impact
	\param nodeID Bone index in mesh object
	\param isDOT True if this is a calculation for DamageOverTime
	*/
	override float ComputeEffectiveDamage(EDamageType damageType, float rawDamage, IEntity hitEntity, HitZone struckHitZone, IEntity damageSource, notnull Instigator instigator, const GameMaterial hitMaterial, int colliderID, inout vector hitTransform[3], const vector impactVelocity, int nodeID, bool isDOT)
	{
		// Incendiary damage has different thresholds
		if (damageType == EDamageType.INCENDIARY && !isDOT)
		{
			ComputeIncendiaryDamage(rawDamage, instigator);
			return 0;
		}

		float damage = rawDamage;
		if (damageType == EDamageType.FIRE)
			damage *= m_fFireMultiplier;

		return super.ComputeEffectiveDamage(damageType, damage, hitEntity, struckHitZone, damageSource, instigator, hitMaterial, colliderID, hitTransform, impactVelocity, nodeID, isDOT);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Modify fire rate based on received incendiary impacts
	\param rawDamage Incendiary damage to be handled
	\param instigator Instigator of the incendiary damage
	*/
	void ComputeIncendiaryDamage(float rawDamage, notnull Instigator instigator)
	{
		//apply base multiplier
		float incendiaryDamage = rawDamage * GetBaseDamageMultiplier();
		//apply damage multiplier for this specific damage type
		incendiaryDamage *= GetDamageMultiplier(EDamageType.INCENDIARY);

		//Extinguishing does not receive reduction or thresholds
		if (incendiaryDamage > 0)
		{
			//apply flat damage reduction
			incendiaryDamage -= m_fIncendiaryReduction;

			//if its less than the damage threshold we discard the damage.
			//if the damage to take becomes negative (healing) because of the flat damage reduction, this should reset it back to 0 dmg to take.
			if (incendiaryDamage < m_fIncendiaryThreshold)
				incendiaryDamage = 0;
		}

		// Ignore insignificant incendiary rate change
		float newFireRate = Math.Clamp(m_fFireRate + incendiaryDamage, 0, m_fFireDamageRateMax);
		if (float.AlmostEqual(newFireRate, m_fFireRate))
			return;

		SetFireRate(newFireRate);

		// Last shot that sets the vehicle on fire is going to be remembered as instigator of fire
		if (m_eFireState == EFireState.SMOKING_IGNITING)
			SetFireInstigator(instigator);
		else if (m_eFireState == EFireState.BURNING && (!m_FireInstigator || m_FireInstigator.GetInstigatorType() == InstigatorType.INSTIGATOR_NONE))
			SetFireInstigator(instigator);
	}

	//------------------------------------------------------------------------------------------------
	//! Inform damage manager about fire instigator
	protected void SetFireInstigator(Instigator instigator)
	{
		m_FireInstigator = instigator;
	}

	//------------------------------------------------------------------------------------------------
	//! Inform damage manager about fire instigator
	Instigator GetFireInstigator()
	{
		if (m_FireInstigator)
			return m_FireInstigator;

		return Instigator.CreateInstigator(null);
	}

	//------------------------------------------------------------------------------------------------
	//! Destruction logic
	override void OnDamageStateChanged()
	{
		// Override instigator before destruction is requested
		DamageManagerComponent damageManager = DamageManagerComponent.Cast(GetHitZoneContainer());
		if (m_FireInstigator && damageManager.GetDefaultHitZone() == this && GetDamageState() == EDamageState.DESTROYED)
			damageManager.SetInstigator(m_FireInstigator);

		super.OnDamageStateChanged();

		if (!GetGame().GetWorld())
			return;

		if (GetDamageState() == EDamageState.DESTROYED)
			StartDestructionFire();
		else if (GetPreviousDamageState() == EDamageState.DESTROYED)
			StopDestructionFire();
	}

	//------------------------------------------------------------------------------------------------
	//! Get rate of fire (dps). If it is below fire damage threshold, no damage will be dealt.
	float GetFireRate()
	{
		return m_fFireRate;
	}

	//------------------------------------------------------------------------------------------------
	//! Set rate of fire (dps). If it is below fire damage threshold, no damage will be dealt.
	void SetFireRate(float fireRate)
	{
		m_fFireRate = fireRate;
		m_bIsFireRateLocked = false;
		UpdateFireRate(true);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get fire rate that will achieve specified fire state
	\param fireState New fire state to be set
	\param weight Part of fire state zone to aim for. Default: -1 (becomes 0.5)
	*/
	float GetFireRateForState(EFireState fireState, float weight = -1)
	{
		if (weight < 0 || weight > 1)
			weight = 0.5;

		float top;
		float bottom;
		switch (fireState)
		{
			case EFireState.BURNING:
			{
				top = m_fFireDamageRateMax;
				bottom = m_fFireDamageRateMin;
				break;
			}
			case EFireState.SMOKING_IGNITING:
			{
				top = m_fFireDamageRateMin;
				bottom = m_fFireDamageRateMin * m_fIgnitingSmokeThreshold;
				break;
			}
			case EFireState.SMOKING_HEAVY:
			{
				top = m_fFireDamageRateMin * m_fIgnitingSmokeThreshold;
				bottom = m_fFireDamageRateMin * m_fHeavySmokeThreshold;
				break;
			}
			case EFireState.SMOKING_LIGHT:
			{
				top = m_fFireDamageRateMin * m_fHeavySmokeThreshold;
				bottom = m_fFireDamageRateMin * m_fLightSmokeThreshold;
				break;
			}
			default:
			{
				top = m_fFireDamageRateMin * m_fLightSmokeThreshold;
				bottom = 0;
			}
		}

		float fireRate = Math.Lerp(bottom, top, weight);
		return fireRate;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns current fire state
	EFireState GetFireState()
	{
		return m_eFireState;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Set fire rate that will achieve specified fire state
	\param fireState New fire state to be set
	\param weight Part of fire state zone to aim for. Default: -1 (if state is different, becomes 0.5, otherwise no change)
	\param rate Allow changing fire rate. Default: true
	*/
	void SetFireState(EFireState fireState, float weight = -1, bool changeRate = true)
	{
		// Continue only if weight is defined or fir state is changed
		bool stateChanged = fireState != m_eFireState;
		m_eFireState = fireState;

		if (stateChanged)
			UpdateFireEffects(fireState);
		else if (weight < 0)
			return;

		if (IsProxy())
			return;

		// Update fire rate
		if (changeRate)
			SetFireRate(GetFireRateForState(fireState, weight));

		if (!stateChanged)
			return;

		// Clean the fire instigator if the fire is extinguished
		if (m_eFireState < EFireState.SMOKING_IGNITING)
			SetFireInstigator(null);

		// Send update to remote clients
		array<HitZone> hitZones = {};
		SCR_HitZoneContainerComponent hitZoneContainer = SCR_HitZoneContainerComponent.Cast(GetHitZoneContainer());

		if (!hitZoneContainer)
			return;

		hitZoneContainer.GetAllHitZones(hitZones);
		int hitZoneIndex = hitZones.Find(this);
		if (hitZoneIndex >= 0)
			hitZoneContainer.Rpc(hitZoneContainer.RpcDo_SetFireState, hitZoneIndex, m_eFireState);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get fire state that will be achieved for specified fire rate
	\param fireRate Fire rate to test for
	\return fireState New fire state
	*/
	EFireState GetFireStateForRate(float fireRate)
	{
		EFireState fireState;
		if (fireRate >= m_fFireDamageRateMin)
			fireState = EFireState.BURNING;
		else if (fireRate >= m_fFireDamageRateMin * m_fIgnitingSmokeThreshold)
			fireState = EFireState.SMOKING_IGNITING;
		else if (fireRate >= m_fFireDamageRateMin * m_fHeavySmokeThreshold)
			fireState = EFireState.SMOKING_HEAVY;
		else if (fireRate >= m_fFireDamageRateMin * m_fLightSmokeThreshold)
			fireState = EFireState.SMOKING_LIGHT;
		else
			fireState = EFireState.NONE;

		return fireState;
	}

	//------------------------------------------------------------------------------------------------
	/*! Determine new fire rate
	\param fireRate Current rate of fire
	\param deltaTime Time passed in seconds since last update
	\return fireRate
	*/
	protected float CalculateNewFireRate(float fireRate, float deltaTime)
	{
		float depth;
		if (fireRate > 0 && SCR_WorldTools.IsObjectUnderwater(GetOwner(), m_vParticleOffset, -1, depth) && depth > m_fWaterDepthThreshold)
		{
			fireRate = 0;
		}
		else if (deltaTime > 0 && fireRate > 0 && fireRate < m_fFireDamageRateMin)
		{
			// Update smoking virtual fire rate, that can get
			if (fireRate < m_fHeavySmokeThreshold * m_fFireDamageRateMin)
				fireRate -= deltaTime * m_fLightSmokeReductionRate;
			else if (fireRate < m_fIgnitingSmokeThreshold * m_fFireDamageRateMin)
				fireRate -= deltaTime * m_fHeavySmokeReductionRate;
			else
				fireRate += deltaTime * m_fIgnitingSmokeStokeRate;

			fireRate = Math.Clamp(fireRate, 0, m_fFireDamageRateMin);
		}
		else
		{
			// On fire
			fireRate = Math.Clamp(fireRate, 0, m_fFireDamageRateMax);
		}

		return fireRate;
	}

	//------------------------------------------------------------------------------------------------
	/*! Apply fire damage to hitzone and to compartment occupants
	\param fireRate Current rate of fire
	\param deltaTime Time passed in seconds since last update
	*/
	protected void ApplyFireDamage(float fireRate, float deltaTime)
	{
		if (fireRate < m_fFireDamageRateMin)
		{
			SetDamageOverTime(EDamageType.FIRE, 0);
			return;
		}

		SetDamageOverTime(EDamageType.FIRE, fireRate);

		Instigator instigator = m_FireInstigator;
		if (!instigator)
			instigator = Instigator.CreateInstigator(null);

		// Damage random physical hitzones
		if (GetHitZoneContainer() == m_RootDamageManager && m_RootDamageManager.GetDefaultHitZone() == this)
			m_RootDamageManager.DamageRandomHitZones(fireRate, EDamageType.FIRE, instigator);

		// Deal damage to crew depending on how much the fire is stoked
		// TODO: Deal damage based on distance from fire
		if (!m_CompartmentManager)
			return;

		float damageOccupants = m_fFireDamageOccupants * deltaTime;
		if (m_fFireDamageRateMin > 0)
			damageOccupants *= fireRate / m_fFireDamageRateMin;

		if (damageOccupants > 0)
			m_CompartmentManager.DamageOccupants(damageOccupants, EDamageType.FIRE, instigator, false, false);
	}

	//------------------------------------------------------------------------------------------------
	//! Flammable hitzone is not immediately set on fire.
	//! At first it generates smoke which can spontaneously extinguish or ignite itself.
	//! Damage is applied once fire rate exceeds damage threshold.
	protected void UpdateFireRate(bool forceUpdate = false)
	{
		if (IsProxy())
			return;

		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return;

		IEntity owner = GetOwner();
		if (!owner)
		{
			StopFireRateUpdate();
			return;
		}

		if (!forceUpdate && m_fUpdateFireTime == 0)
			return;

		// Scale damage by time passed
		float deltaTime;
		float currentTime = world.GetWorldTime();

		if (m_fUpdateFireTime > 0)
			deltaTime = (currentTime - m_fUpdateFireTime) * 0.001; // Convert to seconds

		float previousFireRate = m_fFireRate;
		if (!m_bIsFireRateLocked)
			m_fFireRate = CalculateNewFireRate(previousFireRate, deltaTime);

		// Extinguishing should also force RPC and removal of queued call so that clients stop updating fire rate
		if (m_fFireRate == 0 && previousFireRate > 0)
			forceUpdate = true;

		ApplyFireDamage(m_fFireRate, deltaTime);

		// Clear any scheduled call in the queue
		if (forceUpdate)
			StopFireRateUpdate();

		// Schedule future update
		if (m_fFireRate > 0)
		{
			m_fUpdateFireTime = currentTime;

			if (!m_bIsUpdatingFire)
			{
				// Schedule delayed update every second
				m_bIsUpdatingFire = true;
				GetGame().GetCallqueue().CallLater(UpdateFireRate, 1000, true, false);
			}
		}
		else
		{
			m_fUpdateFireTime = 0;
		}

		// Update fire state
		EFireState newFireState = GetFireStateForRate(m_fFireRate);
		if (newFireState != m_eFireState)
			SetFireState(newFireState, -1, false);
	}

	//------------------------------------------------------------------------------------------------
	//! Stop fire rate updates until requested again
	void StopFireRateUpdate()
	{
		m_bIsUpdatingFire = false;
		GetGame().GetCallqueue().Remove(UpdateFireRate);
	}

	//------------------------------------------------------------------------------------------------
	//! Temporarily stop fire rate changes until incendiary damage is received again
	void LockFireRate(bool locked = true)
	{
		m_bIsFireRateLocked = locked;
	}

	//------------------------------------------------------------------------------------------------
	void StartDestructionFire()
	{
		IEntity owner = GetOwner();
		if (!owner)
			return;

		SCR_HitZoneContainerComponent hitZoneContainer = SCR_HitZoneContainerComponent.Cast(GetHitZoneContainer());

		if (!hitZoneContainer)
			return;

		if (IsProxy() && !hitZoneContainer.IsRplReady())
			return;

		float depth;
		if (SCR_WorldTools.IsObjectUnderwater(owner, m_vParticleOffset, -1, depth) && depth > m_fWaterDepthThreshold)
		{
			StopDestructionFire();
			return;
		}

		SetFireState(EFireState.BURNING);

		ScriptCallQueue queue = GetGame().GetCallqueue();
		queue.CallLater(StopDestructionFire, m_fBurningTime * 1000);
		queue.CallLater(StartDestructionGroundFire, 1000, true);
	}

	//------------------------------------------------------------------------------------------------
	//! Attempts to spawn fire effect on the ground under the burning wreck.
	protected void StartDestructionGroundFire()
	{
		IEntity owner = GetOwner();
		if (!owner)
			return;

		float depth;
		if (SCR_WorldTools.IsObjectUnderwater(owner, m_vParticleOffset, -1, depth) && depth > m_fWaterDepthThreshold)
		{
			StopDestructionFire();
			return;
		}
		else if (SCR_WorldTools.IsObjectUnderwater(owner))
		{
			// Do not create surface fire effect
			return;
		}

		// Make sure the wreck is not moving
		Physics physics = owner.GetPhysics();
		if (physics && physics.GetVelocity().LengthSq() > 0.01)
			return;

		// Measure the height difference between owner and the surface
		vector position = owner.GetOrigin();
		float surfaceY = GetGame().GetWorld().GetSurfaceY(position[0], position[2]);

		// If the difference is within tolerance then proceed to spawn fire effect and stop the loop
		if (position[1] - surfaceY > FIRE_TERRAIN_HEIGHT_TOLERANCE)
			return;

		// Stop the loop
		GetGame().GetCallqueue().Remove(StartDestructionGroundFire);

		// No need to play particles on headless client
		if (System.IsConsoleApp())
			return;

		// Stop existing effect so that it gets removed and does not have to be tracked again
		if (m_BurningGroundParticle)
			m_BurningGroundParticle.StopEmission();

		position[1] = surfaceY;

		if (m_sDestructionParticle.IsEmpty())
			return;

		ParticleEffectEntitySpawnParams spawnParams();
		spawnParams.Transform[3] = position;
		m_BurningGroundParticle = ParticleEffectEntity.SpawnParticleEffect(m_sBurningGroundParticle, spawnParams);
	}

	//------------------------------------------------------------------------------------------------
	// Stops fire on vehicle and on the ground
	void StopDestructionFire()
	{
		SetFireState(EFireState.NONE);

		ScriptCallQueue queue = GetGame().GetCallqueue();
		queue.Remove(StartDestructionGroundFire);
		queue.Remove(StopDestructionFire);

		if (m_BurningGroundParticle)
			m_BurningGroundParticle.StopEmission();
	}

	//------------------------------------------------------------------------------------------------
	//! Logic when vehicle is damaged
	protected void UpdateFireEffects(EFireState fireState)
	{
		// No need to play particles on headless client
		if (System.IsConsoleApp())
			return;

		// Update sound effects
		IEntity owner = GetOwner();
		if (!owner)
			return;

		if (m_SignalsManager && m_iFireStateSignalIdx != -1)
			m_SignalsManager.SetSignalValue(m_iFireStateSignalIdx, fireState);

		// Create particle emitters
		if (!m_DamagedParticleLight || !m_DamagedParticleHeavy || !m_BurningParticle)
		{
			ParticleEffectEntitySpawnParams spawnParams();
			spawnParams.Transform[3] = m_vParticleOffset;
			spawnParams.Parent = owner;
			spawnParams.PlayOnSpawn = false;
			spawnParams.UseFrameEvent = true;

			// Spawn particles on object
			if (!m_DamagedParticleLight && !m_sDamagedParticle.IsEmpty())
				m_DamagedParticleLight = ParticleEffectEntity.SpawnParticleEffect(m_sDamagedParticle, spawnParams);

			if (!m_DamagedParticleHeavy && !m_sDamagedParticleHeavy.IsEmpty())
				m_DamagedParticleHeavy = ParticleEffectEntity.SpawnParticleEffect(m_sDamagedParticleHeavy, spawnParams);

			if (!m_BurningParticle && !m_sBurningParticle.IsEmpty())
				m_BurningParticle = ParticleEffectEntity.SpawnParticleEffect(m_sBurningParticle, spawnParams);
		}

		// Fire light
		if (fireState == EFireState.BURNING)
			FireLightOn();
		else
			FireLightOff();

		// Fire
		if (m_BurningParticle)
		{
			if (fireState == EFireState.BURNING)
				m_BurningParticle.Play();
			else
				m_BurningParticle.StopEmission();
		}

		// Dark smoke
		if (m_DamagedParticleHeavy)
		{
			if (fireState == EFireState.SMOKING_HEAVY || fireState == EFireState.SMOKING_IGNITING)
				m_DamagedParticleHeavy.Play();
			else
				m_DamagedParticleHeavy.StopEmission();
		}

		// Light smoke
		if (m_DamagedParticleLight)
		{
			if (fireState == EFireState.SMOKING_LIGHT)
				m_DamagedParticleLight.Play();
			else
				m_DamagedParticleLight.StopEmission();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void FireLightOn()
	{
		if (m_aLightEntities)
			return;

		IEntity owner = GetOwner();
		if (!owner)
			return;

		vector ownerOrigin = owner.GetOrigin();

		m_aLightEntities = {};

		foreach (SCR_BaseLightData lightdata : m_aLightData)
		{
			LightEntity lightEntity = LightEntity.CreateLight(lightdata.GetLightType(), lightdata.GetLightFlag(), lightdata.GetEffectRadius(), Color.FromVector(lightdata.GetLightColor()), LIGHT_EMISSIVITY_START, ownerOrigin);
			owner.AddChild(lightEntity, -1, EAddChildFlags.AUTO_TRANSFORM);

			// Calculate desired position
			lightEntity.SetOrigin(ownerOrigin + lightdata.GetLightOffset());
			lightEntity.SetLensFlareType(LightLensFlareType.Disabled);
			lightEntity.SetIntensityEVClip(lightdata.GetIntensityClipEV());

			m_aLightEntities.Insert(lightEntity);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void FireLightOff()
	{
		if (!m_aLightEntities)
			return;

		foreach (LightEntity lightEntity : m_aLightEntities)
		{
			if (lightEntity)
				delete lightEntity;
		}

		m_aLightEntities = null;
	}

	//------------------------------------------------------------------------------------------------
	override float GetSecondaryExplosionScale()
	{
		return GetDamageOverTime(EDamageType.FIRE);
	}
}
