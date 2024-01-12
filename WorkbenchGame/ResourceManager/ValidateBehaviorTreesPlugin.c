[WorkbenchPluginAttribute(
	name: "Validate Behavior Trees",
	description: "Opens all behavior trees so that you can verify them for errors",
	wbModules: {"ResourceManager"},
	shortcut: "",
	icon: "",
	category: "")]
class ValidateBehaviorTreesPlugin: WorkbenchPlugin
{
	override void Run()
	{
		Print("ValidateBehaviorTreesPlugin: Start");
		
		array<string> files = {};
		System.FindFiles(files.Insert, "AI/", ".bt");
		
		foreach (string file : files)
		{
			Print(string.Format("Opening: %1", file));
			Workbench.OpenResource(file);
		}
		
		Print("ValidateBehaviorTreesPlugin: End");
	}
	
	void FindFilesBackend(string fileName, FileAttribute attributes = 0, string filesystem = string.Empty)
	{
		Print(string.Format("%1 %2", fileName, filesystem));
	}
};
