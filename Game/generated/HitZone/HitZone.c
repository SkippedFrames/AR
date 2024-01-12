/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup HitZone
\{
*/

class HitZone: ScriptAndConfig
{
	//All non true damage taken will be multiplied by this amount
	proto external float GetBaseDamageMultiplier();
	//returns true if this hitzone has collider nodes.
	proto external bool HasColliderNodes();
	//returns index of collider descriptor attached to collider ID
	proto external int GetColliderDescriptorIndex(int colliderID);
	//Get damage multipler for a type of damage in this hitzone
	proto external float GetDamageMultiplier(EDamageType dmgType);
	//return damage reduction
	proto external float GetDamageReduction();
	//return damage threshold
	proto external float GetDamageThreshold();
	/*!
	\param colliderNames array which is filled with collider names of this hitzone
	*/
	proto external int GetAllColliderNames(out notnull array<string> colliderNames);
	/*!
	Sets damage over time for this particular hitzone. And invoke particular
	event on DamageManagerComponent. It has no effect if there is
	already a DOT of same type and DPS. Cannot be called on proxy.
	\param dmgType Type of damage
	\param dps Damage per second applied to this HitZone
	*/
	proto external void SetDamageOverTime(EDamageType dmgType, float dps);
	//return damage per second of specified type
	proto external float GetDamageOverTime(EDamageType dmgType);
	//Returns number of collider decriptors
	proto external int GetNumColliderDescriptors();
	//script method wrappers
	proto external HitZoneContainerComponent GetHitZoneContainer();
	//Sets the health of this hitzone. Only works when called from server.
	proto external void SetHealth(float health);
	//Sets the scaled health of this hitzone [0, 1]. Only works when called from server.
	proto external void SetHealthScaled(float health);
	//Sets the max health of this hitzone. Only works when called from server.
	proto external void SetMaxHealth(float maxHealth, ESetMaxHealthFlags flag = ESetMaxHealthFlags.NONE);
	//Gets the current health of this hitzone. Avoid tying game logic to GetHealth, there is no guarantee of synchronization! Tie game logic to hitzone damage state.
	proto external float GetHealth();
	//Gets the current scaled health of this hitzone. Avoid tying game logic to this function, there is no guarantee of synchronization! Tie game logic to hitzone damage state.
	proto external float GetHealthScaled();
	//Returns health % needed to trigger the requested damage state.
	proto external float GetDamageStateThreshold(EDamageState damageState);
	//Returns max health of this hitzone
	proto external float GetMaxHealth();
	//Returns hitzone name
	proto external string GetName();
	//Gets current damage state of the hitzone
	proto external EDamageState GetDamageState();
	//Returns the amount of damage to be received at once needed to be considered critical
	proto external float GetCriticalDamageThreshold();
	//Gets previous damage state of the hitzone
	proto external EDamageState GetPreviousDamageState();
	//Hitzone will handle this amount of damage. Damage will only be applied if called from server. Use DamageManager.HandleDamage when possible, using this skips DamageManager.OnDamage
	proto external void HandleDamage(float damage, int damageType, IEntity instigator);
	//Returns true if its a proxy
	proto external bool IsProxy();
	//gets collider description
	proto bool TryGetColliderDescription(IEntity owner, int descIndex, out vector transformLS[4], out int boneIndex, out int nodeID);
	//Gets collider description from name
	proto bool TryGetColliderDescriptionFromName(IEntity owner, string colliderName, out vector transformLS[4], out int boneIndex, out int nodeID);
}

/*!
\}
*/
