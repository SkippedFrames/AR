[WorkbenchPluginAttribute(name: "Generate Image Links", shortcut: "", wbModules: {"LocalizationEditor"}, awesomeFontCode: 0xf1b2)]
class LocLinkImagePlugin: LocalizationEditorPlugin
{
	[Attribute(defvalue: "https://bohemiainteractive.sharepoint.com/:i:/r/sites/reforger/translations/Images/", desc: "Internet path added in front of the image file.")]
	protected string m_Path;
	
	[Attribute(defvalue: "png", desc: "Image extensions which will replace *.edds")]
	protected string m_Extension;
	
	override void OnChange(BaseContainer stringTableItem, string propName, string propValue)
	{
		if (propName != "SourceFile")
			return;
		
		LocalizationEditor locEditor = Workbench.GetModule(LocalizationEditor);
		if (!locEditor)
			return;
		
		UpdateItem(stringTableItem, locEditor);
	}
	override void Run()
	{
		if (!Workbench.ScriptDialog("Generate Image Links", "Set 'Image Link' attribute for all selected items based on their 'Source File' prefab.\n\nCurrently it recognizes images used by in-game editor.", this))
			return;
		
		LocalizationEditor locEditor = Workbench.GetModule(LocalizationEditor);
		locEditor.BeginModify("LocLinkImagePlugin");
		BaseContainerList items = locEditor.GetTable().GetObjectArray("Items");
		
		WBProgressDialog progress = new WBProgressDialog("Processing...", locEditor);
		
		int linkedCount = 0;
		array<int> indexes = new array<int>();
		locEditor.GetSelectedRows(indexes);
		BaseContainer item;
		for (int i = 0, count = indexes.Count(); i < count; i++)
		{
			item = items.Get(indexes[i]);
			if (UpdateItem(item, locEditor))
				linkedCount++;
						
			progress.SetProgress(i / count);
		}
		locEditor.EndModify();
		progress = null;
		
		Workbench.ScriptDialog("Generate Image Links", string.Format("%1 key(s) processed, %2 linked.", indexes.Count(), linkedCount), new LocLinkImagePluginResult);
		
	}
	protected bool UpdateItem(BaseContainer item, LocalizationEditor locEditor)
	{
		ResourceName sourceFile;
		item.Get("SourceFile", sourceFile);
		if (!sourceFile.IsEmpty())
		{
			Resource resource = Resource.Load(sourceFile);
			if (resource && resource.IsValid())
			{
				IEntityComponentSource component = SCR_BaseContainerTools.FindComponentSource(resource, SCR_EditableEntityComponent);
				if (component)
				{
					SCR_EditableEntityUIInfo info = SCR_EditableEntityUIInfo.Cast(SCR_EditableEntityComponentClass.GetInfo(component));
					if (info)
					{
						string image = info.GetImage();
						if (!image.IsEmpty())
						{
							image = FilePath.ReplaceExtension(image, m_Extension);
							image = m_Path + FilePath.StripPath(image);
							locEditor.ModifyProperty(item, item.GetVarIndex("ImageLink"), image);
							return true;
						}
					}
				}
			}
			else
			{
				string id;
				item.Get("Id", id);
				Print(string.Format("Unable to load source file '%1' of '%2'!", sourceFile, id), LogLevel.WARNING);
			}
		}
		
		//--- Set to empty only if the variable is already defined (i.e., don't *create* empty entries)
		if (item.IsVariableSetDirectly("ImageLink"))
			locEditor.ModifyProperty(item, item.GetVarIndex("ImageLink"), "");
		
		return false;
	}
	
	[ButtonAttribute("Run")]
	bool ButtonRun()
	{
		return true;
	}
	[ButtonAttribute("Close")]
	bool ButtonClose()
	{
		return false;
	}
};
class LocLinkImagePluginResult
{
	[ButtonAttribute("Close")]
	bool ButtonClose()
	{
		return false;
	}
};
