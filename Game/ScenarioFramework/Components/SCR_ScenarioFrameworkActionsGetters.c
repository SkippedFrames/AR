//------------------------------------------------------------------------------------------------
class SCR_ScenarioFrameworkParamBase
{
};

class SCR_ScenarioFrameworkParam<Class T> : SCR_ScenarioFrameworkParamBase
{
	T m_Value;
	
	//------------------------------------------------------------------------------------------------
	T GetValue()
	{
		return m_Value;	
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_ScenarioFrameworkParam(T value)
	{
		m_Value = value;
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGet
{
	protected SCR_ScenarioFrameworkParamBase m_Value;
	
	//------------------------------------------------------------------------------------------------
	protected void SetValue(SCR_ScenarioFrameworkParamBase value) 
	{
		m_Value = value;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ScenarioFrameworkParamBase Get()
	{ 
		return m_Value; 
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity FindEntityByName(string name)
	{
		if (!GetGame().GetWorld())
			return null;
		
		IEntity entity = GetGame().GetWorld().FindEntityByName(name);
		if (!entity)
			return null;
	
		return entity;
	}
};


//------------------------------------------------------------------------------------------------
//[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetLastFinishedTaskLayer : SCR_ScenarioFrameworkGet
{
	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		SCR_GameModeSFManager gameModeManager = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
		if (!gameModeManager)
			return null;
		
		return new SCR_ScenarioFrameworkParam<SCR_ScenarioFrameworkLayerTask>(SCR_ScenarioFrameworkLayerTask.Cast(gameModeManager.GetLastFinishedTaskLayer()));
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetLastFinishedTaskEnity : SCR_ScenarioFrameworkGet
{
	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		SCR_GameModeSFManager gameModeManager = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
		if (!gameModeManager)
			return null;
		
		return new SCR_ScenarioFrameworkParam<IEntity>(gameModeManager.GetLastFinishedTask());
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetEntityByName: SCR_ScenarioFrameworkGet
{
	[Attribute()]
	protected string 		m_sEntityName;
	
	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		if (!GetGame().GetWorld())
			return null;
		
		return new SCR_ScenarioFrameworkParam<IEntity>(GetGame().GetWorld().FindEntityByName(m_sEntityName));
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetSpawnedEntity: SCR_ScenarioFrameworkGet
{
	[Attribute()]
	protected string 		m_sLayerName;
	
	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		IEntity entity = FindEntityByName(m_sLayerName);
		SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
		if (!layer)
			return null;
		
		return new SCR_ScenarioFrameworkParam<IEntity>(layer.GetSpawnedEntity());
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetTask: SCR_ScenarioFrameworkGet
{
	[Attribute()]
	protected string 		m_sLayerTaskName;
	
	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		IEntity entity = FindEntityByName(m_sLayerTaskName);
		SCR_ScenarioFrameworkLayerTask layer = SCR_ScenarioFrameworkLayerTask.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerTask));
		if (!layer)
			return null;
		
		return new SCR_ScenarioFrameworkParam<IEntity>(layer.GetTask());
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetLayerTask: SCR_ScenarioFrameworkGet
{
	[Attribute()]
	protected string 		m_sLayerTaskName;
	
	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		IEntity entity = FindEntityByName(m_sLayerTaskName);
		if (!entity)
			return null;
		
		SCR_ScenarioFrameworkLayerTask layer = SCR_ScenarioFrameworkLayerTask.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerTask));
		if (!layer)
			return null;

		return new SCR_ScenarioFrameworkParam<IEntity>(entity);
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetLayerBase: SCR_ScenarioFrameworkGet
{
	[Attribute()]
	protected string 		m_sLayerBaseName;
	
	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		IEntity entity = FindEntityByName(m_sLayerBaseName);
		if (!entity)
			return null;
		
		SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
		if (!layer)
			return null;

		return new SCR_ScenarioFrameworkParam<IEntity>(entity);
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetArea: SCR_ScenarioFrameworkGet
{
	[Attribute()]
	protected string 		m_sAreaName;
	
	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		IEntity entity = FindEntityByName(m_sAreaName);
		if (!entity)
			return null;
		
		SCR_ScenarioFrameworkArea area = SCR_ScenarioFrameworkArea.Cast(entity.FindComponent(SCR_ScenarioFrameworkArea));
		if (!area)
			return null;

		return new SCR_ScenarioFrameworkParam<IEntity>(entity);
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetAreaTrigger: SCR_ScenarioFrameworkGet
{
	[Attribute()]
	protected string 		m_sAreaName;
	
	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		IEntity entity = FindEntityByName(m_sAreaName);
		if (!entity)
			return null;
		
		SCR_ScenarioFrameworkArea area = SCR_ScenarioFrameworkArea.Cast(entity.FindComponent(SCR_ScenarioFrameworkArea));
		if (!area)
			return null;
		
		SCR_CharacterTriggerEntity trigger = SCR_CharacterTriggerEntity.Cast(area.GetTrigger());
		if (!trigger)
			return null;

		return new SCR_ScenarioFrameworkParam<IEntity>(entity);
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetListEntitiesInTrigger : SCR_ScenarioFrameworkGet
{
	[Attribute()]
	protected string 		m_sTriggerName;
	
	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		if (!GetGame().GetWorld())
			return null;
			
		array<IEntity> aEntities = {};
		GetEntitiesInTrigger(aEntities);

		return new SCR_ScenarioFrameworkParam<array<IEntity>>(aEntities);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void GetEntitiesInTrigger(out notnull array<IEntity> aEntities)
	{
		BaseGameTriggerEntity trigger = BaseGameTriggerEntity.Cast(GetGame().GetWorld().FindEntityByName(m_sTriggerName));
		if (!trigger)
			return;
		
		trigger.GetEntitiesInside(aEntities);	
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetCountEntitiesInTrigger : SCR_ScenarioFrameworkGetListEntitiesInTrigger
{
	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		if (!GetGame().GetWorld())
			return null;
		
		array<IEntity> aEntities = {};
		GetEntitiesInTrigger(aEntities);
		
		
		return new SCR_ScenarioFrameworkParam<int>(aEntities.Count());
	}
};

//------------------------------------------------------------------------------------------------
//! Intended mainly for the one player scenario, because it will retrieve the first player it finds
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetPlayerEntity: SCR_ScenarioFrameworkGet
{
	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		if (!GetGame().GetWorld())
			return null;
		
		array<int> playerIDs = {};
		GetGame().GetPlayerManager().GetPlayers(playerIDs);
		IEntity entity;
		foreach (int playerID : playerIDs)
		{
			entity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
			if (!entity)
				continue;
			
			return new SCR_ScenarioFrameworkParam<IEntity>(entity);
		}
		
		return null;
	}
};

//------------------------------------------------------------------------------------------------
//! Gets the closest player to provided getter
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetClosestPlayerEntity: SCR_ScenarioFrameworkGet
{
	[Attribute(desc: "Closest to what - use getter")];
	protected ref SCR_ScenarioFrameworkGet		m_Getter;
	
	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		if (!GetGame().GetWorld())
			return null;
		
		if (!m_Getter)
		{
			Print("ScenarioFramework: GetClosestPlayerEntity - The object the distance is calculated from is missing!", LogLevel.ERROR);
			return null;
		}
		
		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
			return null;

		IEntity entityFrom = IEntity.Cast(entityWrapper.GetValue());
		if (!entityFrom)
			return null;
		
		array<int> playerIDs = {};
		GetGame().GetPlayerManager().GetPlayers(playerIDs);
		
		IEntity closestEntity;
		IEntity entityToBeChecked;
		float fDistance = float.MAX;
		foreach (int playerID : playerIDs)
		{
			entityToBeChecked = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
			if (!entityToBeChecked)
				continue;
			
			float fActualDistance = vector.DistanceSqXZ(entityFrom.GetOrigin(), entityToBeChecked.GetOrigin());

			if (fActualDistance < fDistance)
			{
				closestEntity = entityToBeChecked;
				fDistance = fActualDistance;
			}
		}

		if (!closestEntity)
			return null;
		
		return new SCR_ScenarioFrameworkParam<IEntity>(closestEntity);
	}
};
