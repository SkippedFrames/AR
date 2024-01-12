[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_CommandPrefab", true)]
class SCR_BaseCommandAction : SCR_BaseToggleToolbarAction
{
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Objective or waypoint prefab", "et")]
	protected ResourceName m_CommandPrefab;
	
	protected void FilterEntities(notnull set<SCR_EditableEntityComponent> inEntities, out notnull set<SCR_EditableEntityComponent> outEntities)
	{
		outEntities.Copy(inEntities);
	}
	
	protected set<SCR_EditableEntityComponent> GetSelectedEntities(notnull set<SCR_EditableEntityComponent> inEntities)
	{
		set<SCR_EditableEntityComponent> selectedEnities = new set<SCR_EditableEntityComponent>();
		FilterEntities(inEntities, selectedEnities);
		return selectedEnities;
	}
	
	//--- Called from command toolbar, initiates placing mode
	bool StartPlacing(notnull set<SCR_EditableEntityComponent> selectedEntities)
	{
		SCR_PlacingEditorComponent placingManager = SCR_PlacingEditorComponent.Cast(SCR_PlacingEditorComponent.GetInstance(SCR_PlacingEditorComponent, true));
		if (!placingManager)
		{
			return false;
		}
		return placingManager.SetSelectedPrefab(m_CommandPrefab, recipients: GetSelectedEntities(selectedEntities));
	}
	//--- Called from radial menu, places the command instantly
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags, int param = -1)
	{
		if (cursorWorldPosition == vector.Zero)
			return;
		
		SCR_PlacingEditorComponent placingManager = SCR_PlacingEditorComponent.Cast(SCR_PlacingEditorComponent.GetInstance(SCR_PlacingEditorComponent, true, true));
		if (!placingManager)
			return;
		
		bool isQueue = flags & EEditorCommandActionFlags.IS_QUEUE;
		
		vector transform[4];
		Math3D.MatrixIdentity3(transform);
		transform[3] = cursorWorldPosition;
		SCR_EditorPreviewParams params = SCR_EditorPreviewParams.CreateParams(transform);
		
		placingManager.CreateEntity(m_CommandPrefab, params, !isQueue, false, GetSelectedEntities(selectedEntities));
	}
	/*
	protected SCR_EditableEntityComponent CreatePrefab(vector position, typename expectedType)
	{
		Resource resource = Resource.Load(m_CommandPrefab);
		if (!resource || !resource.IsValid())
		{
			Print(string.Format("Unable to load prefab '%1'!", m_CommandPrefab), LogLevel.ERROR);
			return null;
		}
		
		if (!resource.GetResource().ToEntitySource().GetClassName().ToType().IsInherited(expectedType))
		{
			Print(string.Format("Prefab '%1' must contain entity of type %1!", m_CommandPrefab, expectedType), LogLevel.ERROR);
			return null;
		}
		
		vector transform[4];
		Math3D.MatrixIdentity3(transform);
		transform[3] = position;
		
		return SCR_PlacingEditorComponent.SpawnEntityResource(m_CommandPrefab, transform);
	}
	protected void GetOffsets(int count, out notnull array<vector> outOffsets)
	{
		if (count == 0)
		{
			return;
		}
		if (count == 1)
		{
			outOffsets.Insert(vector.Zero);
			return;
		}
		
		int rowCount = Math.Round(Math.Sqrt(count));
		int columnCount = Math.Ceil(Math.Sqrt(count));
		vector offset = -Vector((rowCount - 1) * m_fSpacing / 2, 0, (columnCount - 1) * m_fSpacing / 2);

		int row, column;
		for (int i = 0; i < count; i++)
		{
			row = Math.Floor(i / columnCount);
			column = i % columnCount;
			outOffsets.Insert(offset + Vector(row * m_fSpacing, 0, column * m_fSpacing));
		}
	}
	*/
	protected void OnCurrentActionChanged()
	{
		SCR_CommandActionsEditorComponent commandActionsManager = SCR_CommandActionsEditorComponent.Cast(SCR_CommandActionsEditorComponent.GetInstance(SCR_CommandActionsEditorComponent));
		if (commandActionsManager)
			Toggle(0, commandActionsManager.IsActionCurrent(this))
	}
	override void Track()
	{
		SCR_CommandActionsEditorComponent commandActionsManager = SCR_CommandActionsEditorComponent.Cast(SCR_CommandActionsEditorComponent.GetInstance(SCR_CommandActionsEditorComponent));
		if (commandActionsManager)
		{
			commandActionsManager.GetOnCurrentActionChanged().Insert(OnCurrentActionChanged);
			OnCurrentActionChanged();
		}
	}
	override void Untrack()
	{
		SCR_CommandActionsEditorComponent commandActionsManager = SCR_CommandActionsEditorComponent.Cast(SCR_CommandActionsEditorComponent.GetInstance(SCR_CommandActionsEditorComponent));
		if (commandActionsManager)
			commandActionsManager.GetOnCurrentActionChanged().Remove(OnCurrentActionChanged);
	}
	override bool IsServer()
	{
		//--- Must be called on client, placing handles server communication itself
		return false;
	}
};
