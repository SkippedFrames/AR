enum EEditableEntityBudget
{
	PROPS, ///< All objects without special functionality, e.g., chairs, rocks, houses, etc.
	AI, ///< Entityes controlled by AI agent
	VEHICLES, ///< Vehicles and static guns
	SYSTEMS, ///< Objects that provide special functionality that's anticipated to be resource intensive, e.g., spawn points, artillery strikes, etc.
	
	CAMPAIGN, ///< Entities that can be constructed in Conflict game mode
	RANK_CORPORAL, ///< Conflict building mode rank limitation
	RANK_SERGEANT ///< Conflict building mode rank limitation
};
