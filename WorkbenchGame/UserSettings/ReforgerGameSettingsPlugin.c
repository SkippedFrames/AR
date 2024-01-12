[WorkbenchPluginAttribute(name: "Edit Game Settings", category: "User Settings", wbModules: {"ResourceManager"}, awesomeFontCode: 0xf406)]
class ReforgerGameSettingsPlugin: WorkbenchPlugin
{
	override void Run()
	{
		ResourceManager rm = Workbench.GetModule(ResourceManager);
		rm.SetOpenedResource("$profile:.save/settings/ReforgerGameSettings.conf");
	}
};
