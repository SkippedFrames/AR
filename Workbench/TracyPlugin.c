[WorkbenchPluginAttribute("Run Tracy Profiler", "Run Tracy Profiler", "", "", {"ResourceManager", "ScriptEditor", "WorldEditor"}, "", 0x0054)]
class TracyPlugin : WorkbenchPlugin
{
	override void Run()
	{
		string binDir;
		Workbench.GetCwd(binDir);
		Workbench.RunProcess(binDir + "/tracy/Tracy.exe");
	}
};
