/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Entities
\{
*/

class DestructibleEntity: StaticModelEntity
{
	proto external void HandleDamage(EDamageType type, float damage, out vector hitPosDirNorm[3]);
	proto external int GetCorrespondingState(float hp);
	//Returns the current health of this destructible. This can be slow so use it wisely.
	proto external float GetCurrentHealth();

	// callbacks

	/*!
	Calculates the amount of damage a hitzone will receive.
	\param damageType Damage type
	\param rawDamage Incoming damage, without any modifiers taken into account
	\param damageSource Projectile
	\param Instigator instigator
	\param hitMaterial Hit surface physics material
	\param hitTransform Hit position, direction and normal
	\param impactVelocity Projectile velocity at the time of impact
	*/
	event float ComputeEffectiveDamage(EDamageType damageType, float rawDamage, IEntity damageSource, notnull Instigator instigator, const GameMaterial hitMaterial, inout vector hitTransform[3], const vector impactVelocity);
	event protected void OnDamage(int previousState, int newState, EDamageType type, float damageTaken, float currentHealth, inout vector hitTransform[3], ScriptBitWriter frameData);
	//Notification when the destructible state has changed. If JIP is true, the state has changed because of synchronization
	event protected void OnStateChanged(int destructibleState, ScriptBitReader frameData, bool JIP);
	event protected void OnBeforeDestroyed();
	event protected bool OnContact(IEntity owner, IEntity other, Contact contact);
}

/*!
\}
*/
