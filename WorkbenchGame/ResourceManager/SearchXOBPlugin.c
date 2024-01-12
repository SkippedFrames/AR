enum ESearchType
{
	ListAllXOBs,
	LoadAllXOBs,
	InvalidMask,
	LayerPreset,
	LayerMask
};

enum ELayerFilter
{
	Any,
	All,
	Exact,
	None
};

class SearchFunctor
{
	protected ref array<ResourceName> m_files = new array<ResourceName>;
	protected int m_counter = 0;
	protected bool m_printFiles = false;
	protected bool m_ignoreNoColliders = false;
	protected ResourceName m_searchFolder;
	protected int m_searchAddon;
	
	void SearchFunctor(bool printFiles, bool ignoreNoColliders, ResourceName SearchFolder, int SearchAddon)
	{
		m_printFiles = printFiles;
		m_ignoreNoColliders = ignoreNoColliders;
		m_searchFolder = SearchFolder;
		m_searchAddon = SearchAddon;
	}
	
	void Search()
	{
		WBProgressDialog progress = new WBProgressDialog("Searching ...", null);
		progress.SetProgress(0);
		
		PrintHeader();
		
		string addon = SCR_AddonTool.GetAddonIndex(m_searchAddon);
		addon = SCR_AddonTool.ToFileSystem(addon);
		
		Workbench.SearchResources(Find, {"xob"},null, addon + m_searchFolder.GetPath());
		int count = m_files.Count();
		int updateMask = SCR_Math.IntegerMask(count / 10);
		
		foreach (int i, ResourceName file : m_files)
		{
			ProcessFile(file);
			
			if ((i & updateMask) == updateMask)
			{
				progress.SetProgress(i / count);
			}
		}
		
		PrintResults();
		m_files.Clear();
	}
	
	protected void PrintHeader();
	protected void ProcessFile(ResourceName file);
	protected void PrintResults();	
	
	private void Find(ResourceName resName)
	{
		m_files.Insert(resName);
	}
};

class ListAllFunctor : SearchFunctor
{
	override void PrintHeader()
	{
		Print("--- List All XOBs --------------------------------------------------------------");
	}
	
	override void ProcessFile(ResourceName file)
	{
		if (m_printFiles)		
			Print(file);
	}
	
	override void PrintResults()
	{
		Print("Total: " + m_files.Count());
	}
};

class LoadAllFunctor : SearchFunctor
{
	override void PrintHeader()
	{
		Print("--- Load All XOBs --------------------------------------------------------------");
	}
	
	override void ProcessFile(ResourceName file)
	{
		Resource res = Resource.Load(file);
		MeshObject mesh = res.GetResource().ToMeshObject();
		if (mesh)
		{
			++m_counter;
		}
	}
	
	override void PrintResults()
	{
		Print("Loaded: " + m_counter);
		Print("Total:  " + m_files.Count());
	}
};

class InvalidMaskFunctor : SearchFunctor
{
	override void PrintHeader()
	{
		Print("--- Find Invalid Mask ----------------------------------------------------------");
	}
	
	override void ProcessFile(ResourceName file)
	{
		Resource res = Resource.Load(file);
		MeshObject mesh = res.GetResource().ToMeshObject();
		if (m_ignoreNoColliders && mesh.GetNumGeoms() == 0)
			return;
		
		if (!mesh.HasValidMask())
		{
			++m_counter;
			if (m_printFiles)
				Print(string.Format("@\"%1\"", file.GetPath()));
		}
	}
	
	override void PrintResults()
	{
		Print("Found: " + m_counter);
		Print("Total: " + m_files.Count());
	}
};

class LayerMaskFunctor : SearchFunctor
{
	int m_layerMask = 0;
	ELayerFilter m_layerFilter = ELayerFilter.Any;
	
	void LayerMaskFunctor(bool printFiles, bool ignoreNoColliders, ResourceName SearchFolder, int SearchAddon, int layerMask, ELayerFilter layerFilter)
	{
		m_layerMask = layerMask;
		m_layerFilter = layerFilter;
	}
	
	override void PrintHeader()
	{
		Print("--- Find Layer Mask ------------------------------------------------------------");
	}
	
	override void ProcessFile(ResourceName file)
	{
		Resource res = Resource.Load(file);
		MeshObject mesh = res.GetResource().ToMeshObject();
		if (m_ignoreNoColliders && mesh.GetNumGeoms() == 0)
			return;
		
		if (mesh.HasLayerMask(m_layerMask, m_layerFilter))
		{
			++m_counter;
			if (m_printFiles)
				Print(string.Format("@\"%1\"", file.GetPath()));
		}
	}
	
	override void PrintResults()
	{
		Print("Found: " + m_counter);
		Print("Total: " + m_files.Count());
	}
};

[WorkbenchPluginAttribute("Search Tool (XOBs)", "Searches XOB files", "", "", {"ResourceManager"},"",0xf002)]
class SearchXOBPlugin: WorkbenchPlugin
{
	[Attribute("0", UIWidgets.ComboBox, "Type of search", "", ParamEnumArray.FromEnum(ESearchType))]
	private ESearchType SearchType;
	
	[Attribute("1", UIWidgets.ComboBox, "In which addon should be search performed", "",ParamEnumAddons.FromEnum())]
	int SearchAddon;
	
	[Attribute(defvalue: "", desc: "Folder where to perform search. If empty, search is performed everywhere", params: "unregFolders")]
	private ResourceName SearchFolder;
	
	[Attribute("0", UIWidgets.ComboBox, "LayerPreset search parameter", "", ParamEnumArray.FromEnum(EPhysicsLayerPresets))]
	private EPhysicsLayerPresets LayerPreset;
	
	[Attribute("0", UIWidgets.ComboBox, "LayerMask search parameter", "", ParamEnumArray.FromEnum(EPhysicsLayerDefs))]
	private ref array<EPhysicsLayerDefs> LayerDefs;
	
	[Attribute("0", UIWidgets.ComboBox, "Filter applied to layer mask", "", ParamEnumArray.FromEnum(ELayerFilter))]
	private ELayerFilter LayerFilter;
	
	[Attribute(defvalue: "0", desc: "Print all found files")]
	private bool PrintFiles;
	
	[Attribute(defvalue: "0", desc: "Ignore files with no physics geometry")]
	private bool IgnoreNoColliders;
	
	override void Run()
	{
		if (Workbench.ScriptDialog("Search XOBs", "Choose search type, set search parameters ...", this))
		{
			SearchFunctor functor = CreateFunctor();
			functor.Search();
		}
	}
	
	override void RunCommandline() 
	{
		ResourceManager module = Workbench.GetModule(ResourceManager);
		module = Workbench.GetModule(ResourceManager);
		
		Print("Command-line functionality not implemented");
		
		//SearchFunctor functor = CreateFunctor();
		//functor.Search();		

		Workbench.Exit(0);
	}
	
	[ButtonAttribute("Run")]
	bool OK()
	{
		return true;
	}
	
	[ButtonAttribute("Cancel")]
	bool Cancel()
	{
		return false;
	}
	
	private int GetLayerMask()
	{
		int mask = 0;
		foreach (int layer : LayerDefs)
		{
			mask |= layer;
		}
		return mask;
	}
	
	private SearchFunctor CreateFunctor()
	{
		SearchFunctor functor;
		switch(SearchType)
		{
			case ESearchType.ListAllXOBs:
				functor = new ListAllFunctor(PrintFiles, IgnoreNoColliders, SearchFolder, SearchAddon);
			break;
			case ESearchType.LoadAllXOBs:
				functor = new LoadAllFunctor(PrintFiles, IgnoreNoColliders, SearchFolder, SearchAddon);
			break;
			case ESearchType.InvalidMask:
				functor = new InvalidMaskFunctor(PrintFiles, IgnoreNoColliders, SearchFolder, SearchAddon);
			break;
			case ESearchType.LayerPreset:
				functor = new LayerMaskFunctor(PrintFiles, IgnoreNoColliders, SearchFolder, SearchAddon, LayerPreset, LayerFilter);
			break;
			case ESearchType.LayerMask:
				functor = new LayerMaskFunctor(PrintFiles, IgnoreNoColliders, SearchFolder, SearchAddon, GetLayerMask(), LayerFilter);
			break;
			default:
				functor = new SearchFunctor(PrintFiles, IgnoreNoColliders, SearchFolder, SearchAddon);
			break;
		}
		return functor;
	}
};
