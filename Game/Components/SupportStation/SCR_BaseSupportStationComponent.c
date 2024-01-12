[ComponentEditorProps(category: "GameScripted/SupportStation", description: "")]
class SCR_BaseSupportStationComponentClass : ScriptComponentClass
{
	[Attribute("0", desc: "If false then the object has no physics to move itself. If true it means the entity is a vehicle (or part of a vehicle) and it can be moved via physics which means it checks the rigid body if it has a velocity and does not allow to be used if it has", category: "General Settings")]
	protected bool m_bIsVehicle;

	[Attribute("1", desc: "If true it will send out a notification if the user uses it. The Inherented class does need to support notifications for it to be send", category: "Specific Settings")]
	protected bool m_bSendNotificationOnUse;

	[Attribute("0", desc: "If true it allows the support station to get the 'FactionAffiliationComponent' from parent. This is only done when no 'FactionAffiliationComponent' is found on self and main use case is for vehicle compartments. Only used if 'FactionUsageCheck' is set", category: "Faction Settings")]
	protected bool m_bAllowGetFactionFromParent;

	[Attribute("{9DD9C6279F4489B4}Sounds/Editor/SupportStations/SupportStations_Vehicles.acp", desc: "File that contains the sound effect", params: "acp", category: "On use Audio")]
	protected ResourceName m_sOnUseSoundEffectFile;

	[Attribute(desc: "Sound effect played when executing is done. Broadcast to players. Leave empty if no sfx", category: "On use Audio")]
	protected string m_sOnUseSoundEffectEventName;

	[Attribute(desc: "Sound played from from user on successfully action. (Some support station might play action on action owner instead of user). Leave empty to not play any voice effects", category: "On use Audio")]
	protected string m_sCharacterVoiceEventOnUse;

	[Attribute("40", desc: "Priority of voice played on use", params: "0 inf 1", category: "On use Audio")]
	protected int m_iCharacterVoicePriority;

	protected ref SCR_AudioSourceConfiguration m_OnUseAudioSourceConfiguration;

	//------------------------------------------------------------------------------------------------
	/*!
	Get if notification should be shown on Action executed
	\return True if should show notification
	*/
	bool GetSendNotificationOnUse()
	{
		return m_bSendNotificationOnUse;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	If false then the object has no physics to move it self. If true it means it can be moved via physics which means it checks the rigid body if it has a velocity and does not allow to be used if it has
	\return false if no physics can move it
	*/
	bool CanMoveWithPhysics()
	{
		return m_bIsVehicle;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Check if prefab allows the faction affiliation component to be obtained from parent
	Used in init
	\return If allowed to get from parent
	*/
	bool AllowGetFactionFromParent()
	{
		return m_bAllowGetFactionFromParent;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get Audio config to play
	Will create it if it not yet exists. Returns null if no SoundFile or SoundEvent is set
	\return Sound Config
	*/
	SCR_AudioSourceConfiguration GetOnUseAudioConfig()
	{
		//~ Create Audio source if it does not yet exist
		if (!m_OnUseAudioSourceConfiguration)
		{
			if (SCR_StringHelper.IsEmptyOrWhiteSpace(m_sOnUseSoundEffectFile) || SCR_StringHelper.IsEmptyOrWhiteSpace(m_sOnUseSoundEffectEventName))
				return null;

			m_OnUseAudioSourceConfiguration = new SCR_AudioSourceConfiguration();
			m_OnUseAudioSourceConfiguration.m_sSoundProject = m_sOnUseSoundEffectFile;
			m_OnUseAudioSourceConfiguration.m_sSoundEventName = m_sOnUseSoundEffectEventName;

			if (!CanMoveWithPhysics())
				m_OnUseAudioSourceConfiguration.m_eFlags = SCR_Enum.SetFlag(m_OnUseAudioSourceConfiguration.m_eFlags, EAudioSourceConfigurationFlag.Static);
			else
				m_OnUseAudioSourceConfiguration.m_eFlags = SCR_Enum.RemoveFlag(m_OnUseAudioSourceConfiguration.m_eFlags, EAudioSourceConfigurationFlag.Static);
		}

		return m_OnUseAudioSourceConfiguration;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get the character voice varriables on support station use
	\param[out] characterVoiceEventOnUse Voice event to use
	\param[out] characterVoicePriority priority of voice event
	*/
	void GetCharacterVoiceSoundEventVariables(out string characterVoiceEventOnUse, out int characterVoicePriority)
	{
		characterVoiceEventOnUse = m_sCharacterVoiceEventOnUse;
		characterVoicePriority = m_iCharacterVoicePriority;
	}
};

//------------------------------------------------------------------------------------------------
class SCR_BaseSupportStationComponent : ScriptComponent
{
	[Attribute("0", desc: "Priority dictates the array order when trying to get the first availible support station. Higher priority means it is closer to the start of the array. Think of grabbing a composition refuel station before a vehicle refuel station.", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(ESupportStationPriority), category: "General Settings")]
	protected ESupportStationPriority m_eSupportStationPriority;

	[Attribute("7", desc: "In meters, Range in which the entity which the player interacts is needs to be. If -1 it will not add itself to the Support Station Manager and can only be used by the action manager that is also a component of the entity this component is part of, note that the action manager will also never check if any are around it only the component on the entity.", category: "General Settings")]
	protected float m_fRange;

	[Attribute("1", desc: "Enable/disable the supply Station.", category: "General Settings")]
	protected bool m_bIsEnabled;

	[Attribute("1", desc: "If true uses supplies", category: "Supplies")]
	protected bool m_bUseSupplies;

	[Attribute("0", desc: "When executing the action and isUsingSupplies this is the base supply cost of the action regardless of any additional supply costs. 0 to have no upfront cost (note inherented classes might still have their own supply cost calculation)", params: "0 inf 1", category: "Supplies")]
	protected int m_iBaseSupplyCostOnUse;

	[Attribute("0", desc: "If Supply station has range it can ignore itself. If you set this false it will also grab itself when looking for stations in range. Think of a fuel tank able to refuel itself. If this is true it cannot if this is false it can. Not that compartments on vehicles are seen as seperate. So a fuel provider cannot refuel itself but it can still refuel the truck for driving", category: "Specific Settings")]
	protected bool m_bIgnoreSelf;

	[Attribute(desc: "Offset of the center of the support station. Note the Area Mesh zone will only visualise this correctly on runtime not in workbench", uiwidget: UIWidgets.Coords, params: "inf inf 0 purpose=coords space=entity", category: "Specific Settings")]
	protected vector m_vOffset;

	[Attribute(desc: "Set faction of user that can use Support Station, will be ignored if non are selected. Requires a 'FactionAffiliationComponent' on the Support Station or (or 'AllowGetFactionFromParent' is true on vehicle compartments). It will always allow interaction if user does not have 'FactionAffiliationComponent'. Default faction is the faction associated with the Initial Support Station faction. Flags are checked in order. DISALLOW ON null is only checked when Default faction check is not set.", uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(ESupportStationFactionUsage), category: "Faction Settings")]
	protected ESupportStationFactionUsage m_eFactionUsageCheck;
	
	[Attribute(desc: "Action name when executing the action. Note that if this is left empty then the default ActionManager action name is used", category: "General Settings")]
	protected LocalizedString m_sOverrideUserActionName;
	
	protected SCR_ResourceConsumer m_ResourceConsumer;
	
	//~ Will send bool over if enabled or not
	protected ref ScriptInvoker Event_OnEnabledChanged;
	
	//Refs
	protected Physics m_Physics;
	protected EDamageState m_eCurrentDamageState = EDamageState.UNDAMAGED;
	protected FactionAffiliationComponent m_FactionAffiliationComponent;
	protected SCR_ServicePointComponent m_ServicePointComponent;
	//~ if the handler exists than supplies are synced with server
	protected SCR_ResourceSystemSubscriptionHandleBase m_ResourceSubscriptionHandleConsumer;

	//======================================== EXECUTE ========================================\\
	/*!
	Executed the support Station effect (Server only).
	Here you can execute things such as removing supplies/fuel from the entity
	\param actionOwner entity that is the owner of the ActionManager which initiated the action
	\param actionUser Entity that started the interaction to use the support station
	\param forceNotification If true will always try to send notification even if it is off. Inherented class does need to support notitification
	*/
	void OnExecutedServer(notnull IEntity actionOwner, notnull IEntity actionUser, notnull SCR_BaseUseSupportStationAction action)
	{
		RplId ownerId;
		RplId userId;
		int playerId;

		FindEntityIds(actionOwner, actionUser, ownerId, userId, playerId);
		
		OnExecute(actionOwner, actionUser, playerId, action);
		Rpc(OnExecuteBroadcast, ownerId, userId, playerId, action.GetActionID());
	}

	//------------------------------------------------------------------------------------------------
	//~ Get ids for replication
	protected void FindEntityIds(IEntity owner, IEntity user, out RplId ownerId, out RplId userId, out int playerId)
	{
		if (owner)
		{
			ownerId = Replication.FindId(owner.FindComponent(RplComponent));
			
			if (ownerId == Replication.INVALID_ID)
			{
				IEntity parent = owner.GetParent();
				if (parent)
					ownerId = Replication.FindId(parent.FindComponent(RplComponent));
			}				
		}
		
		if (user)
			userId = Replication.FindId(user.FindComponent(RplComponent));
		
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (playerManager)
			playerId = playerManager.GetPlayerIdFromControlledEntity(user);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void GetEntitiesFromID(RplId ownerId, RplId userId, out IEntity owner, out IEntity user)
	{
		RplComponent rpl = RplComponent.Cast(Replication.FindItem(ownerId));
		if (rpl)
			owner = rpl.GetEntity();
		
		rpl = RplComponent.Cast(Replication.FindItem(userId));
		if (rpl)
			user = rpl.GetEntity();
	}

	//------------------------------------------------------------------------------------------------
	//~ called by Server to convert ReplicationIDs to entities
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void OnExecuteBroadcast(RplId ownerId, RplId userId, int userPlayerId, int actionId)
	{
		IEntity actionOwner, actionUser;
		GetEntitiesFromID(ownerId, userId, actionOwner, actionUser);

		//~ Get action
		SCR_BaseUseSupportStationAction action;
		if (actionOwner)
		{
			ActionsManagerComponent actionManager = ActionsManagerComponent.Cast(actionOwner.FindComponent(ActionsManagerComponent));

			if (actionManager)
				action = SCR_BaseUseSupportStationAction.Cast(actionManager.FindAction(actionId));
		}

		OnExecute(actionOwner, actionUser, userPlayerId, action);
	}

	//------------------------------------------------------------------------------------------------
	//~ Called by OnExecuteBroadcast and is executed both on server and on client
	//~ playerId can be -1 if the user was not a player
	protected void OnExecute(IEntity actionOwner, IEntity actionUser, int playerId, SCR_BaseUseSupportStationAction action)
	{

	}

	//======================================== AUDIO ========================================\\
	//~ Play SoundEffect
	protected void PlaySoundEffect(SCR_AudioSourceConfiguration audioConfig, notnull IEntity soundOwner, SCR_BaseUseSupportStationAction action)
	{
		//~ No audio given so ignore function. It might be that no event name or file name are assigned
		if (!audioConfig)
			return;

		//~ Get sound component
		SoundComponent soundComponent = SoundComponent.Cast(soundOwner.FindComponent(SoundComponent));

		//~ Has sound component. Play sound with that offset
		if (soundComponent)
		{
			//~ Check if sound can be played. If true simply play the sound and return
			if (soundComponent.GetEventIndex(audioConfig.m_sSoundEventName) > 0)
			{
				if (action)
					soundComponent.SoundEventOffset(audioConfig.m_sSoundEventName, action.GetLocalPositionAction());
				else 
					soundComponent.SoundEventOffset(audioConfig.m_sSoundEventName, vector.Zero);
				return;
			}
			//~ Sound was not played
			else
			{
				Print(string.Format("'SCR_BaseSupportStationComponent': Trying to play sound for '%1' but sound either sound event '%2' or more likely sound file '%3' is not included on the SoundComponent! SCR_SoundManagerEntity is used instead but that means position of sound is not updated if entity moves while sound is playing.", soundOwner, audioConfig.m_sSoundEventName, audioConfig.m_sSoundProject), LogLevel.WARNING);
			}
		}

		//~ No sound manager
		SCR_SoundManagerEntity soundManagerEntity = GetGame().GetSoundManagerEntity();
		if (!soundManagerEntity)
			return;

		vector transform[4];
		soundOwner.GetTransform(transform);
		
		if (action)
			transform[3] = action.GetWorldPositionAction();
		else 
			transform[3] = soundOwner.GetOrigin();

		//~  Play sound
		soundManagerEntity.CreateAndPlayAudioSource(soundOwner, audioConfig, transform);
	}

	//------------------------------------------------------------------------------------------------
	//~ Place voice event for given character if any events are assigned is assigned
	protected void PlayCharacterVoiceEvent(IEntity character)
	{
		if (!character)
			return;

		//~ Grab Communication sound component of character
		CommunicationSoundComponent communicationSoundComponent = CommunicationSoundComponent.Cast(character.FindComponent(CommunicationSoundComponent));
		if (!communicationSoundComponent)
			return;

		//~ Grab class data
		SCR_BaseSupportStationComponentClass classData = SCR_BaseSupportStationComponentClass.Cast(GetComponentData(GetOwner()));
		if (!classData)
			return;

		string characterVoiceEventOnUse;
		int characterVoicePriority;

		//~ Get character voice variables
		classData.GetCharacterVoiceSoundEventVariables(characterVoiceEventOnUse, characterVoicePriority);

		//~ Check if voice has been assigned
		if (!SCR_StringHelper.IsEmptyOrWhiteSpace(characterVoiceEventOnUse))
			communicationSoundComponent.SoundEventPriority(characterVoiceEventOnUse, characterVoicePriority, true);
	}

	//======================================== IS VALID ========================================\\
	/*!
	Executed when action has been executed.
	Here you can execute things such as removing supplies/fuel from the entity
	\param actionOwner entity on which the ActionManager is
	\param actionUser Entity that started the interaction
	\param[out] reasonInvalid Reason why action is invalid
	\param[out] supply cost when using the support station
	*/
	bool IsValid(IEntity actionOwner, IEntity actionUser, SCR_BaseUseSupportStationAction action, vector actionPosition, out ESupportStationReasonInvalid reasonInvalid, out int supplyCost)
	{
		//~ Check if enabled
		if (!IsEnabled())
		{
			//~ Check if uses range and if it should ignore self and is self. In that case simply show not in range
			if (UsesRange() && IgnoreSelf() && actionOwner == GetOwner())
				reasonInvalid = ESupportStationReasonInvalid.NOT_IN_RANGE;
			//~ Inform players if for example trying to repair a vehicle. If the support station is on the Action owner itself than the action will simply not show
			else 
				reasonInvalid = ESupportStationReasonInvalid.DISABLED;

			return false;
		}
		
		//~ If has service point and is not online show not in range
		if (m_ServicePointComponent && m_ServicePointComponent.GetServiceState() != SCR_EServicePointStatus.ONLINE)
		{
			//~ Todo: Add various ESupportStationReasonInvalid for status such as BROKEN
			reasonInvalid = ESupportStationReasonInvalid.NOT_IN_RANGE;
			return false;
		}
		
		//~ Check if it should ignore itself. If so check if the action owner is itself
		if (IgnoreSelf() && actionOwner == GetOwner())
		{
			reasonInvalid = ESupportStationReasonInvalid.NOT_IN_RANGE;
			return false;
		}

		//~ If uses range check if the station is in range
		if (UsesRange())
		{
			//~ Check if in range
			if (vector.DistanceSq(actionPosition, GetPosition()) > GetRange())
			{
				reasonInvalid = ESupportStationReasonInvalid.NOT_IN_RANGE;
				return false;
			}
		}
		
		//~ Check if Station is destroyed
		if (m_eCurrentDamageState == EDamageState.DESTROYED)
		{
			reasonInvalid = ESupportStationReasonInvalid.DESTROYED_STATION;
			return false;
		}

		//~ Check if station is a valid faction
		if (!IsUserValidFaction(actionUser))
		{
			reasonInvalid = ESupportStationReasonInvalid.INVALID_FACTION;
			return false;
		}

		//~ Check if it uses supplies
		if (IsUsingSupplies())
		{
			//~ Set amount of supplies used on action
			supplyCost = GetSupplyCostAction(actionOwner, actionUser, action);

			//~ Check if there are enough supplies
			if (!HasEnoughSupplies(actionOwner, actionUser, supplyCost))
			{
				reasonInvalid = ESupportStationReasonInvalid.NO_SUPPLIES;
				return false;
			}
		}
		//~ No supplies used so set cost -1 so system knows not to display supply amount
		else
		{
			supplyCost = -1;
		}

		//~ Check if the support station is moving
		if (IsMoving())
		{
			reasonInvalid = ESupportStationReasonInvalid.IS_MOVING;
			return false;
		}

		return true;
	}

	//======================================== GETTERS / SETTERS ========================================\\
	//---------------------------------------- Type ----------------------------------------\\
	/*!
	Get Support station type.
	\return type of support station
	*/
	ESupportStationType GetSupportStationType()
	{
		//~  OVERWRITE IN INHERIT CLASS
		return ESupportStationType.NONE;
	}

	//---------------------------------------- Priority ----------------------------------------\\
	/*!
	Get Support station priority.
	\return priority of support station
	*/
	ESupportStationPriority GetSupportStationPriority()
	{
		return m_eSupportStationPriority;
	}


	//---------------------------------------- Enabled ----------------------------------------\\
	/*!
	Set support station enabled. If disabled it will not allow players to use it (Server only)
	\param enable true is enabled
	*/
	void SetEnabled(bool enable)
	{
		if (m_bIsEnabled == enable)
			return;

		SetEnabledBroadcast(enable);
		Rpc(SetEnabledBroadcast, enable);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void SetEnabledBroadcast(bool enable)
	{
		if (m_bIsEnabled == enable)
			return;
		
		m_bIsEnabled = enable;
		
		//~ Create and invoke onEnabledChanged
		if (!Event_OnEnabledChanged)
			Event_OnEnabledChanged = new ref ScriptInvoker;
		
		Event_OnEnabledChanged.Invoke(m_bIsEnabled);
		
		//~ Todo: Needs new mapmarker function and the map marker should have an inherented function that listens to the support station not the otherway around
		//~ Set map marker active or not if any
		/*SCR_CampaignServiceMapDescriptorComponent mapMarker = SCR_CampaignServiceMapDescriptorComponent.Cast(GetOwner().FindComponent(SCR_CampaignServiceMapDescriptorComponent));
		if (mapMarker)
			mapMarker.SetServiceMarkerActive(enable);*/
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get if support station is enabled.
	\return true if enabled
	*/
	bool IsEnabled()
	{
		return m_bIsEnabled;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get scriptinvoker when support station is enabled changes.
	Sends over bool with enabled state
	\return script invoker
	*/
	ScriptInvoker GetOnEnabledChanged()
	{
		if (!Event_OnEnabledChanged)
			Event_OnEnabledChanged = new ref ScriptInvoker;
		
		return Event_OnEnabledChanged;
	}


	//---------------------------------------- IsMoving ----------------------------------------\\
	protected bool IsMoving()
	{
		//~ If has physics assigned check if it is moving. Otherwise return false
		if (!m_Physics)
			return false;

		return m_Physics.GetVelocity() != vector.Zero || m_Physics.GetAngularVelocity() != vector.Zero;
	}

	//---------------------------------------- Faction Check ----------------------------------------\\
	protected bool IsUserValidFaction(IEntity user)
	{
		//~ No Flags set so Ignore
		if (m_eFactionUsageCheck == 0 || !user)
			return true;

		//~ Support station does not have a faction affiliation component so it is always valid
		if (!m_FactionAffiliationComponent)
			return true;

		//~ Check if user has Faction affiliation. If no then user is considered neutral and always true
		FactionAffiliationComponent userFactionAffiliation = FactionAffiliationComponent.Cast(user.FindComponent(FactionAffiliationComponent));
		if (!userFactionAffiliation)
			return true;

		//~ Check if user has Faction. If no then user is considered neutral and always true
		Faction userFaction = userFactionAffiliation.GetAffiliatedFaction();
		if (!userFaction)
			return true;

		Faction supportStationFaction;

		//~ Checks current faction
		if (SCR_Enum.HasFlag(m_eFactionUsageCheck, ESupportStationFactionUsage.SAME_CURRENT_FACTION) || SCR_Enum.HasFlag(m_eFactionUsageCheck, ESupportStationFactionUsage.FRIENDLY_CURRENT_FACTION))
		{
			//~ Only check if has current faction
			supportStationFaction = m_FactionAffiliationComponent.GetAffiliatedFaction();
			if (supportStationFaction)
			{
				//~ If faction is the same
				if (SCR_Enum.HasFlag(m_eFactionUsageCheck, ESupportStationFactionUsage.SAME_CURRENT_FACTION) && supportStationFaction.GetFactionKey() == userFaction.GetFactionKey())
					return true;

				//~ If faction is friendly
				if (SCR_Enum.HasFlag(m_eFactionUsageCheck, ESupportStationFactionUsage.FRIENDLY_CURRENT_FACTION) && supportStationFaction.IsFactionFriendly(userFaction))
					return true;

				//~ Current faction exists but is not the same (or if friendly is checked and is not friendly)
				return false;
			}
		}

		//~ Checks default faction
		if (SCR_Enum.HasFlag(m_eFactionUsageCheck, ESupportStationFactionUsage.SAME_DEFAULT_FACTION) || SCR_Enum.HasFlag(m_eFactionUsageCheck, ESupportStationFactionUsage.FRIENDLY_DEFAULT_FACTION))
		{
			//~ No default faction so return true
			supportStationFaction = m_FactionAffiliationComponent.GetDefaultAffiliatedFaction();
			if (!supportStationFaction)
				return true;

			//~ If default faction is the same
			if (SCR_Enum.HasFlag(m_eFactionUsageCheck, ESupportStationFactionUsage.SAME_DEFAULT_FACTION) && supportStationFaction.GetFactionKey() == userFaction.GetFactionKey())
				return true;

			//~ If default faction is friendly
			if (SCR_Enum.HasFlag(m_eFactionUsageCheck, ESupportStationFactionUsage.FRIENDLY_DEFAULT_FACTION) && supportStationFaction.IsFactionFriendly(userFaction))
				return true;

			//~ Default faction exists but is not the same (or if friendly is checked and is not friendly)
			return false;
		}

		//~	Has no current faction and did not check Default faction. So check if it is allowed to be used neutral
		return !SCR_Enum.HasFlag(m_eFactionUsageCheck, ESupportStationFactionUsage.DISALLOW_USE_ON_CURRENT_FACTION_NULL);
	}

	//---------------------------------------- Faction Check ----------------------------------------\\
	/*!
	Get current faction of Support Station (Or if allowed get default faction)
	\return Faction of Support Station. Returns null if no faction is assigned
	*/
	SCR_Faction GetFaction()
	{
		if (!m_FactionAffiliationComponent)
			return null;

		SCR_Faction faction = SCR_Faction.Cast(m_FactionAffiliationComponent.GetAffiliatedFaction());
		if (faction)
			return faction;
		
		//~ If allow get default do that if no faction was found
		if (!SCR_Enum.HasFlag(m_eFactionUsageCheck, ESupportStationFactionUsage.DISALLOW_USE_ON_CURRENT_FACTION_NULL))
			faction = SCR_Faction.Cast(m_FactionAffiliationComponent.GetDefaultAffiliatedFaction());

		return faction;
	}

	//======================================== SUPPLIES ========================================\\
	//~ Get the minimum amount of supplies needed for the action. This is the amount of supplies needed to pass the IsValid check when using supplies and when executed
	//~ Function is generally overwritten in inherented support stations
	protected int GetSupplyCostAction(IEntity actionOwner, IEntity actionUser, SCR_BaseUseSupportStationAction action)
	{
		return m_iBaseSupplyCostOnUse;
	}

	//------------------------------------------------------------------------------------------------
	//~ Scan for the max availible supplies that can be used for the action
	int GetMaxAvailableSupplies()
	{
		if (!m_ResourceConsumer || !IsUsingSupplies())
			return int.MAX;
		
		//~ Make sure to create or poke the consumer handler so server sends updates for clients
		if (Replication.IsClient())
		{
			if (m_ResourceSubscriptionHandleConsumer)
				m_ResourceSubscriptionHandleConsumer.Poke();
			else
				m_ResourceSubscriptionHandleConsumer = GetGame().GetResourceSystemSubscriptionManager().RequestSubscriptionListenerHandleGraceful(m_ResourceConsumer, Replication.FindId(SCR_ResourcePlayerControllerInventoryComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_ResourcePlayerControllerInventoryComponent))));
		}
		//~ Simply update if server
		else
		{
			GetGame().GetResourceGrid().UpdateInteractor(m_ResourceConsumer);
		}
		
		return m_ResourceConsumer.GetAggregatedResourceValue();
	}

	//------------------------------------------------------------------------------------------------
	//~ Check if enough supplies to execute the action
	protected bool HasEnoughSupplies(IEntity actionOwner, IEntity actionUser, int supplyAmountToCheck)
	{
		if (supplyAmountToCheck <= 0 || !IsUsingSupplies())
			return true;
		
		//~ Make sure to create or poke the consumer handler so server sends updates for clients
		if (Replication.IsClient())
		{
			if (m_ResourceSubscriptionHandleConsumer)
				m_ResourceSubscriptionHandleConsumer.Poke();
			else
				m_ResourceSubscriptionHandleConsumer = GetGame().GetResourceSystemSubscriptionManager().RequestSubscriptionListenerHandleGraceful(m_ResourceConsumer, Replication.FindId(SCR_ResourcePlayerControllerInventoryComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_ResourcePlayerControllerInventoryComponent))));
		}
		//~ Simply update if server
		else
		{
			GetGame().GetResourceGrid().UpdateInteractor(m_ResourceConsumer);
		}
		
		//~ Check availability 		
		return m_ResourceConsumer.GetAggregatedResourceValue() >= supplyAmountToCheck;
	}

	//------------------------------------------------------------------------------------------------
	//~ Action is executed and supplies are used (Server Only)
	//~ Returns true if there were enough supplies and they are removed
	protected bool OnConsumeSuppliesServer(int amount)
	{
		if (amount <= 0 || !IsUsingSupplies())
			return true;

		//~ Check if consumption was successful 
		SCR_ResourceConsumtionResponse response = m_ResourceConsumer.RequestConsumtion(amount);
		
		return response.GetReason() == EResourceReason.SUFFICIENT;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Set support station uses supplies or not. It will never look if it has enough supplies if disabled (Server only)
	\param useSupplies true is Uses supplies
	*/
	void SetUseSupplies(bool useSupplies)
	{
		if (!m_ResourceConsumer || m_bUseSupplies == useSupplies)
			return;

		SetUseSuppliesBroadcast(useSupplies);
		Rpc(SetUseSuppliesBroadcast, useSupplies);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void SetUseSuppliesBroadcast(bool useSupplies)
	{
		m_bUseSupplies = useSupplies;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get if support station should check for supplies
	\return true if should check for supplies
	*/
	bool IsUsingSupplies()
	{
		return m_bUseSupplies && m_ResourceConsumer;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	\return Resource Consumer if entity has any
	*/
	SCR_ResourceConsumer GetResourceConsumer()
	{
		return m_ResourceConsumer;
	}
	
	//---------------------------------------- Range ----------------------------------------\\
	/*!
	Returns range. If range is disabled it will be -1
	\return Range power of 2 for cheap distance calc
	*/
	float GetRange()
	{
		return m_fRange;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	If true than the action will use range if false it will not use range and only execute if the action and the action manager is on the entity itself
	\return True if using range
	*/
	bool UsesRange()
	{
		return m_fRange > 0;
	}

	//---------------------------------------- Position ----------------------------------------\\
	/*!
	Get the center of the Support station that is used with range + offset
	\return Returns center + offset
	*/
	vector GetPosition()
	{
		return GetOwner().CoordToParent(GetOffset());
	}
	
	//---------------------------------------- Position Offset ----------------------------------------\\
	/*!
	\return returns the offset of the position
	*/
	vector GetOffset()
	{
		return m_vOffset;
	}


	//---------------------------------------- Other ----------------------------------------\\
	/*!
	If true and the action uses range it will never check itself. If false it will check itself though use the standard priority for it
	\return True if it ignores self if it uses range
	*/
	bool IgnoreSelf()
	{
		return m_bIgnoreSelf;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	\return Get Override UserAction string that is used when the action is displayed to the player
	*/
	LocalizedString GetOverrideUserActionName()
	{
		return m_sOverrideUserActionName;
	}

	//---------------------------------------- Notification ----------------------------------------\\
	protected bool GetSendNotificationOnUse()
	{
		SCR_BaseSupportStationComponentClass classData = SCR_BaseSupportStationComponentClass.Cast(GetComponentData(GetOwner()));
		return classData && classData.GetSendNotificationOnUse();
	}

	//---------------------------------------- Sound ----------------------------------------\\
	protected SCR_AudioSourceConfiguration GetOnUseAudioConfig()
	{
		SCR_BaseSupportStationComponentClass classData = SCR_BaseSupportStationComponentClass.Cast(GetComponentData(GetOwner()));
		if (!classData)
			return null;

		return classData.GetOnUseAudioConfig();
	}

	//======================================== PLAYERS IN VEHICLE ========================================\\
	//~ Get an array of all player Ids within vehicle
	protected void GetPlayerIdsInVehicle(IEntity vehicle, out notnull array<int> playerIds)
	{
		playerIds.Clear();

		SCR_BaseCompartmentManagerComponent compartmentManager = SCR_BaseCompartmentManagerComponent.Cast(vehicle.FindComponent(SCR_BaseCompartmentManagerComponent));
		if (!compartmentManager)
		{
			IEntity parent = vehicle.GetParent();
			if (parent)
				compartmentManager = SCR_BaseCompartmentManagerComponent.Cast(parent.FindComponent(SCR_BaseCompartmentManagerComponent));
		
			if (!compartmentManager)
				return;
		}

		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return;

		array<IEntity> occupants = {};
		compartmentManager.GetOccupants(occupants);
		CompartmentAccessComponent compartmentAccess;

		int playerID;

		foreach (IEntity occupant : occupants)
		{
			playerID = playerManager.GetPlayerIdFromControlledEntity(occupant);

			if (playerID > 0)
				playerIds.Insert(playerID);
		}
	}

	//======================================== DAMAGE ========================================\\
	/*!
	Allows devs to functionally destroy the Support station (or other state) without the need of a DamageManager.
	If a damage system exists on the entity then it is adviced to let it handle the logic.
	\param damageState State to set
	\param checkIfHasDamageSystem If true it will check if any damage system is present and if so will not over the state. If false it will simply override it
	\return True if damage state was succesfully overridden
	*/
	bool OverrideDamageState(EDamageState damageState, bool checkIfHasDamageSystem)
	{
		//~ Already the same state
		if (damageState == m_eCurrentDamageState)
			return false;

		//~ Check if there is a damage system and ignore this call if there is
		if (checkIfHasDamageSystem)
		{
			//~ Check has damage manager
			ScriptedDamageManagerComponent damageManager = ScriptedDamageManagerComponent.Cast(GetOwner().FindComponent(ScriptedDamageManagerComponent));
			if (damageManager)
				return false;

			HitZoneContainerComponent hitZoneContainer = HitZoneContainerComponent.Cast(GetOwner().FindComponent(HitZoneContainerComponent));
			if (hitZoneContainer)
			{
				ScriptedHitZone defaultHitZone = ScriptedHitZone.Cast(hitZoneContainer.GetDefaultHitZone());
				if (defaultHitZone)
					return false;
			}
		}

		OnDamageStateChanged(damageState);
		return true;
	}

	//---------------------------------------- On HitZone damage state changed ----------------------------------------\\
	//~ Called when default hitZone damage state changed. Not called if entity has a damage manager
	protected void OnHitZoneDamageStateChanged(ScriptedHitZone defaultHitZone)
	{
		OnDamageStateChanged(defaultHitZone.GetDamageState());
	}

	//---------------------------------------- On damage state changed ----------------------------------------\\
	//~ Called when damage manager (or default hitZone) damage state changes
	protected void OnDamageStateChanged(EDamageState damageState)
	{
		//~ Same state so ignore
		if (damageState == m_eCurrentDamageState)
			return;

		//~ If using range make sure to add self or remove self from manager
		if (UsesRange())
		{
			//~ Is destroyed so remove self from support stations
			if (damageState == EDamageState.DESTROYED)
			{
				SCR_SupportStationManagerComponent supportStationManager = SCR_SupportStationManagerComponent.GetInstance();
				if (!supportStationManager)
					return;

				supportStationManager.RemoveSupportStation(this);
			}
			//~ No longer destroyed so add itself back to the manager
			else if (m_eCurrentDamageState == EDamageState.DESTROYED)
			{
				SCR_SupportStationManagerComponent supportStationManager = SCR_SupportStationManagerComponent.GetInstance();
				if (!supportStationManager)
					return;

				supportStationManager.AddSupportStation(this);
			}
		}

		//~ Update damage state
		m_eCurrentDamageState = damageState;
	}

	//---------------------------------------- Subscribe/Unsubscribe On damage state changed ----------------------------------------\\
	//~ Grabs either damage manager or HitZoneContainer to subscribe to OnDamageState Changed
	protected void AddRemoveOnDamageStateChanged(IEntity owner, bool subscribe)
	{
		ScriptedDamageManagerComponent damageManager = ScriptedDamageManagerComponent.Cast(owner.FindComponent(ScriptedDamageManagerComponent));
		if (damageManager)
		{
			//~ Subscribe to on damage state changed
			if (subscribe)
			{
				//~ Also set current state
				m_eCurrentDamageState = damageManager.GetState();
				damageManager.GetOnDamageStateChanged().Insert(OnDamageStateChanged);
			}
			//~ Unsubscribe to on damage state changed
			else
			{
				damageManager.GetOnDamageStateChanged().Remove(OnDamageStateChanged);
			}

		}

		HitZoneContainerComponent hitZoneContainer = HitZoneContainerComponent.Cast(owner.FindComponent(HitZoneContainerComponent));
		if (hitZoneContainer)
		{
			ScriptedHitZone defaultHitZone = ScriptedHitZone.Cast(hitZoneContainer.GetDefaultHitZone());

			if (defaultHitZone)
			{
				//~ Subscribe to on damage state changed
				if (subscribe)
				{
					//~ Also set current state
					m_eCurrentDamageState = defaultHitZone.GetDamageState();
					defaultHitZone.GetOnDamageStateChanged().Insert(OnHitZoneDamageStateChanged);
				}
				//~ Unsubscribe to on damage state changed
				else
				{
					defaultHitZone.GetOnDamageStateChanged().Remove(OnHitZoneDamageStateChanged);
				}
			}
		}
	}

	//======================================== RPL ========================================\\
	override bool RplSave(ScriptBitWriter writer)
	{
		writer.WriteBool(m_bIsEnabled);
		writer.WriteBool(m_bUseSupplies);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		int enabled, usesSupplies;

		reader.ReadBool(enabled);
		reader.ReadBool(usesSupplies);

		SetEnabledBroadcast(enabled);
		SetUseSuppliesBroadcast(usesSupplies);

		return true;
	}

	//======================================== INIT ========================================\\
	protected bool InitValidSetup()
	{
		if (GetSupportStationType() == ESupportStationType.NONE)
		{
			Print("'" + GetOwner().GetName() + "' No Support Station type assigned in the inherented class. Overwrite the 'GetSupportStationType()' function and do not use the base class!", LogLevel.ERROR);
			return false;
		}

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Temp solution until replication fixed
	protected void TEMP_OnInteractorReplicated()
	{
		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.FindResourceComponent(GetOwner());
		if (resourceComponent)
			m_ResourceConsumer = resourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		//~ Call init later to make sure vehicle slot has a parent
		GetGame().GetCallqueue().CallLater(DelayedInit, param1: owner);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DelayedInit(IEntity owner)
	{
		if (!InitValidSetup() || !owner)
			return;
		
		if (m_bUseSupplies)
		{
			SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.FindResourceComponent(owner);
			if (!resourceComponent && owner.GetParent())
				resourceComponent = SCR_ResourceComponent.FindResourceComponent(owner.GetParent());

			if (resourceComponent)
			{
				m_ResourceConsumer = resourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
				if (!m_ResourceConsumer)
					Print("'SCR_BaseSupportStationComponent' 'EOnInit': '" + GetOwner() + "' Support Station is set to use supplies but the consumer configuration in m_ResourceConsumer is missing", LogLevel.ERROR);
				//~ Temp solution until replication fixed
				else 
					resourceComponent.TEMP_GetOnInteractorReplicated().Insert(TEMP_OnInteractorReplicated);
			}
			else
			{
				Print("'SCR_BaseSupportStationComponent' 'EOnInit': '" + GetOwner() + "' Support Station is set to use supplies  but it has no SCR_ResourceComponent", LogLevel.ERROR);
			}
		}

		//~ Subscribe to on damage state changed
		AddRemoveOnDamageStateChanged(owner, true);

		SCR_BaseSupportStationComponentClass classData = SCR_BaseSupportStationComponentClass.Cast(GetComponentData(owner));

		//~ Not static so get Physics component
		if (classData && classData.CanMoveWithPhysics())
		{
			Vehicle vehicle = Vehicle.Cast(owner);
			if (!vehicle)
				vehicle = Vehicle.Cast(owner.GetParent());
			
			if (vehicle)
				m_Physics = vehicle.GetPhysics();
		}
			
		//~ If faction is checked on use set faction affiliation ref
		m_FactionAffiliationComponent = FactionAffiliationComponent.Cast(owner.FindComponent(FactionAffiliationComponent));

		//~ Could not find Faction affiliation component
		if (!m_FactionAffiliationComponent)
		{
			//~ Get Faction affiliation from parent
			if (classData && classData.AllowGetFactionFromParent())
			{
				IEntity parent = owner.GetParent();
				if (parent)
					m_FactionAffiliationComponent = FactionAffiliationComponent.Cast(parent.FindComponent(FactionAffiliationComponent));
			}

			//~ Support Station (nor parent if checked) has Faction affiliation component and it needs to be there for the IsValid check
			if (!m_FactionAffiliationComponent && m_eFactionUsageCheck)
				Print(string.Format("'SCR_BaseSupportStationComponent' of type '%1' has Faction usage check set to not ignore but is missing a 'FactionAffiliationComponent'", typename.EnumToString(ESupportStationType, GetSupportStationType())), LogLevel.ERROR);
		}

		//~ Uses range so add self to the manager if not destroyed
		if (UsesRange())
		{
			//~ Has damage manager and is destroyed so do not add self to manager
			if (m_eCurrentDamageState == EDamageState.DESTROYED)
				return;

			SCR_SupportStationManagerComponent supportStationManager = SCR_SupportStationManagerComponent.GetInstance();
			if (!supportStationManager)
			{
				//~ Entity exists in the world when loaded instead of later spawned
				if (owner.IsLoaded())
					Print("'SCR_BaseSupportStationComponent' Support station on entity that exists in world when loaded has range but no 'SCR_SupportStationManagerComponent' availible on GameMode. So it won't work!", LogLevel.WARNING);
				//~ Show VME for any new support stations added
				else 
					Debug.Error2("SCR_BaseSupportStationComponent", "Support station exists with range but no 'SCR_SupportStationManagerComponent' availible on GameMode!");
				
				return;
			}

			//~ Add self to manager
			supportStationManager.AddSupportStation(this);
		}
		
		//~ Get service point to see if it is online
		m_ServicePointComponent = SCR_ServicePointComponent.Cast(owner.FindComponent(SCR_ServicePointComponent));
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode())
			return;

		SetEventMask(owner, EntityEvent.INIT);

		//~ Make sure to use distanceSq
		if (m_fRange > 0)
			m_fRange = Math.Pow(m_fRange, 2);
	}

	//======================================== DESTROY ========================================\\
	override void OnDelete(IEntity owner)
	{
		if (SCR_Global.IsEditMode() || !InitValidSetup())
			return;

		//~ Unsubscribe to onDamage state changed
		AddRemoveOnDamageStateChanged(owner, false);

		//~ Does not use range so do not remove self to supportStationManager
		if (!UsesRange())
			return;

		if (m_eCurrentDamageState == EDamageState.DESTROYED)
			return;

		SCR_SupportStationManagerComponent supportStationManager = SCR_SupportStationManagerComponent.GetInstance();
		if (!supportStationManager)
			return;

		supportStationManager.RemoveSupportStation(this);
		
		if (m_bUseSupplies && m_ResourceConsumer)
		{
			SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.FindResourceComponent(owner);
			if (!resourceComponent && owner.GetParent())
				resourceComponent = SCR_ResourceComponent.FindResourceComponent(owner.GetParent());

			if (resourceComponent)
				resourceComponent.TEMP_GetOnInteractorReplicated().Remove(TEMP_OnInteractorReplicated);
		}
	}
};

enum ESupportStationFactionUsage
{
	SAME_CURRENT_FACTION = 1, ///< The user with the same faction as the current faction can interact with the support station
	FRIENDLY_CURRENT_FACTION = 2, ///< The user with a faction that is a friendly faction as the current faction can interact with the support GetSupportStationType
	SAME_DEFAULT_FACTION = 4, ///< The user with the same faction as the default faction of the support station can interact with the support station
	FRIENDLY_DEFAULT_FACTION = 8, ///< he user with a faction that is a friendly faction to the default faction of the support station can interact with the support station
	DISALLOW_USE_ON_CURRENT_FACTION_NULL = 16, ///< If set it disallows the support station to be used if the faction of the Support station is not set. Only valid if Default faction is not checked
};
