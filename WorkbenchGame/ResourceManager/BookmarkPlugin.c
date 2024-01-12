[WorkbenchPluginAttribute(name: "Bookmark 1", category: "Bookmarks", shortcut: "Ctrl+1", icon: "", wbModules: {"ResourceManager"}, awesomeFontCode: 0xf02e)]
class BookmarkPlugin1: WorkbenchPlugin
{
	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail)]
	protected ResourceName m_File;
	
	protected int GetBookmarkIndex()
	{
		return 1;
	}
	
	override void Run()
	{
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		
		array<ResourceName> selection = {};
		resourceManager.GetResourceBrowserSelection(selection.Insert, true);
		
		int action = -1;
		if (selection.IsEmpty() || m_File == selection[0])
			action = 0;
		else if (m_File)
			action = Workbench.ScriptDialog("Open or Replace", string.Format("Do you want to open existing bookmark '%1'\nor replace it by selected file '%2'?", FilePath.StripPath(m_File), FilePath.StripPath(selection[0])), new BookmarkPluginPrompt);
		else
			action = 1;
		
		switch (action)
		{
			case -1:
				return;
			case 0:
				if (m_File)
					resourceManager.SetOpenedResource(m_File);
				break;
			case 1:
				m_File = selection[0];
				PrintFormat("Bookmark %1 now points to @\"%2\"", GetBookmarkIndex(), m_File.GetPath());
				break;
		}
	}
	
	override void Configure()
	{
		Workbench.ScriptDialog(string.Format("Configure 'Bookmark %1' plugin", GetBookmarkIndex()), "Select which file will be opened upon pressing bookmark's shortcut.", this);
	}
	
	[ButtonAttribute("Close")]
	bool Close()
	{
		return true;
	}
};
[WorkbenchPluginAttribute(name: "Bookmark 2", category: "Bookmarks", shortcut: "Ctrl+2", icon: "", wbModules: {"ResourceManager"}, awesomeFontCode: 0xf02e)]
class BookmarkPlugin2: BookmarkPlugin1
{
	override protected int GetBookmarkIndex() { return 2; }
};
[WorkbenchPluginAttribute(name: "Bookmark 3", category: "Bookmarks", shortcut: "Ctrl+3", icon: "", wbModules: {"ResourceManager"}, awesomeFontCode: 0xf02e)]
class BookmarkPlugin3: BookmarkPlugin1
{
	override protected int GetBookmarkIndex() { return 3; }
};
[WorkbenchPluginAttribute(name: "Bookmark 4", category: "Bookmarks", shortcut: "Ctrl+4", icon: "", wbModules: {"ResourceManager"}, awesomeFontCode: 0xf02e)]
class BookmarkPlugin4: BookmarkPlugin1
{
	override protected int GetBookmarkIndex() { return 4; }
};
[WorkbenchPluginAttribute(name: "Bookmark 5", category: "Bookmarks", shortcut: "Ctrl+5", icon: "", wbModules: {"ResourceManager"}, awesomeFontCode: 0xf02e)]
class BookmarkPlugin5: BookmarkPlugin1
{
	override protected int GetBookmarkIndex() { return 5; }
};
[WorkbenchPluginAttribute(name: "Bookmark 6", category: "Bookmarks", shortcut: "Ctrl+6", icon: "", wbModules: {"ResourceManager"}, awesomeFontCode: 0xf02e)]
class BookmarkPlugin6: BookmarkPlugin1
{
	override protected int GetBookmarkIndex() { return 6; }
};
[WorkbenchPluginAttribute(name: "Bookmark 7", category: "Bookmarks", shortcut: "Ctrl+7", icon: "", wbModules: {"ResourceManager"}, awesomeFontCode: 0xf02e)]
class BookmarkPlugin7: BookmarkPlugin1
{
	override protected int GetBookmarkIndex() { return 7; }
};
[WorkbenchPluginAttribute(name: "Bookmark 8", category: "Bookmarks", shortcut: "Ctrl+8", icon: "", wbModules: {"ResourceManager"}, awesomeFontCode: 0xf02e)]
class BookmarkPlugin8: BookmarkPlugin1
{
	override protected int GetBookmarkIndex() { return 8; }
};
[WorkbenchPluginAttribute(name: "Bookmark 9", category: "Bookmarks", shortcut: "Ctrl+9", icon: "", wbModules: {"ResourceManager"}, awesomeFontCode: 0xf02e)]
class BookmarkPlugin9: BookmarkPlugin1
{
	override protected int GetBookmarkIndex() { return 9; }
};
[WorkbenchPluginAttribute(name: "Bookmark 10", category: "Bookmarks", shortcut: "Ctrl+0", icon: "", wbModules: {"ResourceManager"}, awesomeFontCode: 0xf02e)]
class BookmarkPlugin0: BookmarkPlugin1
{
	override protected int GetBookmarkIndex() { return 10; }
};

class BookmarkPluginPrompt
{
	/*
	[ButtonAttribute("Cancel")]
	int Cancel()
	{
		return -1;
	}
	*/
	[ButtonAttribute("Replace")]
	int Replace()
	{
		return 1;
	}
	[ButtonAttribute("Open", true)]
	int Open()
	{
		return 0;
	}
};
