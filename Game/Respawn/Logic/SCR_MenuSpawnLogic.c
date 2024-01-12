[BaseContainerProps(category: "Respawn")]
class SCR_MenuSpawnLogic : SCR_SpawnLogic
{
	[Attribute("", uiwidget: UIWidgets.EditBox, category: "Respawn", desc: "Default faction for players to spawn with or empty if none.")]
	protected FactionKey m_sForcedFaction;
	
	[Attribute("{A1CE9D1EC16DA9BE}UI/layouts/Menus/MainMenu/SplashScreen.layout", desc: "Layout shown before deploy menu opens on client")]
	protected ResourceName m_sLoadingLayout;

	protected Widget m_wLoadingPlaceholder;
	protected SCR_LoadingSpinner m_LoadingPlaceholder;

	//------------------------------------------------------------------------------------------------
	override void OnInit(SCR_RespawnSystemComponent owner)
	{
		if (!System.IsConsoleApp())
			CreateLoadingPlaceholder();
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerRegistered_S(int playerId)
	{
		super.OnPlayerRegistered_S(playerId);

		// Probe reconnection component first
		IEntity returnedEntity;
		if (ResolveReconnection(playerId, returnedEntity))
		{
			// User was reconnected, their entity was returned
			return;
		}

		Faction forcedFaction;
		if (GetForcedFaction(forcedFaction))
		{
			PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);
			SCR_PlayerFactionAffiliationComponent playerFactionComp = SCR_PlayerFactionAffiliationComponent.Cast(pc.FindComponent(SCR_PlayerFactionAffiliationComponent));
			if (playerFactionComp)
				playerFactionComp.RequestFaction(forcedFaction);
		}

		// Send a notification to registered client:
		// Always ensure to hook OnLocalPlayer callbacks prior to sending such notification,
		// otherwise the notification will be disregarded
		GetPlayerRespawnComponent_S(playerId).NotifyReadyForSpawn_S();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerEntityLost_S(int playerId)
	{
		super.OnPlayerEntityLost_S(playerId);

		GetPlayerRespawnComponent_S(playerId).NotifyReadyForSpawn_S();
	}

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
	protected void CreateLoadingPlaceholder()
	{
		m_wLoadingPlaceholder = GetGame().GetWorkspace().CreateWidgets(m_sLoadingLayout);
		if (!m_wLoadingPlaceholder)
			return;

		Widget spinner = m_wLoadingPlaceholder.FindAnyWidget("Spinner");
		if (!spinner)
			return;

		m_LoadingPlaceholder = SCR_LoadingSpinner.Cast(spinner.FindHandler(SCR_LoadingSpinner));
	}

	//------------------------------------------------------------------------------------------------
	bool UpdateLoadingPlaceholder(float dt)
	{
		if (!m_wLoadingPlaceholder || !m_LoadingPlaceholder)
			return true;

		m_LoadingPlaceholder.Update(dt);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	void DestroyLoadingPlaceholder()
	{
		if (!m_wLoadingPlaceholder)
			return;

		m_wLoadingPlaceholder.RemoveFromHierarchy();
		m_wLoadingPlaceholder = null;
		m_LoadingPlaceholder = null;
	}
};
