class ScriptedHitZone : HitZone
{
	private ref ScriptInvoker Event_EOnHealthSet;
	private ref ScriptInvoker Event_EOnDamageStateChanged;
	
	//------------------------------------------------------------------------------------------------
	/*!
	Hitzone group getter to be overriden
	\return Get Hitzone group
	*/
	EHitZoneGroup GetHitZoneGroup()
	{
		return EHitZoneGroup.VIRTUAL;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get event called when hitzone damage changes.
	\return Script invoker
	*/
	ref notnull ScriptInvoker GetOnHealthChanged(bool createNew = true)
	{
		if (!Event_EOnHealthSet && createNew)
			Event_EOnHealthSet = new ScriptInvoker();
		return Event_EOnHealthSet;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get event called when hitzone damage state changes.
	\return Script invoker
	*/
	ref notnull ScriptInvoker GetOnDamageStateChanged(bool createNew = true)
	{
		if (!Event_EOnDamageStateChanged && createNew)
			Event_EOnDamageStateChanged = new ScriptInvoker();
		return Event_EOnDamageStateChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	\return Owner entity of the HitZone
	*/
	IEntity GetOwner()
	{
		HitZoneContainerComponent container = GetHitZoneContainer();
		if (container)
			return container.GetOwner();
		
		return null;
	}
	
	//! Called when hit zone is initialized
	void OnInit(IEntity pOwnerEntity, GenericComponent pManagerComponent);
	
	//------------------------------------------------------------------------------------------------
	//! Called every frame if this hitzone is simulated (SetSimulationState)
	void OnSimulate(IEntity owner, float timeSlice);
	
	//------------------------------------------------------------------------------------------------
	/*!
	Called after damage multipliers and thresholds are applied to received impacts and damage is applied to hitzone. 
	This method is not guaranteed to be in sync with server, use at your own risk.
	\param type Type of damage
	\param damage Amount of damage received
	\param pOriginalHitzone Original hitzone that got dealt damage, as this might be transmitted damage.
	\param damageSource Projectile object
	\param killerEntity Damage source entity (soldier, vehicle, ...)
	\param killer Damage source (soldier, vehicle, ...)
	\param hitTransform [hitPosition, hitDirection, hitNormal]
	\param speed Projectile speed in time of impact
	\param colliderID ID of the collider receiving damage
	\param nodeID ID of the node of the collider receiving damage
	*/
	void OnLocalDamage(EDamageType type, float damage, HitZone pOriginalHitzone, IEntity damageSource, IEntity killerEntity, notnull Instigator killer, inout vector hitTransform[3], float speed, int colliderID, int nodeID);
	
	//------------------------------------------------------------------------------------------------
	/*!
	Called after damage multipliers and thresholds are applied to received impacts and damage is applied to hitzone.
	This is also called when transmitting the damage to parent hitzones!
	\param type Type of damage
	\param damage Amount of damage received
	\param pOriginalHitzone Original hitzone that got dealt damage, as this might be transmitted damage.
	\param instigator Damage source parent entity (soldier, vehicle, ...)
	\param hitTransform [hitPosition, hitDirection, hitNormal]
	\param speed Projectile speed in time of impact
	\param colliderID ID of the collider receiving damage
	\param nodeID ID of the node of the collider receiving damage
	*/
	void OnDamage(EDamageType type, float damage, HitZone pOriginalHitzone, notnull Instigator instigator, inout vector hitTransform[3], float speed, int colliderID, int nodeID);
	
	//------------------------------------------------------------------------------------------------
	/*!
	Called when the damage is being set by means different than damage over time.
	*/
	protected void OnHealthSet()
	{
		if (Event_EOnHealthSet)
			Event_EOnHealthSet.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Called when the damage state changes.
	*/
	protected void OnDamageStateChanged()
	{
		if (Event_EOnDamageStateChanged)
			Event_EOnDamageStateChanged.Invoke(this);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Called when the max damage changes.
	*/
	protected void OnMaxHealthChanged();
	
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
	float ComputeEffectiveDamage(EDamageType damageType, float rawDamage, IEntity hitEntity, HitZone struckHitZone, IEntity damageSource, notnull Instigator instigator, const GameMaterial hitMaterial, int colliderID, inout vector hitTransform[3], const vector impactVelocity, int nodeID, bool isDOT)
	{
		if (rawDamage == 0)
			return 0;

		if (damageType == EDamageType.TRUE)
			return rawDamage;

		//apply base multiplier
		float effectiveDamage = rawDamage * GetBaseDamageMultiplier();
		//apply damage multiplier for this specific damage type
		effectiveDamage *= GetDamageMultiplier(damageType);
		
		//DOT doesn't get affected by damage reduction/thresholds, and neither does healing.
		if (isDOT || effectiveDamage < 0)
			return effectiveDamage;

		//apply flat damage reduction
		effectiveDamage -= GetDamageReduction();

		//if its less than the damage threshold we discard the damage.
		//if the damage to take becomes negative (healing) because of the flat damage reduction, this should reset it back to 0 dmg to take.
		if (effectiveDamage < GetDamageThreshold())
			effectiveDamage = 0;

		return effectiveDamage;
	}
	
	//	TODO@FAC. To be implemented in gamecode in future also add regen scale multiplier!!!
	//------------------------------------------------------------------------------------------------
	/*!
	Give any particular hitzone a regeneration over time
	*/
	void CustomRegeneration(IEntity target, float itemRegenerationDuration, float itemRegenerationSpeed = 0, float itemAbsoluteRegenerationAmount = 0)
	{
		if (itemRegenerationSpeed)	// If a regeneration time is set, regen will occur for given amount of time at the itemRegenerationSpeed
		{
			GetGame().GetCallqueue().CallLater(RemoveCustomRegeneration, itemRegenerationDuration * 1000, false, target, itemRegenerationSpeed);
		}
		else if (itemAbsoluteRegenerationAmount)	// If an absolute regen amount is set instead of a duration, the regen will last until the amount of points has been distributed at the itemRegenerationSpeed
		{
			itemRegenerationSpeed = itemAbsoluteRegenerationAmount / itemRegenerationDuration;
			GetGame().GetCallqueue().CallLater(RemoveCustomRegeneration, itemRegenerationDuration * 1000, false, target, itemRegenerationSpeed);
		}
		else	// If regenerating value is 0, quit.
		{
			Print("Consumable with regenerating abilities was used but no duration or amount was defined", LogLevel.WARNING);
			return;
		}
		
		float currentRegen = GetDamageOverTime(EDamageType.HEALING);
		SetDamageOverTime(EDamageType.HEALING, currentRegen + itemRegenerationSpeed * -1);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	void RemoveCustomRegeneration(IEntity target, float itemRegenerationSpeed)
	{
		// if healing was removed somehow before the regeneration was finished, don't remove again
		if (GetDamageOverTime(EDamageType.HEALING) + itemRegenerationSpeed > 0)
			SetDamageOverTime(EDamageType.HEALING, 0);
		else
			SetDamageOverTime(EDamageType.HEALING, (GetDamageOverTime(EDamageType.HEALING) + itemRegenerationSpeed));
	}

#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	void DrawDebug();
#endif
}
