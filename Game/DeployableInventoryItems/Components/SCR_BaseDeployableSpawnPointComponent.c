[EntityEditorProps(category: "GameScripted/DeployableItems", description: "")]
class SCR_BaseDeployableSpawnPointComponentClass : SCR_BaseDeployableInventoryItemComponentClass
{
}

//------------------------------------------------------------------------------------------------
//! Base class which all deployable spawn points / radios inherit from
class SCR_BaseDeployableSpawnPointComponent : SCR_BaseDeployableInventoryItemComponent
{
	protected static ref array<SCR_BaseDeployableSpawnPointComponent> s_aActiveDeployedSpawnPoints = {};

	[Attribute("{84680168E273774F}Prefabs/MP/Spawning/DeployableItemSpawnPoint_Base.et")]
	protected ResourceName m_sSpawnPointPrefab;

	[Attribute()]
	protected ResourceName m_sReplacementPrefab;

	[Attribute()]
	protected FactionKey m_FactionKey;

	protected SCR_DeployableSpawnPoint m_SpawnPoint;

	protected IEntity m_ReplacementEntity;

	protected vector m_vOriginalTransform[4];
	
	protected bool m_bDeployableSpawnPointsEnabled;

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_SetTransformBroadcast(vector transform[4])
	{
		GetOwner().SetTransform(transform);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Unreliable, RplRcver.Broadcast)]
	protected void RPC_PlaySoundBroadcast(bool deploy)
	{
		SoundComponent soundComp = SoundComponent.Cast(GetOwner().FindComponent(SoundComponent));
		if (!soundComp)
			return;
		
		if (deploy)
			soundComp.SoundEvent(SCR_SoundEvent.SOUND_DEPLOYED_RADIO_DEPLOY);
		else
			soundComp.SoundEvent(SCR_SoundEvent.SOUND_DEPLOYED_RADIO_UNDEPLOY);
	}

	//------------------------------------------------------------------------------------------------
	protected void ToggleRadioChatter(bool enable)
	{
		SignalsManagerComponent signalsManagerComp = SignalsManagerComponent.Cast(GetOwner().FindComponent(SignalsManagerComponent));
		if (!signalsManagerComp)
			return;

		int radioChatter = signalsManagerComp.FindSignal("DeployableRadioChatter");

		if (enable)
			signalsManagerComp.SetSignalValue(radioChatter, 1);
		else
			signalsManagerComp.SetSignalValue(radioChatter, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCompositionDestroyed()
	{
		Dismantle();
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnRplDeployed()
	{
		ToggleRadioChatter(m_bIsDeployed);
	}

	//------------------------------------------------------------------------------------------------
	//! Spawns replacement composition and attaches owner entity to it - called from SCR_DeployItemBaseAction.PerformAction
	override void Deploy(IEntity userEntity = null)
	{
		//~ Not allowed to deploy
		if (!m_bDeployableSpawnPointsEnabled)
			return;
		
		RplComponent rplComp = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (!rplComp || rplComp.IsProxy())
			return;

		vector transform[4];
		SCR_TerrainHelper.GetTerrainBasis(GetOwner().GetOrigin(), transform, GetGame().GetWorld(), false, new TraceParam());

		m_vOriginalTransform = transform;

		EntitySpawnParams params = new EntitySpawnParams();
		params.Transform = transform;
		params.TransformMode = ETransformMode.WORLD;

		Resource resource = Resource.Load(m_sReplacementPrefab);
		if (!resource.IsValid())
			return;

		m_ReplacementEntity = GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), params);
		if (!m_ReplacementEntity)
			return;

		RplComponent rplCompReplacementEntity = RplComponent.Cast(m_ReplacementEntity.FindComponent(RplComponent));
		if (!rplCompReplacementEntity)
			return;

		SCR_DeployableInventoryItemReplacementComponent replacementComp = SCR_DeployableInventoryItemReplacementComponent.Cast(m_ReplacementEntity.FindComponent(SCR_DeployableInventoryItemReplacementComponent));
		if (!replacementComp)
			return;

		vector compositionTransform[4], combinedTransform[4];
		replacementComp.GetItemTransform(compositionTransform);
		Math3D.MatrixMultiply4(transform, compositionTransform, combinedTransform);

		RPC_SetTransformBroadcast(combinedTransform);
		Rpc(RPC_SetTransformBroadcast, combinedTransform);

		replacementComp.GetOnCompositionDestroyed().Insert(OnCompositionDestroyed);

		resource = Resource.Load(m_sSpawnPointPrefab);
		if (!resource.IsValid())
			return;

		m_SpawnPoint = SCR_DeployableSpawnPoint.Cast(GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), params));
		if (!m_SpawnPoint)
			return;

		RplComponent rplCompSpawnpoint = RplComponent.Cast(m_SpawnPoint.FindComponent(RplComponent));
		if (!rplCompSpawnpoint)
			return;

		m_SpawnPoint.SetDeployableSpawnPointComponent(this);
		m_SpawnPoint.SetFactionKey(m_FactionKey);

		m_bIsDeployed = true;
		Replication.BumpMe();

		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return;

		if (userEntity)
			m_iItemOwnerID = playerManager.GetPlayerIdFromControlledEntity(userEntity);

		s_aActiveDeployedSpawnPoints.Insert(this);
		
		ToggleRadioChatter(true);		
		RPC_PlaySoundBroadcast(true);
		Rpc(RPC_PlaySoundBroadcast, true);

		ChimeraWorld world = GetOwner().GetWorld();
		if (!world)
			return;

		GarbageSystem garbageSystem = world.GetGarbageSystem();
		if (!garbageSystem)
			return;

		garbageSystem.Withdraw(GetOwner()); // Withdraw owner entity from garbage collection while radio is deployed
	}

	//------------------------------------------------------------------------------------------------
	//! Delete replacement composition and spawnpoint and set position of owner entity to it's original position - called from SCR_DismantleItemBaseAction.PerformAction
	override void Dismantle(IEntity userEntity = null)
	{
		RplComponent rplComp = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
			if (!rplComp || rplComp.IsProxy())
				return;

		if (m_SpawnPoint)
		{
			rplComp = RplComponent.Cast(m_SpawnPoint.FindComponent(RplComponent));
			if (!rplComp)
				return;

			rplComp.DeleteRplEntity(m_SpawnPoint, false);
		}

		if (m_ReplacementEntity)
		{
			rplComp = RplComponent.Cast(m_ReplacementEntity.FindComponent(RplComponent));
			if (!rplComp)
				return;

			rplComp.DeleteRplEntity(m_ReplacementEntity, false);
		}

		RPC_SetTransformBroadcast(m_vOriginalTransform);
		Rpc(RPC_SetTransformBroadcast, m_vOriginalTransform);

		m_bIsDeployed = false;
		Replication.BumpMe();

		m_iItemOwnerID = -1;

		s_aActiveDeployedSpawnPoints.RemoveItem(this);
		
		ToggleRadioChatter(false);		
		RPC_PlaySoundBroadcast(false);
		Rpc(RPC_PlaySoundBroadcast, false);

		ChimeraWorld world = GetOwner().GetWorld();
		if (!world)
			return;

		GarbageSystem garbageSystem = world.GetGarbageSystem();
		if (!garbageSystem)
			return;

		garbageSystem.Insert(GetOwner()); // Insert owner entity back into garbage collection after item is dismantled
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSpawnPointDeployingEnabledChanged(bool enabled)
	{
		m_bDeployableSpawnPointsEnabled = enabled;
	}

	//------------------------------------------------------------------------------------------------
	static array<SCR_BaseDeployableSpawnPointComponent> GetActiveDeployedSpawnPoints()
	{
		return s_aActiveDeployedSpawnPoints;
	}

	//------------------------------------------------------------------------------------------------
	SCR_SpawnPoint GetSpawnPoint()
	{
		return m_SpawnPoint;
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;
		
		SCR_PlayerSpawnPointManagerComponent playerSpawnPointManager = SCR_PlayerSpawnPointManagerComponent.Cast(gameMode.FindComponent(SCR_PlayerSpawnPointManagerComponent));
		if (!playerSpawnPointManager)
			return;
		
		OnSpawnPointDeployingEnabledChanged(playerSpawnPointManager.IsDeployingSpawnPointsEnabled());
		playerSpawnPointManager.GetOnSpawnPointDeployingEnabledChanged().Insert(OnSpawnPointDeployingEnabledChanged);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_BaseDeployableSpawnPointComponent()
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (gameMode)
		{
			SCR_PlayerSpawnPointManagerComponent playerSpawnPointManager = SCR_PlayerSpawnPointManagerComponent.Cast(gameMode.FindComponent(SCR_PlayerSpawnPointManagerComponent));
			if (playerSpawnPointManager)
				playerSpawnPointManager.GetOnSpawnPointDeployingEnabledChanged().Remove(OnSpawnPointDeployingEnabledChanged);
		}
		
		RplComponent rplComp = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
			if (!rplComp || rplComp.IsProxy())
				return;

		if (m_bIsDeployed)
			s_aActiveDeployedSpawnPoints.RemoveItem(this); // Remove this deployed item from the static array in case someone shoots and destroys it
	}
}
