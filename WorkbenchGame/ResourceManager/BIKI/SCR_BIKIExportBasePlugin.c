#ifdef WORKBENCH

class SCR_BIKIExportBasePlugin : WorkbenchPlugin
{
	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		Print("The Run method has not been overridden", LogLevel.ERROR);
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowResult(string result)
	{
		Workbench.ScriptDialog(
			"Result to copy/paste",
			"Copy/paste the text below - be sure to expand the field to get all the line returns.",
			new SCR_BIKIExportPlugin_Result(result));
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("OK", true)]
	protected bool ButtonOK()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	protected bool ButtonCancel()
	{
		return false;
	}
}

class SCR_BIKIExportPlugin_Result
{
	[Attribute(uiwidget: UIWidgets.EditBoxMultiline, category: "Result")]
	protected string m_sResult;

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Close", true)]
	protected bool ButtonClose()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_BIKIExportPlugin_Result(string result)
	{
		m_sResult = result;
	}
}

#endif // WORKBENCH
