/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Projectile
\{
*/

class BaseMoveComponent: BaseProjectileComponent
{
	proto external void EnableSimulation(IEntity owner);
	proto external void Launch(vector direction, vector parentVelocity, float initSpeedCoef, IEntity projectileEntity, IEntity gunner, IEntity parentEntity, IEntity lockedTarget, IEntity weaponComponent);
	/*!
	* Queries the rewind duration (equal to the RTT of the remote shooter's connection at the time of the shot) from the projectile for lag compensation.
	* \warning This has to be performed on authority, and before a shell is launched or on non-network shots, the return value will be zero.
	* \return The current rewind duration in seconds.
	*/
	proto external float GetRewindDuration();
	/*!
	* Sets the rewind duration (equal to the RTT of the remote shooter's connection) of the projectile for lag compensation.
	* \param duration The new rewind duration in milliseconds.
	* \warning This has to be performed on authority, and must be done after the shell is launched.
	*/
	proto external void SetRewindDuration(float duration);
}

/*!
\}
*/
