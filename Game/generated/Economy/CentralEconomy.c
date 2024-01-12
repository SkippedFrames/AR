/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Economy
\{
*/

class CentralEconomyClass: GenericEntityClass
{
}

class CentralEconomy: GenericEntity
{
	//! Start/ Continue processing of economy
	proto external void SimStart();
	//! Stop/ Pause processing of economy
	proto external void SimStop();
	//! Invoke Restart - restart core and reinitialize all parameters, all spawners start from scratch
	proto external void SimRestart();
	//! Invoke Cleanup - delete all content and
	proto external void SimCleanup();
	/*!
	//! TEST: Add zone to spawner
	*/
	proto external void AddType( string sName, string sUID );
	/*!
	//! TEST: Add event
	*/
	proto external void AddEvent( string sName );
	/*!
	//! TEST: Add zone to spawner
	*/
	proto external void AddZone( vector pos );
	//! Spawn specified Item or Vehicle (Animal) at Position, with Rotation and Placement Flags
	proto external IEntity SpawnItem( string name, vector pos, float angle, int flags );
	//! Spawn specified group of items or Vehicles (Herd of Animals) at Position, with Rotation and Placement Flags
	proto external int SpawnEvent( string name, vector pos, float angle, int flags );
	//! Show diag
	proto external void ShowDiag( bool bEnable );

	// callbacks

	//! When something get spawned
	event void EOnSpawn( IEntity entity );
	//! When something get removed
	event void EOnRemove();
}

/*!
\}
*/
