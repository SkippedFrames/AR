[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "Base for gamemode scripted component.")]
class SCR_NotificationSenderComponentClass: SCR_BaseGameModeComponentClass
{
};


class SCR_NotificationSenderComponent : SCR_BaseGameModeComponent
{		
	[Attribute("1", desc: "Show player left notification if player left.")]
	protected bool m_bShowPlayerLeftNotification;
	
	[Attribute("10", desc: "Is killfeed enabled and what info will be displayed (Full killfeed is always displayed in open unlimited Editor)", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EKillFeedType))]
	protected EKillFeedType m_iKillFeedType;
	
	[Attribute("40", desc: "If killfeed is enabled, what is the relationship between the player that died and the local player. (Full killfeed is always displayed in open unlimited Editor)", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EKillFeedReceiveType))]
	protected EKillFeedReceiveType m_iReceiveKillFeedType;
	
	[Attribute("0", desc: "Array of Killfeed names")]
	protected ref array<ref SCR_NotificationKillfeedTypeName> m_aKillfeedTypeNames;
	
	[Attribute("0", desc: "Array of Killfeed receive names")]
	protected ref array<ref SCR_NotificationKillfeedreceiveTypeName> m_aKillfeedReceiveTypeNames;
	
	[Attribute("{D3BFEE28E7D5B6A1}Configs/ServerBrowser/KickDialogs.conf", desc: "If disconnect reason has a preset set to it then it will send a kick/ban notification", params: "conf")]
	protected ref SCR_ConfigurableDialogUiPresets m_PlayerKickReasonsConfig;
	
	protected Faction m_FactionOnSpawn;
	
	protected SCR_FactionManager m_FactionManager;
	
	//States
	protected bool m_bListeningToWeatherChanged;
	
	override void OnControllableDestroyed(IEntity entity, IEntity killerEntity, notnull Instigator killer)
	{
		//~ hot fix for On Controllable Destroyed issues \/
		if (Replication.IsClient())
			return;
		
		if (!entity)
			return;
		
		RplComponent entityRpl = RplComponent.Cast(entity.FindComponent(RplComponent));
		RplComponent instigatorRpl;
		
		if (killerEntity)
			instigatorRpl = RplComponent.Cast(killerEntity.FindComponent(RplComponent));
		
		RplId entityRplId = RplId.Invalid();
		RplId instigatorRplId = RplId.Invalid();
		
		if (entityRpl)
			entityRplId = entityRpl.Id();
		
		if (instigatorRpl)
			instigatorRplId = instigatorRpl.Id();
		
		OnControllableDestroyedBroadCast(entityRplId, instigatorRplId);
		Rpc(OnControllableDestroyedBroadCast, entityRplId,instigatorRplId);
		
		//~ hot fix for On Controllable Destroyed issues /\
	}
	
	
	//~ Todo: This is a hot a hotfix for On Controllable Destroyed issues \/
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void OnControllableDestroyedBroadCast(RplId entityRplId, RplId instigatorRplId)
	{
		IEntity entity;
		IEntity instigator;
		
		RplComponent entityRpl = RplComponent.Cast(Replication.FindItem(entityRplId));
		RplComponent instigatorRpl = RplComponent.Cast(Replication.FindItem(instigatorRplId));
		
		if (entityRpl)
			entity = entityRpl.GetEntity();
		
		if (instigatorRpl)
			instigator = instigatorRpl.GetEntity();
		
		OnControllableDestroyedHotfix(entity, instigator);
	}
	//~ hot fix for On Controllable Destroyed  issues /\
	
	//~ Todo: hot fix for On Controllable Destroyed  issues - Move logic to OnControllableDestroyed if fixed
	protected void OnControllableDestroyedHotfix(IEntity entity, IEntity instigator)
	{		
		//~	No entity destroyed
		if (!entity)
			return;
		
		bool isUnlimitedEditorOpened = false;
		
		//~ Check if player has unlimited editor and if the editor is open
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
			isUnlimitedEditorOpened = !editorManager.IsLimited() && editorManager.IsOpened();
		
		//~ Killfeed is disabled and unlimited editor is not open
		if (!isUnlimitedEditorOpened && m_iKillFeedType == EKillFeedType.DISABLED)
			return;
		
		int playerId = SCR_PossessingManagerComponent.GetPlayerIdFromControlledEntity(entity);
		if (playerId <= 0)
			return;
		
		//~ Check if killed player message can be seen if Limited editor (or editor not open)
		if (!isUnlimitedEditorOpened && m_iReceiveKillFeedType != EKillFeedReceiveType.ALL)
		{
			//~ No faction manager so don't show killfeed?
			if (!m_FactionManager)
				return;
			
			Faction localPlayerFaction = m_FactionManager.GetLocalPlayerFaction();

			//~ No local faction so don't show killfeed
			if (!localPlayerFaction)
				return;
			
			int localPlayerID = SCR_PlayerController.GetLocalPlayerId();
			Faction killedPlayerFaction = m_FactionManager.GetPlayerFaction(playerId);
			
			switch (m_iReceiveKillFeedType)
			{
				//~ check if in group
				case EKillFeedReceiveType.GROUP_ONLY :
				{
					//~ Check if local player is not the same as killed otherwise they are always in the same group
					if (localPlayerID != playerId)
					{
						//~ Factions not friendly so don't show killfeed
						if (!localPlayerFaction.IsFactionFriendly(killedPlayerFaction))
							return;
						
						//~ No group manager so don't send
						SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
						if (!groupManager)
							return;
						
						SCR_AIGroup localPlayerGroup = groupManager.GetPlayerGroup(localPlayerID);
						
						//~ If not in group or not in the same group do not send
						if (!localPlayerGroup || !localPlayerGroup.IsPlayerInGroup(playerId))
							return;
					}
					
					break;
				}
				//~ Check if the same faction
				case EKillFeedReceiveType.SAME_FACTION_ONLY :
				{
					//~ Check if local player is not the same as killed otherwise they are always the same faction
					if (localPlayerID != playerId)
					{
						//~ If no local faction or if not the same faction do not show killfeed
						if (!localPlayerFaction || localPlayerFaction != killedPlayerFaction)
							return;
						
						//~ If Faction is hostile towards itself still do not show killfeed (DM)
						if (!localPlayerFaction.IsFactionFriendly(localPlayerFaction))
							return;
					}
					
					break;
				}
				//~ Check if allies
				case EKillFeedReceiveType.ALLIES_ONLY :
				{
					//~ Check if local player is not the same as killed otherwise they are always allied
					if (localPlayerID != playerId)
					{
						//~ Factions not friendly so don't show killfeed
						if (!localPlayerFaction || !localPlayerFaction.IsFactionFriendly(killedPlayerFaction))
							return;
					}
					
					break;
				}
				//~ Check if enemies
				case EKillFeedReceiveType.ENEMIES_ONLY :
				{
					//~ If local player killed it is never an enemy
					if (localPlayerID == playerId)
						return;
					
					//~ Factions friendly so don't show killfeed
					if (localPlayerFaction.IsFactionFriendly(killedPlayerFaction))
						return;
					
					break;
				}
			}
		}
		
		//~ Never show killer so simply show player died if limited editor
		if (!isUnlimitedEditorOpened && m_iKillFeedType == EKillFeedType.UNKNOWN_KILLER)
		{
			SCR_NotificationsComponent.SendLocal(ENotification.PLAYER_DIED, playerId);
			return;
		}
		
		int killerId;
		if (entity == instigator)
			killerId = playerId;
		else 
			killerId = SCR_PossessingManagerComponent.GetPlayerIdFromControlledEntity(instigator);

		SCR_EditableCharacterComponent killerEditableCharacterComponent;
		//~ If there is a killer and the killer is not the player itself
		if (instigator && killerId <= 0)
		{
			killerEditableCharacterComponent = SCR_EditableCharacterComponent.Cast(instigator.FindComponent(SCR_EditableCharacterComponent));
		
			//~ Killer was not character so get killer in vehicle
			if (!killerEditableCharacterComponent)
				killerEditableCharacterComponent = GetKillerFromVehicle(instigator, instigator.IsInherited(Vehicle));
			
			if (killerEditableCharacterComponent)
				killerId = SCR_PossessingManagerComponent.GetPlayerIdFromControlledEntity(killerEditableCharacterComponent.GetOwner());
		}

		bool playerIsPossessed = false; 
		bool killerIsPossessed = false; 
		
		//~ Check if player or killer where possessed
		SCR_PossessingManagerComponent possesionManager = SCR_PossessingManagerComponent.GetInstance();
		if (possesionManager)
		{
			playerIsPossessed = possesionManager.IsPossessing(playerId);
			
			if (killerId > 0)
				killerIsPossessed = possesionManager.IsPossessing(killerId);
		}
			
		//Death notification	
		//Suicide	
		if (playerId == killerId || !instigator)
		{			
			//Player Suicide
			if (!playerIsPossessed)
				SCR_NotificationsComponent.SendLocal(ENotification.PLAYER_DIED, playerId);
			//Possessed Suicide
			else 
				SCR_NotificationsComponent.SendLocalUnlimitedEditor(ENotification.POSSESSED_AI_DIED, playerId);
		}
		//If killed by other player
		else if (killerId > 0)
		{
			//Player killed player
			if (!playerIsPossessed && !killerIsPossessed)
			{
				SCR_NotificationsComponent.SendLocal(ENotification.PLAYER_KILLED_PLAYER, killerId, playerId);
			}
			//Possesed player killed by other player (Show player killed by NPC and for GM: possesed GM killed player)
			else if (!playerIsPossessed && killerIsPossessed)
			{		
				SCR_NotificationsComponent.SendLocalUnlimitedEditor(ENotification.POSSESSED_AI_KILLED_PLAYER, killerId, playerId);
				SCR_NotificationsComponent.SendLocalLimitedEditor(ENotification.AI_KILLED_PLAYER, killerId, playerId);
			}
			//Player killed possessed player (Show to GM only)
			else if (playerIsPossessed && !killerIsPossessed)
			{
				SCR_NotificationsComponent.SendLocalUnlimitedEditor(ENotification.PLAYER_KILLED_POSSESSED_AI, killerId, playerId);
			}
			//Possessed AI Killed Possessed AI (GM Only)
			else
			{
				SCR_NotificationsComponent.SendLocalUnlimitedEditor(ENotification.POSSESSED_AI_KILLED_POSSESSED_AI, killerId, playerId);
			}
		}
		//Killed by NPC
		else if (killerEditableCharacterComponent)
		{
			int killerRplId = Replication.FindId(killerEditableCharacterComponent);
			
			if (!playerIsPossessed)
				SCR_NotificationsComponent.SendLocal(ENotification.AI_KILLED_PLAYER, killerRplId, playerId);
			else 
				SCR_NotificationsComponent.SendLocalUnlimitedEditor(ENotification.AI_KILLED_POSSESSED_AI, killerRplId, playerId);
		}
		//Unknown killer
		else 
		{
			if (!playerIsPossessed)
				SCR_NotificationsComponent.SendLocal(ENotification.PLAYER_DIED, playerId);
			else 
				SCR_NotificationsComponent.SendLocalUnlimitedEditor(ENotification.POSSESSED_AI_DIED, playerId);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_EditableCharacterComponent GetKillerFromVehicle(IEntity veh, bool pilot)
	{
		BaseCompartmentManagerComponent compartmentManager = BaseCompartmentManagerComponent.Cast(veh.FindComponent(BaseCompartmentManagerComponent));
			
		if (!compartmentManager)
			return null;
		
		array<BaseCompartmentSlot> compartments = new array <BaseCompartmentSlot>();
		
		for (int i = 0, compart = compartmentManager.GetCompartments(compartments); i < compart; i++)
		{
			BaseCompartmentSlot slot = compartments[i];
			
			if (pilot && slot.Type() == PilotCompartmentSlot)
			{
				if (slot.GetOccupant())
					return SCR_EditableCharacterComponent.Cast(slot.GetOccupant().FindComponent(SCR_EditableCharacterComponent));
			}
			else if (!pilot && slot.Type() == TurretCompartmentSlot)
			{
				if (slot.GetOccupant())
					return SCR_EditableCharacterComponent.Cast(slot.GetOccupant().FindComponent(SCR_EditableCharacterComponent));
			}
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEditorLimitedChanged(bool isLimited)
	{
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (!editorManager) 
			return;
		
		int playerID = editorManager.GetPlayerID();
		
		//Became Player
		if (isLimited)
			SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_PLAYER_NO_LONGER_GM, playerID);
		//Became GM
		else 
			SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_PLAYER_BECAME_GM, playerID);
	}
	
	override void OnPlayerRegistered(int playerId) 
	{
		//Join notification
		SCR_NotificationsComponent.SendLocal(ENotification.PLAYER_JOINED, playerId);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout) 
	{
		//~ Should never be called for clients but just in case
		if (!GetGameMode().IsMaster())
			return;
		
		SCR_EditorManagerEntity editorManager;
		
		//~ Get editorManager if has any
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (core)
		{
			 editorManager = core.GetEditorManager(playerId);
		}

		bool hasUnlimitedEditor = editorManager && !editorManager.IsLimited();
		
		//~ Is GM, Always show GM left notification even if kicked/banned to notify players that the GM has left
		if (hasUnlimitedEditor)
			SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_GM_LEFT, playerId);		
		
		bool isKickedOrBanned = false;
		
		//~ Check if disconnect cause has message attached to it. If true: show kick/ban reason. If false: only show gm/player left
		if (m_PlayerKickReasonsConfig)
		{
			string groupId, reasonId;
			KickCauseGroup2 groupInt;
			int reasonInt;
			
			//~ Get disconnect message preset
			GetGame().GetFullKickReason(cause, groupInt, reasonInt, groupId, reasonId);
			SCR_ConfigurableDialogUiPreset preset = m_PlayerKickReasonsConfig.FindPreset(groupId + "_" + reasonId);
			
			//~ If has kick/Ban message it will send out a notification
			isKickedOrBanned = preset != null && !preset.m_sMessage.IsEmpty();
		}
		//~ No config
		else 
		{
			Print("'SCR_NotificationSenderComponent' has no 'm_PlayerKickReasonsConfig'! Make sure it is added else it will never know if a player was kicked!", LogLevel.ERROR);
		}
		
		//~ Is kicked/banned. Will also send ban notification if for some reason there is a timeout attached even if there is no specific kick message
		if (isKickedOrBanned || timeout != 0)
		{
			SCR_DataCollectorComponent dataCollector = GetGame().GetDataCollector();
			if (dataCollector)
			{
				SCR_PlayerData playerData = dataCollector.GetPlayerData(playerId);
				
				if (playerData)
				{
					float banTimeOut = playerData.GetTimeOut();
				
					//~ If playerData has ban timeout which is greater then timeout use that instead. This is because Heavy ban kicks the player and bans it via backend. So the timeout is set somewhere else
					if (banTimeOut > 0 && banTimeOut > timeout)
						timeout = banTimeOut;
				}
			}
			
			//~ Player kicked 
			if (timeout == 0)
				SCR_NotificationsComponent.SendToEveryone(ENotification.PLAYER_KICKED, playerId, cause, timeout);	
			//~ Player perminent ban
			else if (timeout < 0)
				SCR_NotificationsComponent.SendToEveryone(ENotification.PLAYER_BANNED_NO_DURATION, playerId, cause, timeout);
			//~ Player temp ban
			else
				SCR_NotificationsComponent.SendToEveryone(ENotification.PLAYER_BANNED, playerId, cause, timeout);
		}		
		//~ Is Not kicked/banned, is player and should send on leave notification. 
		else if (m_bShowPlayerLeftNotification && !hasUnlimitedEditor)
		{
			SCR_NotificationsComponent.SendToEveryone(ENotification.PLAYER_LEFT, playerId);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerSpawned(int playerId, IEntity controlledEntity)
	{
		//Check if faction changed and send notification if diffrent
		if (m_FactionManager) 
		{
			//Get factions using ID
			Faction playerFaction = m_FactionManager.GetPlayerFaction(playerId);
		
			//Faction is diffrent
			if (m_FactionOnSpawn != playerFaction)
			{
				//Send player joined faction notification
				SCR_NotificationsComponent.SendToUnlimitedEditorPlayers(ENotification.PLAYER_JOINED_FACTION, playerId);
				m_FactionOnSpawn = playerFaction;
			}
		}
	}	
	
	

	
	//======================================== WEATHER SET TO LOOPING ========================================\\
	/*!
	Called when weather is set to looping or when weather itself is changed (which auto sets it on looping)
	This is to make sure the notification, that weather is set, is only called once
	*/
	void OnWeatherChangedNotification(int playerID)
	{
		if (Replication.IsClient())
			return;
		
		if (!m_bListeningToWeatherChanged)
		{
			m_bListeningToWeatherChanged = true;
			GetGame().GetCallqueue().CallLater(OnWeatherChangedNotificationDelay, 0 , false, playerID);
		}
	}
	
	protected void OnWeatherChangedNotificationDelay(int playerID)
	{
		m_bListeningToWeatherChanged = false;
		
		ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		if (!world) 
			return;
		
		TimeAndWeatherManagerEntity weatherManager = world.GetTimeAndWeatherManager();
		if (!weatherManager) 
			return;
		
		WeatherState currentState = weatherManager.GetCurrentWeatherState();
		
		if (!currentState)
			return;
		
		SCR_NotificationsComponent.SendToUnlimitedEditorPlayers(ENotification.EDITOR_ATTRIBUTES_WEATHER_CHANGED, playerID, currentState.GetStateID());
	}
	
	
	//======================================== KILLFEED ========================================\\	
	/*!
	Get an list of all killfeed types and the localized name
	\param killFeedTypeNames list of killfeed type and name
	\return the amount of names in the list
	*/
	int GetKillFeedTypeNames(notnull array<ref SCR_NotificationKillfeedTypeName> killFeedTypeNames)
	{
		killFeedTypeNames.Clear();
		foreach (SCR_NotificationKillfeedTypeName killfeedTypeName: m_aKillfeedTypeNames)
			killFeedTypeNames.Insert(killfeedTypeName);
		
		return killFeedTypeNames.Count();
	}
	
	/*!
	Get an list of all killfeed receive types and the localized name
	\param killFeedReceiveTypeNames list of killfeed receive type and name
	\return the amount of names in the list
	*/
	int GetKillFeedReceiveTypeNames(notnull array<ref SCR_NotificationKillfeedreceiveTypeName> killFeedReceiveTypeNames)
	{
		killFeedReceiveTypeNames.Clear();
		foreach (SCR_NotificationKillfeedreceiveTypeName killfeedReceiveTypeName: m_aKillfeedReceiveTypeNames)
			killFeedReceiveTypeNames.Insert(killfeedReceiveTypeName);
		
		return killFeedReceiveTypeNames.Count();
	}
	
	/*!
	Get the current killfeed type
	\return the type of killfeed that is displayed
	*/
	EKillFeedType GetKillFeedType()
	{
		return m_iKillFeedType;
	}
	
	/*!
	Server set killfeed type
	\param killFeedType new killfeed type to set
	\param playerNotificationId add player ID of player that changed the type to display a notification
	*/
	void SetKillFeedType(EKillFeedType killFeedType, int playerNotificationId = -1)
	{
		if (!GetGameMode().IsMaster() || killFeedType == m_iKillFeedType)
			return;
		
		SetKillFeedTypeBroadcast(killFeedType);
		Rpc(SetKillFeedTypeBroadcast, killFeedType);
		
		if (playerNotificationId > 0)
			SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_CHANGED_KILLFEED_TYPE, playerNotificationId, killFeedType, false);
	}
	
	/*!
	Get the current killfeed receive type
	\return the type from whom the player receives killfeed
	*/
	EKillFeedReceiveType GetReceiveKillFeedType()
	{
		return m_iReceiveKillFeedType;
	}
	
	/*!
	Server set killfeed receive type
	\param receiveKillFeedType new killfeed reveive type to set
	\param playerNotificationId add player ID of player that changed the type to display a notification
	*/
	void SetReceiveKillFeedType(EKillFeedReceiveType receiveKillFeedType, int playerNotificationId = -1)
	{
		if (!GetGameMode().IsMaster() || receiveKillFeedType == m_iReceiveKillFeedType)
			return;
		
		SetReceiveKillFeedTypeBroadcast(receiveKillFeedType);
		Rpc(SetReceiveKillFeedTypeBroadcast, receiveKillFeedType);
		
		if (playerNotificationId > 0)
			SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_CHANGED_KILLFEED_RECEIVE_TYPE, playerNotificationId, receiveKillFeedType, true);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void SetKillFeedTypeBroadcast(EKillFeedType killFeedType)
	{
		m_iKillFeedType = killFeedType;
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void SetReceiveKillFeedTypeBroadcast(EKillFeedReceiveType receiveKillFeedType)
	{
		m_iReceiveKillFeedType = receiveKillFeedType;
	}
	
	//======================================== RPL ========================================\\
	override bool RplSave(ScriptBitWriter writer)
    {	
       	writer.WriteInt(m_iKillFeedType); 
       	writer.WriteInt(m_iReceiveKillFeedType); 
		
        return true;
    }
     
    override bool RplLoad(ScriptBitReader reader)
    {
		EKillFeedType killFeedType;
		EKillFeedReceiveType receiveKillFeedType;
		
        reader.ReadInt(killFeedType);
        reader.ReadInt(receiveKillFeedType);

		SetKillFeedTypeBroadcast(killFeedType);
		SetReceiveKillFeedTypeBroadcast(receiveKillFeedType);
		
        return true;
    }
	
	//======================================== INIT ========================================\\
	override void EOnInit(IEntity owner)
	{
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
			editorManager.GetOnLimitedChange().Insert(OnEditorLimitedChanged);
		
		m_FactionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		
		super.EOnInit(owner);
	}
	
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
		
		super.OnPostInit(owner);
	}
	
	
	//======================================== DELETE ========================================\\
	protected override void OnDelete(IEntity owner)
	{
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
			editorManager.GetOnLimitedChange().Remove(OnEditorLimitedChanged);
		
		super.OnDelete(owner);
	}
};

/*!
What kind of killfeed will be displayed in the gamemode. 
Noted that players with unlimited Editor will always see full killfeed if Editor is open
*/
enum EKillFeedType
{
	//~ If changes are made in this enum make sure to change m_aKillfeedTypeNames
	DISABLED = 0, ///< Killfeed is disabled
	UNKNOWN_KILLER = 10, ///< Will see killfeed messages but will never know who the killer is
	FULL = 20, ///< Will see every player (except possessed pawns) killed messages. Including killer
};


/*!
From who will players receive kill notifications
Noted that players with unlimited Editor will always see full killfeed if Editor is open
*/
enum EKillFeedReceiveType
{
	//~ If changes are made in this enum make sure to change m_aKillfeedReceiveTypeNames
	ALL = 0, ///< Will see killfeed from allies and enemies alike
	ENEMIES_ONLY = 10, ///< Will see killfeed from enemies only
	ALLIES_ONLY = 20, ///< Will see killfeed from allies only
	SAME_FACTION_ONLY = 30, ///< Will see killfeed from same faction only
	GROUP_ONLY = 40, ///< Will see killfeed from group members only
};

/*!
Class to get Killfeed type name
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EKillFeedType, "m_iKillfeedType")]
class SCR_NotificationKillfeedTypeName
{
	[Attribute(desc: "Killfeed type", uiwidget : UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EKillFeedType))]
	protected EKillFeedType m_iKillfeedType;
	
	[Attribute(desc: "Name displayed in Notification", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sKillfeedTypeName;
	
	/*!
	Get the killfeed type
	\return the killfeed type enum
	*/
	EKillFeedType GetKillfeedType()
	{
		return m_iKillfeedType;
	}
	
	/*!
	Get the killfeed Type name
	\return the killfeed type name as string
	*/
	string GetName()
	{
		return m_sKillfeedTypeName;
	}
};

/*!
Class to get Killfeed receive type name
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EKillFeedReceiveType, "m_iKillfeedreceiveType")]
class SCR_NotificationKillfeedreceiveTypeName
{
	[Attribute(desc: "Killfeed receive type", uiwidget : UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EKillFeedReceiveType))]
	protected EKillFeedReceiveType m_iKillfeedreceiveType;
	
	[Attribute(desc: "Name displayed in Notification", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sKillfeedReceiveTypeName;
	
	/*!
	Get the killfeed receive Type
	\return the killfeed receive type enum
	*/
	EKillFeedType GetKillfeedReceiveType()
	{
		return m_iKillfeedreceiveType;
	}
	
	/*!
	Get the killfeed receive type name
	\return the killfeed receive type name as string
	*/
	string GetName()
	{
		return m_sKillfeedReceiveTypeName;
	}
};

