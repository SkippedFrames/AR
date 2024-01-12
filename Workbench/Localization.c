// Script File
[WorkbenchPluginAttribute(name: "LocModifiedPlugin", wbModules: {"LocalizationEditor"})]
class LocModifiedPlugin: LocalizationEditorPlugin
{
	override void OnChange(BaseContainer stringTableItem, string propName, string propValue) 
	{
		LocalizationEditor editor = Workbench.GetModule(LocalizationEditor);
		editor.ModifyProperty(stringTableItem, stringTableItem.GetVarIndex("Modified"), Workbench.GetPackedUtcTime().ToString());
		
		string userName;
		Workbench.GetUserName(userName);
		
		if (propName == "Id")
		{
			editor.ModifyProperty(stringTableItem, stringTableItem.GetVarIndex("Author"), userName);
		}

		editor.ModifyProperty(stringTableItem, stringTableItem.GetVarIndex("LastChanged"), userName);
	}
	
	override void OnImport(BaseContainer newItem, BaseContainer oldItem) 
	{
		newItem.Set("Modified", Workbench.GetPackedUtcTime());
		
		string userName;
		Workbench.GetUserName(userName);
		
		if (oldItem)
			newItem.Set("LastChanged", userName);
		else
			newItem.Set("Author", userName);
	}
};

[WorkbenchPluginAttribute(name: "LocExportPlugin", wbModules: {"LocalizationEditor"})]
class LocExportPlugin: LocalizationEditorPlugin
{
	override string GetExportColumn(BaseContainer item, string languageCode)
	{
#ifndef LOCALIZATION_BUILD_HIDDEN		
		bool hidden = false;
		item.Get("Hidden", hidden);
		
		if (hidden)
			return ""; // do not export this item
#endif		
		if (languageCode == "en_us")
		{
			string edited;
			item.Get("Target_en_us_edited", edited);
			
			if (!edited.IsEmpty())
				return "Target_en_us_edited";
		}
		else
		{
			string translation;
			item.Get("Target_"+languageCode, translation);
			if (translation.IsEmpty())
				return "Target_en_us"; 
		}
		
		return "Target_" + languageCode;
	}
};

/*[WorkbenchPluginAttribute(name: "LocTestPlugin", wbModules: {"LocalizationEditor"})]
class LocTestPlugin: LocalizationEditorPlugin
{
	override void Run()
	{
		LocalizationEditor editor = Workbench.GetModule(LocalizationEditor);
		editor.AddUserFilter({1,2,3,4,5}, "Test");
	}
};*/

/* Obsolete, handled by status now
[WorkbenchPluginAttribute(name: "Lock", shortcut: "Ctrl+L", wbModules: {"LocalizationEditor"})]
class LocLockPlugin: LocalizationEditorPlugin
{
	override void Run()
	{
		LocalizationEditor editor = Workbench.GetModule(LocalizationEditor);
		BaseContainer stringTableSrc = editor.GetTable();
		array<int> indices = new array<int>;
		string id;
		BaseContainerList rows = stringTableSrc.GetObjectArray("Items");
		
		editor.GetSelectedRows(indices);
		if (indices.Count() == 0)
		{
			editor.GetFilteredRows(indices);
		}
		
		if (indices.Count())
		{
			editor.BeginModify("Lock");
			
			foreach (int idx: indices)
			{
				BaseContainer row = rows.Get(idx);
				int lockPropIndex = row.GetVarIndex("Locked");
				row.Get("Id", id);	
				Print("LocalisationEditor: Locking \'" + id + "\'", LogLevel.VERBOSE);
				editor.ModifyProperty(row, lockPropIndex, "1");
			}
			
			editor.EndModify();
		}
	}
	
	override bool IsReadOnly(BaseContainer item)
	{
		bool locked = false;
		item.Get("Locked", locked);
		return locked;
	}
};

[WorkbenchPluginAttribute(name: "Unlock", shortcut: "Ctrl+U", wbModules: {"LocalizationEditor"})]
class LocUnlockPlugin: LocalizationEditorPlugin
{
	override void Run()
	{
		LocalizationEditor editor = Workbench.GetModule(LocalizationEditor);
		BaseContainer stringTableSrc = editor.GetTable();
		array<int> indices = new array<int>;
		string id;
		BaseContainerList rows = stringTableSrc.GetObjectArray("Items");
		
		editor.GetSelectedRows(indices);
		if (indices.Count() == 0)
		{
			editor.GetFilteredRows(indices);
		}
		
		if (indices.Count())
		{
			editor.BeginModify("Lock");
			
			foreach (int idx: indices)
			{
				BaseContainer row = rows.Get(idx);
				int lockPropIndex = row.GetVarIndex("Locked");
				row.Get("Id", id);	
				Print("LocalisationEditor: Unlocking \'" + id + "\'", LogLevel.VERBOSE);
				editor.ModifyProperty(row, lockPropIndex, "0");
			}
			
			editor.EndModify();
		}
	}
};
*/

[WorkbenchPluginAttribute(name: "Check length", wbModules: {"LocalizationEditor"})]
class LocLengthPlugin: LocalizationEditorPlugin
{
	override void Run()
	{
		LocalizationEditor editor = Workbench.GetModule(LocalizationEditor);
		BaseContainer stringTableSrc = editor.GetTable();
		if (!stringTableSrc)
		{
			return;
		}
		array<int> indices = new array<int>;
		array<string> targets;
		string id;
		BaseContainerList rows = stringTableSrc.GetObjectArray("Items");
		int count = rows.Count();
		
		for (int iRow = 0; iRow < count; iRow++)
		{
			BaseContainer row = rows[iRow];
			
			if (!targets)
			{
				targets = new array<string>;
				for (int iCol; iCol < row.GetNumVars(); iCol++)
				{
					string colName = row.GetVarName(iCol);
					if (colName.Contains("Target_"))
					{
						targets.Insert(colName);
					}
				}
			}
			
			string ID;
			row.Get("Id", ID);
			
			foreach (string targetCol: targets)
			{
				int maxLength;
				row.Get("MaxLength", maxLength);
				
				if (maxLength > 0)
				{
					string targetText;
					row.Get(targetCol, targetText);
					
					if (targetText.Length() > maxLength)
					{
						indices.Insert(iRow);
						PrintFormat("LocLengthPlugin: ID: '%1', %2: '%3' is too long (%4/%5)", ID, targetCol, targetText, targetText.Length(), maxLength);
					}
				}
			}
		}
		
		if (indices.Count())
		{
			editor.AddUserFilter(indices, "Long texts");
		}
	}
};
