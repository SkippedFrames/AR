class SCR_VotingStickyNotificationUIComponent: SCR_StickyNotificationUIComponent
{
	[Attribute("#AR-Notification_Sticky_MultiplePlayerVotes", desc: "When there is more then 1 vote in progress", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sMultipleVotesText;
	
	[Attribute("VotingInstantVote")]
	protected string m_sInstantVoteLayoutName;
	
	[Attribute("Time")]
	protected string m_sRemainingVotingTimeName;
	
	[Attribute("#AR-Voting_PlayerCountFormatting")]
	protected string m_sCurrentVoteCountFormatting;
	
	protected Widget m_InstantVoteLayout;
	protected TextWidget m_RemainingVotingTime;
	
	protected SCR_VotingManagerComponent m_VotingManagerComponent;
	
	//~ If one vote is set then the duration left is shown
	protected EVotingType m_eActiveSingularVoteType;
	protected int m_eActiveSingularVoteValue;
	
	protected bool m_bIsUpdatingTime;
	protected bool m_bIsUpdatingActionContext;
	
	//------------------------------------------------------------------------------------------------
	protected void OnVotingChanged(bool isInit = false)
	{		
		array<EVotingType> validActiveVotingTypes = {};
		array<int> votingValues = {};
		int count = m_VotingManagerComponent.GetAllVotingsWithValue(validActiveVotingTypes, votingValues, false, true);
		
		for (int i = count - 1; i >= 0; i--)
		{
			//~ Ignore any abstained, unavailible or voted votes
			if (!m_VotingManagerComponent.IsVotingAvailable(validActiveVotingTypes[i], votingValues[i]) || m_VotingManagerComponent.HasAbstainedLocally(validActiveVotingTypes[i], votingValues[i]) || m_VotingManagerComponent.IsLocalVote(validActiveVotingTypes[i], votingValues[i]))
			{
				validActiveVotingTypes.RemoveOrdered(i);
				votingValues.RemoveOrdered(i);
			}	
		}
		
		int votingCount = validActiveVotingTypes.Count();
		
		//If no votes active
		if (votingCount <= 0)
		{
			//Hide notification
			SetStickyActive(false, !isInit);
			
			EnableVotingTimerUI(false);
			EnableContextAction(false);
			return;
		}
		//Multiple votes active
		else if (votingCount > 1)
		{
			m_Text.SetTextFormat(m_sMultipleVotesText, votingCount.ToString());
			
			if (m_InstantVoteLayout)
				m_InstantVoteLayout.SetVisible(false);
			if (m_OptionalMessageLayout)
				m_OptionalMessageLayout.SetVisible(true);
			
			EnableVotingTimerUI(false);
			EnableContextAction(false);
		}
		//One vote active
		else 
		{		
			m_eActiveSingularVoteType = validActiveVotingTypes[0];
			m_eActiveSingularVoteValue = votingValues[0];
			
			if (m_eActiveSingularVoteValue != SCR_VotingBase.DEFAULT_VALUE)
			{
				string playerName;
				
				PlayerManager playermanager = GetGame().GetPlayerManager();
				if (playermanager)
					playerName = playermanager.GetPlayerName(votingValues[0]);
				
				string text = WidgetManager.Translate(m_VotingManagerComponent.GetVotingInfo(m_eActiveSingularVoteType).GetStickyNotificationText(), playerName);
				
				int currentVotes, VotesRequired;
				if (m_VotingManagerComponent.GetVoteCounts(m_eActiveSingularVoteType, m_eActiveSingularVoteValue, currentVotes, VotesRequired))
					m_Text.SetTextFormat(m_sCurrentVoteCountFormatting, text, currentVotes, VotesRequired);
				else 
					m_Text.SetTextFormat(text);
			}
			
			if (m_InstantVoteLayout)
				m_InstantVoteLayout.SetVisible(true);
			if (m_OptionalMessageLayout)
				m_OptionalMessageLayout.SetVisible(false);
			
			EnableVotingTimerUI(true);
			EnableContextAction(true);
		}		
		
		//Show Notification
		SetStickyActive(true, !isInit);
	}
	
	//~ When the UI is visible the player can use the instant vote actions (If there is only one vote active)
	protected void EnableContextAction(bool enable)
	{
		if (enable)
		{
			if (m_bIsUpdatingActionContext)
				return;
			
			GetGame().GetInputManager().AddActionListener("InstantVote", EActionTrigger.DOWN, SCR_VoterComponent.InstantVote);
			GetGame().GetInputManager().AddActionListener("InstantVoteAbstain", EActionTrigger.DOWN, SCR_VoterComponent.InstantRemoveAndAbstainVote);
			GetGame().GetCallqueue().CallLater(InstantVotingContextUpdate, repeat: true);
		}
		else 
		{
			if (!m_bIsUpdatingActionContext)
				return;
			
			GetGame().GetInputManager().RemoveActionListener("InstantVote", EActionTrigger.DOWN, SCR_VoterComponent.InstantVote);
			GetGame().GetInputManager().RemoveActionListener("InstantVoteAbstain", EActionTrigger.DOWN, SCR_VoterComponent.InstantRemoveAndAbstainVote);
			GetGame().GetCallqueue().Remove(InstantVotingContextUpdate);
		}
		
		m_bIsUpdatingActionContext = enable;
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Updates each frame to keep InstantVotingContext active
	protected void InstantVotingContextUpdate()
	{
		GetGame().GetInputManager().ActivateContext("InstantVotingContext");
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Enable the voting timer UI
	protected void EnableVotingTimerUI(bool enable)
	{
		if (!m_RemainingVotingTime)
			return;
		
		if (enable)
		{
			if (m_bIsUpdatingTime)
				return;
			
			VotingTimeUpdate();
			m_RemainingVotingTime.SetVisible(true);
			GetGame().GetCallqueue().CallLater(VotingTimeUpdate, 1000, true);
		}
		else 
		{
			if (!m_bIsUpdatingTime)
				return;
			
			m_RemainingVotingTime.SetVisible(false);
			GetGame().GetCallqueue().Remove(VotingTimeUpdate);
		}
		
		m_bIsUpdatingTime = enable;
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Called when only 1 vote is active. Called every 1 second to update
	protected void VotingTimeUpdate()
	{
		m_RemainingVotingTime.SetText(SCR_FormatHelper.GetTimeFormatting(Math.Clamp(m_VotingManagerComponent.GetRemainingDurationOfVote(m_eActiveSingularVoteType, m_eActiveSingularVoteValue), 0, float.MAX), ETimeFormatParam.DAYS | ETimeFormatParam.HOURS, ETimeFormatParam.DAYS | ETimeFormatParam.HOURS | ETimeFormatParam.MINUTES));	
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnVoteStartOrChanged(EVotingType type, int value)
	{
		//~ One frame later to be sure that all votes were correctly counted
		GetGame().GetCallqueue().CallLater(OnVotingChanged, param1: false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnVoteCountChanged(EVotingType type, int value, int voteCount)
	{
		//~ One frame later to be sure that all votes were correctly counted
		GetGame().GetCallqueue().CallLater(OnVotingChanged, param1: false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnVotingEnd(EVotingType type, int value, int winner)
	{
		//~ One frame later to be sure that all votes were correctly counted
		GetGame().GetCallqueue().CallLater(OnVotingChanged, param1: false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlayerJoinOrLeaveServer(int playerId)
	{
		//~ No need to do anything if sticky notification is not active
		if (!m_bStickyNotificationActive)
			return;
		
		//~ Call later to make sure everything is correctly setup
		GetGame().GetCallqueue().CallLater(OnVotingChanged, param1: false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlayerFactionChanged(Faction faction, int newCount)
	{
		//~ No need to do anything if sticky notification is not active
		if (!m_bStickyNotificationActive)
			return;

		//~ Call later to make sure everything is correctly setup
		GetGame().GetCallqueue().CallLater(OnVotingChanged, param1: false);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(SCR_NotificationsLogComponent notificationLog)
	{					
		super.OnInit(notificationLog);
		
		m_InstantVoteLayout = m_Root.FindAnyWidget(m_sInstantVoteLayoutName);
		if (m_InstantVoteLayout)
			m_RemainingVotingTime = TextWidget.Cast(m_InstantVoteLayout.FindAnyWidget(m_sRemainingVotingTimeName));	
			
		m_VotingManagerComponent = SCR_VotingManagerComponent.GetInstance();
		if (m_VotingManagerComponent)
		{			
			m_VotingManagerComponent.GetOnVotingStart().Insert(OnVoteStartOrChanged);
			m_VotingManagerComponent.GetOnVotingEnd().Insert(OnVotingEnd);
			m_VotingManagerComponent.GetOnVoteLocal().Insert(OnVoteStartOrChanged);
			m_VotingManagerComponent.GetOnRemoveVoteLocal().Insert(OnVoteStartOrChanged);
			m_VotingManagerComponent.GetOnAbstainVoteLocal().Insert(OnVoteStartOrChanged);
			m_VotingManagerComponent.GetOnVoteCountChanged().Insert(OnVoteCountChanged);
			OnVotingChanged(true);
		}
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
		{
			gameMode.GetOnPlayerRegistered().Insert(OnPlayerJoinOrLeaveServer);
			gameMode.GetOnPlayerDisconnected().Insert(OnPlayerJoinOrLeaveServer);
		}
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (factionManager)
			factionManager.GetOnPlayerFactionCountChanged().Insert(OnPlayerFactionChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnButton()
	{
		MenuManager menuManager = GetGame().GetMenuManager();
				
		if (menuManager)
			menuManager.OpenDialog(ChimeraMenuPreset.PlayerListMenu);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnDestroy()
	{
		if (m_VotingManagerComponent)
		{
			m_VotingManagerComponent.GetOnVotingStart().Remove(OnVoteStartOrChanged);
			m_VotingManagerComponent.GetOnVotingEnd().Remove(OnVotingEnd);
			m_VotingManagerComponent.GetOnVoteLocal().Remove(OnVoteStartOrChanged);
			m_VotingManagerComponent.GetOnRemoveVoteLocal().Remove(OnVoteStartOrChanged);
			m_VotingManagerComponent.GetOnAbstainVoteLocal().Remove(OnVoteStartOrChanged);
			m_VotingManagerComponent.GetOnVoteCountChanged().Remove(OnVoteCountChanged);
		}
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
		{
			gameMode.GetOnPlayerRegistered().Remove(OnPlayerJoinOrLeaveServer);
			gameMode.GetOnPlayerDisconnected().Remove(OnPlayerJoinOrLeaveServer);
		}
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (factionManager)
			factionManager.GetOnPlayerFactionCountChanged().Remove(OnPlayerFactionChanged);
		
		if (m_bIsUpdatingActionContext)
		{
			InputManager inputManager = GetGame().GetInputManager();
			
			if (inputManager)
			{
				inputManager.RemoveActionListener("InstantVote", EActionTrigger.DOWN, SCR_VoterComponent.InstantVote);
				inputManager.RemoveActionListener("InstantVoteAbstain", EActionTrigger.DOWN, SCR_VoterComponent.InstantRemoveAndAbstainVote);
			}
			
			GetGame().GetCallqueue().Remove(InstantVotingContextUpdate);
		}
		if (m_bIsUpdatingTime)
			GetGame().GetCallqueue().Remove(VotingTimeUpdate);
	}	
};
