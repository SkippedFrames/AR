/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class HitZoneContainerComponent: GameComponent
{
	proto external IEntity GetOwner();
	proto external HitZone GetDefaultHitZone();
	proto external HitZone GetHitZone(string colliderName);
	//! Clears and fills the specified outHitZones array with all HZs.
	//! Returns the count of elements that were filled into the array.
	proto external int GetAllHitZones(out notnull array<HitZone> outHitZones);

	// callbacks

	// --------------------------------------------------------------------------------
	event protected bool OnRplSave(ScriptBitWriter writer) { return true; };
	event protected bool OnRplLoad(ScriptBitReader reader) { return true; };
}

/*!
\}
*/
