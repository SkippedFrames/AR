[WorkbenchPluginAttribute(name: "Mark shapes editor only", shortcut: "", wbModules: {"WorldEditor"}, awesomeFontCode: 0xf023)]
class MarkShapesEditorOnlyPlugin : WorkbenchPlugin
{
	ref array<string> m_EditorOnlyGenerators = {
		"ForestGeneratorEntity",
		"PrefabGeneratorEntity",
		"RoadGeneratorEntity",
		"SCR_PowerlineGeneratorEntity",
		"WallGeneratorEntity"
	};

	ref array<ref ContainerIdPathEntry> m_EmptyContainerPath = new array<ref ContainerIdPathEntry>();

	[Attribute("false", UIWidgets.CheckBox)]
	bool ActiveLayerOnly;

	[ButtonAttribute("OK")]
	void OkButton()	{}

	override void Run()
	{
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		if (!worldEditor)
			return;

		WorldEditorAPI api = worldEditor.GetApi();
		if (!api)
			return;

		BaseWorld world = api.GetWorld();
		if (!world)
			return;

		if (!Workbench.ScriptDialog("Configure editor only marking", "", this))
			return;

		if (!api.BeginEntityAction())
			return;

		Print("--- Start marking shapes editor only");

		// Progress dialog
		int approximateCount = api.GetEditorEntityCount();
		WorldEditor we = Workbench.GetModule(WorldEditor);
		WBProgressDialog progress = new WBProgressDialog("Marking", we);

		api.ToggleGeneratorEvents(false);

		EditorEntityIterator iter(api);
		IEntitySource src = iter.GetNext();
		while (src)
		{
			ProcessEntity(api, src);
			src = iter.GetNext();

			progress.SetProgress(iter.GetCurrentIdx() / approximateCount);
		}

		api.ToggleGeneratorEvents(true);
		api.EndEntityAction();

		Print("--- Done marking shapes editor only");
	}

	private void SetEditorOnly(WorldEditorAPI api, IEntitySource src)
	{
		int flags;
		src.Get("Flags", flags);
		if ((flags & EntityFlags.EDITOR_ONLY) == 0)
		{
			flags |= EntityFlags.EDITOR_ONLY;
			api.SetVariableValue(src, m_EmptyContainerPath, "Flags", flags.ToString());
		}
	}

	private void ProcessEntity(WorldEditorAPI api, IEntitySource src)
	{
		if (ActiveLayerOnly)
		{
			int activeLayer = api.GetCurrentEntityLayerId();
			if (src.GetLayerID() != activeLayer)
				return;
		}

		int childCount = src.GetNumChildren();

		if (src.GetClassName() == "PolylineShapeEntity" || src.GetClassName() == "SplineShapeEntity")
		{
			// All shape entites are considered Editor Only by default
			bool shapeEdOnly = true;

			// All children must "support" editor only for the shape to be eligable
			for (int i = 0; i < childCount; ++i)
			{
				// Check if given child falls into the "eligable" category
				bool isEdOnlyGenerator = false;
				IEntitySource childSrc = src.GetChild(i);
				for (int j = 0; j < m_EditorOnlyGenerators.Count(); ++j)
				{
					if (childSrc.GetClassName() == m_EditorOnlyGenerators[j])
					{
						isEdOnlyGenerator = true;
						// Even if the whole shape may not be editor only, the generator itself can
						SetEditorOnly(api, childSrc);
						break;
					}
				}

				// We have some entity which may need the shape in game, let's be conservative and don't mark it ed only
				if (!isEdOnlyGenerator)
				{
					shapeEdOnly = false;
					break;
				}
			}

			if (shapeEdOnly)
				SetEditorOnly(api, src);
		}
		else
		{
			for (int i = 0; i < childCount; ++i)
			{
				ProcessEntity(api, src.GetChild(i));
			}
		}
	}
};
