//------------------------------------------------------------------------------------------------
//! Data class for transmissions
class TransmissionData
{
	// Widgets
	ref SCR_VoNOverlay_ElementWidgets m_Widgets;
	Widget m_wBaseWidget;

	bool m_bForceUpdate;	// when transmission update is required
	bool m_bVisible;		// element visible, false if widget not visible
	bool m_bIsActive;		// active, false when widget starts fade out
	bool m_bIsAnimating;	// in process of fade in/out
	bool m_bIsAdditional;
	int m_iPlayerID;		// incoming tranmissions playerID
	float m_fAlpha;			// opacity
	float m_fActiveTimeout;	// delay before active element starts fading
	float m_fFrequency;		// current frequency
	float m_fQuality;
	IEntity m_Entity;		// transmitting character
	Faction m_Faction;
	BaseTransceiver m_RadioTransceiver;

	//------------------------------------------------------------------------------------------------
	// Hide transmission, skip animation
	void HideTransmission()
	{
		m_fAlpha = 0;
		m_bIsActive = false;
		m_bIsAnimating = false;
		m_fActiveTimeout = 0;
		m_bVisible = false;
		if (m_wBaseWidget)
			m_wBaseWidget.SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	void TransmissionData(Widget base, int iPlayerID)
	{
		m_wBaseWidget = base;
		m_wBaseWidget.SetOpacity(0);

		m_Widgets = new SCR_VoNOverlay_ElementWidgets();
		m_Widgets.Init(m_wBaseWidget);

		m_iPlayerID = iPlayerID;
	}
}

//------------------------------------------------------------------------------------------------
//! VON display of active outgoing and incoming transmissions
class SCR_VonDisplay : SCR_InfoDisplayExtended
{
	[Attribute("{2EFEA2AF1F38E7F0}UI/Textures/Icons/icons_wrapperUI-64.m_sImageSet")];
	protected string m_sImageSet;

	[Attribute("{ABC6B36856013403}UI/Textures/Icons/icons_wrapperUI-64-glow.m_sImageSet")];
	protected string m_sImageSetGlow;

	[Attribute("{25221F619214A722}UI/layouts/HUD/VON/VoNOverlay_Element.layout")];
	protected string m_sReceivingTransmissionLayout;

	const string ICON_DIRECT_SPEECH = "VON_directspeech";
	const string ICON_RADIO = "VON_frequency";

	const string LABEL_FREQUENCY_UNITS = "#AR-VON_FrequencyUnits_MHz";
	const string LABEL_UNKNOWN_SOURCE = "#AR-VON_UnknownSource";
	const string LABEL_CHANNEL_PLATOON = "#AR-VON_ChannelPlatoon-UC";
	const string WIDGET_INCOMING = "VonIncoming";
	const string WIDGET_TRANSMIT = "VonTransmitting";
	const string WIDGET_OVERFLOW = "VonAdditional";
	const string WIDGET_OVERFLOW_TEXT = "number";
	const string WIDGET_SELECTED_ROOT = "VonSelected";
	const string WIDGET_SELECTED_VON = "Selected_VONChannel";
	const string WIDGET_SELECTED_FREQUENCY = "Selected_Frequency";

	const ref Color COLOR_WHITE = Color.FromSRGBA(255, 255, 255, 255);
	const ref Color COLOR_ORANGE = Color.FromSRGBA(226, 167, 79, 255);

	protected int m_iTransmissionSlots = 4;			// max amount of receiving transmissions
	const float FADEOUT_TIMER_THRESHOLD = 1;
	const float FADEIN_SPEED = 10;
	const float FADEOUT_SPEED = 5;
	const int SELECTED_HINT_FADE_SPEED = 3000; // ms
	const int HEIGHT_DIVIDER = 35;

	protected bool m_bIsVONUIDisabled;
	protected bool m_bIsVONDirectDisabled;
	protected bool m_bIsDirectToggled;			// direct speech toggle state
	protected bool m_bIsChannelToggled;			// channel toggle EFireState

	protected ref TransmissionData m_OutTransmission;												// outgoing transmission data class
	protected ref array<Widget> m_aWidgetsIncomingVON = {};											// available widgets for receiving transmission display
	protected ref array<ref TransmissionData> m_aTransmissions = {}; 								// array of existing incoming transmission data classes
	protected ref map<int, TransmissionData> m_aTransmissionMap = new map<int, TransmissionData>;	// map for lookup when transmission is received
	protected ref array<ref TransmissionData> m_aAdditionalSpeakers = {};							// array of additional speakers that cant be displayed

	protected PlayerManager m_PlayerManager;
	protected SCR_VONController m_VONController;
	protected SCR_InfoDisplaySlotHandler m_SlotHandler;

	protected Widget m_wVerticalLayout;
	protected Widget m_wSelectedHint;
	protected TextWidget m_wSelectedVON;
	protected TextWidget m_wSelectedFrequency;
	protected Widget m_wTalkingAmountWidget;
	protected Widget m_wAdditionalSpeakersWidget;
	protected RichTextWidget m_wAdditionalSpeakersText;

	//------------------------------------------------------------------------------------------------
	//! Count of active incoming transmissions
	int GetActiveTransmissionsCount()
	{
		return m_aTransmissions.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! Whether outgoing transmission is currently active
	bool IsCapturingTransmisionActive()
	{
		return m_OutTransmission != null && m_OutTransmission.m_bIsActive;
	}

	//------------------------------------------------------------------------------------------------
	//! VONComponent event
	event void OnCapture(BaseTransceiver transmitter)
	{
		if (!m_wRoot)
			return;

		int frequency;

		if (transmitter)	// can be null when using direct speech
			frequency = transmitter.GetFrequency();

		// update only when first activation / device changed / frequency changed
		if (m_OutTransmission.m_bForceUpdate == true
			|| m_OutTransmission.m_bIsActive == false
			|| m_OutTransmission.m_RadioTransceiver != transmitter
			|| (transmitter && m_OutTransmission.m_fFrequency != transmitter.GetFrequency())
		)
		{
			UpdateTransmission(m_OutTransmission, transmitter, frequency, false);
		}

		m_OutTransmission.m_bIsActive = true;
		m_OutTransmission.m_fActiveTimeout = 0;
	}

	protected Widget GetWidget()
	{
		Widget widget;
		int iWidgetIdx = 0;

		while (iWidgetIdx < m_iTransmissionSlots)
		{
			widget = m_aWidgetsIncomingVON.Get(iWidgetIdx);
			if (!widget.IsVisible())
				return widget;

			iWidgetIdx++;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! VONComponent event
	event void OnReceive(int playerId, BaseTransceiver receiver, int frequency, float quality)
	{
		if (!m_wRoot || m_bIsVONUIDisabled) // ignore receiving transmissions if VON UI is off
			return;

		// Check if there is an active transmission from given player
		TransmissionData pTransmission = m_aTransmissionMap.Get(playerId);


		// Active transmission from the player not found, create it
		if (!pTransmission)
		{
			if (!receiver && m_bIsVONDirectDisabled)	// direct UI off
				return;

			// No free widget
			if (!GetWidget())
			{
				//Insert new widget to display other transmissions as number
				pTransmission = new TransmissionData(m_wAdditionalSpeakersWidget, playerId);
				m_aAdditionalSpeakers.Insert(pTransmission);
				m_wAdditionalSpeakersText.SetText("+" + m_aAdditionalSpeakers.Count().ToString());
				m_wAdditionalSpeakersWidget.SetVisible(true);
				m_wAdditionalSpeakersWidget.SetOpacity(1);
				pTransmission.m_bIsAdditional = true;
				pTransmission.m_bVisible = true;
			}
			else
			{
				pTransmission = new TransmissionData(GetWidget(), playerId);
				pTransmission.m_bIsAdditional = false;
			}

			pTransmission.m_fQuality = quality;

			m_aTransmissions.Insert(pTransmission);
			m_aTransmissionMap.Insert(playerId, pTransmission);
		}

		// update only when first activation / device changed / frequency changed
		if (pTransmission.m_bIsActive == false
			|| pTransmission.m_RadioTransceiver != receiver
			|| (receiver && m_OutTransmission.m_fFrequency != frequency)
		)
		{
			bool filtered = UpdateTransmission(pTransmission, receiver, frequency, true);
			if (!filtered)
			{
				pTransmission.HideTransmission();
				return;
			}
		}

		pTransmission.m_bIsActive = true;
		pTransmission.m_fActiveTimeout = 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Update transmission data
	//! \param TransmissionData is the subject
	//! \param radioTransceiver is the used transceiver for the transmission
	//! \param IsReceiving is true when receiving transmission, false when transmitting
	//! \param isAdditionalSpeaker is true when all incomming transmission widgets are full
	//! \return false if the transimission is filtered out to not be visible
	protected bool UpdateTransmission(TransmissionData data, BaseTransceiver radioTransceiver, int frequency, bool IsReceiving)
	{
		data.m_RadioTransceiver = radioTransceiver;
		data.m_bForceUpdate = false;

		if (!radioTransceiver && m_bIsVONDirectDisabled && IsReceiving)	// can happen when existing RADIO transmission switches to direct
			return false;

		// faction
		Faction playerFaction;
		int controlledID = GetGame().GetPlayerController().GetPlayerId();

		SCR_FactionManager factionMgr = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (controlledID && factionMgr)
		{
			playerFaction = factionMgr.SGetPlayerFaction(controlledID);
			data.m_Faction = factionMgr.SGetPlayerFaction(data.m_iPlayerID);
		}

		bool enemyTransmission;
		if (IsReceiving && playerFaction && playerFaction.IsFactionEnemy(data.m_Faction))	// enemy transmission case
		{
			if (!radioTransceiver)
				return false;		// if direct ignore

			enemyTransmission = true;
		}

		if (!data.m_wBaseWidget)
			return false;

		if (data.m_bIsAdditional)
			return true;

		string sDeviceIcon;
		data.m_bIsAnimating = true;		// start anim
		data.m_wBaseWidget.SetVisible(true);
		data.m_Widgets.m_wIcon.SetVisible(true);
		data.m_Widgets.m_wMicFrame.SetVisible(false);
		data.m_Widgets.m_wChannelFrame.SetVisible(false);
		data.m_Widgets.m_wFrequency.SetVisible(false);
		data.m_Widgets.m_wSeparator.SetVisible(false);

		// radio
		if (radioTransceiver)
		{
			sDeviceIcon = ICON_RADIO;

			float adjustedFreq;
			data.m_fFrequency = frequency;
			adjustedFreq = Math.Round(data.m_fFrequency * 0.1) / 100;
			data.m_Widgets.m_wFrequency.SetText(adjustedFreq.ToString() + " " + LABEL_FREQUENCY_UNITS);
			data.m_Widgets.m_wFrequency.SetVisible(true);
		}
		// direct speech
		else
		{
			sDeviceIcon = ICON_DIRECT_SPEECH;
		}

		if (sDeviceIcon != string.Empty)
		{
			data.m_Widgets.m_wIcon.LoadImageFromSet(0, m_sImageSet, sDeviceIcon);
			data.m_Widgets.m_wIconBackground.LoadImageFromSet(0, m_sImageSetGlow, sDeviceIcon);
		}

		// incoming
		if (IsReceiving)
		{
			data.m_Widgets.m_wIcon.SetColor(COLOR_WHITE);

			if (!enemyTransmission)
				data.m_Widgets.m_wName.SetText(m_PlayerManager.GetPlayerName(data.m_iPlayerID));
			else
				data.m_Widgets.m_wName.SetText(LABEL_UNKNOWN_SOURCE);

			data.m_Widgets.m_wName.SetVisible(true);

			if (radioTransceiver)
				data.m_Widgets.m_wSeparator.SetVisible(false);
		}
		else	// outgoing
		{
			data.m_Widgets.m_wIcon.SetColor(COLOR_ORANGE);
			data.m_Widgets.m_wName.SetText(string.Empty);
			data.m_Widgets.m_wName.SetVisible(false);

			if (radioTransceiver)	// direct vs radio
			{
				data.m_Widgets.m_wMicFrame.SetVisible(m_bIsChannelToggled);

				if (SCR_Faction.Cast(playerFaction))	// show platoon
				{
					int factionHQFrequency = SCR_Faction.Cast(playerFaction).GetFactionRadioFrequency();
					if (factionHQFrequency == frequency)
					{
						data.m_Widgets.m_wChannelText.SetText(LABEL_CHANNEL_PLATOON);
						data.m_Widgets.m_wChannelFrame.SetVisible(true);
					}
				}

			}
			else
			{
				data.m_Widgets.m_wMicFrame.SetVisible(m_bIsDirectToggled);
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Fade in/out elements
	//! \param TransmissionData is the subject
	//! \param timeSlice is the OnFrame slice
	protected void OpacityFade(TransmissionData data, float timeSlice, bool isAdditional = false)
	{
		if (!data.m_wBaseWidget)
			return;

		if (data.m_bIsActive)
		{
			data.m_wBaseWidget.SetVisible(true);
			data.m_bVisible = true;

			data.m_fAlpha = Math.Lerp(data.m_fAlpha, 1, FADEIN_SPEED * timeSlice);
			if (data.m_fAlpha > 0.99)
			{
				data.m_fAlpha = 1;
				data.m_bIsAnimating = false;
			}
		}
		else
		{
			data.m_fAlpha = Math.Lerp(data.m_fAlpha, 0, FADEOUT_SPEED * timeSlice);
			if (data.m_fAlpha < 0.01)
			{
				if (isAdditional)
				{
					data.m_fAlpha = 0;
					data.m_bIsActive = false;
					data.m_bIsAnimating = false;
					data.m_fActiveTimeout = 0;
					data.m_bVisible = false;
				}
				else
				{
					data.HideTransmission();
				}

			}
		}

		if (!isAdditional)
			data.m_wBaseWidget.SetOpacity(data.m_fAlpha);
	}

	//------------------------------------------------------------------------------------------------
	//! Show hint displaying which VON method was selected
	void ShowSelectedVONHint(SCR_VONEntry entry)
	{
		SCR_VONEntryRadio radioEntry = SCR_VONEntryRadio.Cast(entry);
		if (!radioEntry || !m_wSelectedHint || !radioEntry.GetUIInfo())
			return;

		m_wSelectedHint.SetOpacity(1);
		m_wSelectedVON.SetText(radioEntry.GetUIInfo().GetName() + " Ch" + radioEntry.GetTransceiverNumber());
		m_wSelectedFrequency.SetText("[" + radioEntry.GetDisplayText() + "]");

		GetGame().GetCallqueue().Remove(FadeSelectedVONHint);								// in case of multiple selections
		GetGame().GetCallqueue().CallLater(FadeSelectedVONHint, SELECTED_HINT_FADE_SPEED);	// start fading after a certain display time

	}

	//------------------------------------------------------------------------------------------------
	protected void FadeSelectedVONHint()
	{
		AnimateWidget.Opacity(m_wSelectedHint, 0, 4);
	}

	//------------------------------------------------------------------------------------------------
	//! SCR_VONController event -> Toggle microphone indication
	protected void OnVONActiveToggled(bool directState, bool channelState)
	{
		m_bIsDirectToggled = directState;
		m_bIsChannelToggled = channelState;

		if (m_OutTransmission)
		{
			m_OutTransmission.m_bForceUpdate = true;
			OnCapture(m_OutTransmission.m_RadioTransceiver);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! SCR_PlayerController Event
	protected void OnDestroyed(Instigator killer, IEntity killerEntity)
	{
		if (m_OutTransmission)
			m_OutTransmission.HideTransmission();

		int count = m_aTransmissions.Count();
		for (int i = 0; i < count; i++)
		{
			m_aTransmissions[i].HideTransmission();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Initialize
	protected void InitDisplay()
	{
		m_bIsVONUIDisabled = GetGame().IsVONUIDisabledByServer();
		m_bIsVONDirectDisabled = GetGame().IsVONDirectSpeechUIDisabledByServer();

		m_PlayerController.m_OnDestroyed.Insert(OnDestroyed);

		m_VONController = SCR_VONController.Cast(m_PlayerController.FindComponent(SCR_VONController));
		m_VONController.GetOnVONActiveToggledInvoker().Insert(OnVONActiveToggled);
		m_VONController.SetDisplay(this);	// we set this from here instead of the other way around to avoid load order issues

		m_OutTransmission = new TransmissionData(m_wRoot.FindAnyWidget(WIDGET_TRANSMIT), 0);

		m_wVerticalLayout = m_wRoot.FindAnyWidget(WIDGET_INCOMING);
		m_wAdditionalSpeakersWidget = m_wRoot.FindAnyWidget(WIDGET_OVERFLOW);
		m_wAdditionalSpeakersText = RichTextWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_OVERFLOW_TEXT));

		m_wSelectedHint = m_wRoot.FindAnyWidget(WIDGET_SELECTED_ROOT);
		m_wSelectedVON = TextWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_SELECTED_VON));
		m_wSelectedFrequency = TextWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_SELECTED_FREQUENCY));

		m_SlotHandler = SCR_InfoDisplaySlotHandler.Cast(GetHandler(SCR_InfoDisplaySlotHandler));
		if (m_SlotHandler)
			m_SlotHandler.GetSlotUIComponent().GetOnResize().Insert(OnSlotUIResize);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSlotUIResize()
	{
		int height = m_SlotHandler.GetSlotUIComponent().GetHeight();
		m_iTransmissionSlots = ((int)height / HEIGHT_DIVIDER) - 1;
		if (m_iTransmissionSlots <= 0)
			m_iTransmissionSlots = 1;

		UpdateWidgets();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateWidgets()
	{
		int count = m_aWidgetsIncomingVON.Count();

		if (count == m_iTransmissionSlots)
			return;

		if (count > m_iTransmissionSlots)
		{
			for (int i = count - 1; i > m_iTransmissionSlots; i--)
			{
				Widget w = m_aWidgetsIncomingVON.Get(i);
				if (w)
				{
					w.RemoveFromHierarchy();
					m_aWidgetsIncomingVON.RemoveItem(w);
				}
			}
		}
		else
		{
			for (int i = count; i < m_iTransmissionSlots; i++)
			{
				Widget w = GetGame().GetWorkspace().CreateWidgets(m_sReceivingTransmissionLayout, m_wVerticalLayout);
				if (w)
				{
					w.SetVisible(false);
					m_aWidgetsIncomingVON.Insert(w);
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	// Overrides
	//------------------------------------------------------------------------------------------------
	override void DisplayUpdate(IEntity owner, float timeSlice)
	{
		// update visibility timer
		if (m_OutTransmission.m_bIsActive)
		{
			m_OutTransmission.m_fActiveTimeout += timeSlice;

			if (m_OutTransmission.m_fActiveTimeout > FADEOUT_TIMER_THRESHOLD)
			{
				m_OutTransmission.m_bIsActive = false;
				m_OutTransmission.m_bIsAnimating = true;
			}
		}

		// update fade
		if (m_OutTransmission.m_bIsAnimating)
			OpacityFade(m_OutTransmission, timeSlice);

		// update incoming transmissions
		if (m_aTransmissions.IsEmpty())
			return;

		TransmissionData pTransmission;
		int count = m_aTransmissions.Count() - 1;

		for (int i = count; i >= 0; i--)
		{
			if (i > m_aTransmissions.Count() - 1)
			{
				count -1;
				continue;
			}

			pTransmission = m_aTransmissions[i];

			bool isAdditional = m_aAdditionalSpeakers.Contains(pTransmission);

			// update visibility timer
			if (pTransmission.m_bIsActive)
			{
				pTransmission.m_fActiveTimeout += timeSlice;

				if (pTransmission.m_fActiveTimeout > FADEOUT_TIMER_THRESHOLD)
				{
					pTransmission.m_bIsActive = false;
					pTransmission.m_bIsAnimating = true;
				}
			}

			// update fade
			if (pTransmission.m_bIsAnimating)
				OpacityFade(pTransmission, timeSlice, isAdditional);

			// remove faded transmissions
			if (!pTransmission.m_bVisible)
			{
				if (pTransmission.m_iPlayerID)
					m_aTransmissionMap.Remove(pTransmission.m_iPlayerID);

				m_aTransmissions.Remove(i);

				if (isAdditional)
				{
					m_aAdditionalSpeakers.Remove(m_aAdditionalSpeakers.Find(pTransmission));
					m_wAdditionalSpeakersText.SetText("+" + m_aAdditionalSpeakers.Count().ToString());

					if (m_aAdditionalSpeakers.IsEmpty())
						m_wAdditionalSpeakersWidget.SetVisible(false);
				}

				if (m_aAdditionalSpeakers.Count() > 0 && (m_aTransmissions.Count() - m_aAdditionalSpeakers.Count()) < m_aWidgetsIncomingVON.Count())
				{
					m_aAdditionalSpeakers[0].m_bIsActive = false;
					m_aTransmissionMap.Remove(m_aAdditionalSpeakers[0].m_iPlayerID);
					m_aTransmissions.Remove(m_aTransmissions.Find(m_aAdditionalSpeakers[0]));

					OnReceive(m_aAdditionalSpeakers[0].m_iPlayerID, m_aAdditionalSpeakers[0].m_RadioTransceiver, m_aAdditionalSpeakers[0].m_fFrequency, m_aAdditionalSpeakers[0].m_fQuality);

					m_aAdditionalSpeakers.Remove(0);
					m_wAdditionalSpeakersText.SetText("+" + m_aAdditionalSpeakers.Count().ToString());

					if (m_aAdditionalSpeakers.Count() == 0)
						m_wAdditionalSpeakersWidget.SetVisible(false);
				}

				count - 1;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool DisplayStartDrawInit(IEntity owner)
	{
		m_PlayerManager = GetGame().GetPlayerManager();

		return m_PlayerManager != null;
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		if (!m_wRoot)
			return;

		InitDisplay();
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayControlledEntityChanged(IEntity from, IEntity to)
	{
		if (m_OutTransmission)
			m_OutTransmission.HideTransmission();

		int count = m_aTransmissions.Count();
		for (int i = 0; i < count; i++)
		{
			m_aTransmissions[i].HideTransmission();
		}
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayStopDraw(IEntity owner)
	{
		if (m_PlayerController)
			m_PlayerController.m_OnDestroyed.Remove(OnDestroyed);

		if (m_VONController)
			m_VONController.GetOnVONActiveToggledInvoker().Remove(OnVONActiveToggled);
	}
}
