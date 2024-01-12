class SCR_MissionHeaderCampaign : SCR_MissionHeader
{
	[Attribute("-1", UIWidgets.EditBox, "How many control points are required to win (override, -1 for default)")]
	int m_iControlPointsCap;
	
	[Attribute("-1", UIWidgets.EditBox, "How long a faction needs to hold the control points, in seconds (override, -1 for default)")]
	float m_fVictoryTimeout;
	
	[Attribute("-1", UIWidgets.EditBox, "Maximum amount of active respawn radios (override, -1 for default)")]
	int m_iMaximumRespawnRadios;

	[Attribute("-1", UIWidgets.EditBox, "How much supplies should the main HQ start with")]
	int m_iStartingHQSupplies;
	
	[Attribute("-1", UIWidgets.EditBox, "Minimum starting amount of supplies in small bases (override, -1 for default)")]
	int m_iMinimumBaseSupplies;
	
	[Attribute("-1", UIWidgets.EditBox, "Maximum starting amount of supplies in small bases (override, -1 for default")]
	int m_iMaximumBaseSupplies;
	
	[Attribute("0", UIWidgets.CheckBox, "Use custom base list whitelist instead of blacklist")]
	bool m_bCustomBaseWhitelist;
	
	[Attribute("0", UIWidgets.CheckBox, "Disable rank requirements for spawning vehicles ")]
	bool m_bIgnoreMinimumVehicleRank;
	
	[Attribute()]
	ref array<ref SCR_CampaignCustomBase> m_aCampaignCustomBaseList;
};
