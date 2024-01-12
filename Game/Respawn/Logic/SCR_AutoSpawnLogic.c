//------------------------------------------------------------------------------------------------
/*
	Object responsible for handling respawn logic on the authority side.
*/
[BaseContainerProps(category: "Respawn")]
class SCR_AutoSpawnLogic : SCR_SpawnLogic
{
	[Attribute("", uiwidget: UIWidgets.EditBox, category: "Respawn", desc: "Default faction for players to spawn with or empty if none.")]
	protected FactionKey m_sForcedFaction;
	
	[Attribute("", uiwidget: UIWidgets.EditBox, category: "Respawn", desc: "Default loadout for players to spawn with or empty if none")]
	protected string m_sForcedLoadout;
	
	protected ref set<int> m_DisconnectingPlayers = new set<int>();
	
	//------------------------------------------------------------------------------------------------
	protected bool GetForcedFaction(out Faction faction)
	{
		if (m_sForcedFaction.IsEmpty())
			return false;
		
		faction = GetGame().GetFactionManager().GetFactionByKey(m_sForcedFaction);
		if (!faction)
		{
			Print(string.Format("Auto spawn logic did not find faction by name: %1", m_sForcedFaction), LogLevel.WARNING);
			return false;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool GetForcedLoadout(out SCR_BasePlayerLoadout loadout)
	{
		if (m_sForcedLoadout.IsEmpty())
			return false;
		
		loadout = GetGame().GetLoadoutManager().GetLoadoutByName(m_sForcedLoadout);
		if (!loadout)
		{
			Print(string.Format("Auto spawn logic did not find loadout by name: %1", m_sForcedLoadout), LogLevel.WARNING);
			return false;
		}
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(SCR_RespawnSystemComponent owner)
	{
		super.OnInit(owner);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerRegistered_S(int playerId)
	{
		super.OnPlayerRegistered_S(playerId);
		
		// In cases where we pushed provided player into disconnecting ones, but never resolved it,
		// ensure that this player is no longer marked as disconnecting
		int indexOf = m_DisconnectingPlayers.Find(playerId);
		if (indexOf != -1)
		{
			m_DisconnectingPlayers.Remove(indexOf);
		}
		
		// In certain cases, the player can receive a controlled entity (e.g. spawn from camera position)
		// during the first game tick and since our spawn operation would usually be enqueued (before this)
		// and processed only after (the entity is given), it would result in losing the initial entity.
		// TODO@AS: Possibly improve this on gc->script level
		GetGame().GetCallqueue().CallLater(DoInitialSpawn, 0, false, playerId);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerDisconnected_S(int playerId, KickCauseCode cause, int timeout)
	{
		super.OnPlayerDisconnected_S(playerId, cause, timeout);
		m_DisconnectingPlayers.Insert(playerId);
	}

	
	//------------------------------------------------------------------------------------------------
	private void DoInitialSpawn(int playerId)
	{
		// Probe reconnection component first
		IEntity returnedEntity;
		if (ResolveReconnection(playerId, returnedEntity))
		{
			// User was reconnected, their entity was returned
			return;
		}	
		
		// Spawn player the usual way, if no entity has been given yet
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
		IEntity controlledEntity = playerController.GetControlledEntity();
		if (controlledEntity)
			return;
		
		Spawn(playerId);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerEntityLost_S(int playerId)
	{
		super.OnPlayerEntityLost_S(playerId);
		Spawn(playerId);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerSpawnFailed_S(int playerId)
	{
		super.OnPlayerSpawnFailed_S(playerId);

		int delay = Math.RandomFloat(900, 1100);
		GetGame().GetCallqueue().CallLater(Spawn, delay, false, playerId);
	}

	//------------------------------------------------------------------------------------------------
	protected void Spawn(int playerId)
	{
		// Player is disconnecting (and disappearance of controlled entity started this feedback loop).
		// Simply ignore such requests as it would create unwanted entities.
		int indexOf = m_DisconnectingPlayers.Find(playerId);
		if (indexOf != -1)
		{
			m_DisconnectingPlayers.Remove(indexOf);
			return;
		}
		
		array<Faction> factions = {};
		GetGame().GetFactionManager().GetFactionsList(factions);		
		
		Faction targetFaction;
		if (!GetForcedFaction(targetFaction))
			targetFaction = factions.GetRandomElement();
		
		// Request both
		if (!GetPlayerFactionComponent_S(playerId).RequestFaction(targetFaction))
		{
			// Try again later
		}

		SCR_BasePlayerLoadout targetLoadout;
		if (!GetForcedLoadout(targetLoadout))
			targetLoadout = GetGame().GetLoadoutManager().GetRandomFactionLoadout(targetFaction);		
		
		if (!GetPlayerLoadoutComponent_S(playerId).RequestLoadout(targetLoadout))
		{
			// Try again later
		}

		Faction faction = GetPlayerFactionComponent_S(playerId).GetAffiliatedFaction();
		if (!faction)
		{
			OnPlayerSpawnFailed_S(playerId);
			return;
		}

		SCR_BasePlayerLoadout loadout = GetPlayerLoadoutComponent_S(playerId).GetLoadout();
		if (!loadout)
		{
			OnPlayerSpawnFailed_S(playerId);
			return;
		}

		SCR_SpawnPoint point = SCR_SpawnPoint.GetRandomSpawnPointForFaction(faction.GetFactionKey());
		if (!point)
		{
			OnPlayerSpawnFailed_S(playerId);
			return;
		}

		SCR_SpawnPointSpawnData data = new SCR_SpawnPointSpawnData(loadout.GetLoadoutResource(), point.GetRplId());
		if (GetPlayerRespawnComponent_S(playerId).CanSpawn(data))
			DoSpawn(playerId, data);
		else
			OnPlayerSpawnFailed_S(playerId);
	}

	protected void DoSpawn(int playerId, SCR_SpawnData data)
	{
		if (!GetPlayerRespawnComponent_S(playerId).RequestSpawn(data))
		{
			// Try again later
		}
	}
};
