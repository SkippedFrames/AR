/*!
\addtogroup Damage
\{
*/

class Instigator: Managed
{
	//use CreateInstigator instead
	private void Instigator()
	{
		
	}
	
	//! Set instigator with automatic conversion from entity to player if necessary
	proto external void SetInstigator(IEntity entity);
	proto external void SetInstigatorByPlayerID(int playerID);
	
	/* Get the instigator entity, player ID will be automatically converted to current entity. Can return null if the player is dead.
	e.g.: Players can wait on the respawn screen, and this will return null, don't rely on IEntity, use PlayerID instead */
	proto external IEntity GetInstigatorEntity();
	
	//Gets instigator player ID if the instigator is a player. Returns -1 otherwise
	proto external int GetInstigatorPlayerID();
	proto external InstigatorType GetInstigatorType();
	
	static proto ref Instigator CreateInstigator(IEntity instigator);
}

/*!
\}
*/
