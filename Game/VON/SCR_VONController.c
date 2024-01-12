//------------------------------------------------------------------------------------------------
//! Type of a players current VON transmission
enum EVONTransmitType
{
	NONE,
	DIRECT,
	CHANNEL,
	LONG_RANGE
}

//------------------------------------------------------------------------------------------------
// invoker typedefs
void OnVONActiveToggled(bool isToggledDirect, bool isToggledChannel);	// when direct toggle activates
typedef func OnVONActiveToggled;

void OnEntriesChanged(SCR_VONEntry entry, bool isAdded);				// when entry array is modified, true = added
typedef func OnEntriesChanged;

//------------------------------------------------------------------------------------------------
class SCR_VONControllerClass: ScriptGameComponentClass
{}

//------------------------------------------------------------------------------------------------
//! Scripted VON input and control, attached to SCR_PlayerController
class SCR_VONController : ScriptComponent
{
 	// TODO more robust von component selection management
	
	[Attribute("", UIWidgets.Object)]
	protected ref SCR_VONMenu m_VONMenu;
		
	protected const string VON_DIRECT_HOLD = "VONDirect";
	protected const string VON_CHANNEL_HOLD = "VONChannel";
	protected const string VON_LONG_RANGE_HOLD = "VONLongRange";
	protected const float TOGGLE_OFF_DELAY = 0.5;		// seconds, delay before toggle state can get canceled, to avoid collision of double click and press
	
	static bool s_bIsInit;					// component init done, static so its only done once
	protected bool m_bIsDisabled; 			// VON control is disabled
	protected bool m_bIsUnconscious; 		// Character is unconscious --> VON control is disabled
	protected bool m_bIsActive;				// VON is active
	protected bool m_bIsToggledDirect;		// VON direct speech toggle is active
	protected bool m_bIsToggledChannel;		// VON channel speech toggle is active
	protected bool m_bIsActiveModeDirect;	// used for controller, switch between direct VON = true / channel VON = false
	protected bool m_bIsActiveModeLong;		// used for controller, switch between long range VON = true / channel VON = false
	protected float m_fToggleOffDelay;		// used to track delay before toggle can be cancelled
	protected string m_sActiveHoldAction;	// tracker for ending the hold action VON 
	protected string m_sLocalEncryptionKey;		// local players faction encryption key
	protected EVONTransmitType m_eVONType;	// currently active VON type
	protected InputManager m_InputManager;

	protected SCR_VoNComponent m_VONComp;					// VON component used for transmission
	protected SCR_VonDisplay m_VONDisplay;					// VON transmission display
	protected SCR_VONEntry m_ActiveEntry;					// active entry (non direct speech)
	protected SCR_VONEntry m_LongRangeEntry;				// entry for long range radio, if available
	protected SCR_VONEntry m_SavedEntry;					// entry used for switching to different active entry after current one is done, f.e. after VONLongRange
	protected ref SCR_VONEntry m_DirectSpeechEntry;			// separate direct speech entry
	protected ref array<ref SCR_VONEntry> m_aEntries = {};	// all entries except direct
	
	protected ref ScriptInvokerBase<OnEntriesChanged> 	m_OnEntriesChanged = new ScriptInvokerBase<OnEntriesChanged>();						
	protected ref ScriptInvokerBase<OnVONActiveToggled> m_OnVONActiveToggled = new ScriptInvokerBase<OnVONActiveToggled>();
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerBase<OnEntriesChanged> GetOnEntriesChangedInvoker()
	{
		return m_OnEntriesChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerBase<OnVONActiveToggled> GetOnVONActiveToggledInvoker()
	{
		return m_OnVONActiveToggled;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetDisplay(SCR_VonDisplay display)
	{ 
		m_VONDisplay = display; 
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_VonDisplay GetDisplay()
	{ 
		return m_VONDisplay; 
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_VONMenu GetVONMenu()
	{ 
		return m_VONMenu; 
	}
	
	//------------------------------------------------------------------------------------------------
	void SetVONDisabled(bool state)
	{
		m_bIsDisabled = state;
		UpdateSystemState();
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsVONDisabled()
	{
		return m_bIsDisabled;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_VoNComponent GetVONComponent()
	{ 
		return m_VONComp; 
	}
		
	//------------------------------------------------------------------------------------------------
	//! Set component for transmit of VON
	//! \param VONComp is the subject
	void SetVONComponent(SCR_VoNComponent VONComp)
	{			
		m_VONComp = VONComp;
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Get current entries	TODO return this as out array to save alloc
	//! \return copied array of entries
	array<ref SCR_VONEntry> GetVONEntries()
	{
		array<ref SCR_VONEntry> entries = {};

		for (int i = 0, count = m_aEntries.Count(); i < count; i++)
		{
			entries.Insert(m_aEntries[i]);
		}

		return entries;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get number of entries
	int GetVONEntryCount()
	{
		return m_aEntries.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get active entry for outgoing transmission
	//! \return active entry
	SCR_VONEntry GetActiveEntry()
	{
		return m_ActiveEntry;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set entry as active for outgoing transmission
	//! \param entry is subject entry
	//! \param resetLRR determines whether the long range radio state should be reset by this selection
	void SetEntryActive(SCR_VONEntry entry, bool resetLRR = false)
	{
		if (entry == m_ActiveEntry)
			return;
				
		m_ActiveEntry = entry;
		
		if (m_ActiveEntry && !m_ActiveEntry.m_bIsEnabled)	// turn off toggle when switching to disabled entry
			OnVONToggle(0,0);
		
		if (resetLRR)	// cancel long range state if switching to personal radio
		{
			SCR_VONEntryRadio radioEntry = SCR_VONEntryRadio.Cast(entry);
			if (radioEntry && !radioEntry.m_bIsLongRange)
				m_bIsActiveModeLong = false;			
		}
	}
		
	//------------------------------------------------------------------------------------------------
	//! Add new VON entry
	//! Set as active entry if m_ActiveEntry is null
	//! \param entry is subject entry
	void AddEntry(SCR_VONEntry entry)
	{
		m_aEntries.Insert(entry);
		entry.InitEntry();
		
		if (SCR_VONEntryRadio.Cast(entry) && SCR_VONEntryRadio.Cast(entry).m_bIsLongRange)	// if long range entry, set it here
			m_LongRangeEntry = entry;
		
		m_OnEntriesChanged.Invoke(entry, true);
		
		if (!m_ActiveEntry)
			SetEntryActive(entry);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Remove existing VON entry.
	//! Sets m_ActiveEntry to null, when removing active entry
	//! \param entry is subject entry
	void RemoveEntry(SCR_VONEntry entry)
	{
		m_OnEntriesChanged.Invoke(entry, false);
		
		//Deactivate if the entry being removed is active
		if (entry == m_ActiveEntry)
		{
			if (m_bIsToggledChannel)
				OnVONToggle(2, 0);
			else 
				ActivateVON(m_ActiveEntry, false);
			
			m_ActiveEntry = null;
		}
		
		if (entry == m_LongRangeEntry)	// unset long range entry
			m_LongRangeEntry = null;
				
		m_aEntries.RemoveItem(entry);
	}
	
	//------------------------------------------------------------------------------------------------
	//! VON direct speech listener callback
	protected void OnVONDirect(float value, EActionTrigger reason)
	{		
		if (reason == EActionTrigger.DOWN)
			VONDirect(true);
		else
			VONDirect(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! VON channel speech listener callback
 	protected void OnVONChannel(float value, EActionTrigger reason)
	{
		if (reason == EActionTrigger.DOWN)
		{
			m_bIsActiveModeLong = false;
			VONChannel(true);
		}
		else
			VONChannel(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! VON channel speech listener callback for long range radios
 	protected void OnVONLongRange(float value, EActionTrigger reason)
	{
		if (reason == EActionTrigger.DOWN)
		{
			m_bIsActiveModeLong = false;
			VONLongRange(true);
		}
		else
			VONLongRange(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! VON channel speech listener callback
 	protected void OnVONGamepad(float value, EActionTrigger reason)
	{		
		if (reason == EActionTrigger.UP)
			m_eVONType = EVONTransmitType.NONE;
		
		if (m_bIsActiveModeDirect)
		{
			if (reason == EActionTrigger.PRESSED && m_eVONType == EVONTransmitType.DIRECT)
				return;
			
			if (reason == EActionTrigger.PRESSED)
				VONDirect(true);
			else 
				VONDirect(false);
		}
		else if (m_bIsActiveModeLong)
		{
			if (reason == EActionTrigger.PRESSED && m_eVONType == EVONTransmitType.LONG_RANGE)
				return;
		
			if (reason == EActionTrigger.PRESSED)
				VONLongRange(true);
			else 
				VONLongRange(false);
		
		}
		else
		{
			if (reason == EActionTrigger.PRESSED && m_eVONType == EVONTransmitType.CHANNEL)
				return;
			
			if (reason == EActionTrigger.PRESSED)
				VONChannel(true);
			else 
				VONChannel(false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
 	protected void OnVONLongRangeGamepad(float value, EActionTrigger reason)
	{				
		if (m_bIsActiveModeLong || !m_LongRangeEntry || !m_LongRangeEntry.m_bIsEnabled)
		{
			m_bIsActiveModeLong = false;
			if (m_bIsActive)
				VONLongRange(false);
		}
		else 
		{
			m_bIsActiveModeLong = true;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! VON toggle   val 0 = off / 1 = direct / 2 = channel
 	protected void OnVONToggle(float value, EActionTrigger reason)
	{
		if (!m_VONComp)
		{
			if (!AssignVONComponent())
			{
				m_sActiveHoldAction = string.Empty;
				return;
			}
		}
		
		if (value == 0)
		{
			m_fToggleOffDelay = 0;
			m_bIsToggledDirect = false;
			m_bIsToggledChannel = false;
			ActivateVON(m_DirectSpeechEntry, false);
		}
		else if (value == 1)
		{
			m_fToggleOffDelay = TOGGLE_OFF_DELAY;
			m_bIsToggledDirect = !m_bIsToggledDirect;
			m_bIsToggledChannel = false;
			ActivateVON(m_DirectSpeechEntry, m_bIsToggledDirect);
		}
		else 
		{
			if (!m_ActiveEntry || !m_ActiveEntry.m_bIsEnabled)
				return;
			
			m_bIsActiveModeLong = false;
			m_fToggleOffDelay = TOGGLE_OFF_DELAY;
			m_bIsToggledDirect = false;
			m_bIsToggledChannel = !m_bIsToggledChannel;
			ActivateVON(m_ActiveEntry, m_bIsToggledChannel);
		}
						
		m_OnVONActiveToggled.Invoke(m_bIsToggledDirect, m_bIsToggledChannel);
	}
	
	//------------------------------------------------------------------------------------------------
	//! VON toggle controller
 	protected void OnVONToggleGamepad(float value, EActionTrigger reason)
	{
		if (m_bIsActiveModeDirect)
			OnVONToggle(1, EActionTrigger.DOWN);	// direct toggle
		else 
			OnVONToggle(2, EActionTrigger.DOWN);	// channel toggle
	}
	
	//------------------------------------------------------------------------------------------------
	//! VON switch between direct/channel mode
 	protected void OnVONSwitch(float value, EActionTrigger reason)
	{
		m_bIsActiveModeDirect = !m_bIsActiveModeDirect;
		
		if (m_bIsToggledDirect || m_bIsToggledChannel)
			OnVONToggleGamepad(0, EActionTrigger.UP);
		else
		{
			OnVONGamepad(0, EActionTrigger.PRESSED); 
			GetGame().GetCallqueue().CallLater(OnVONGamepad, 100, false, 0, EActionTrigger.UP);	// visualisation of switching from direct to radio VON when using controller
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void VONDirect(bool activate)
	{
		if (!m_VONComp)
		{
			if (!AssignVONComponent())
			{
				m_sActiveHoldAction = string.Empty;
				return;
			}
		}
		
		if (!m_DirectSpeechEntry || !m_DirectSpeechEntry.m_bIsEnabled)
			return;
		
		if (m_bIsToggledDirect && m_fToggleOffDelay <= 0)	// direct speech toggle is active, cancel it
			OnVONToggle(0,0);
		
		m_sActiveHoldAction = VON_DIRECT_HOLD;
		
		if (activate)
		{
			m_eVONType = EVONTransmitType.DIRECT;
			ActivateVON(m_DirectSpeechEntry, true);
		}
		else
			ActivateVON(m_DirectSpeechEntry, false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void VONChannel(bool activate)
	{
		if (!m_VONComp)
		{
			if (!AssignVONComponent())
			{
				m_sActiveHoldAction = string.Empty;
				return;
			}
		}
	
		if (!m_ActiveEntry || !m_ActiveEntry.m_bIsEnabled)
			return;
		
		if (m_bIsToggledChannel && m_fToggleOffDelay <= 0) // channel speech toggle is active, cancel it
			OnVONToggle(0,0);
		
		m_sActiveHoldAction = VON_CHANNEL_HOLD;
		
		if (activate)
		{
			m_eVONType = EVONTransmitType.CHANNEL;
			ActivateVON(m_ActiveEntry, true);
		}
		else
			ActivateVON(m_ActiveEntry, false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void VONLongRange(bool activate)
	{
		if (!m_VONComp)
		{
			if (!AssignVONComponent())
			{
				m_sActiveHoldAction = string.Empty;
				return;
			}
		}
		
		if (!m_LongRangeEntry || !m_LongRangeEntry.m_bIsEnabled)
			return;
		
		if ((m_bIsToggledChannel || m_bIsToggledDirect) && m_fToggleOffDelay <= 0) 
			OnVONToggle(0,0);
		
		m_sActiveHoldAction = VON_LONG_RANGE_HOLD;
		
		if (activate)
		{
			if (!m_SavedEntry)	// saved entry to switch back to after using long range keybind
			{
				if (m_bIsActiveModeDirect)
					m_SavedEntry = m_DirectSpeechEntry;
				else
					m_SavedEntry = m_ActiveEntry;
			}
			
			m_bIsActiveModeDirect = false;
			
			m_eVONType = EVONTransmitType.LONG_RANGE;
			ActivateVON(m_LongRangeEntry, true);
		}
		else
		{
			ActivateVON(m_LongRangeEntry, false);	
			
			if (m_SavedEntry)	// restore last saved entry if there is one
			{
				SetActiveTransmit(m_SavedEntry);
				if (m_SavedEntry.GetVONMethod() == EVONTransmitType.DIRECT)
					m_bIsActiveModeDirect = true;
				
				m_SavedEntry = null;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! VON activation
	//! \param entry is the subject VON entry
	//! \param state determines target state ON/OFF
 	protected void ActivateVON(SCR_VONEntry entry, bool state)
	{				
		if (!entry || !m_VONComp)
			return;
				
		if (!state)
		{			
			m_bIsActive = false;
			m_VONComp.SetCapture(false);
						
			if (m_bIsToggledDirect) // direct toggle is active so ending VON should not end capture
			{
				SetActiveTransmit(entry);
				ActivateVON(m_DirectSpeechEntry, m_bIsToggledDirect);
				m_sActiveHoldAction = string.Empty;
			}
			else if (m_bIsToggledChannel) // channel toggle is active so ending VON should not end capture
			{
				SetActiveTransmit(entry);
				ActivateVON(m_ActiveEntry, m_bIsToggledChannel);
				m_sActiveHoldAction = string.Empty;
			}
		}
		else
		{
			if (!GetGame().GetVONCanTransmitCrossFaction())
			{
				if (!m_sLocalEncryptionKey)
					InitEncryptionKey();
			
				SCR_VONEntryRadio radioEntry = SCR_VONEntryRadio.Cast(entry);
				if (radioEntry && m_sLocalEncryptionKey != string.Empty && radioEntry.GetTransceiver().GetRadio().GetEncryptionKey() != m_sLocalEncryptionKey)
					return;
			}
						
			SetActiveTransmit(entry);
			m_VONComp.SetCapture(true);
			m_bIsActive = true;
		}
	}
		
	//------------------------------------------------------------------------------------------------
	//! Set transmission method depending on entry type when starting VON transmit
	protected void SetActiveTransmit(notnull SCR_VONEntry entry)
	{		
		if (entry.GetVONMethod() == ECommMethod.SQUAD_RADIO)
		{
			m_VONComp.SetCommMethod(ECommMethod.SQUAD_RADIO);
			m_VONComp.SetTransmitRadio(SCR_VONEntryRadio.Cast(entry).GetTransceiver());
			SetEntryActive(entry);
		}
		else 
		{
			m_VONComp.SetCommMethod(ECommMethod.DIRECT);
			m_VONComp.SetTransmitRadio(null);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! VON hold timeout, stops active von transmission in case of specific condition which would prevent for it to end normally
	protected void TimeoutVON()
	{
		if (m_sActiveHoldAction == VON_DIRECT_HOLD)
			ActivateVON(m_DirectSpeechEntry, false);
		else if (m_sActiveHoldAction == VON_CHANNEL_HOLD)
			ActivateVON(m_ActiveEntry, false);
		else if (m_sActiveHoldAction == VON_LONG_RANGE_HOLD)
			ActivateVON(m_LongRangeEntry, false);
		
		m_sActiveHoldAction = string.Empty;
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_PlayerController Event
	//! Used to reinit VON when new entity is controlled
	protected void OnControlledEntityChanged(IEntity from, IEntity to)
	{				
		if (m_bIsToggledDirect || m_bIsToggledChannel)
			OnVONToggle(0,0);
		
		if (to)
		{
			m_sLocalEncryptionKey = string.Empty;
			SetVONComponent(SCR_VoNComponent.Cast(to.FindComponent(SCR_VoNComponent)));
		}
		else 
			SetVONComponent(null);	
		
		if (from)
		{
			EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(from.FindComponent(EventHandlerManagerComponent));
			if (eventHandlerManager)
				eventHandlerManager.RemoveScriptHandler("OnConsciousnessChanged", this, OnConsciousnessChanged);
		}
		
		bool unconsciousVONEnabled;
		SCR_BaseGameMode baseGameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!baseGameMode)
			return;
		
		SCR_GameModeHealthSettings healthSettingsComp = SCR_GameModeHealthSettings.Cast(baseGameMode.GetGameModeHealthSettings());
		if (healthSettingsComp)
			unconsciousVONEnabled = healthSettingsComp.IsUnconsciousVONPermitted();
		
		if (unconsciousVONEnabled)
			return;
		
		if (to)		
		{
			ChimeraCharacter character = ChimeraCharacter.Cast(to);
			if (!character)
				return;
			
			CharacterControllerComponent characterController = character.GetCharacterController();
			if (characterController)
				m_bIsUnconscious = characterController.IsUnconscious();
			
			UpdateSystemState();
			
			EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(to.FindComponent(EventHandlerManagerComponent));
			if (eventHandlerManager)
				eventHandlerManager.RegisterScriptHandler("OnConsciousnessChanged", this, OnConsciousnessChanged);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_PlayerController Event
	//! Used to deactivate VON for dead players
	protected void OnDestroyed(Instigator killer, IEntity killerEntity)
	{		
		if (m_bIsToggledDirect || m_bIsToggledChannel)
			OnVONToggle(0,0);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_BaseGameMode event
	//! Used to unregister VON for deleted players
	protected void OnPlayerDeleted(int playerId, IEntity player)
	{
		if (playerId != GetGame().GetPlayerController().GetPlayerId())
			return;
		
		if (m_bIsToggledDirect || m_bIsToggledChannel)
			OnVONToggle(0,0);
		
		m_ActiveEntry = null;
		m_LongRangeEntry = null;
		
		m_aEntries.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Game Event
	protected void OnInputDeviceIsGamepad(bool isGamepad)
	{
		if (!isGamepad)
		{
			m_bIsActiveModeDirect = false;
			m_bIsActiveModeLong = false;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! PauseMenuUI Event
	protected void OnPauseMenuOpened()
	{
		SetVONDisabled(true);
	}
	
	//------------------------------------------------------------------------------------------------
	//! PauseMenuUI Event
	protected void OnPauseMenuClosed()
	{
		SetVONDisabled(false);
	}
			
	//------------------------------------------------------------------------------------------------
	//! Assign VON comp by fetching it from controlled entity
	bool AssignVONComponent()
	{
		IEntity controlledEnt = SCR_PlayerController.Cast(GetOwner()).GetControlledEntity();
		if (controlledEnt)
			SetVONComponent(SCR_VoNComponent.Cast(controlledEnt.FindComponent(SCR_VoNComponent)));
		
		if(!m_VONComp)
			return false;
		
		return true;			
	}
	
	//------------------------------------------------------------------------------------------------
	void OnConsciousnessChanged(bool conscious)
	{
		m_bIsUnconscious = !conscious;
		
		UpdateSystemState();
		
		if (conscious)
			return;
		
		if (m_bIsToggledDirect || m_bIsToggledChannel)
			OnVONToggle(0,0);
			
		TimeoutVON();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void InitEncryptionKey()
	{
		SCR_FactionManager fManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (fManager)
		{
			SCR_Faction faction = SCR_Faction.Cast(fManager.GetLocalPlayerFaction());
			if (faction)
				m_sLocalEncryptionKey = faction.GetFactionRadioEncryptionKey();
		}
	}
		
	//------------------------------------------------------------------------------------------------
	//! Initialize component, done once per controller
	protected void Init(IEntity owner)
	{	
		if (s_bIsInit || System.IsConsoleApp())	// hosted server will have multiple controllers, init just the first one // dont init on dedicated server
		{
			Deactivate(owner);
			return;
		}
		
		m_InputManager = GetGame().GetInputManager();
		if (!m_InputManager)
			return;
		
		m_InputManager.AddActionListener(VON_DIRECT_HOLD, EActionTrigger.DOWN, OnVONDirect);
		m_InputManager.AddActionListener(VON_DIRECT_HOLD, EActionTrigger.UP, OnVONDirect);
		m_InputManager.AddActionListener(VON_CHANNEL_HOLD, EActionTrigger.DOWN, OnVONChannel);
		m_InputManager.AddActionListener(VON_CHANNEL_HOLD, EActionTrigger.UP, OnVONChannel);
		m_InputManager.AddActionListener(VON_LONG_RANGE_HOLD, EActionTrigger.DOWN, OnVONLongRange);
		m_InputManager.AddActionListener(VON_LONG_RANGE_HOLD, EActionTrigger.UP, OnVONLongRange);
		m_InputManager.AddActionListener("VONGamepad", EActionTrigger.PRESSED, OnVONGamepad);
		m_InputManager.AddActionListener("VONGamepad", EActionTrigger.UP, OnVONGamepad);
		m_InputManager.AddActionListener("VONGamepadLongRange", EActionTrigger.DOWN, OnVONLongRangeGamepad);
		m_InputManager.AddActionListener("VONDirectToggle", EActionTrigger.DOWN, OnVONToggle);
		m_InputManager.AddActionListener("VONChannelToggle", EActionTrigger.DOWN, OnVONToggle);
		m_InputManager.AddActionListener("VONToggleGamepad", EActionTrigger.DOWN, OnVONToggleGamepad);
		m_InputManager.AddActionListener("VONSwitch", EActionTrigger.DOWN, OnVONSwitch);
		
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetOwner());
		playerController.m_OnControlledEntityChanged.Insert(OnControlledEntityChanged);
		playerController.m_OnDestroyed.Insert(OnDestroyed);
		PauseMenuUI.m_OnPauseMenuOpened.Insert(OnPauseMenuOpened);
		PauseMenuUI.m_OnPauseMenuClosed.Insert(OnPauseMenuClosed);
		GetGame().OnInputDeviceIsGamepadInvoker().Insert(OnInputDeviceIsGamepad);
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
			gameMode.GetOnPlayerDeleted().Insert(OnPlayerDeleted);
		
		m_DirectSpeechEntry = new SCR_VONEntry(); // Init direct speech entry
		m_DirectSpeechEntry.m_bIsEnabled = true;
		
		ConnectToHandleUpdateVONControllersSystem();
		
		s_bIsInit = true;
		
		if (m_VONMenu)
			m_VONMenu.Init(this);
		
		UpdateSystemState();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Cleanup component
	protected void Cleanup()
	{
		m_VONMenu = null;
		
		UpdateSystemState();
		
		if (GetGame().GetCallqueue())
			GetGame().GetCallqueue().Remove(OnVONGamepad);
		
		m_InputManager = GetGame().GetInputManager();
		if (!m_InputManager)
			return;
		
		m_InputManager.RemoveActionListener(VON_DIRECT_HOLD, EActionTrigger.DOWN, OnVONDirect);
		m_InputManager.RemoveActionListener(VON_DIRECT_HOLD, EActionTrigger.UP, OnVONDirect);
		m_InputManager.RemoveActionListener(VON_CHANNEL_HOLD, EActionTrigger.DOWN, OnVONChannel);
		m_InputManager.RemoveActionListener(VON_CHANNEL_HOLD, EActionTrigger.UP, OnVONChannel);
		m_InputManager.RemoveActionListener(VON_LONG_RANGE_HOLD, EActionTrigger.DOWN, OnVONLongRange);
		m_InputManager.RemoveActionListener(VON_LONG_RANGE_HOLD, EActionTrigger.UP, OnVONLongRange);
		m_InputManager.RemoveActionListener("VONGamepad", EActionTrigger.PRESSED, OnVONGamepad);
		m_InputManager.RemoveActionListener("VONGamepad", EActionTrigger.UP, OnVONGamepad);
		m_InputManager.RemoveActionListener("VONGamepadLongRange", EActionTrigger.DOWN, OnVONLongRangeGamepad);
		m_InputManager.RemoveActionListener("VONDirectToggle", EActionTrigger.DOWN, OnVONToggle);
		m_InputManager.RemoveActionListener("VONDChannelToggle", EActionTrigger.DOWN, OnVONToggle);
		m_InputManager.RemoveActionListener("VONToggleGamepad", EActionTrigger.DOWN, OnVONToggleGamepad);
		m_InputManager.RemoveActionListener("VONSwitch", EActionTrigger.DOWN, OnVONSwitch);
	
		IEntity ent = PlayerController.Cast(GetOwner()).GetControlledEntity();
		if (ent)
		{
			EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(ent.FindComponent(EventHandlerManagerComponent));
			if (eventHandlerManager)
				eventHandlerManager.RemoveScriptHandler("OnConsciousnessChanged", this, OnConsciousnessChanged);
		}
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
			gameMode.GetOnPlayerDeleted().Remove(OnPlayerDeleted);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Debug
	protected void UpdateDebug()
	{
		DbgUI.Begin("VON debug");
		string dbg = "VONcomp: %1 | entries: %2";
		DbgUI.Text( string.Format( dbg, m_VONComp, m_aEntries.Count() ) );
		string dbg2 = "Gpad Direct mode: %1 | Gpad LRR mode: %2 | Active entry: %3";
		DbgUI.Text( string.Format( dbg2, m_bIsActiveModeDirect, m_bIsActiveModeLong, m_ActiveEntry ) );
		string dbg3 = "Saved Entry: %1";
		DbgUI.Text( string.Format( dbg3, m_SavedEntry ) );
		DbgUI.End();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		Init(owner);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void EOnDeactivate(IEntity owner)
	{
		Cleanup();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateSystemState()
	{
		if ((!m_bIsDisabled && !m_bIsUnconscious) || m_VONMenu)
			ConnectToHandleUpdateVONControllersSystem();
		else
			DisconnectFromHandleUpdateVONControllersSystem();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ConnectToHandleUpdateVONControllersSystem()
	{
		World world = GetOwner().GetWorld();
		HandleUpdateVONControllersSystem updateSystem = HandleUpdateVONControllersSystem.Cast(world.FindSystem(HandleUpdateVONControllersSystem));
		if (!updateSystem)
			return;
		
		updateSystem.Register(this);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DisconnectFromHandleUpdateVONControllersSystem()
	{
		World world = GetOwner().GetWorld();
		HandleUpdateVONControllersSystem updateSystem = HandleUpdateVONControllersSystem.Cast(world.FindSystem(HandleUpdateVONControllersSystem));
		if (!updateSystem)
			return;
		
		updateSystem.Unregister(this);
	}
	
	//------------------------------------------------------------------------------------------------
	void Update(float timeSlice)
	{
		if (!m_bIsDisabled && !m_bIsUnconscious)
		{
			m_InputManager.ActivateContext("VONContext");
			
			if (m_fToggleOffDelay > 0)
				m_fToggleOffDelay -= timeSlice;
			
			/* 	When non overlaying context such as chat is activated during hold action, we will no longer receive the EActionTrigger.UP callback from the previous context, which is currently intended
				This timeouts it in such case so the VON will not be stuck in an active state	*/
			if (m_sActiveHoldAction != string.Empty)
			{
				if (!m_InputManager.IsActionActive(m_sActiveHoldAction))
					TimeoutVON();
			}
		}
		
		if (m_VONMenu)
			m_VONMenu.Update(timeSlice);
		
		#ifdef VON_DEBUG
			UpdateDebug();
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		s_bIsInit = false;
		Cleanup();
		DisconnectFromHandleUpdateVONControllersSystem();
		
		super.OnDelete(owner);
	}
};
