[ComponentEditorProps(category: "GameScripted/Editor", description: "")]
class SCR_EditorLinkComponentClass : ScriptComponentClass
{
	[Attribute()]
	ref array<ref SCR_EditorLinkEntry> m_aEntries;
}

/** @ingroup Editable_Entities
*/

/*!
Link which creates replicated entity. To be use in composition prefabs which cannot contain nested replicated entities;
*/
class SCR_EditorLinkComponent : ScriptComponent
{
	protected static bool s_bIgnoreSpawning;
	
	protected bool m_bIsSpawned;
	protected ref ScriptInvoker m_OnLinkedEntitiesSpawned;

	/*!
	\return Event called after all linked entities were spawned
	*/
	ScriptInvoker GetOnLinkedEntitiesSpawned()
	{
		if (!m_OnLinkedEntitiesSpawned)
			m_OnLinkedEntitiesSpawned = new ScriptInvoker();

		return m_OnLinkedEntitiesSpawned;
	}
	
	/*!
	\return true if spawning is set to be ignored
	*/
	bool IsSpawningIgnored()
	{
		return s_bIgnoreSpawning;
	}
	
	/*!
	Ignore spawning entity links in the next spawned prefab with this component.
	Used when some other system handles spawning independently.
	Has to be set before spawning the prefab, is reset to false afterwards.
	\param ignore True to ignore spawning
	*/
	static void IgnoreSpawning(bool ignore)
	{
		s_bIgnoreSpawning = ignore;
	}
	
	/*!
	Check if linked children of editable entity were spawned.
	\param entity Editable entity
	\return True if all linked children were spawned
	*/
	static bool IsSpawned(SCR_EditableEntityComponent entity)
	{
		SCR_EditorLinkComponent linkComponent = SCR_EditorLinkComponent.Cast(entity.GetOwner().FindComponent(SCR_EditorLinkComponent));
		return linkComponent && linkComponent.IsSpawned();
	}
	
	/*!
	\return True if all linked children were spawned
	*/
	bool IsSpawned()
	{
		return m_bIsSpawned;
	}

	override void EOnInit(IEntity owner)
	{
		if (s_bIgnoreSpawning)
		{
			s_bIgnoreSpawning = false;
			return;
		}

		SpawnComposition();
	}

	override void OnPostInit(IEntity owner)
	{
		if ((Replication.IsClient() && !Replication.Loadtime()) || SCR_Global.IsEditMode(owner))
			return;

		SCR_EditableEntityComponent parent = SCR_EditableEntityComponent.GetEditableEntity(owner);
		if (!parent)
			return;

		SetEventMask(owner, EntityEvent.INIT);
	}

	void SpawnComposition()
	{
		if ((Replication.IsClient() && !Replication.Loadtime()) || SCR_Global.IsEditMode(GetOwner()))
			return;

		SCR_EditorLinkComponentClass prefabData = SCR_EditorLinkComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData)
			return;

		Game game = GetGame();
		BaseWorld world = GetOwner().GetWorld();

		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.LOCAL;
		spawnParams.Parent = GetOwner();

		foreach (SCR_EditorLinkEntry entry : prefabData.m_aEntries)
		{
			entry.SetSpawnParams(spawnParams);
			if (!game.SpawnEntityPrefab(Resource.Load(entry.m_Prefab), world, spawnParams))
				Print(string.Format("Unable to spawn linked entity @\"%1\"!", entry.m_Prefab.GetPath()), LogLevel.WARNING);
		}
		
		m_bIsSpawned = true;

		if (m_OnLinkedEntitiesSpawned)
			m_OnLinkedEntitiesSpawned.Invoke(this);
	}
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_Prefab", true)]
class SCR_EditorLinkEntry
{
	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, params: "et")]
	ResourceName m_Prefab;

	[Attribute()]
	vector m_vPosition;

	[Attribute()]
	vector m_vAngles;

	[Attribute(defvalue: "1")]
	float m_fScale;

	void SetSpawnParams(out EntitySpawnParams spawnParams)
	{
		Math3D.AnglesToMatrix(Vector(m_vAngles[1], m_vAngles[0], m_vAngles[2]), spawnParams.Transform);
		spawnParams.Scale = m_fScale;
		spawnParams.Transform[3] = m_vPosition;
	}

	void SCR_EditorLinkEntry(ResourceName prefab, vector position, vector angles, float scale)
	{
		if (!prefab.IsEmpty())
		{
			m_Prefab = prefab;
			m_vPosition = position;
			m_vAngles = angles;
			m_fScale = scale;
		}
	}
}
