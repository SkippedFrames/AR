#define ENABLE_BASE_DESTRUCTION
//------------------------------------------------------------------------------------------------
class SCR_DestructibleHitzone : ScriptedHitZone
{
	[Attribute("0", UIWidgets.Slider, "Scale of damage passed to parent default hitzone\nIgnores base damage multiplier, reduction and threshold\nDamage type multipliers are applied", "0 100 0.001")]
	protected float m_fPassDamageToParentScale;

	[Attribute(desc: "Rules for passing damage to parent and root damage managers")]
	protected ref array<ref SCR_DamagePassRule> m_aDamagePassRules;

	[Attribute(desc: "Secondary explosion reference point", category: "Secondary damage")]
	protected ref PointInfo m_SecondaryExplosionPoint;

	[Attribute("", UIWidgets.Auto, desc: "Destruction Handler")]
	protected ref SCR_DestructionBaseHandler m_pDestructionHandler;

	[Attribute(desc: "Destruction sound event name", category: "Effects")]
	protected string m_sDestructionSoundEvent;

	[Attribute(desc: "Particle effect for destruction", params: "ptc", category: "Effects")]
	protected ResourceName m_sDestructionParticle;

	[Attribute(uiwidget: UIWidgets.Coords, "Position of the effect in model space", category: "Effects")]
	protected vector m_vParticleOffset;

	protected ParticleEffectEntity					m_DstParticle; // Destruction particle
	protected SCR_DamageManagerComponent			m_ParentDamageManager; // Damage manager of the direct parent
	protected SCR_DamageManagerComponent			m_RootDamageManager;
	protected SCR_BaseCompartmentManagerComponent	m_CompartmentManager; // The part may have occupants that we want to damage

#ifdef ENABLE_BASE_DESTRUCTION
	//------------------------------------------------------------------------------------------------
	ref SCR_DestructionBaseHandler GetDestructionHandler()
	{
		return m_pDestructionHandler;
	}

	//------------------------------------------------------------------------------------------------
	override void OnInit(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.OnInit(pOwnerEntity, pManagerComponent);

		m_CompartmentManager = SCR_BaseCompartmentManagerComponent.Cast(pOwnerEntity.FindComponent(SCR_BaseCompartmentManagerComponent));

		// Parent damage manager
		IEntity parent = pOwnerEntity.GetParent();
		if (parent)
			m_ParentDamageManager = SCR_DamageManagerComponent.Cast(parent.FindComponent(SCR_DamageManagerComponent));

		// Root damage manager
		IEntity root = pOwnerEntity.GetRootParent();
		if (root)
			m_RootDamageManager = SCR_DamageManagerComponent.Cast(root.FindComponent(SCR_DamageManagerComponent));

		if (m_pDestructionHandler)
			m_pDestructionHandler.Init(pOwnerEntity, this);
		
		if (m_SecondaryExplosionPoint)
			m_SecondaryExplosionPoint.Init(pOwnerEntity);
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

		
		// Forward non-DOT damage to parent, ignoring own base damage multiplier
		if (!isDOT && m_fPassDamageToParentScale != 0)
			PassDamageToParent(damageType, rawDamage * m_fPassDamageToParentScale, instigator, hitTransform, hitMaterial);
		
		// Forward non-DOT damage to root or parent default hitzone, ignoring own base damage multiplier
		EDamageType type;
		foreach (SCR_DamagePassRule rule : m_aDamagePassRules)
		{
			if (isDOT && !rule.m_bAllowDOT)
				continue;

			// If damage types are defined, only allow passing specified damage types
			if (!rule.m_aSourceDamageTypes.IsEmpty() && !rule.m_aSourceDamageTypes.Contains(damageType))
				continue;

			// If damage states are defined, only allow passing while damage state is allowed
			if (!rule.m_aDamageStates.IsEmpty() && !rule.m_aDamageStates.Contains(GetDamageState()))
				continue;

			if (rule.m_eOutputDamageType == EDamageType.TRUE)
				type = damageType;
			else
				type = rule.m_eOutputDamageType;

			if (rule.m_bPassToRoot)
				PassDamageToRoot(type, rawDamage * rule.m_fMultiplier, instigator, hitTransform, hitMaterial);

			if (rule.m_bPassToParent)
				PassDamageToParent(type, rawDamage * rule.m_fMultiplier, instigator, hitTransform, hitMaterial);
			
			if (rule.m_bPassToDefaultHitZone)
				PassDamageToDefaultHitZone(type, rawDamage * rule.m_fMultiplier, instigator, hitTransform, hitMaterial);
		}

		return super.ComputeEffectiveDamage(damageType, rawDamage, hitEntity, struckHitZone, damageSource, instigator, hitMaterial, colliderID, hitTransform, impactVelocity, nodeID, isDOT);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Pass damage to default hitzone of parent damage manager, ignoring base damage multiplier, reduction and threshold
	\param type - damage type, use TRUE damage to ignore thresholds and multipliers of default hitzone
	\param damage - amount of damage passed to default hitzone of parent damage manager, multiplied by damage type multiplier of current hitzone
	\param Instigator - Instigator
	\param transform - hit position, direction and normal
	\param surface - hit surface properties
	*/
	void PassDamageToParent(EDamageType type, float damage, notnull Instigator instigator, inout vector transform[3], SurfaceProperties surface = null)
	{
		if (!m_ParentDamageManager)
			return;

		if (damage == 0)
			return;

		m_ParentDamageManager.HandleDamage(type, damage, transform, m_ParentDamageManager.GetOwner(), m_ParentDamageManager.GetDefaultHitZone(), instigator, surface, -1, -1);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Pass damage to default hitzone of root damage manager, ignoring base damage multiplier, reduction and threshold
	\param type - damage type, use TRUE damage to ignore thresholds and multipliers of default hitzone
	\param damage - amount of damage passed to default hitzone of root damage manager, multiplied by damage type multiplier of current hitzone
	\param Instigator - Instigator
	\param transform - hit position, direction and normal
	\param surface - hit surface properties
	*/
	void PassDamageToRoot(EDamageType type, float damage, notnull Instigator instigator, inout vector transform[3], SurfaceProperties surface = null)
	{
		if (!m_RootDamageManager)
			return;

		if (damage == 0)
			return;

		HitZone defaultHZ = m_RootDamageManager.GetDefaultHitZone();
		if (defaultHZ != this)
			m_RootDamageManager.HandleDamage(type, damage, transform, m_RootDamageManager.GetOwner(), defaultHZ, instigator, surface, -1, -1);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Pass damage to default hitzone of owner damage manager, ignoring base damage multiplier, reduction and threshold
	\param type - damage type, use TRUE damage to ignore thresholds and multipliers of default hitzone
	\param damage - amount of damage passed to default hitzone of owner damage manager, multiplied by damage type multiplier of current hitzone
	\param Instigator - Instigator
	\param transform - hit position, direction and normal
	\param surface - hit surface properties
	*/
	void PassDamageToDefaultHitZone(EDamageType type, float damage, notnull Instigator instigator, inout vector transform[3], SurfaceProperties surface = null)
	{
		SCR_DamageManagerComponent damageManager = SCR_DamageManagerComponent.Cast(GetHitZoneContainer());
		if (!damageManager || !damageManager.GetDefaultHitZone())
			return;

		if (damage == 0)
			return;

		HitZone defaultHZ = damageManager.GetDefaultHitZone();
		if (defaultHZ != this)
			damageManager.HandleDamage(type, damage, transform, damageManager.GetOwner(), defaultHZ, instigator, surface, -1, -1);
	}

	//------------------------------------------------------------------------------------------------
	//! Get secondary explosion desired scale. It will determine the prefab retrieved from secondary explosion config.
	float GetSecondaryExplosionScale()
	{
		return GetMaxHealth();
	}

	//------------------------------------------------------------------------------------------------
	//! Get secondary explosion desired scale. It will determine the prefab retrieved from secondary explosion config.
	PointInfo GetSecondaryExplosionPoint()
	{
		return m_SecondaryExplosionPoint;
	}

	//------------------------------------------------------------------------------------------------
	//! Kill occupants and start destruction
	override void OnDamageStateChanged()
	{
		super.OnDamageStateChanged();

		EDamageState state = GetDamageState();

		// Restrict AI from accessing this parts compartments
		if (m_CompartmentManager)
			m_CompartmentManager.SetCompartmentsAccessibleForAI(state != EDamageState.DESTROYED);

		// Change from destroyed can only mean repair
		if (GetPreviousDamageState() == EDamageState.DESTROYED)
		{
			StopDestruction();
			return;
		}

		if (state == EDamageState.DESTROYED)
			StartDestruction();

		PlayDestructionSound(state);
		PlayDestructionParticle(state);
	}

	//------------------------------------------------------------------------------------------------
	//! Start destruction effects
	void StartDestruction(bool immediate = false)
	{
		SCR_VehicleDamageManagerComponent parentDamageManager = FindParentVehicleDamageManager();

		// Kill and eject occupants
		// Only main hitzone should deal damage to occupants.
		// TODO: Make passengers suffer random fire and bleeding instead, and let them get out if they are conscious.
		// TODO: Depends on ability to move the occupants to proper positions inside wrecks.
		HitZoneContainerComponent hitZoneContainer = GetHitZoneContainer();
		if (hitZoneContainer && hitZoneContainer.GetDefaultHitZone() == this && m_CompartmentManager && !IsProxy())
		{
			SCR_DamageManagerComponent damageManager = parentDamageManager;
			if (!damageManager)
				damageManager = SCR_DamageManagerComponent.Cast(GetHitZoneContainer());

			Instigator instigator;
			if (damageManager)
				instigator = damageManager.GetInstigator();

			m_CompartmentManager.KillOccupants(instigator, true);
		}

		if (m_pDestructionHandler)
			m_pDestructionHandler.StartDestruction(immediate);
	}

	//------------------------------------------------------------------------------------------------
	//! Stop destruction effects
	protected void StopDestruction()
	{
		// Must not change model outside frame
		if (m_pDestructionHandler)
			GetGame().GetCallqueue().CallLater(m_pDestructionHandler.OnRepair);
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_VehicleDamageManagerComponent FindParentVehicleDamageManager()
	{
		// Futile if there is no hitzonecontainer anymore
		HitZoneContainerComponent hitZoneContainer = GetHitZoneContainer();
		if (!hitZoneContainer)
			return null;

		SCR_VehicleDamageManagerComponent damageManager = SCR_VehicleDamageManagerComponent.Cast(hitZoneContainer);
		if (damageManager)
			return damageManager;

		damageManager = SCR_VehicleDamageManagerComponent.Cast(m_ParentDamageManager);
		if (damageManager)
			return damageManager;

		IEntity parent = GetOwner().GetParent();
		while (parent)
		{
			damageManager = SCR_VehicleDamageManagerComponent.Cast(parent.FindComponent(SCR_VehicleDamageManagerComponent));
			if (damageManager)
				return damageManager;

			parent = parent.GetParent();
		}

		return damageManager;
	}

	//------------------------------------------------------------------------------------------------
	//! Start destruction sounds
	protected void PlayDestructionSound(EDamageState damageState)
	{
		// Sounds are only relevant for non-dedicated clients
		if (System.IsConsoleApp())
			return;

		if (damageState != EDamageState.DESTROYED)
			return;

		if (m_sDestructionSoundEvent.IsEmpty())
			return;

		IEntity owner = GetOwner();
		if (!owner)
			return;

		// Play destruction effects only if not streaming in
		SCR_HitZoneContainerComponent hitZoneContainer = SCR_HitZoneContainerComponent.Cast(GetHitZoneContainer());
		if (IsProxy() && hitZoneContainer && !hitZoneContainer.IsRplReady())
			return;

		IEntity mainParent = SCR_EntityHelper.GetMainParent(owner);
		if (mainParent)
		{
			// Get position offset for slotted entities
			vector mins;
			vector maxs;
			owner.GetBounds(mins, maxs);
			vector center = vector.Lerp(mins, maxs, 0.5);

			// Add bone offset
			EntitySlotInfo slotInfo = EntitySlotInfo.GetSlotInfo(owner);
			if (slotInfo)
			{
				vector mat[4];
				slotInfo.GetModelTransform(mat);
				center = center + mat[3];
			}

			SoundComponent soundComponent = SoundComponent.Cast(mainParent.FindComponent(SoundComponent));
			if (!soundComponent)
				return;

			soundComponent.SoundEventOffset(m_sDestructionSoundEvent, center);
		}
		else
		{
			SoundComponent soundComponent = SoundComponent.Cast(owner.FindComponent(SoundComponent));
			if (!soundComponent)
				return;

			soundComponent.SoundEvent(m_sDestructionSoundEvent);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Start destruction particle
	void PlayDestructionParticle(EDamageState state)
	{
		// Particles are only relevant for non-dedicated clients
		if (System.IsConsoleApp())
			return;

		if (state == EDamageState.DESTROYED && !m_DstParticle && !m_sDestructionParticle.IsEmpty())
		{
			ParticleEffectEntitySpawnParams spawnParams();
			spawnParams.Transform[3] = m_vParticleOffset;
			spawnParams.Parent = GetOwner();
			spawnParams.UseFrameEvent = true;
			m_DstParticle = ParticleEffectEntity.SpawnParticleEffect(m_sDestructionParticle, spawnParams);
		}
	}
#endif
}
