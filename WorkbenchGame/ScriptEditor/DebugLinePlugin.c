[WorkbenchPluginAttribute(name: "Insert Debug Line", description: "", shortcut: "Ctrl+Shift+D", wbModules: { "ScriptEditor" }, awesomeFontCode: 0xf1cd)]
class DebugLinePlugin: WorkbenchPlugin
{
	[Attribute(defvalue: "DEBUG LINE", desc: "Text shown at the beginning of each debug line.")]
	protected string m_sPrefix;

	[Attribute(desc: "When enabled, debug text will be inserted at the cursor position instead of on a new line.")]
	protected bool m_bInline;

	[Attribute(desc: "When enabled, current script file name will be shown as full path, i.e., including directories.")]
	protected bool m_bShowFullPath;

	[Attribute(desc: "When enabled, debug line will include template for custom variables.")]
	protected bool m_bIncludeParams;

	[Attribute(defvalue: SCR_Enum.GetDefault(LogLevel.DEBUG), uiwidget: UIWidgets.ComboBox, desc: "Log level of the debug line.", enums: ParamEnumArray.FromEnum(LogLevel))]
	protected LogLevel m_LogLevel;

	protected string GetDebugLine(string fileFormat)
	{
		string prefix = m_sPrefix;
		if (!prefix.IsEmpty())
			prefix = "\"" + prefix + " | \" + ";

		string lineText;
		if (m_bIncludeParams)
			lineText = "Print(string.Format(%1" + fileFormat + " + \":" + __LINE__ + " | %%1\", this), LogLevel.%2);";
		else
			lineText = "Print(%1" + fileFormat + " + \":\" + __LINE__, LogLevel.%2);";

		return string.Format(lineText, prefix, typename.EnumToString(LogLevel, m_LogLevel));
	}

	override void Run()
	{
		ScriptEditor scriptEditor = Workbench.GetModule(ScriptEditor);

		string fileFormat;
		if (!scriptEditor.GetCurrentFile(fileFormat))
		{
			Print("Cannot insert debug line, no file is opened!", LogLevel.WARNING);
			return;
		}

		if (m_bShowFullPath)
			fileFormat = "__FILE__";
		else
			fileFormat = "FilePath.StripPath(__FILE__)";

		string lineText;
		scriptEditor.GetLineText(lineText);

		if (m_bInline)
		{
			lineText += GetDebugLine(fileFormat);
			scriptEditor.SetLineText(lineText);
		}
		else
		{
			int tabIndex = lineText.LastIndexOf("	");
			if (lineText.EndsWith("{"))
				tabIndex++;

			string tabPrefix = string.Empty;
			for (int i = 0; i <= tabIndex; i++)
			{
				tabPrefix += "	";
			}

			int nextLine = scriptEditor.GetCurrentLine() + 1;
			scriptEditor.InsertLine(tabPrefix + GetDebugLine(fileFormat), nextLine);
		}
	}

	override void Configure()
	{
		Workbench.ScriptDialog("Configure 'Insert Debug Line' plugin", "", this);
	}

	[ButtonAttribute("Close")]
	bool ButtonClose()
	{
	}
};
