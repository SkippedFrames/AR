[EntityEditorProps(category: "GameScripted/GameMode", description: "Spawn point entity", visible: false)]
class SCR_RadioVehicleSpawnPointClass : SCR_SpawnPointClass
{
}

class SCR_RadioVehicleSpawnPoint : SCR_SpawnPoint
{
	protected SCR_PlayerSpawnPointManagerComponent m_PlayerSpawnPointManager;
	protected Physics m_Physics;
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (gameMode)
			m_PlayerSpawnPointManager = SCR_PlayerSpawnPointManagerComponent.Cast(gameMode.FindComponent(SCR_PlayerSpawnPointManagerComponent));
		
		IEntity parent = owner.GetParent();
		while (parent && !m_Physics)
		{
			if (Vehicle.Cast(parent))
			{
				m_Physics = parent.GetPhysics();
				parent = parent.GetParent();
				break;
			}
			
			parent = parent.GetParent();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsSpawnPointEnabled()
	{
		return m_bSpawnPointEnabled && (!m_PlayerSpawnPointManager || m_PlayerSpawnPointManager.IsRadioVehicleSpawningEnabled());
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanReserveFor_S(int playerId, out SCR_ESpawnResult result = SCR_ESpawnResult.SPAWN_NOT_ALLOWED)
	{
		if (!super.CanReserveFor_S(playerId, result))
		{
			if (m_PlayerSpawnPointManager && !m_PlayerSpawnPointManager.IsRadioVehicleSpawningEnabled())
				result = SCR_ESpawnResult.NOT_ALLOWED_RADIO_VEHICLE_SPAWNING_DISABLED;
			
			return false;
		}
		
		//~ Vehicle is moving
		if (m_Physics)
		{
			if (m_Physics.GetVelocity() != vector.Zero || m_Physics.GetAngularVelocity() != vector.Zero)
			{
				result = SCR_ESpawnResult.NOT_ALLOWED_VEHICLE_MOVING;
				return false;
			}
		}
		
	
		//~ Check if occupied
		if (m_FactionAffiliationComponent)
		{
			Faction defaultFaction = m_FactionAffiliationComponent.GetDefaultAffiliatedFaction();
			Faction currentFaction = m_FactionAffiliationComponent.GetAffiliatedFaction();
			
			if (!currentFaction)
				return true;
			
			if (defaultFaction != currentFaction)
			{
				SCR_Faction scrDefaultFaction = SCR_Faction.Cast(defaultFaction);
				
				if (!scrDefaultFaction || !scrDefaultFaction.IsFactionFriendly(currentFaction))
				{
					result = SCR_ESpawnResult.NOT_ALLOWED_SPAWNPOINT_OCCUPIED_BY_HOSTILE;
					return false;
				}
			}
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void InitFactionAffiliation(IEntity owner)
	{
		IEntity parent = owner.GetParent();
		while (parent && !m_FactionAffiliationComponent)
		{
			m_FactionAffiliationComponent = SCR_FactionAffiliationComponent.Cast(parent.FindComponent(SCR_FactionAffiliationComponent));
			if (!m_FactionAffiliationComponent)
				parent = parent.GetParent();
		}
		
		if (!m_FactionAffiliationComponent)
			m_FactionAffiliationComponent = SCR_FactionAffiliationComponent.Cast(owner.FindComponent(SCR_FactionAffiliationComponent));
		
		if (m_FactionAffiliationComponent)
		{
			Faction faction = m_FactionAffiliationComponent.GetDefaultAffiliatedFaction();
			if (faction)
				m_sFaction = faction.GetFactionKey();
		}
	}
}
