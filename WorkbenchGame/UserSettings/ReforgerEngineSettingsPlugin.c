[WorkbenchPluginAttribute(name: "Edit Engine Settings", category: "User Settings", wbModules: {"ResourceManager"}, awesomeFontCode: 0xf4fe)]
class ReforgerEngineSettingsPlugin: WorkbenchPlugin
{
	override void Run()
	{
		ResourceManager rm = Workbench.GetModule(ResourceManager);
		rm.SetOpenedResource("$profile:.save/settings/ReforgerEngineSettings.conf");
	}
};
