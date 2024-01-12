/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Entities
\{
*/

sealed class EntityUtils
{
	private void EntityUtils();
	private void ~EntityUtils();

	//! Returns true if \param pEntity is a player
	static proto bool IsPlayer(IEntity pEntity);
	//! Returns true if \param pEntity is a vehicle the local player is controlling
	static proto bool IsPlayerVehicle(IEntity pEntity);
	//! Returns the local player entity if any
	static proto IEntity GetPlayer();
	//! Returns true if \param pEntity can be treated as PROXY. Currently true for all non-STATIC and non-particle effect entities
	static proto bool CanTreatAsProxy(IEntity pEntity, IEntity pParent);
}

/*!
\}
*/
