class SCR_GameLogHelper
{
	//------------------------------------------------------------------------------------------------
	static void LogScenariosConfPaths(int maxWorkshopScenarios = 500)
	{
		array<ResourceName> officialScenarioResourceNames = GetGame().GetDefaultGameConfigs();
		array<MissionWorkshopItem> missionWorkshopItems = {};

		WorkshopApi workshopApi = GetGame().GetBackendApi().GetWorkshop();
		if (workshopApi)
			workshopApi.GetPageScenarios(missionWorkshopItems, 0, maxWorkshopScenarios);

		Print("--------------------------------------------------");
		if (!officialScenarioResourceNames)
		{
			Print("could not get official scenarios conf paths", LogLevel.WARNING);
		}
		else
		{
			BaseContainer container;
			SCR_MissionHeader missionHeader;
			string missionName;
			PrintFormat("Official scenarios (%1 entries)", officialScenarioResourceNames.Count());
			Print("--------------------------------------------------");
			foreach (ResourceName officialScenarioResourceName : officialScenarioResourceNames)
			{
				missionHeader = SCR_ConfigHelperT<SCR_MissionHeader>.GetConfigObject(officialScenarioResourceName);
				if (missionHeader)
				{
					missionName = missionHeader.m_sName;
				}
				else
				{
					container = SCR_ConfigHelper.GetBaseContainer(officialScenarioResourceName);
					if (container)
						container.Get("m_sName", missionName);
					else
						missionName = "n/a";
				}
				if (missionName.Contains("#"))
					missionName = WidgetManager.Translate(missionName);
				PrintFormat("%1 (%2)", officialScenarioResourceName, missionName);
			}
		}
		Print("--------------------------------------------------");

		if (!workshopApi)
		{
			Print("could not get Workshop API to read mod scenarios conf paths", LogLevel.WARNING);
			Print("--------------------------------------------------");
			return;
		}

		if (missionWorkshopItems.IsEmpty())
			return;

		PrintFormat("Workshop scenarios (%1 entries)", missionWorkshopItems.Count() - officialScenarioResourceNames.Count());
		Print("--------------------------------------------------");
		foreach (MissionWorkshopItem mission : missionWorkshopItems)
		{
			// mods cannot be loaded here, so ResourceName will do
			if (mission.GetOwner())
				Print(mission.Id());
		}
		Print("--------------------------------------------------");
	}
};
