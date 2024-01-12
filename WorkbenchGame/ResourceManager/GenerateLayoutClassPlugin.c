/*!
This plugin generates a scripted class with variables for widgets and code to find widgets by their name.
It also generates code for finding all components of widgets.
*/
[WorkbenchPluginAttribute(name: "Generate Class from Layout", description: "Description", shortcut: "", icon: "", wbModules: {"ResourceManager"}, category: "", awesomeFontCode: 0xf0db)]
class GenerateLayoutClassPlugin : WorkbenchPlugin
{
	protected string m_sScriptClassName;	// Name of generated script class
	protected string m_sFileOutName;		// File name
	protected string m_sDestinationPath;	// Full path with file name
	
	protected bool m_bInitSuccess;
	protected BaseContainer m_RootExportRule;	// Base container of root export rule component

	const string m_sPluginVersion = "0.3.0"; // Version, it will be printed in the generated file too

	[ButtonAttribute("Generate")]
	bool OK()
	{
		if (!m_bInitSuccess)
			return false;
		
		WidgetSource widgetSource = WidgetSource.Cast(Workbench.GetModule(ResourceManager).GetContainer());

		this.Generate(widgetSource, m_sScriptClassName, m_sFileOutName);
		return true;
	}

	[ButtonAttribute("Cancel")]
	bool Cancel()
	{
		return false;
	}

	override void Run()
	{
		const string dlgName = "Generate Class from Layout";
		const string introText = "This plugin autogenerates widget-binding scripts for .layout files.\n\n";
		m_bInitSuccess = false;
		_print("Run()");

		WidgetSource widgetSource = WidgetSource.Cast(Workbench.GetModule(ResourceManager).GetContainer());		
		
		// Bail if we are not in layout editor
		if (!widgetSource)
		{
			Workbench.ScriptDialog(dlgName, introText + "You need to open a .layout file first!", this);
			return;
		}
		
		// Bail if SCR_WidgetExportRuleRoot was not found
		m_RootExportRule = SCR_WidgetExportRuleRoot.FindInWidgetSource(widgetSource);
		if (!m_RootExportRule)
		{
			Workbench.ScriptDialog(dlgName, introText + "You need to attach a SCR_WidgetExportRuleRoot component to root widget of your layout!", this);
			return;
		}
		
		string layoutPath = widgetSource.GetName();
		m_sScriptClassName = GenerateScriptClassName(layoutPath);
		m_sFileOutName = m_sScriptClassName + ".c";
		m_sDestinationPath = ResolveDestinationPath(m_sFileOutName);
		string dialogText = introText;
		dialogText = dialogText + "By default the generator exports widgets and their components\nif widget name starts with 'm_'.\n\n";

		dialogText = dialogText + "Attach SCR_WidgetExportRule component to a widget to alter its export rules\n\n";

		dialogText = dialogText + "- - - - - - - - - - - - - - - - - - - - \n";
		dialogText = dialogText + string.Format("Generated script class name:\n%1\n\n", m_sScriptClassName);

		dialogText = dialogText + string.Format("Generated script file name: \n%1\n\n", m_sFileOutName);
		
		dialogText = dialogText + "WARNING: The destination file will be overriden if already exists.\n\n";

		m_bInitSuccess = true;
		Workbench.ScriptDialog(dlgName, dialogText, this);
		return;
	}

	//! Generates the class name for the generated class
	protected void Generate(WidgetSource widgetSource, string scriptClassName, string fileOutName)
	{
		// Build array of all elements we are going to generate code for
		array<WidgetSource> widgets = new array<WidgetSource>;
		array<ref array<WidgetSource>> paths = new array<ref array<WidgetSource>>;
		BuildWidgetArray(widgetSource, new array<WidgetSource>, widgets, paths);


		// Generate the output class name
		string layoutPath = widgetSource.GetName();
		_print(string.Format("Layout path: %1", layoutPath));
		_print(string.Format("Script class name: %1", scriptClassName));

		// Strings with the generated sections
		string variablesDeclaration;
		string variablesBinding;

		// Generate the code
		_print("Iterating widgets...");
		bool m_bGenerateFullWidgetPath = SCR_WidgetExportRuleRoot.GetGenerateFullWidgetPath(m_RootExportRule);
		for (int widgetId = 0; widgetId < widgets.Count(); widgetId++)
		{
			WidgetSource thisWidget = widgets[widgetId];
			array<WidgetSource> pathToThisWidget = paths[widgetId];

			string wName = GetWidgetName(thisWidget);

			// Issue a warning if widget name is not correct
			if (IsWidgetExportRequired(thisWidget, pathToThisWidget))
			{
				if (ValidateWidget(thisWidget))
				{
					string widgetVariableName = ResolveWidgetVariableName(thisWidget);
					string pathToThisWidgetStr = GetStringPathToWidget(paths[widgetId]);

					_print(string.Format("Exporting widget: %1, %2, %3", wName, widgetVariableName, pathToThisWidgetStr));

					// Declare variable for this widget
					string wClassName = thisWidget.GetClassName();
					wClassName = wClassName.Substring(0, wClassName.Length() - 5); // Remove "Class" from the end
					variablesDeclaration = variablesDeclaration + string.Format("\t%1 %2;\n", wClassName, widgetVariableName);

					// Perform variable binding for this widget
					if (m_bGenerateFullWidgetPath)
						variablesBinding = variablesBinding + string.Format("\t\t%1 = %2.Cast(root.FindWidget(\"%3\"));\n", widgetVariableName, wClassName, pathToThisWidgetStr);
					else
						variablesBinding = variablesBinding + string.Format("\t\t%1 = %2.Cast(root.FindAnyWidget(\"%3\"));\n", widgetVariableName, wClassName, GetWidgetName(thisWidget));
						
					// Declare variables for widget components
					array<BaseContainer> components = ResolveWidgetComponentsForExport(thisWidget);
					for (int i = 0; i < components.Count(); i++)
					{
						BaseContainer comp = components[i];
						string compClassName = comp.GetClassName();
						string compVarName;


						// Component variable name
						if (components.Count() == 1)
							compVarName = string.Format("%1Component", widgetVariableName);
						else
							compVarName = string.Format("%1Component%2", widgetVariableName, i);


						// Variable for component

						variablesDeclaration = variablesDeclaration + string.Format("\t%1 %2;\n", compClassName, compVarName);

						// Variable binding for component
						variablesBinding = variablesBinding + string.Format("\t\t%1 = %2.Cast(%3.FindHandler(%4));\n",
							compVarName, compClassName, widgetVariableName, compClassName);
					}

					variablesDeclaration = variablesDeclaration + "\n";
					variablesBinding = variablesBinding + "\n";
				}
			}
		}

		//_print("Variable declaration:");
		//_print("\n" + variablesDeclaration);

		//_print("Variable binding:");
		//_print("\n" + variablesBinding);


		// Generate whole code
		string gc;

		gc = gc + string.Format("// Autogenerated by the Generate Class from Layout plugin, version %1\n", m_sPluginVersion);
		gc = gc + string.Format("// Layout file: %1\n\n", layoutPath);

		// Generate: class declaration
		gc = gc + string.Format("class %1\n{\n", scriptClassName);

		// Generate: variable with layout path
		gc = gc + string.Format("\tstatic const ResourceName s_sLayout = \"%1\";\n", widgetSource.GetResourceName());
		gc = gc + "\tResourceName GetLayout() { return s_sLayout; }\n\n";

		// Generate: class variables
		gc = gc + variablesDeclaration;

		// Generate: variable binding
		gc = gc + "\tbool Init(Widget root)\n\t{\n";
		gc = gc + variablesBinding;
		gc = gc + "\t\treturn true;\n\t}\n";

		// Close class
		gc = gc + "}\n";

		//_print("Generated code:");
		//_print(gc);
		//_print("\n");

		// Save everything to file
		string fileOutPath = m_sDestinationPath;

		_print(string.Format("Exporting to file: %1", fileOutPath));
		FileHandle f = FileIO.OpenFile(fileOutPath, FileMode.WRITE);
		if (!f)
		{
			_print("Error opening file!");
			return;
		}

		f.Write(gc);
		f.Close();
		_print("Export finished!");

		ScriptEditor scriptEditor = Workbench.GetModule(ScriptEditor);
		if (scriptEditor)
		{
			scriptEditor.SetOpenedResource(fileOutPath);
		}
	}

	// ---------------- Misc functions ---------------

	//! Checks if conditions for widget export are satisfied
	protected bool IsWidgetExportRequired(WidgetSource ws, array<WidgetSource> path)
	{
		// Check if any widgets in the path explicitly disables export
		// Ignore the last widget (this widget), even if its child widgets are not exported,
		// It still can be exported

		for (int i = 0; i < path.Count() - 1; i++)
		{
			WidgetSource pathWs = path[i];
			BaseContainer rule = SCR_WidgetExportRule.FindInWidgetSource(pathWs);
			if (rule)
			{
				bool exportChildren = SCR_WidgetExportRule.GetExportChildWidgets(rule);
				if (!exportChildren)
					return false;
			}
		}




		// Check properties of this widget

		BaseContainer rule = SCR_WidgetExportRule.FindInWidgetSource(ws);
		string wName = GetWidgetName(ws);

		if (wName.StartsWith("m_"))
		{
			if (!rule)
			{
				// Widget name matches pattern, no rule provided - default behaviour - true
				return true;
			}
			else
			{
				// Widget name matches pattern, rule is provided - get value from rule
				return SCR_WidgetExportRule.GetExportThisWidget(rule);
			}
		}
		else
		{
			if (!rule)
			{
				// Widget name doesn't match pattern, no rule provided - default behaviour - false
				return false;
			}
			else
			{
				// Widget name matches pattern, rule is provided - get value from rule
				return SCR_WidgetExportRule.GetExportThisWidget(rule);
			}
		}
	}

	//! Resolves variable name for widget
	protected string ResolveWidgetVariableName(WidgetSource ws)
	{
		BaseContainer rule = SCR_WidgetExportRule.FindInWidgetSource(ws);
		string wName = GetWidgetName(ws);

		if (!rule)
		{
			// No rule, default behaviour - same as widget name
			return wName;
		}
		else
		{
			string wNameFromRule = SCR_WidgetExportRule.GetWidgetVariableName(rule);
			if (!wNameFromRule.IsEmpty())
			{
				return wNameFromRule;
			}
			else
			{
				return wName;
			}
		}
	}

	//! Performs various validations related to exporting of this widget
	protected bool ValidateWidget(WidgetSource ws)
	{
		string wName = GetWidgetName(ws);

		if (wName.Contains(" "))
		{
			_print(string.Format("Widget name contains space: %1", wName), LogLevel.ERROR);
			return false;
		}

		BaseContainer rule = SCR_WidgetExportRule.FindInWidgetSource(ws);
		if (rule)
		{
			string wNameFromRule = SCR_WidgetExportRule.GetWidgetVariableName(rule);

			if (!wNameFromRule.IsEmpty() && wNameFromRule.Contains(" "))
			{
				_print(string.Format("Widget name in the export rule contains space: %1, widget: %2", wNameFromRule, wName), LogLevel.ERROR);
				return false;
			}
		}

		return true;
	}


	//! Generates an array of components which will be exported for this widget
	protected array<BaseContainer> ResolveWidgetComponentsForExport(WidgetSource ws)
	{
		ref BaseContainerList components = ws.GetObjectArray("components");

		array<BaseContainer> outArray = new array<BaseContainer>;

		for (int i = 0; i < components.Count(); i++)
		{
			BaseContainer comp = components.Get(i);
			string compClassName = comp.GetClassName();
			if (compClassName != "SCR_WidgetExportRule" && compClassName != "SCR_WidgetExportRuleRoot") // We don't want to export this one obviously
			{
				outArray.Insert(comp);
			}
		}

		return outArray;
	}


	protected string GenerateScriptClassName(string path)
	{
		int slashId = path.LastIndexOf("/");
		int dotId = path.LastIndexOf(".");
		int cutSize = dotId - slashId - 1;
		string fileNameNoPathNoExt = path.Substring(slashId + 1, cutSize);
		
		string prefix, suffix;
		SCR_WidgetExportRuleRoot.GetClassPrefixAndSuffix(m_RootExportRule, prefix, suffix);
		
		string _out = string.Format("%1%2%3", prefix, fileNameNoPathNoExt, suffix);
		return _out;
	}

	//! Converts a widget tree into a widget array
	//! outArray - each element will contain a WidgetSource
	//! outArrayPaths - each element will contain a path to this widget, including this widget
	protected static void BuildWidgetArray(WidgetSource ws, array<WidgetSource> pathToThis, array<WidgetSource> outArray, array<ref array<WidgetSource>> outArrayPaths)
	{
		array<WidgetSource> fullPathToThis = new array<WidgetSource>;
		fullPathToThis.Copy(pathToThis);
		fullPathToThis.Insert(ws); // Path to this widget also includes this widget

		outArrayPaths.Insert(fullPathToThis);

		outArray.Insert(ws);

		for (int e = 0, count = ws.GetNumChildren(); e < count; e++)
		{
			BuildWidgetArray(ws.GetChild(e), fullPathToThis, outArray, outArrayPaths);
		}
	}

	// Returns path to widget separated with dots
	// First widget is omitted
	// The returned path complies with Widget.FindWidget()
	protected static string GetStringPathToWidget(array<WidgetSource> path)
	{
		int c = path.Count();

		if (c <= 1)
		{
			return "_error_";
		}

		string pathStr = "";

		for (int i = 1; i < c - 1; i++)
		{
			pathStr = pathStr + GetWidgetName(path[i]) + ".";
		}

		pathStr = pathStr + GetWidgetName(path[c - 1]); // Last widget name without a dot

		return pathStr;
	}

	protected static string GetWidgetName(WidgetSource ws)
	{
		string _out;
		ws.Get("Name", _out);
		return _out;
	}

	// Resolves the targer directory where the file must be saved
	protected string ResolveDestinationPath(string fileOutName)
	{
		// First look up the parameters in the root widget
		WidgetSource widgetSource = WidgetSource.Cast(Workbench.GetModule(ResourceManager).GetContainer());

		if (!widgetSource)
			return string.Empty;

		string destinationPath;
		string scriptAddon;

		BaseContainer rule = SCR_WidgetExportRuleRoot.FindInWidgetSource(widgetSource);
		if (rule)
		{
			destinationPath = SCR_WidgetExportRuleRoot.GetDestinationPath(rule);
			scriptAddon = SCR_WidgetExportRuleRoot.GetScriptAddon(rule);
		}

		const string _fixTabs = "{"; // To fix bug in script editor when bracket characters influence tab rules
		int bracketId = destinationPath.LastIndexOf("}");
		string fileOutPath = destinationPath.Substring(bracketId + 1, destinationPath.Length() - bracketId - 1);
		if (!fileOutPath.EndsWith("/"))
			fileOutPath = fileOutPath + "/";
		fileOutPath = fileOutPath + fileOutName;
		fileOutPath = scriptAddon + fileOutPath;

		return fileOutPath;
	}

	protected static void _print(string str, LogLevel logLevel = LogLevel.NORMAL)
	{
		Print(string.Format("[GenerateLayoutClassPlugin] %1", str), logLevel);
	}
};
