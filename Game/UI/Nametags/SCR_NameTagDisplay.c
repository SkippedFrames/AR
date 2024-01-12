//------------------------------------------------------------------------------------------------
//! Nametag UI 
//! Attached to SCR_HUDManagerComponent which is a component of SCR_PlayerController
class SCR_NameTagDisplay : SCR_InfoDisplayExtended
{																
	const int PREPARED_WIDGETS = 10;		// amount of prepared widgets for nametags, more are created dynamically if required
	const int PREPARED_VEH_WIDGETS = 3;		// amount of prepared widgets for vehicle nametags, more are created dynamically if required 
	
	// Arrays		
	protected ref array<IEntity> m_aFilteredEntities = new array<IEntity>();		// filtered entities which await nametag creation
	protected ref array<ref SCR_NameTagData> m_aUninitializedTags = {};				// prepared/inactive nametag widgets 
	protected ref array<ref SCR_VehicleTagData> m_aUninitializedVehTags = {};		// prepared/inactive vehicle nametag widgets				
	protected ref array<ref SCR_NameTagData> m_aNameTags = {};						// nametags
	protected ref map<IEntity, ref SCR_NameTagData> m_aNameTagEntities = new map<IEntity, ref SCR_NameTagData>(); 	// entity > nametag map
	// Vars
	protected bool m_bIsRulesetInit = false;		// ruleset init successful
	protected bool m_bSleepDisplay = false;			// stop updating nametags flag
	protected int m_iCurrentPlayerID;				// ID of current player
	
	ref SCR_NameTagData m_CurrentPlayerTag;			// nametag data of current player									
	protected Faction m_CurrentFaction;						
	protected PlayerManager m_PlayerManager;
	
	protected static ref SCR_NameTagConfig s_NametagCfg;
			
	//------------------------------------------------------------------------------------------------
	// Getters
	//------------------------------------------------------------------------------------------------
	//! Get zone array from assigned zone config
	//! \return array of nametag zones
	static array<ref SCR_NameTagZone> GetNametagZones()
	{								
		if (!s_NametagCfg)
			return null;
					
		return s_NametagCfg.m_aZones;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Retrieve nametag from entity map
	SCR_NameTagData GetEntityNameTag(IEntity ent)
	{		
		SCR_NameTagData nametag = m_aNameTagEntities.Get(ent);
		return nametag;
	}
		
	//------------------------------------------------------------------------------------------------
	// Events
	//------------------------------------------------------------------------------------------------
		
	//------------------------------------------------------------------------------------------------
	//! SCR_GameModeBase event
	protected void OnControllableDestroyed(IEntity entity, IEntity killerEntity, notnull Instigator killer)
	{
		if (!m_CurrentPlayerTag)
			return;
		
		if (entity == m_CurrentPlayerTag.m_Entity)	// cleanup after death
		{
			CleanupAllTags();
			m_CurrentPlayerTag = null;
			StopUpdate();
		}
			
		SCR_NameTagData data = m_aNameTagEntities.Get(entity);
		if (data)
		{
			data.ActivateEntityState(ENameTagEntityState.DEAD);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_GameModeBase event
	protected void OnControllableDeleted(IEntity entity)
	{
		if (!m_CurrentPlayerTag)
			return;
		
		if (entity == m_CurrentPlayerTag.m_Entity)	// cleanup after deletion (such as GM delete)
		{
			CleanupAllTags();
			m_CurrentPlayerTag = null;
			StopUpdate();
		}
		
		SCR_NameTagData data = m_aNameTagEntities.Get(entity);
		if (data)
			data.Cleanup();
	}
		
	//------------------------------------------------------------------------------------------------
	void OnNewVehicleOccupant(IEntity vehicle, SCR_NameTagData occupant)
	{
		if (!m_CurrentPlayerTag)
			return;
		
		bool isControlled = m_CurrentPlayerTag == occupant;
		
		SCR_VehicleTagData vehData = SCR_VehicleTagData.Cast(m_aNameTagEntities.Get(vehicle));
		if (vehData)
			vehData.UpdatePassenger(occupant, true, isControlled);
		else 
		{
			// init tag & insert into arrays			
			if (m_aUninitializedVehTags.IsEmpty())
				CreateTagWidget(ENameTagEntityType.VEHICLE);
			
			SCR_VehicleTagData tag = m_aUninitializedVehTags[0];
			if ( tag.InitTag(this, vehicle, s_NametagCfg) )
			{	
				m_aNameTags.Insert(tag);
				m_aNameTagEntities.Insert(vehicle, tag);
				tag.UpdatePassenger(occupant, true, isControlled);
			}
			
			m_aUninitializedVehTags.Remove(0);
		
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnLeaveVehicleOccupant(IEntity vehicle, SCR_NameTagData occupant)
	{
		if (!m_CurrentPlayerTag)
			return;
		
		bool isControlled = m_CurrentPlayerTag == occupant;
		
		SCR_VehicleTagData vehData = SCR_VehicleTagData.Cast(m_aNameTagEntities.Get(vehicle));
		if (vehData)
			vehData.UpdatePassenger(occupant, false, isControlled);
	}
	
	//------------------------------------------------------------------------------------------------
	// SCR_AIGroup event OnPlayerAdded
	protected void OnGroupJoined(SCR_AIGroup group, int playerID)
	{
		IEntity ent = m_PlayerManager.GetPlayerControlledEntity(playerID);
		if (ent)
		{
			SCR_NameTagData tagData = m_aNameTagEntities.Get(ent);
			if (tagData)
				tagData.SetGroup(group);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// SCR_AIGroup event OnPlayerRemoved
	protected  void OnGroupLeft(SCR_AIGroup group, int playerID)
	{
		IEntity ent = m_PlayerManager.GetPlayerControlledEntity(playerID);
		if (ent)
		{
			SCR_NameTagData tagData = m_aNameTagEntities.Get(ent);
			if (tagData)
				tagData.SetGroup(null);
		}
	}
			
		
	//------------------------------------------------------------------------------------------------
	// Display methods
	//------------------------------------------------------------------------------------------------
	//! Decide which entities should have their nametags drawn
	//! \param timeSlice is the OnFrame timeslice	
	protected void Update(float timeSlice)
	{	
		ProcessFiltered();	// Process newly spawned entities & add to array
		
		if (!m_CurrentPlayerTag)	// if no controlled ent exists, stop here
			return;
								
		// update entities
		if (s_NametagCfg.m_aSourceEntities.UpdatePerformed(timeSlice, m_CurrentPlayerTag))	// only fetch the array if it updated
			m_aFilteredEntities = s_NametagCfg.m_aSourceEntities.GetEntities();
				
		// update ruleset
		if (!m_bIsRulesetInit)
			m_bIsRulesetInit = s_NametagCfg.m_aVisibilityRuleset.Init(this, s_NametagCfg);
		else
			s_NametagCfg.m_aVisibilityRuleset.Update(timeSlice, m_aNameTags, m_CurrentPlayerTag);
		
		/*DbgUI.Begin("NameTag debug");
		string dbg = "nametags: %1 | filtered: %2 | uninit tags: %3";
		DbgUI.Text( string.Format( dbg, m_aNameTags.Count(), m_aFilteredEntities.Count(), m_aUninitializedTags.Count() ) );
		DbgUI.End();*/
	}
			
	//------------------------------------------------------------------------------------------------
	//! Update all elements of the requested tag
	//! \param data is nametag data struct
	void UpdateTagElements(SCR_NameTagData data)
	{
		if (data.m_iZoneID != -1)
			s_NametagCfg.m_aZones[data.m_iZoneID].UpdateElementDefs(data);
	}
										
	//------------------------------------------------------------------------------------------------
	//! Check if entity is faction friendly to current players faction
	//! \param entity is a player entity
	//! \return Returns true if checked entity is faction friendly
	protected bool IsFactionFriendly(IEntity entity)
	{
		FactionAffiliationComponent factionComponent = FactionAffiliationComponent.Cast( entity.FindComponent( FactionAffiliationComponent ) );			
		
		if (!factionComponent)
			return false;
		
		if (m_CurrentFaction)
		{
			Faction faction = factionComponent.GetAffiliatedFaction();
			if (!faction)
				return false;
			
			//  Is friendly
			if ( faction.IsFactionFriendly(m_CurrentFaction) )
				return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Adjust zone range based on zoom
	//! \param adjust determines if we are adjusting the ranges or restorign them to default
	//! \param FOVZoomed is the new FOV we are adjusting to
	protected void AdjustRange(bool adjust, float FOVZoomed)
	{
		if (!s_NametagCfg.m_bAdjustZoneRangeOnZoom)
			return;
		
		if (adjust)
			s_NametagCfg.AdjustZoneRanges(FOVZoomed);
		else 
			s_NametagCfg.RestoreZoneRanges();

	}
					
	//------------------------------------------------------------------------------------------------
	// Data methods
	//------------------------------------------------------------------------------------------------
	//! Basic initialization of invokers
	protected void InitNameTags()
	{										
		// Spawn/death events
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast( GetGame().GetGameMode() );
		if (gameMode)
		{
			gameMode.GetOnControllableDestroyed().Insert(OnControllableDestroyed);
			gameMode.GetOnControllableDeleted().Insert(OnControllableDeleted);
		}
		
		SCR_2DOpticsComponent.s_OnSightsADSChanged.Insert(AdjustRange);
		
		SCR_AIGroup.GetOnPlayerAdded().Insert(OnGroupJoined);
		SCR_AIGroup.GetOnPlayerRemoved().Insert(OnGroupLeft);
		
		// init ruleset
		m_bIsRulesetInit = s_NametagCfg.m_aVisibilityRuleset.Init(this, s_NametagCfg);
		
		for (int i = 0; i < PREPARED_WIDGETS; i++)
		{
			CreateTagWidget();
		}
		
		for (int i = 0; i < PREPARED_VEH_WIDGETS; i++)
		{
			CreateTagWidget(ENameTagEntityType.VEHICLE);
		}
	}
		
	//------------------------------------------------------------------------------------------------
	//! Process newly gathered entities 
	protected void ProcessFiltered()
	{
		if (!m_CurrentPlayerTag)	// controlled player tag needs to be init first
		{
			IEntity curEntity = SCR_PlayerController.GetLocalControlledEntity();
			if (curEntity)
				InitializeTag(curEntity);
			else 
				return;
		}
		
		if ( !m_aFilteredEntities.IsEmpty() )
		{
			int count =  m_aFilteredEntities.Count();
			bool processedTag;
			
			for (int i; i <  count; i++)
			{		
				IEntity ent = m_aFilteredEntities[i];
									
				if ( ent && !m_aNameTagEntities.Get(ent) )
				{		
					InitializeTag(ent);
					processedTag = true;
				}
				
				m_aFilteredEntities.Remove(i);
				i--;
				count--;
				// Only process one per frame to spread the load
				if (processedTag)
					return;
			}
		}
	}
		
	//------------------------------------------------------------------------------------------------
	//! Initialize name tag contex instance
	//! \param entity is a character entity
	//! \param friendlyOnly determines whether faction check is required for creation
	protected void InitializeTag(IEntity entity, bool friendlyOnly = true)
	{
		if (!m_wRoot)
			return;
		
		// if this is a current controlled entity
		if (entity == SCR_PlayerController.GetLocalControlledEntity())
		{
			if (m_aUninitializedTags.IsEmpty())
				CreateTagWidget();
			
			// init tag & insert into arrays
			SCR_NameTagData tag = m_aUninitializedTags[0];
			tag.InitTag(this, entity, s_NametagCfg, true);
			m_CurrentPlayerTag = tag;
			
			m_aNameTags.Insert(tag);
			m_aNameTagEntities.Insert(entity, tag);
			m_aUninitializedTags.Remove(0);
			
			// Init FactionAffiliationComponent, required to detect factions
			FactionManager factionMgr = GetGame().GetFactionManager();
			if (factionMgr)
			{
				FactionAffiliationComponent factionComponent = FactionAffiliationComponent.Cast( entity.FindComponent(FactionAffiliationComponent) );
				if (factionComponent)
					m_CurrentFaction = factionComponent.GetAffiliatedFaction();
			}
		
			return;
		}
		
		if (friendlyOnly)
		{
			if ( !IsFactionFriendly(entity) ) 	// Do not create tags for enemy factions
				return;
		}
			
		// init tag & insert into arrays			
		if (m_aUninitializedTags.IsEmpty())
			CreateTagWidget();
		
		SCR_NameTagData tag = m_aUninitializedTags[0];
		if ( tag.InitTag(this, entity, s_NametagCfg) )
		{	
			m_aNameTags.Insert(tag);
			m_aNameTagEntities.Insert(entity, tag);
			m_aUninitializedTags.Remove(0);
		}
	}
	//------------------------------------------------------------------------------------------------
	//! Create widget for nametag
	protected void CreateTagWidget(ENameTagEntityType type = 0)
	{
		if (type == ENameTagEntityType.VEHICLE)
		{
			SCR_VehicleTagData tagData = new SCR_VehicleTagData(s_NametagCfg.m_sNametagLayout, m_wRoot);
			m_aUninitializedVehTags.Insert(tagData);
		}	
		else 
		{
			SCR_NameTagData tagData = new SCR_NameTagData(s_NametagCfg.m_sNametagLayout, m_wRoot);
			m_aUninitializedTags.Insert(tagData);
		}
	}
		
	//------------------------------------------------------------------------------------------------
	// Cleanup methods
	//------------------------------------------------------------------------------------------------
	//! Unregister events and move to unitialized tag pool
	//! \param data is the subject nametag
	//! \param removeFromArray determines whether the tag is removed from main array, this is not desired when entrire array is being cleaned up
	void CleanupTag(notnull SCR_NameTagData data, bool removeFromArray = true)
	{	
		if (data.m_NameTagWidget)	
			data.m_NameTagWidget.SetVisible(false);
		
		// Vehicle enter/leave event
		if (data.m_Entity)
		{
			SCR_CompartmentAccessComponent accessComp = SCR_CompartmentAccessComponent.Cast( data.m_Entity.FindComponent(SCR_CompartmentAccessComponent) );
			if (accessComp)
			{
				accessComp.GetOnCompartmentEntered().Remove(data.OnVehicleEntered);
				accessComp.GetOnCompartmentLeft().Remove(data.OnVehicleLeft);
			}
		}
		
		if ( (data.m_Flags & ENameTagFlags.VEHICLE) && data.m_VehicleParent )	// remove from vehicle if disconnected without triggering event
			data.RemoveVehicleOccupant(data.m_VehicleParent.m_Entity);
		
		if (SCR_VehicleTagData.Cast(data))
			m_aUninitializedVehTags.Insert(SCR_VehicleTagData.Cast(data));
		else 
			m_aUninitializedTags.Insert(data);
		
		data.ResetTag();
		
		m_aNameTagEntities.Remove(data.m_Entity);
		
		if (removeFromArray)
			m_aNameTags.RemoveItem(data);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Unregister events of all tags and move to unitialized tag pool
	void CleanupAllTags()
	{
		for (int i = 0; i < m_aNameTags.Count(); i++ )	// refresh count every iteration because cleanup of all tags from a vehicle will trigger its own cleanup and adjust the count
		{
			m_aNameTags[i].Cleanup(false);
		}
		
		m_aNameTags.Clear();
		m_aNameTagEntities.Clear();
	} 
		
	//------------------------------------------------------------------------------------------------
	//! Clear all tags and their widgets
	//! \param destroyWidgets determines whether the widgets are kept or destroyed
	protected void CleanupDisplay(bool destroyWidgets)
	{						
		int tagCount = m_aNameTags.Count();
		SCR_NameTagData tag;
		
		for ( int i = 0; i < tagCount; i++ )
		{
			tag = m_aNameTags.Get(i) ;
			if (!tag || !tag.m_NameTagWidget)
				continue;
			
			if (destroyWidgets)
				tag.m_NameTagWidget.RemoveFromHierarchy();
			else
				tag.m_NameTagWidget.SetVisible(false);
		}
		
		if (destroyWidgets)
		{
			tagCount = m_aUninitializedTags.Count();
			for ( int i = 0; i < tagCount; i++ )
			{
				tag = m_aUninitializedTags.Get(i);
				if (!tag || !tag.m_NameTagWidget)
					continue;
				
				tag.m_NameTagWidget.RemoveFromHierarchy();
			}
			
			tagCount = m_aUninitializedVehTags.Count();
			for ( int i = 0; i < tagCount; i++ )
			{
				tag = m_aUninitializedVehTags.Get(i);
				if (!tag || !tag.m_NameTagWidget)
					continue;
				
				tag.m_NameTagWidget.RemoveFromHierarchy();
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Stop updating nametags
	void StopUpdate()
	{
		m_bSleepDisplay = true;
		CleanupDisplay(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Debug
	protected void UpdateDebug()
	{
		DbgUI.Begin("NameTag debug");
		string dbg = "nametags: %1 | filtered: %2 | sleep: %3 | uninit veh: %4 | range: %5 ";
		DbgUI.Text( string.Format( dbg, m_aNameTags.Count(), m_aFilteredEntities.Count(), m_bSleepDisplay.ToString(), m_aUninitializedVehTags.Count(), s_NametagCfg.m_fFarthestZoneRange ) );
		if (m_CurrentPlayerTag)
		{
			DbgUI.Text("groupID: " + m_CurrentPlayerTag.m_iGroupID.ToString());
		}
		
		string line =  "name: %1 | distance: %2 | opacity base: %3 | opacity fade: %4 | %5";
		foreach ( SCR_NameTagData tag : m_aNameTags )
		{
			DbgUI.Text( string.Format( line, tag.m_sName, (int)tag.m_fDistance, tag.m_fVisibleOpacity, tag.m_fOpacityFade, tag.m_NameTagWidget.GetOpacity() ));
		}
		
		DbgUI.End();
	}
	
	#ifndef DISABLE_NAMETAGS
		//------------------------------------------------------------------------------------------------
		// Overrides
		//------------------------------------------------------------------------------------------------
		override void DisplayUpdate(IEntity owner, float timeSlice)
		{				
			if (!m_bSleepDisplay)				
				Update(timeSlice);
		
			#ifdef NAMETAG_DEBUG
				UpdateDebug();
			#endif
		}
		
		//------------------------------------------------------------------------------------------------
		override bool DisplayStartDrawInit(IEntity owner)
		{
			// owner not controller
			PlayerController playerController = PlayerController.Cast(owner);
			if (!playerController)
				return false;
			
			m_iCurrentPlayerID = playerController.GetPlayerId();
			
			m_PlayerManager = GetGame().GetPlayerManager();;
			if (!m_PlayerManager)
				return false;
		
			// Load config
			BaseGameMode gameMode = GetGame().GetGameMode();
			if (!gameMode)
				return false;
		
			SCR_NametagConfigComponent configComp = SCR_NametagConfigComponent.Cast(gameMode.FindComponent(SCR_NametagConfigComponent));
			if (!configComp)
				return false;
		
			ResourceName path = configComp.GetConfigPath();
			if (path.IsEmpty())
				return false;
			
			Resource container = BaseContainerTools.LoadContainer(path);
			if (!container)
				return false;
					
			s_NametagCfg = SCR_NameTagConfig.Cast( BaseContainerTools.CreateInstanceFromContainer( container.GetResource().ToBaseContainer() ) );
			if (s_NametagCfg.m_iZoneCount == 0 || s_NametagCfg.m_aZones[0].m_iElementsCount == 0)	// no zones or elements in primary zone
				return false;
			
			if (s_NametagCfg.m_aSourceEntities)
				s_NametagCfg.m_aSourceEntities.Init(s_NametagCfg, this);
					
			return true;
		}
	
		//------------------------------------------------------------------------------------------------
		override void DisplayStartDraw(IEntity owner)
		{
			InitNameTags();
		}
	
		//------------------------------------------------------------------------------------------------
		//! Reinit curent player tag after new entity is controlled
		override void DisplayControlledEntityChanged(IEntity from, IEntity to)
		{
			CleanupAllTags();
			m_CurrentPlayerTag = null;
			
			m_aFilteredEntities.Insert(to);
			ProcessFiltered();
			
			if (to)
				m_bSleepDisplay = false;
			else 
				m_bSleepDisplay = true;
		}	
		
		//------------------------------------------------------------------------------------------------
		override void DisplayStopDraw(IEntity owner)
		{		
			CleanupDisplay(true);
		
			// Spawn/death events
			SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast( GetGame().GetGameMode() );
			if (gameMode)
			{
				gameMode.GetOnControllableDestroyed().Remove(OnControllableDestroyed);
				gameMode.GetOnControllableDeleted().Remove(OnControllableDeleted);
			}
			
			SCR_2DOpticsComponent.s_OnSightsADSChanged.Remove(AdjustRange);
		
			if (SCR_AIGroup.GetOnPlayerAdded())
				SCR_AIGroup.GetOnPlayerAdded().Remove(OnGroupJoined);
		
			if (SCR_AIGroup.GetOnPlayerRemoved())
				SCR_AIGroup.GetOnPlayerRemoved().Remove(OnGroupLeft);
		
			m_bIsRulesetInit = false;
			s_NametagCfg = null;
		
			m_aFilteredEntities.Clear();
			m_aUninitializedTags.Clear();
			m_aUninitializedVehTags.Clear();
			m_aNameTags.Clear();
			m_aNameTagEntities.Clear();
		}
	#endif
};
