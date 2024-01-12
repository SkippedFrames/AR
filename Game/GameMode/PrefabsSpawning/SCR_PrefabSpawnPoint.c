[EntityEditorProps(style: "cylinder", category: "GameScripted/GameMode/PrefabsSpawn", description: "Prefab Spawn Point Entity", sizeMin: "-0.25 0 -0.25", color: "64 0 64 255")]
class SCR_PrefabSpawnPointClass: GenericEntityClass
{
};

//------------------------------------------------------------------------------------------------
//! Prefab Spawn Point Entity defines position where prefab can be spawned.
class SCR_PrefabSpawnPoint : GenericEntity
{
	[Attribute("0", UIWidgets.EditComboBox, "Defines which spawner uses this Spawn Point", "", ParamEnumArray.FromEnum(EPrefabSpawnType) )]
	protected EPrefabSpawnType m_eType;
	
	[Attribute("0", UIWidgets.CheckBox, "Should be SP snaped in script to ground? -- GetWorld().GetSurfaceY()", "", ParamEnumArray.FromEnum(EPrefabSpawnType) )]
	protected bool m_bSnapToGroud;
	
	//------------------------------------------------------------------------------------------------
	EPrefabSpawnType GetType()
	{
		return m_eType;
	}
	
	//------------------------------------------------------------------------------------------------
	bool ShouldSnapToGound()
	{
		return m_bSnapToGroud;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_PrefabSpawnPoint(IEntitySource src, IEntity parent)
	{
		#ifdef WORKBENCH
			SetFlags(EntityFlags.TRACEABLE, false);
		#else
			SetFlags(EntityFlags.NO_LINK, false);
		#endif
		SCR_PrefabsSpawnerManager.RegisterPrefabSpawnPoint(this);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_PrefabSpawnPoint()
	{
		SCR_PrefabsSpawnerManager.UnregisterPrefabSpawnPoint(this);
	}
};
