/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class DamageManagerComponent: SCR_HitZoneContainerComponent
{
	//Enables damage handling. It will only be done if called from server.
	proto external void EnableDamageHandling(bool enable);
	//Returns true if this damage manager and its hitzones can take damage
	proto external bool IsDamageHandlingEnabled();
	proto external float GetMovementDamage();
	proto external float GetAimingDamage();
	proto external void SetMovementDamage(float damage);
	proto external void SetAimingDamage(float damage);
	//Get default hitzone's health [0...1]
	proto external float GetHealthScaled();
	//sets the health scaled to the default HitZone [0, ..., 1]
	proto external bool SetHealthScaled(float health);
	//Returns current amount of health on the default hitzone
	proto external float GetHealth();
	//Returns default hitzone's max health
	proto external float GetMaxHealth();
	// Returns default hitzone's DamageState
	proto external EDamageState GetState();
	//returns true if the default hitzone is in Destroyed damage state
	proto external bool IsDestroyed();
	//Sets the given entity as the last instigator of damage.
	proto external void SetInstigatorEntity(IEntity instigator);
	proto external void SetInstigator(notnull Instigator instigator);
	//Returns last instigator
	proto external notnull Instigator GetInstigator();
	/*!
	Call HandleDamage on a specified hitzone
	\param dType Type of damage
	\param damage Amount of damage received
	\param hitPosDirNorm [hitPosition, hitDirection, hitNormal]
	\param hitEntity Entity to be damaged
	\param struckHitZone HitZone to be damaged
	\param instigator instigator of the damage
	\param surface Properties of the surface struck
	\param colliderID ID of the collider receiving damage
	\param externNodeIndex External node index
	*/
	proto external void HandleDamage(EDamageType dType, float damage, out vector hitPosDirNorm[3],	IEntity hitEntity, HitZone struckHitZone, notnull Instigator instigator, SurfaceProperties surface, int colliderID, int externNodeIndex);
	/*!
	\return true if there is active DOT of specified type.
	*/
	proto external bool IsDamagedOverTime(EDamageType dType);
	/*!
	\return total DPS inflicted by all DoTs of specified type.
	*/
	proto external float GetDamageOverTime(EDamageType dType);
	/*!
	Removes DOT of provided damageType from all currently affected HitZones.
	*/
	proto external void RemoveDamageOverTime(EDamageType dType);

	// callbacks

	//! Not all armors are physical so the surface that gets struck by projectiles will not be the one the armor, but the hitzone.
	//! This is a workaround to solve that issue. It gets called every time a projectile strikes a hitzone
	//! We can override material of the hit with the corresponding one from the armor (if needed)
	event GameMaterial OverrideHitMaterial(HitZone struckHitzone);
	/*!
	Invoked every time the DoT is added to certain hitzone.
	*/
	event protected void OnDamageOverTimeAdded(EDamageType dType, float dps, HitZone hz);
	/*!
	Invoked when provided damage type is removed from certain hitzone.
	*/
	event protected void OnDamageOverTimeRemoved(EDamageType dType, HitZone hz);
	/*!
	Called when the damagestate changes.
	*/
	event protected void OnDamageStateChanged(EDamageState state);
}

/*!
\}
*/
