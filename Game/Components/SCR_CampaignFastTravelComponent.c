//------------------------------------------------------------------------------------------------
[EntityEditorProps(category: "GameScripted/FastTravel", description: "Handles client > server communication for Fast travel in Conflict. Should be attached to PlayerController.")]
class SCR_CampaignFastTravelComponentClass : SCR_FastTravelComponentClass
{
	[Attribute("50", params: "0 inf 1")]
	protected int m_iSupplyCost;

	[Attribute("100", desc: "Tolerance when player identifies their location in the map (meters)", params: "0 inf 1")]
	protected int m_iPickupRadius;

	[Attribute("200", desc: "If there are enemies closer that this distance to the player, the request will fail", params: "0 inf 1")]
	protected int m_iNoEnemyRadius;

	//------------------------------------------------------------------------------------------------
	int GetSupplyCost()
	{
		return m_iSupplyCost;
	}

	//------------------------------------------------------------------------------------------------
	int GetPickupRadius()
	{
		return m_iPickupRadius;
	}

	//------------------------------------------------------------------------------------------------
	int GetNoEnemyRadius()
	{
		return m_iNoEnemyRadius;
	}
}

//------------------------------------------------------------------------------------------------
class SCR_CampaignFastTravelComponent : SCR_FastTravelComponent
{
	protected static const ResourceName TASKS_IMAGESET = "{10C0A9A305E8B3A4}UI/Imagesets/Tasks/Task_Icons.imageset";

	protected bool m_bSelectionInProgress;
	protected bool m_bEnemiesNearby;

	protected SCR_MapRadialUI m_MapContextualMenu;

	protected SCR_CampaignFaction m_PlayerFaction;

	protected RplId m_iFactionDestinationId = RplId.Invalid();
	protected RplId m_iGroupDestinationId = RplId.Invalid();
	protected RplId m_iGroupLeaderId = RplId.Invalid();

	protected SCR_SelectionMenuEntry m_FactionTargetEntry;
	protected SCR_SelectionMenuEntry m_GroupTargetEntry;

	protected vector m_vGroupLeaderPosition;

	//------------------------------------------------------------------------------------------------
	override protected bool ServerSanityCheck(notnull IEntity target)
	{
		if (!super.ServerSanityCheck(target))
			return false;

		SCR_ChimeraCharacter player = SCR_ChimeraCharacter.Cast(m_PlayerController.GetControlledEntity());

		if (!player)
			return false;

		if (!m_PlayerFaction)
			m_PlayerFaction = SCR_CampaignFaction.Cast(player.GetFaction());

		SCR_CampaignFastTravelComponentClass componentData = SCR_CampaignFastTravelComponentClass.Cast(GetComponentData(GetOwner()));

		if (!componentData)
			return false;

		m_bEnemiesNearby = false;
		GetGame().GetWorld().QueryEntitiesBySphere(player.GetOrigin(), componentData.GetNoEnemyRadius(), ProcessEntity, FilterEntity, EQueryEntitiesFlags.DYNAMIC | EQueryEntitiesFlags.WITH_OBJECT);

		if (m_bEnemiesNearby)
			SCR_NotificationsComponent.SendToPlayer(m_PlayerController.GetPlayerId(), ENotification.FASTTRAVEL_ENEMIES_NEARBY);

		return !m_bEnemiesNearby;
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_ReadLeaderPosition()
	{
		vector position;
		RplId id = RplId.Invalid();
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();

		if (groupsManager)
		{
			SCR_AIGroup group = groupsManager.GetPlayerGroup(m_PlayerController.GetPlayerId());
	
			if (group)
			{
				int leaderId = group.GetLeaderID();
		
				ChimeraCharacter leader = ChimeraCharacter.Cast(GetGame().GetPlayerManager().GetPlayerControlledEntity(leaderId));
		
				if (leader)
				{
					id = Replication.FindId(leader);
					SCR_CharacterDamageManagerComponent damageManager = SCR_CharacterDamageManagerComponent.Cast(leader.GetDamageManager());
		
					if (damageManager && !damageManager.IsDestroyed() && !damageManager.GetIsUnconscious())
						position = leader.GetOrigin();
				}
			}
		}

		Rpc(RpcDo_WriteLeaderPosition, position, id);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_WriteLeaderPosition(vector position, int id)
	{
		m_vGroupLeaderPosition = position;
		m_iGroupLeaderId = id;
		UpdateEntries();
	}

	//------------------------------------------------------------------------------------------------
	//! Reads the cooldown value from player's rank
	override protected int GetCooldown()
	{
		SCR_CampaignFactionManager fManager = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager());

		if (!fManager)
			return m_iCooldown;

		IEntity player = m_PlayerController.GetControlledEntity();

		if (!player)
			return m_iCooldown;

		return fManager.GetRankFastTravelCooldown(SCR_CharacterRankComponent.GetCharacterRank(player));
	}

	//------------------------------------------------------------------------------------------------
	protected bool FilterEntity(IEntity ent)
	{
		return SCR_ChimeraCharacter.Cast(ent) != null;
	}

	//------------------------------------------------------------------------------------------------
	protected bool ProcessEntity(IEntity ent)
	{
		SCR_ChimeraCharacter char = SCR_ChimeraCharacter.Cast(ent);
		Faction entityFaction = char.GetFaction();

		if (!entityFaction.IsFactionEnemy(m_PlayerFaction))
			return true;

		if (char.GetCharacterController().IsDead())
			return true;

		m_bEnemiesNearby = true;
		return false;
	}

	//------------------------------------------------------------------------------------------------
	void OnMapOpen(MapConfiguration config)
	{
		if (!m_MapContextualMenu)
			m_MapContextualMenu = SCR_MapRadialUI.GetInstance();

		if (!m_MapContextualMenu)
			return;

		m_MapContextualMenu.GetOnMenuInitInvoker().Insert(SetupMapRadialMenuEntries);
		m_MapContextualMenu.GetOnEntryPerformedInvoker().Insert(OnMapFastTravelRequested);
	}

	//------------------------------------------------------------------------------------------------
	void OnMapClose(MapConfiguration config)
	{
		SCR_MapEntity.GetOnSelection().Remove(SendPlayerCoords);
		GetGame().GetCallqueue().Remove(RefreshShownCooldown);
		GetGame().GetCallqueue().Remove(UpdateEntries);
		SCR_HintManagerComponent.HideHint();

		if (m_bSelectionInProgress)
			SCR_NotificationsComponent.SendLocal(ENotification.FASTTRAVEL_PLAYER_LOCATION_CANCELLED);

		ToggleDestinationSelection(false);

		if (!m_MapContextualMenu)
			return;

		m_MapContextualMenu.GetOnMenuInitInvoker().Remove(SetupMapRadialMenuEntries);
		m_MapContextualMenu.GetOnEntryPerformedInvoker().Remove(OnMapFastTravelRequested);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMapFastTravelRequested(SCR_SelectionMenuEntry element, float[] worldPos)
	{
		if (element != m_FactionTargetEntry && element != m_GroupTargetEntry)
			return;

		SCR_CampaignFeedbackComponent feedbackComp = SCR_CampaignFeedbackComponent.GetInstance();

		if (feedbackComp)
			feedbackComp.ShowHint(EHint.CONFLICT_TRANSPORT_PICKUP, true, true);

		ToggleDestinationSelection(true);

		if (element == m_FactionTargetEntry)
			SetDestination(m_iFactionDestinationId, string.Empty);
		else if (element == m_GroupTargetEntry)
			SetDestination(m_iGroupDestinationId, string.Empty);

		SCR_MapEntity.GetOnSelection().Remove(SendPlayerCoords);
		SCR_MapEntity.GetOnSelection().Insert(SendPlayerCoords);
	}

	//------------------------------------------------------------------------------------------------
	protected void SendPlayerCoords(vector coords)
	{
		SCR_MapEntity.GetOnSelection().Remove(SendPlayerCoords);
		SCR_HintManagerComponent.HideHint();

		ToggleDestinationSelection(false);

		if (!m_PlayerController)
			return;

		IEntity player = m_PlayerController.GetControlledEntity();

		if (!player)
			return;

		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();

		if (!mapEntity)
			return;

		SCR_CampaignFastTravelComponentClass componentData = SCR_CampaignFastTravelComponentClass.Cast(GetComponentData(GetOwner()));

		if (!componentData)
			return;

		if (player && mapEntity.IsOpen())
		{
			SCR_GadgetManagerComponent comp = SCR_GadgetManagerComponent.GetGadgetManager(player);

			if (comp)
				comp.RemoveHeldGadget();
		}

		float x, y;
		mapEntity.ScreenToWorld(coords[0], coords[2], x, y);
		coords[0] = x;
		coords[2] = y;

		if (vector.DistanceXZ(player.GetOrigin(), coords) > componentData.GetPickupRadius())
		{
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.ACTION_FAILED);
			SCR_NotificationsComponent.SendLocal(ENotification.FASTTRAVEL_PLAYER_LOCATION_WRONG);
			return;
		}

		SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_MAP_CLICK_POINT_ON);
		FastTravel();
	}

	//------------------------------------------------------------------------------------------------
	protected void SetupMapRadialMenuEntries()
	{
		if (!m_MapContextualMenu || !m_PlayerController)
			return;

		ChimeraWorld world = GetGame().GetWorld();
		WorldTimestamp timestamp = world.GetServerTimestamp();

		// Parent for fast travel radial menu entries
		SCR_SelectionMenuCategoryEntry entry = m_MapContextualMenu.AddRadialCategory("#AR-Tasks_TitleTransport");
		entry.SetIconFromDeafaultImageSet("terrainIcon");

		// Disable fast travel entry if its cooldown is still in progress
		bool available = !m_fNextTravelAvailableAt || timestamp.GreaterEqual(m_fNextTravelAvailableAt);
		entry.Enable(available);

		SCR_ChimeraCharacter player = SCR_ChimeraCharacter.Cast(m_PlayerController.GetControlledEntity());

		if (!player)
			return;

		SCR_CampaignFactionManager fManager = SCR_CampaignFactionManager.Cast(GetGame().GetFactionManager());
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();

		if (player.IsInVehicle())
		{
			entry.Enable(false);
			entry.SetName(string.Empty);
			return;
		}

		// Disable fast travel entry if player is renegade
		if (fManager && fManager.IsRankRenegade(SCR_CharacterRankComponent.GetCharacterRank(player)))
		{
			entry.Enable(false);
			entry.SetName("#AR-Rank_Renegade");
			return;
		}

		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.FindResourceComponent(player);

		if (resourceComponent)
		{
			SCR_ResourceConsumer consumer = resourceComponent.GetConsumer(EResourceGeneratorID.SELF, EResourceType.SUPPLIES);

			if (consumer)
			{
				//Print(consumer.GetAggregatedResourceValue());
			}
		}

		if (campaign && !campaign.GetBaseManager().IsEntityInFactionRadioSignal(player, player.GetFaction()))
		{
			entry.Enable(false);
			entry.SetName("#AR-Campaign_NoRadioSignal");
			return;
		}

		// Run cooldown timer refresh
		if (!available)
			GetGame().GetCallqueue().CallLater(RefreshShownCooldown, SCR_GameModeCampaign.UI_UPDATE_DELAY, true, entry);

		m_FactionTargetEntry = m_MapContextualMenu.AddRadialEntry(string.Empty, entry);
		m_FactionTargetEntry.SetIcon(TASKS_IMAGESET, "Icon_M_Task_Outline");
		m_FactionTargetEntry.SetDescription("#AR-Campaign_PriorityTask");

		m_GroupTargetEntry = m_MapContextualMenu.AddRadialEntry(string.Empty, entry);

		Rpc(RpcAsk_ReadLeaderPosition);

		UpdateEntries();
		GetGame().GetCallqueue().CallLater(UpdateEntries, SCR_GameModeCampaign.DEFAULT_DELAY, true); 	// Keep entries up to date with game progress
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshShownCooldown(SCR_SelectionMenuCategoryEntry entry)
	{
		ChimeraWorld world = GetGame().GetWorld();
		WorldTimestamp timestamp = world.GetServerTimestamp();
		Widget radialRoot = m_MapContextualMenu.GetRadialDisplay().GetRootWidget();
		SCR_SelectionMenuEntry currentEntry = m_MapContextualMenu.GetRadialController().GetRadialMenu().GetSelectionEntry();

		if (!m_fNextTravelAvailableAt || timestamp.GreaterEqual(m_fNextTravelAvailableAt))
		{
			// Cooldown finished, re-enable fast travel
			GetGame().GetCallqueue().Remove(RefreshShownCooldown);

			entry.Enable(true);
			entry.SetName("#AR-Tasks_TitleTransport");

			if (currentEntry == entry)
				entry.SetNameTo(TextWidget.Cast(radialRoot.FindAnyWidget("InfoName")));
		}
		else
		{
			// Update timer
			entry.Enable(false);
			entry.SetName(SCR_FormatHelper.GetTimeFormatting(m_fNextTravelAvailableAt.DiffMilliseconds(timestamp) * 0.001, ETimeFormatParam.DAYS | ETimeFormatParam.HOURS));

			if (currentEntry == entry)
				entry.SetNameTo(TextWidget.Cast(radialRoot.FindAnyWidget("InfoName")));
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateEntries()
	{
		if (!m_PlayerController)
			return;

		SCR_ChimeraCharacter player = SCR_ChimeraCharacter.Cast(m_PlayerController.GetControlledEntity());

		if (!player)
			return;

		if (!m_PlayerFaction)
			m_PlayerFaction = SCR_CampaignFaction.Cast(player.GetFaction());

		if (!m_PlayerFaction)
			return;

		RplId groupTargetIdPrev = m_iGroupDestinationId;
		RplId factionTargetIdPrev = m_iFactionDestinationId;

		SCR_CampaignMilitaryBaseComponent priorityTarget = m_PlayerFaction.GetPrimaryTarget();

		if (priorityTarget)
		{
			m_iFactionDestinationId = FindDestinationId(priorityTarget.GetOwner());

			if (IsDestinationReachable(m_iFactionDestinationId))
			{
				m_FactionTargetEntry.SetName(priorityTarget.GetBaseName());
				m_FactionTargetEntry.Enable(true);
			}
			else
			{
				m_FactionTargetEntry.SetName("#AR-Campaign_Unreachable");
				m_FactionTargetEntry.Enable(false);
			}
		}
		else
		{
			m_FactionTargetEntry.SetName(string.Empty);
			m_FactionTargetEntry.Enable(false);
			m_iFactionDestinationId = RplId.Invalid();
		}

		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();

		if (!groupsManager)
		{
			m_GroupTargetEntry.SetName("#AR-Campaign_Unreachable");
			m_GroupTargetEntry.Enable(false);
			return;
		}

		SCR_AIGroup group = groupsManager.GetPlayerGroup(m_PlayerController.GetPlayerId());

		if (!group)
		{
			m_GroupTargetEntry.SetName("#AR-Campaign_Unreachable");
			m_GroupTargetEntry.Enable(false);
			return;
		}

		int leaderId = group.GetLeaderID();
		IEntity leader = GetGame().GetPlayerManager().GetPlayerControlledEntity(leaderId);
		SCR_BaseTaskExecutor executor = SCR_BaseTaskExecutor.FindTaskExecutorByID(leaderId);

		if (!executor)
		{
			m_GroupTargetEntry.SetName("#AR-Campaign_Unreachable");
			m_GroupTargetEntry.Enable(false);
			return;
		}

		SCR_CampaignMilitaryBaseComponent base;
		SCR_CampaignBaseTask task = SCR_CampaignBaseTask.Cast(executor.GetAssignedTask());

		if (task)
			base = task.GetTargetBase();

		if (base && base != priorityTarget)
		{
			m_iGroupDestinationId = FindDestinationId(base.GetOwner());
			m_GroupTargetEntry.SetIcon(TASKS_IMAGESET, "Icon_Task_Outline");
			m_GroupTargetEntry.SetDescription("#AR-Campaign_GroupTask");

			if (IsDestinationReachable(m_iFactionDestinationId))
			{
				m_GroupTargetEntry.SetName(base.GetBaseName());
				m_GroupTargetEntry.Enable(true);
			}
			else
			{
				m_GroupTargetEntry.SetName("#AR-Campaign_Unreachable");
				m_GroupTargetEntry.Enable(false);
			}
		}
		else if (leader != player)
		{
			m_iGroupDestinationId = m_iGroupLeaderId;
			m_GroupTargetEntry.SetIconFromDeafaultImageSet("join");
			m_GroupTargetEntry.SetDescription("#AR-Campaign_GroupLeader");

			if (m_vGroupLeaderPosition != vector.Zero && IsDestinationReachable(m_vGroupLeaderPosition))
			{
				m_GroupTargetEntry.SetName(GetGame().GetPlayerManager().GetPlayerName(leaderId));
				m_GroupTargetEntry.Enable(true);
			}
			else
			{
				m_GroupTargetEntry.SetName("#AR-Campaign_Unreachable");
				m_GroupTargetEntry.Enable(false);
			}
		}
		else
		{
			m_GroupTargetEntry.SetName(string.Empty);
			m_GroupTargetEntry.SetIcon(TASKS_IMAGESET, "Icon_Task_Outline");
			m_GroupTargetEntry.SetDescription("#AR-Campaign_GroupTask");
			m_iGroupDestinationId = RplId.Invalid();
			m_GroupTargetEntry.Enable(false);
		}

		if (!m_bSelectionInProgress)
			return;

		// Check if selected destination is no longer available; if so, cancel the request
		bool destinationChanged;

		if (m_iDestinationId == groupTargetIdPrev && m_iGroupDestinationId != groupTargetIdPrev)
			destinationChanged = true;
		else if (m_iDestinationId == factionTargetIdPrev && m_iFactionDestinationId != factionTargetIdPrev)
			destinationChanged = true;

		if (!destinationChanged)
			return;

		SCR_MapEntity.GetOnSelection().Remove(SendPlayerCoords);
		SCR_HintManagerComponent.HideHint();

		ToggleDestinationSelection(false);
	}

	//------------------------------------------------------------------------------------------------
	protected void ToggleDestinationSelection(bool enable)
	{
		bool updateCursor = (m_bSelectionInProgress != enable);
		m_bSelectionInProgress = enable;

		if (!updateCursor)
			return;

		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();

		if (!mapEntity)
			return;

		SCR_MapCursorModule module = SCR_MapCursorModule.Cast(mapEntity.GetMapModule(SCR_MapCursorModule));

		if (!module)
			return;

		module.ToggleFastTravelDestinationSelection(enable);
	}

	//------------------------------------------------------------------------------------------------
	bool IsDestinationReachable(RplId id)
	{
		IEntity destination = GetEntityByDestinationId(id);

		if (!destination)
			return false;

		return !ChimeraWorldUtils.TryGetWaterSurfaceSimple(GetGame().GetWorld(), CalculateDestinationVector(destination));
	}

	//------------------------------------------------------------------------------------------------
	bool IsDestinationReachable(vector destination)
	{
		if (destination == vector.Zero)
			return false;

		return !ChimeraWorldUtils.TryGetWaterSurfaceSimple(GetGame().GetWorld(), CalculateDestinationVector(destination));
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		SCR_MapEntity.GetOnMapOpen().Insert(OnMapOpen);
		SCR_MapEntity.GetOnMapClose().Insert(OnMapClose);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_CampaignFastTravelComponent()
	{
		SCR_MapEntity.GetOnMapOpen().Remove(OnMapOpen);
		SCR_MapEntity.GetOnMapClose().Remove(OnMapClose);
		SCR_MapEntity.GetOnSelection().Remove(SendPlayerCoords);
	}
}
