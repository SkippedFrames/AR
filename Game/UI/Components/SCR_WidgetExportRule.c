/*
This class provides extra rules for the "Generate class from layout" plugin (check GenerateLayoutClassPlugin.c)


!! Getters must be updated if variable names are changed.
!! FindInWidgetSource must be updated if variable name is changed.

Author: Saveliy Tronza September 2021
*/

class SCR_WidgetExportRule : ScriptedWidgetComponent
{
	// Variables and methods of this class are only needed in the workbench during export
	
//#ifdef WORKBENCH

	[Attribute("true", UIWidgets.CheckBox, "Export this widget and its components")]
	bool ExportThisWidget;
	
	[Attribute("true", UIWidgets.CheckBox, "Export children of this widget")]
	bool ExportChildWidgets;
	
	[Attribute("", UIWidgets.EditBox, "Variable name for this widget. If not provided, it will be deduced from widget name.")]
	string WidgetVariableName;
	
	
	
	//----------------------------------------------------------------------------
	static BaseContainer FindInWidgetSource(WidgetSource ws)
	{
		ref BaseContainerList components = ws.GetObjectArray("components");
		
		for (int i = 0; i < components.Count(); i++)
		{
			BaseContainer comp = components.Get(i);
			string compClassName = comp.GetClassName();
			
			if (compClassName == "SCR_WidgetExportRule") // !!!! Update this if class name changes !!!!
			{
				return comp;
			}
		}
		
		return null;
	}
	
	
	// ----------- Getters ----------
	
	
	//----------------------------------------------------------------------------
	static void ExtractVariablesFromBaseContainer(BaseContainer container,
		out bool out_ExportThisWidget,
		out bool out_ExportChildWidgets,
		out string out_WidgetVariableName)
	{
		container.Get("ExportThisWidget", out_ExportThisWidget);
		container.Get("ExportChildWidgets", out_ExportChildWidgets);
		container.Get("WidgetVariableName", out_WidgetVariableName);
	}
	
	//----------------------------------------------------------------------------
	static bool GetExportThisWidget(BaseContainer container)
	{
		bool outVar;
		container.Get("ExportThisWidget", outVar);
		return outVar;
	}
	
	//----------------------------------------------------------------------------
	static bool GetExportChildWidgets(BaseContainer container)
	{
		bool outVar;
		container.Get("ExportChildWidgets", outVar);
		return outVar;
	}
	
	
	//----------------------------------------------------------------------------
	static string GetWidgetVariableName(BaseContainer container)
	{
		string outVar;
		container.Get("WidgetVariableName", outVar);
		return outVar;
	}

//#endif
};


/*
This export rule helps you store export parameters for this file which do not change between exports.
*/

class SCR_WidgetExportRuleRoot : ScriptedWidgetComponent
{
//#ifdef WORKBENCH
	
	[Attribute(defvalue: "", desc: "Folder where the file will be saved", params: "unregFolders", enums: NULL, category: "")]
	ResourceName DestinationPath;
	
	[Attribute("$ArmaReforger:", UIWidgets.EditBox, "File system in which new script will be created.", "")]
	string ScriptAddon;
	
	[Attribute("true", UIWidgets.CheckBox, "When true, widget is resolved with FindWidget() in generated code. When false, widget is resolved with FindAnyWidget().")]
	bool GenerateFullWidgetPath;
	
	[Attribute("SCR_", UIWidgets.EditBox, "Prefix which will be added to the name of the generated class")]
	string ScriptClassNamePrefix;
	
	[Attribute("Widgets", UIWidgets.EditBox, "Suffix which will be added to the name of the generated class")]
	string ScriptClassNameSuffix;
	
	//----------------------------------------------------------------------------
	static BaseContainer FindInWidgetSource(WidgetSource ws)
	{
		ref BaseContainerList components = ws.GetObjectArray("components");
		
		for (int i = 0; i < components.Count(); i++)
		{
			BaseContainer comp = components.Get(i);
			string compClassName = comp.GetClassName();
			
			if (compClassName == "SCR_WidgetExportRuleRoot") // !!!! Update this if class name changes !!!!
			{
				return comp;
			}
		}
		
		return null;
	}

	//----------------------------------------------------------------------------
	static ResourceName GetDestinationPath(BaseContainer container)
	{
		ResourceName outVar;
		container.Get("DestinationPath", outVar);
		return outVar;
	}
	
	//----------------------------------------------------------------------------
	static string GetScriptAddon(BaseContainer container)
	{
		string outVar;
		container.Get("ScriptAddon", outVar);
		return outVar;
	}
	
	//----------------------------------------------------------------------------
	static bool GetGenerateFullWidgetPath(BaseContainer container)
	{
		bool outVar;
		container.Get("GenerateFullWidgetPath", outVar);
		return outVar;
	}
	
	//----------------------------------------------------------------------------
	static void GetClassPrefixAndSuffix(BaseContainer container, out string prefix, out string suffix)
	{
		string _prefix;
		string _suffix;
		container.Get("ScriptClassNamePrefix", _prefix);
		container.Get("ScriptClassNameSuffix", _suffix);
		prefix = _prefix;
		suffix = _suffix;
	}
	
//#endif
};
