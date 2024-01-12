// ackchyually, these SVN plugins are VCS plugins (could work with e.g git)

class SCR_SVNPluginBase : WorkbenchPlugin
{
	//------------------------------------------------------------------------------------------------
	protected override void Run()
	{
		ScriptEditor scriptEditor = Workbench.GetModule(ScriptEditor);
		if (!scriptEditor)
			return;

		string file;
		string absPath;
		if (!scriptEditor.GetCurrentFile(file) || !Workbench.GetAbsolutePath(file, absPath))
			return;

		string command = GetCommandLine();
		command.Replace("$path", "\"" + absPath + "\"");
		Workbench.RunCmd(command);
	}

	//------------------------------------------------------------------------------------------------
	protected string GetCommandLine()
	{
		return string.Empty;
	}

}

[WorkbenchPluginAttribute("SVN Log", "Show SVN log of the selected file", "Alt+Shift+L", "", { "ScriptEditor" }, "SVN", 0xF1DE)]
class SCR_SVNLogScriptEditorPlugin : SCR_SVNPluginBase
{
	[Attribute(defvalue: "TortoiseProc /command:log /path:$path")]
	protected string m_sCommandLine;

	//------------------------------------------------------------------------------------------------
	protected override string GetCommandLine()
	{
		return m_sCommandLine;
	}

	//------------------------------------------------------------------------------------------------
	protected override void Configure()
	{
		Workbench.ScriptDialog("Configure SVN Log", "Usage: \n$path - will be replaced with file name", this);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("OK")]
	protected void OkButton()
	{
	}
}

[WorkbenchPluginAttribute("SVN Log", "Show SVN log of the selected file", "Alt+Shift+L", "", { "ResourceManager" }, "SVN", 0xF1DE)]
class SCR_SVNLogResourceManagerPlugin : SCR_SVNLogScriptEditorPlugin
{
	//------------------------------------------------------------------------------------------------
	protected override void Run()
	{
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		if (!resourceManager)
			return;

		array<ResourceName> selection = {};
		SCR_WorkbenchSearchResourcesCallbackArray context = new SCR_WorkbenchSearchResourcesCallbackArray(selection);
		resourceManager.GetResourceBrowserSelection(context.Insert, true);

		foreach (ResourceName resource : selection)
		{
			string absPath;
			if (!Workbench.GetAbsolutePath(resource.GetPath(), absPath))
				continue;

			string command = m_sCommandLine;
			command.Replace("$path", "\""+ absPath + "\"");
			Workbench.RunCmd(command);
		}
	}
}

[WorkbenchPluginAttribute("SVN Diff", "Show local SVN difference of the selected file", "Alt+Shift+I", "", { "ScriptEditor" }, "SVN", 0xF002)]
class SCR_SVNDiffPlugin : SCR_SVNPluginBase
{
	[Attribute(defvalue: "TortoiseProc /command:diff /path:$path")]
	protected string m_sCommandLine;

	//------------------------------------------------------------------------------------------------
	protected override string GetCommandLine()
	{
		return m_sCommandLine;
	}

	//------------------------------------------------------------------------------------------------
	protected override void Configure()
	{
		Workbench.ScriptDialog("Configure SVN Diff", "Usage: \n$path - will be replaced with file name", this);
	}
}

[WorkbenchPluginAttribute("SVN Blame", "Show local SVN difference of the selected file", "Alt+Shift+B", "", { "ScriptEditor" }, "SVN", 0xF4FC)]
class SCR_SVNBlamePlugin : SCR_SVNPluginBase
{
	[Attribute("TortoiseProc /command:blame /path:$path /startrev:1 /endrev:-1 /ignoreeol /ignoreallspaces /line:$line", UIWidgets.EditBox)]
	protected string m_sCommandLine;

	//------------------------------------------------------------------------------------------------
	protected override string GetCommandLine()
	{
		return m_sCommandLine;
	}

	//------------------------------------------------------------------------------------------------
	protected override void Configure()
	{
		Workbench.ScriptDialog("Configure SVN Blame", "Usage: \n$path - will be replaced with file name\n$line - will be replaced with current line number", this);
	}
}
