[EntityEditorProps(category: "GameScripted/Building", description: "Component attached to a laout of the composition, holding informations about final composition to be build")]
class SCR_CampaignBuildingLayoutComponentClass : ScriptComponentClass
{

}

//------------------------------------------------------------------------------------------------
class SCR_CampaignBuildingLayoutComponent : ScriptComponent
{
	[RplProp(onRplName: "SpawnPreviewIfBuildingModeOpened")]
	protected int m_iPrefabId = INVALID_PREFAB_ID;

	[RplProp()]
	protected int m_iToBuildValue;

	// The value is float, because GM can set a building progress in percentage and if the value would be an int, the rounding of value is causing a confusion.
	[RplProp()]
	protected float m_fCurrentBuildValue;

	protected SCR_BasePreviewEntity m_PreviewEntity;

	protected const static int EMPTY_BUILDING_VALUE = 0;
	protected static const int INVALID_PREFAB_ID = -1;
	protected ref ScriptInvokerInt m_OnAddBuildingValueInt;
	protected ref ScriptInvokerVoid m_OnAddBuildingValueVoid;
	protected ref ScriptInvokerVoid m_OnCompositionIdSet;

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		GetOnAddBuildingValueInt().Insert(EvaluateBuildingStatus);
		GetOnCompositionIdSet().Insert(SpawnPreviewIfBuildingModeOpened);

		if (!SetOnEditorOpenEvent())
		{
			SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
			if (core)
				core.Event_OnEditorManagerInitOwner.Insert(SetOnEditorOpenEvent);
		}

		GetOnAddBuildingValueVoid().Insert(LockCompositionInteraction);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets the event on editor mode opening.
	bool SetOnEditorOpenEvent()
	{
		SCR_EditorManagerEntity editorManagerEntity = SCR_EditorManagerEntity.GetInstance();
		if (editorManagerEntity)
		{
			editorManagerEntity.GetOnOpened().Insert(OnEditorModeOpen);
			editorManagerEntity.GetOnModeChange().Insert(OnEditorModeChanged);
		}

		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (core)
			core.Event_OnEditorManagerInitOwner.Remove(SetOnEditorOpenEvent);

		return editorManagerEntity;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerInt GetOnAddBuildingValueInt()
	{
		if (!m_OnAddBuildingValueInt)
			m_OnAddBuildingValueInt = new ScriptInvokerInt();

		return m_OnAddBuildingValueInt;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnAddBuildingValueVoid()
	{
		if (!m_OnAddBuildingValueVoid)
			m_OnAddBuildingValueVoid = new ScriptInvokerVoid();

		return m_OnAddBuildingValueVoid;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnCompositionIdSet()
	{
		if (!m_OnCompositionIdSet)
			m_OnCompositionIdSet = new ScriptInvokerVoid();

		return m_OnCompositionIdSet;
	}

	//------------------------------------------------------------------------------------------------
	//! Event called when the editor mode was opened to check if it's free roam mode and if so, spawn the previews of future compositions.
	void OnEditorModeOpen()
	{
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (!editorManager)
			return;

		if (editorManager.GetCurrentMode() == EEditorMode.BUILDING)
			SpawnPreview();
	}

	//------------------------------------------------------------------------------------------------
	//! Event called when the editor mode changed to delete the composition preview.
	void OnEditorModeChanged(SCR_EditorModeEntity currentModeEntity, SCR_EditorModeEntity prevModeEntity)
	{
		if (currentModeEntity && currentModeEntity.GetModeType() == EEditorMode.BUILDING)
			SpawnPreview();
		else
			DeletePreview();
	}

	//------------------------------------------------------------------------------------------------
	// Returns true if the building preview for this layout exists.
	bool HasBuildingPreview()
	{
		return m_PreviewEntity;
	}

	//------------------------------------------------------------------------------------------------
	void EvaluateBuildingStatus(int currentBuildValue)
	{
#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_INSTANT_BUILDING))
		{
			// Calling the method one frame later, otherwise it can causing spawning the same composition twice.
			GetGame().GetCallqueue().CallLater(SpawnComposition, 0, false);
			return;
		}
#endif

		if (currentBuildValue < m_iToBuildValue)
			return;

		SpawnComposition();
		GetOnAddBuildingValueInt().Remove(EvaluateBuildingStatus);
	}

	//------------------------------------------------------------------------------------------------
	// Get the building value that defines how long it takes to build a composition (based on the tool, number of people building it etc.)
	int GetBuildingValue(int prefabID)
	{
		ResourceName resName = GetCompositionResourceName(prefabID);
		if (resName.IsEmpty())
			return EMPTY_BUILDING_VALUE;

		SCR_CampaignBuildingManagerComponent buildingManagerComponent = GetBuildingManagerComponent();
		if (!buildingManagerComponent)
			return EMPTY_BUILDING_VALUE;

		SCR_CampaignBuildingCompositionOutlineManager outlineManager = buildingManagerComponent.GetOutlineManager();
		if (!outlineManager)
			return EMPTY_BUILDING_VALUE;

		return outlineManager.GetCompositionBuildingValue(resName);
	}

	//------------------------------------------------------------------------------------------------
	// Return resource name based on prefab ID.
	protected ResourceName GetCompositionResourceName(int prefabID)
	{
		SCR_CampaignBuildingManagerComponent buildingManagerComponent = GetBuildingManagerComponent();
		if (!buildingManagerComponent)
			return string.Empty;

		return buildingManagerComponent.GetCompositionResourceName(prefabID);
	}

	//------------------------------------------------------------------------------------------------
	//Get BuildingManager component
	protected SCR_CampaignBuildingManagerComponent GetBuildingManagerComponent()
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return null;

		return SCR_CampaignBuildingManagerComponent.Cast(gameMode.FindComponent(SCR_CampaignBuildingManagerComponent));
	}

	//------------------------------------------------------------------------------------------------
	// Save prefab ID of the composition, which is used by a Free Roam mode to know what composition this preview represents.
	void SetPrefabId(int prefabId)
	{
		m_iPrefabId = prefabId;
		m_iToBuildValue = GetBuildingValue(prefabId);

		if (m_OnCompositionIdSet)
			m_OnCompositionIdSet.Invoke(prefabId);

		EvaluateBuildingStatus(m_fCurrentBuildValue);

		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	int GetPrefabId()
	{
		return m_iPrefabId;
	}

	//------------------------------------------------------------------------------------------------
	// Spawns the composition belonging to this layout.
	void SpawnComposition()
	{
		IEntity ent = GetOwner().GetRootParent();
		if (!ent)
			return;

		SCR_EditorLinkComponent linkComponent = SCR_EditorLinkComponent.Cast(ent.FindComponent(SCR_EditorLinkComponent));
		if (!linkComponent)
			return;

		linkComponent.SpawnComposition();

		// Call this as the composition can be spawned directly if the building value is set to zero. Yet we want to lock the composition to disable the interaction.
		LockCompositionInteraction();

		EntitySpawnParams spawnParams = new EntitySpawnParams;
		ent.GetWorldTransform(spawnParams.Transform);

		SCR_EditableEntityComponent entity = SCR_EditableEntityComponent.GetEditableEntity(ent);
		if (entity)
		{
			SCR_EditorPreviewParams params = SCR_EditorPreviewParams.CreateParams(spawnParams.Transform, EEditorTransformVertical.TERRAIN);
			SCR_RefPreviewEntity.SpawnAndApplyReference(entity, params);
		}

		SCR_EntityHelper.DeleteEntityAndChildren(GetOwner());
	}

	//------------------------------------------------------------------------------------------------
	void SpawnPreview()
	{
		if (m_PreviewEntity)
			return;

		IEntity ent = GetOwner().GetRootParent();
		if (!ent)
			return;

		ResourceName resName = GetCompositionResourceName(m_iPrefabId);
		if (resName.IsEmpty())
			return;

		Resource res = Resource.Load(resName);
		if (!res.IsValid())
			return;

		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;

		SCR_CampaignBuildingManagerComponent buildingManagerComponent = SCR_CampaignBuildingManagerComponent.Cast(gameMode.FindComponent(SCR_CampaignBuildingManagerComponent));
		if (!buildingManagerComponent)
			return;

		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		ent.GetWorldTransform(spawnParams.Transform);

		m_PreviewEntity = SCR_PrefabPreviewEntity.SpawnPreviewFromPrefab(res, "SCR_PrefabPreviewEntity", ent.GetWorld(), spawnParams, "{58F07022C12D0CF5}Assets/Editor/PlacingPreview/Preview.emat");
		if (!m_PreviewEntity)
			return;

		m_PreviewEntity.Update();

		ent.AddChild(m_PreviewEntity, 0, EAddChildFlags.AUTO_TRANSFORM | EAddChildFlags.RECALC_LOCAL_TRANSFORM);

		SCR_EditorModeEntity modeEntity = GetBuildingModeEntity();
		if (!modeEntity)
			return;

		modeEntity.GetOnClosed().Insert(DeletePreview);
	}

	//------------------------------------------------------------------------------------------------
	// Spawns the composition preview, if the building mode is active (open)
	void SpawnPreviewIfBuildingModeOpened()
	{
		SCR_EditorModeEntity modeEntity = GetBuildingModeEntity();
		if (!modeEntity)
			return;

		if (modeEntity.IsOpened())
			SpawnPreview();
	}

	//------------------------------------------------------------------------------------------------
	void DeletePreview()
	{
		// if there was open an editor, remove the invoker
		SCR_EditorModeEntity modeEntity = GetBuildingModeEntity();
		if (modeEntity)
			modeEntity.GetOnClosed().Remove(DeletePreview);

		SCR_EntityHelper.DeleteEntityAndChildren(m_PreviewEntity);
	}

	//------------------------------------------------------------------------------------------------
	void AddBuildingValue(int value)
	{
		m_fCurrentBuildValue += value;
		m_fCurrentBuildValue = Math.Clamp(m_fCurrentBuildValue, 0, m_iToBuildValue);

		if (m_OnAddBuildingValueInt)
			m_OnAddBuildingValueInt.Invoke(m_fCurrentBuildValue);

		if (m_OnAddBuildingValueVoid)
			m_OnAddBuildingValueVoid.Invoke();

		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	void SetBuildingValue(float newValue)
	{
		newValue = Math.Clamp(newValue, 0, m_iToBuildValue);
		m_fCurrentBuildValue = newValue;

		if (m_OnAddBuildingValueInt)
			m_OnAddBuildingValueInt.Invoke(m_fCurrentBuildValue);

		if (m_OnAddBuildingValueVoid)
			m_OnAddBuildingValueVoid.Invoke();

		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	// Returns value that needs to be reached to spawn the composition.
	int GetToBuildValue()
	{
		return m_iToBuildValue;
	}

	//------------------------------------------------------------------------------------------------
	// Returns current value
	float GetCurrentBuildValue()
	{
		return m_fCurrentBuildValue;
	}

	//------------------------------------------------------------------------------------------------
	// Try to search for a editor mode entity (exists only when the player is in editor mode)
	SCR_EditorModeEntity GetBuildingModeEntity()
	{
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return null;

		SCR_EditorManagerEntity editorManager = core.GetEditorManager();
		if (!editorManager)
			return null;

		return editorManager.FindModeEntity(EEditorMode.BUILDING);
	}

	//------------------------------------------------------------------------------------------------
	// Once someone for the 1st time add a value to build a composition, set the lock at this composition to disallow any manipulation (deleting, moving) with this composition until it's fully build.
	void LockCompositionInteraction()
	{
		IEntity rootEnt = GetOwner().GetRootParent();
		if (!rootEnt)
			return;

		SCR_CampaignBuildingCompositionComponent compositionComponent = SCR_CampaignBuildingCompositionComponent.Cast(rootEnt.FindComponent(SCR_CampaignBuildingCompositionComponent));
		if (!compositionComponent)
			return;

		compositionComponent.SetInteractionLockServer(true);
		GetOnAddBuildingValueVoid().Remove(LockCompositionInteraction);
	}

	//------------------------------------------------------------------------------------------------
	protected void SnapEntityToTerrain(IEntity entity)
	{
		if (!entity)
			return;
		
		vector transform[4];
		Math3D.MatrixIdentity4(transform);
		
		transform[3] = entity.GetOrigin();
		
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return;
		
		transform[3][1] = world.GetSurfaceY(transform[3][0], transform[3][2]);
		entity.SetTransform(transform);
	}

	//------------------------------------------------------------------------------------------------
	override void OnChildAdded(IEntity parent, IEntity child)
	{
		super.OnChildAdded(parent, child);
		
		// child local transform is applied one frame later on server so snapping to terrain needs to be delayed by one frame
		RplComponent rplComp = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (!rplComp || rplComp.IsProxy())
			SnapEntityToTerrain(child);
		else
			GetGame().GetCallqueue().CallLater(SnapEntityToTerrain, 0, false, child);
	}

	//------------------------------------------------------------------------------------------------
	// Destructor
	void ~SCR_CampaignBuildingLayoutComponent()
	{
		// Delete preview, when the layout was removed (composition build, layout deleted from editor mode, etc.);
		DeletePreview();
	}
}
