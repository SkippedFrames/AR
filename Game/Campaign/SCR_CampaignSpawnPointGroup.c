[EntityEditorProps(category: "GameScripted/GameMode", description: "Campaign Spawn point group entity", visible: false)]
class SCR_CampaignSpawnPointGroupClass : SCR_SpawnPointClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignSpawnPointGroup : SCR_SpawnPoint
{
	//------------------------------------------------------------------------------------------------
	override string GetSpawnPointName()
	{
		SCR_CampaignMilitaryBaseComponent base = SCR_CampaignMilitaryBaseComponent.Cast(GetParent().FindComponent(SCR_CampaignMilitaryBaseComponent));
		if (base)
			return base.GetBaseName();

		return "BASED";
	}
};
