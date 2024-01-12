#include "scripts/Game/config.c"
//! Action to unload supplies from a Supply truck in Campaign
// OBSOLETE
class SCR_CampaignLoadSuppliesUserAction : ScriptedUserAction
{
};

enum SCR_CampaignSuppliesInteractionFeedback
{
	DO_NOT_SHOW = 0,
	POSSIBLE = 1,
	EMPTY = 2,
	FULL = 3,
	BASE_ENEMY = 4,
	BASE_FULL = 5,
	BASE_EMPTY = 6
};
