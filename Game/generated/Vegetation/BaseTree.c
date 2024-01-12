/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Vegetation
\{
*/

class BaseTreeClass: StaticModelEntityClass
{
}

class BaseTree: StaticModelEntity
{
	/*!
	Called when the damage has been dealt by the server.
	\param damage Damage value.
	\param type Damage type.
	\param pHitEntity Damaged entity.
	\param outMat Hit position/direction/normal.
	\param damageSource Projectile Entity.
	\param damageSourceParent Damage source parent entity (soldier / vehicle).
	\param colliderID Collider ID if exists otherwise enf::BAD_INDEX.
	\param speed Projectile velocity in m/s.
	*/
	event void OnDamage(float damage, EDamageType type, IEntity pHitEntity, inout vector outMat[3], IEntity damageSource, notnull Instigator instigator, int colliderID, float speed);
}

/*!
\}
*/
