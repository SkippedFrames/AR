[WorkbenchPluginAttribute(name: "Localize Selected Files", shortcut: "Ctrl+Alt+L", wbModules: {"ResourceManager"}, awesomeFontCode: 0xf1ab)]
class LocParserPlugin: WorkbenchPlugin
{
	[Attribute(defvalue: "{D3EAEB413B6F0AA3}Configs/Workbench/LocParser/Default.conf")]
	protected ResourceName m_ConfigPath;
	
	[Attribute(defvalue: "", params: "st")]
	protected ResourceName m_StringTableOverride;
	
	[Attribute(defvalue: "")]
	protected string m_sPrefixOverride;
	
	protected bool m_bLogOnly;
	
	override void Run()
	{
		string header = "Localize strings in selected files.";
		header += "\nNot all strings will be affected, e.g., texture paths will be ommitted.";
		header += "\nOpen the config to review processing rules.";
		header += "\n\nPress [Log] to find and log all strings which can be localized.";
		header += "\n\nPress [Localize] to add the strings to the string table.";
		header += "\nWARNING: THIS ACTION WILL MODIFY FILES!";
		
		if (Workbench.ScriptDialog("Localize Selected", header, this))
		{
			LocParserManager.Run(m_ConfigPath, m_bLogOnly, m_StringTableOverride, m_sPrefixOverride);
		}
	}
	
	[ButtonAttribute("Log", focused: true)]
	void ButtonLog()
	{
		m_bLogOnly = true;
	}
	
	[ButtonAttribute("Localize")]
	void ButtonLocalize()
	{
		m_bLogOnly = false;
	}
	
	[ButtonAttribute("Cancel")]
	bool ButtonCancel()
	{
		return false;
	}
};
