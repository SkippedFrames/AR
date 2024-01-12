#ifdef WORKBENCH
[WorkbenchPluginAttribute(
	name: "MultiPhase Destruction Soundless Prefab Search",
	description: "Search Prefabs using multiphase destruction where Material Sound Type is set to NONE",
	wbModules: { "ResourceManager" },
	awesomeFontCode: 0xF7CE)]
class SCR_MultiPhaseDestructionSoundlessPrefabSearchPlugin : WorkbenchPlugin
{
	[Attribute(desc: "Ignore SCR_DestructibleEntity entities that have destruction disabled", category: "Options")]
	protected bool m_bIgnoreDestructibleEntitiesWithDisabledDestruction;

	[Attribute(desc: "Ignore components that are disabled", category: "Options")]
	protected bool m_bIgnoreDisabledComponents;

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		if (!Workbench.ScriptDialog("MultiPhase Destruction Soundless Prefab Search Options", string.Empty, this))
			return;

		DetectPrefabs();
	}

	//------------------------------------------------------------------------------------------------
	protected void DetectPrefabs()
	{
		array<ResourceName> resourceNames = {};

		array<string> addonGUIDs = {};
		GameProject.GetLoadedAddons(addonGUIDs);

		Print("==================================================", LogLevel.NORMAL);
		Print("Looking for Prefabs using NONE as Destruction's Material Sound Type", LogLevel.NORMAL);
		Print("==================================================", LogLevel.NORMAL);
		foreach (string addonGUID : addonGUIDs)
		{
			string addonID = GameProject.GetAddonID(addonGUID);
			Print("Looking into " + addonID + "/Prefabs", LogLevel.NORMAL);
			resourceNames.InsertAll(SCR_WorldEditorToolHelper.SearchWorkbenchResources({ "et" }, null, SCR_AddonTool.ToFileSystem(addonID) + "Prefabs/"));
		}

		BaseContainer baseContainer;
		SCR_EMaterialSoundTypeBreak value;
		bool isComponentEnabled;
		array<ResourceName> results = {};

		foreach (ResourceName resourceName : resourceNames)
		{
			value = -1;

			baseContainer = SCR_ConfigHelper.GetBaseContainer(resourceName);
			if (!baseContainer)
			{
				Print("INVALID PREFAB: error loading " + resourceName, LogLevel.ERROR);
				continue;
			}

			if (baseContainer.GetClassName() == "SCR_DestructibleEntity")
			{
				if (m_bIgnoreDestructibleEntitiesWithDisabledDestruction)
				{
					if (!baseContainer.Get("Enabled", isComponentEnabled) || !isComponentEnabled)
						continue;
				}
			}
			else
			{
				baseContainer = SCR_ConfigHelper.GetBaseContainer(resourceName, "components/SCR_DestructionMultiPhaseComponent");
				if (!baseContainer)
					continue;

				if (m_bIgnoreDisabledComponents)
				{
					if (!baseContainer.Get("Enabled", isComponentEnabled) || !isComponentEnabled)
						continue;
				}
			}

			if (!baseContainer.Get("m_eMaterialSoundType", value))
				continue;

			if (value == SCR_EMaterialSoundTypeBreak.NONE)
				results.Insert(resourceName);
		}

		// second loop to separate from errors when loading problematic Prefabs
		foreach (ResourceName result : results)
		{
			Print("NONE used as Material Sound Type in " + result, LogLevel.WARNING);
		}

		Print(results.Count().ToString() + " Prefabs found using NONE as Destruction's Material Sound Type", LogLevel.NORMAL);
	}

	[ButtonAttribute("Process", true)]
	protected bool ButtonOK()
	{
		return true;
	}

	[ButtonAttribute("Cancel")]
	protected bool ButtonCancel()
	{
		return false;
	}
}
#endif
