/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Projectile_Trigger
\{
*/

class BaseTriggerComponent: BaseProjectileComponent
{
	proto external IEntity GetOwner();
	proto external void OnUserTrigger(IEntity owner);
	//Only used when overriding the instigator is necessary. E.g.: Player A shoots explosive of player B. The instigator needs to be overriden with player A.
	proto external void OnUserTriggerOverrideInstigator(IEntity owner, Instigator instigator);
	proto external void SetLive();
	//! Get projectile effects that are inherited from projectileType parameter.
	proto external void GetProjectileEffects(typename projectileType, out notnull array<BaseProjectileEffect> outProjectileEffects);

	// callbacks

	/*!
	Event after entity is allocated and initialized.
	\param owner The owner entity
	*/
	event protected void EOnInit(IEntity owner);
	/*!
	Event when physics engine registered contact with other RigidBody
	\param owner The owner entity
	\param other Other Entity who contacted us
	\param contact Structure describing the contact
	*/
	event protected void EOnContact(IEntity owner, IEntity other, Contact contact);
}

/*!
\}
*/
