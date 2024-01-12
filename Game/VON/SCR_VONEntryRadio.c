//------------------------------------------------------------------------------------------------
//! VONEntry class for radio entries
class SCR_VONEntryRadio : SCR_VONEntry
{	
	const string LABEL_FREQUENCY_UNITS = "#AR-VON_FrequencyUnits_MHz";
	
	bool m_bIsLongRange;				// whether this is personal or backpack radio
	protected int m_iFrequency;			// current frequency
	protected int m_iTransceiverNumber;
	protected string m_sChannelText;
	
	protected BaseTransceiver m_RadioTransceiver;
	protected SCR_GadgetComponent m_GadgetComp; 
	
	//------------------------------------------------------------------------------------------------
	//! Associated transceiver
	BaseTransceiver GetTransceiver()
	{
		return m_RadioTransceiver;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Associated transceiver id - starts with 1
	int GetTransceiverNumber()
	{
		return m_iTransceiverNumber;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Local frequency getter since atm the transceiver getter is delayed (network?) and not sufficient for instant UI changes
	int GetEntryFrequency()
	{
		return m_iFrequency;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Gadget component associated with this entry
	SCR_GadgetComponent GetGadget()
	{
		return m_GadgetComp;
	}
	
	//------------------------------------------------------------------------------------------------
	UIInfo GetUIInfo()
	{
		InventoryItemComponent itemComponent = InventoryItemComponent.Cast(m_GadgetComp.GetOwner().FindComponent(InventoryItemComponent));
		if (!itemComponent)
			return null;

		return itemComponent.GetUIInfo();
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRadioEntry(BaseTransceiver transceiver, int number, SCR_GadgetComponent gadgetComp)
	{
		m_RadioTransceiver = transceiver;
		m_iTransceiverNumber = number;
		m_GadgetComp = gadgetComp;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetChannelText(string channel)
	{
		m_sChannelText = channel;
	}
	
	//------------------------------------------------------------------------------------------------
	override void InitEntry()
	{
		SetCustomLayout("{033302D7C8158EF8}UI/layouts/HUD/VON/VONEntry.layout");
		
		m_bIsEnabled = m_RadioTransceiver.GetRadio().IsPowered();
		
		AdjustEntryModif(0);
		
		if (m_GadgetComp.GetType() == EGadgetType.RADIO_BACKPACK)
			m_bIsLongRange = true;
		
		SetChannelText(SCR_VONMenu.GetKnownChannel(m_RadioTransceiver.GetFrequency()));
	}
		
	//------------------------------------------------------------------------------------------------ 
	override void AdjustEntryModif(int modifier)
	{		
		if (!m_bIsEnabled && modifier != 0)
			return;
		
		if (!m_RadioTransceiver)
			return;
		
		// Get & adjust frequency
		m_iFrequency = m_RadioTransceiver.GetFrequency();
		int minFreq = m_RadioTransceiver.GetMinFrequency();
		int maxFreq = m_RadioTransceiver.GetMaxFrequency();
		
		// TODO sound temp here until moved to radio level
		if ( (modifier > 0  && m_iFrequency == maxFreq) || (modifier < 0 && m_iFrequency == minFreq) )
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_RADIO_CHANGEFREQUENCY_ERROR);
		else if (modifier != 0)
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_RADIO_CHANGEFREQUENCY);
		
		m_iFrequency = m_iFrequency + (modifier * m_RadioTransceiver.GetFrequencyResolution());
		m_iFrequency = Math.ClampInt(m_iFrequency, minFreq, maxFreq);
		
		RadioHandlerComponent rhc = RadioHandlerComponent.Cast(GetGame().GetPlayerController().FindComponent(RadioHandlerComponent));
		if (rhc)
			rhc.SetFrequency(m_RadioTransceiver, m_iFrequency); // Set new frequency
		else 
			return;
		
		float fFrequency = Math.Round(m_iFrequency * 0.1) * 0.01; 	// Format the frequency text: round and convert to 2 digits with one possible decimal place (39500 -> 39.5)
		m_sText = fFrequency.ToString(3, 1) + " " + LABEL_FREQUENCY_UNITS;		
	}
	
	//------------------------------------------------------------------------------------------------ 
	override void AdjustEntry(int modifier)
	{
		if (!GetGame().GetGameMode())
			return;
		
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_GroupsManagerComponent));
		SCR_FactionManager factionMgr =  SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if ( !groupManager || !factionMgr || !m_RadioTransceiver)
			return;
		
		SCR_Faction playerFaction = SCR_Faction.Cast(factionMgr.SGetPlayerFaction(GetGame().GetPlayerController().GetPlayerId()));
		if (!playerFaction)
			return;

		int factionFreq = playerFaction.GetFactionRadioFrequency();
		int currentFreq = m_RadioTransceiver.GetFrequency();
		
		array<SCR_AIGroup> groups = groupManager.GetPlayableGroupsByFaction(playerFaction);
		if (!groups || groups.IsEmpty())
			return;

		int newFreq;
		
		if (currentFreq == factionFreq)		// if platoon frequency
		{
			if (modifier == -1)
				newFreq = groups[0].GetRadioFrequency();	// go to first squad freq
			else 
				return; // top freq in list
		}
		else // non platoon frequency
		{
			int count = groups.Count();
			if (modifier == 1 && currentFreq == groups[0].GetRadioFrequency())	// input up from first group frequency to platoon, if its not zero
			{
				if (factionFreq != 0)
					newFreq = factionFreq;
				else 
					return; // top freq in list
			}
			else if (modifier == -1 && currentFreq == groups[count - 1].GetRadioFrequency())	// input down from last group frequency to platoon, if its not zero
			{
				return; // last freq in the list
			}
			else 
			{
				bool isMatched;
				
				for (int i = 0; i < count; i++)
				{
					if (currentFreq == groups[i].GetRadioFrequency())
					{
						if (modifier == 1)
							newFreq = groups[i-1].GetRadioFrequency();
						else 
							newFreq = groups[i+1].GetRadioFrequency();
						
						isMatched = true;
						break;
					}
				}	
				
				if (!isMatched)
					newFreq = groups[0].GetRadioFrequency();
			}
		}

		m_iFrequency = newFreq;
		
		RadioHandlerComponent rhc = RadioHandlerComponent.Cast(GetGame().GetPlayerController().FindComponent(RadioHandlerComponent));
		if (rhc)
			rhc.SetFrequency(m_RadioTransceiver, m_iFrequency); // Set new frequency
		else 
			return;
		
		float fFrequency = Math.Round(m_iFrequency / 10); // Format the frequency text
		fFrequency = fFrequency / 100;			
		m_sText = fFrequency.ToString(3, 1) + " " + LABEL_FREQUENCY_UNITS;		
		
		SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_RADIO_FREQUENCY_CYCLE);
	}
	
	//------------------------------------------------------------------------------------------------ 
	override void ToggleEntry()
	{
		// Constructor allows the entry to not have m_RadioTransceiver assigned
		if (!m_RadioTransceiver)
			return;
		
		BaseRadioComponent radio = m_RadioTransceiver.GetRadio();

		m_bIsEnabled = !radio.IsPowered();
		radio.SetPower(m_bIsEnabled);
		
		AdjustEntryModif(0);	
		
		if (m_bIsEnabled)
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_RADIO_TURN_OFF);
		else
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_RADIO_TURN_ON);
	}
		
	//------------------------------------------------------------------------------------------------
	override string GetIconResource()
	{
		if (!m_RadioTransceiver)
			return string.Empty;
		

		IEntity entity = m_RadioTransceiver.GetRadio().GetOwner();
		// Item will be available, since Transceiver should not exist without RadioComponent
		
		InventoryItemComponent pInvItemComponent = InventoryItemComponent.Cast(entity.FindComponent(InventoryItemComponent));
		if (!pInvItemComponent)
			return string.Empty;			

		ItemAttributeCollection pInvItemAttributes = pInvItemComponent.GetAttributes();
		if (!pInvItemAttributes)
			return string.Empty;
		
		UIInfo uiInfo = pInvItemAttributes.GetUIInfo();
		if (!uiInfo)
			return string.Empty;
		
		return uiInfo.GetIconPath();
	}	
	
	//------------------------------------------------------------------------------------------------
	override ECommMethod GetVONMethod()
	{
		return ECommMethod.SQUAD_RADIO;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update entry visuals
	override void Update()
	{
		super.Update();
				
		SCR_VONEntryComponent entryComp = SCR_VONEntryComponent.Cast(m_EntryComponent);			
		if (!entryComp)	// first update procs when this is not fetchable yet
			return;
		
		entryComp.SetTransceiverText("CH" + m_iTransceiverNumber.ToString());
		entryComp.SetFrequencyText(m_sText);
		entryComp.SetChannelText(m_sChannelText);
		entryComp.SetActiveIcon(m_bIsActive);
		
		m_bIsEnabled = m_RadioTransceiver.GetRadio().IsPowered();	
		entryComp.SetPowerIcon(m_bIsEnabled);
		
		if (m_bIsActive)
		{
			entryComp.SetTransceiverOpacity(1);
			
			if (m_bIsSelected)
				entryComp.SetFrequencyColor(GUIColors.ORANGE_BRIGHT);
			else 
				entryComp.SetFrequencyColor(GUIColors.ORANGE);
		}
		else 
		{
			entryComp.SetTransceiverOpacity(0.5);
			
			if (m_bIsSelected)
				entryComp.SetFrequencyColor(GUIColors.ORANGE_BRIGHT);
			else 
				entryComp.SetFrequencyColor(Color.White);
		}
	}
};
