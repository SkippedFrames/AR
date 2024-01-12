/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup AI
\{
*/

class AIWeaponTargetSelector: ScriptAndConfig
{
	proto external void Init(notnull IEntity owner);
	/*!
	Initializes properties for target/weapon selection
	*/
	proto external void SetSelectionProperties(float maxLastSeenDirect, float maxLastSeenIndirect, float maxDistanceInfantry, float maxDistanceVehicles, float maxTimeSinceEndangered, float maxDistanceDisarmed);
	/*!
	Sets constants for target score calculation. Formula is: score = offset + slope*distance
	*/
	proto external void SetTargetScoreConstants(EAIUnitType targetUnitType, float offset, float slope);
	/*!
	Selects weapon and target. Returns true when a valid option is selected. Call GetSelectedTarget, GetSelectedWeapon, GetSelectedWeaponProperties to get result.
	*/
	proto external bool SelectWeaponAndTarget(notnull array<IEntity> assignedTargets, float assignedTargetsScoreIncrement, float dangerTargetsScoreIncrement, bool useCompartmentWeapons, array<int> weaponTypesWhitelist = null, array<int> weaponTypesBlacklist = null);
	/*!
	Returns selected target
	*/
	proto BaseTarget GetSelectedTarget();
	/*!
	Returns selected weapon, muzzle, magazine
	*/
	proto void GetSelectedWeapon(out BaseWeaponComponent outWeapon, out int outMuzzleId, out BaseMagazineComponent outMagazine);
	/*!
	Return unit types which we can attack based on available weapons. Uses cached value from last SelectWeaponAndTarget call.
	*/
	proto external EAIUnitType GetUnitTypesCanAttack();
	/*!
	Returns properties of selected weapon/magazine
	*/
	proto void GetSelectedWeaponProperties(out float outMinDistance, out float outMaxDistance, out bool outDirectDamage);
	/*!
	Returns most relevant target which we can't attack.
	*/
	proto void GetMostRelevantTargetCantAttack(out BaseTarget target, out float targetScore);
	/*!
	Selects weapon against abstract target of given type. Call GetSelectedWeapon, GetSelectedWeaponProperties to get result.
	*/
	proto external bool SelectWeaponAgainstUnitType(EAIUnitType targetUnitType, bool useCompartmentWeapons);
	/*!
	Checks if character has a wepaon of type. It's a fast check which uses a cache.
	*/
	proto external bool CharacterHasWeaponOfType(EWeaponType weaponType);
	/*!
	Returns amount of magazines for magazine well. Performs a fast lookup from internal cache.
	*/
	proto external int GetCharacterMagazineCount(typename magazineWellType);
	/*!
	Returns score of the target.
	*/
	proto external float CalculateTargetScore(BaseTarget target);
}

/*!
\}
*/
