//------------------------------------------------------------------------------------------------
[BaseContainerProps(configRoot: true), BaseContainerCustomTitleField("m_sLoadoutName")]
class SCR_PlayerArsenalLoadout : SCR_FactionPlayerLoadout
{	
	const string ARSENALLOADOUT_FACTIONKEY_NONE = "none";
	const string ARSENALLOADOUT_KEY = "arsenalLoadout";
	const string ARSENALLOADOUT_FACTION_KEY = "faction";
	
	//------------------------------------------------------------------------------------------------
	override bool IsLoadoutAvailable(int playerId)
	{
		SCR_ArsenalManagerComponent arsenalManager;
		if (SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManager))
		{
			string arsenalLoadout;
			return arsenalManager.GetPlayerArsenalLoadout(playerId, arsenalLoadout);
		}
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsLoadoutAvailableClient()
	{
		SCR_ArsenalManagerComponent arsenalManager;
		if (SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManager))
		{
			return arsenalManager.GetLocalPlayerLoadoutAvailable();
		}
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnLoadoutSpawned(GenericEntity pOwner, int playerId)
	{
		super.OnLoadoutSpawned(pOwner, playerId);
		SCR_ArsenalManagerComponent arsenalManager;
		if (!SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManager)) 
		{
			return;
		}
		
		GameEntity playerEntity = GameEntity.Cast(pOwner);
		string playerArsenalItems = string.Empty;
		if (!playerEntity || !arsenalManager.GetPlayerArsenalLoadout(playerId, playerArsenalItems))
		{
			return;
		}
		
		FactionAffiliationComponent factionComponent = FactionAffiliationComponent.Cast(playerEntity.FindComponent(FactionAffiliationComponent));
		if (!factionComponent)
		{
			return;
		}
		
		SCR_JsonLoadContext context = new SCR_JsonLoadContext();
		bool loadSuccess = true;
		loadSuccess &= context.ImportFromString(playerArsenalItems);
		// Read faction key and ensure same faction, otherwise delete saved arsenal loadout
		string factionKey;
		loadSuccess &= context.ReadValue(ARSENALLOADOUT_FACTION_KEY, factionKey) && factionKey != ARSENALLOADOUT_FACTIONKEY_NONE;		
		loadSuccess &= factionKey == factionComponent.GetAffiliatedFaction().GetFactionKey();
		loadSuccess &= context.ReadValue(ARSENALLOADOUT_KEY, playerEntity);
		
		// Deserialization failed, delete saved arsenal loadout
		if (!loadSuccess)
		{
			arsenalManager.SetPlayerArsenalLoadout(playerId, null, null);
		}
	}
};
