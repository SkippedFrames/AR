class SCR_GroupSubMenu : SCR_SubMenuBase
{
	[Attribute("6", params: "1 100 1")]
	protected int m_iMaxColumnNumber;
	
	[Attribute()]
	protected ResourceName m_ButtonLayout;
	
	[Attribute("#AR-PauseMenu_Continue", UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sContinueButtonText;
	
	protected Widget m_wGridWidget;
	
	protected const string CREATE_GROUP = "#AR_DeployMenu_AddNewGroup";
	protected const string JOIN_GROUP = "#AR-DeployMenu_JoinGroup";
	protected const string ACCEPT_INVITE = "#AR-DeployMenu_AcceptInvite";
	
	protected static SCR_PlayerControllerGroupComponent s_PlayerGroupController;
	protected SCR_InputButtonComponent m_AddGroupButton;
	protected SCR_InputButtonComponent m_JoinGroupButton;
	protected SCR_InputButtonComponent m_AcceptInviteButton;
	protected SCR_InputButtonComponent m_GroupSettingsButton;
	protected Faction m_PlayerFaction;
	protected SCR_GroupsManagerComponent m_GroupManager;
	protected SCR_ChatPanel m_ChatPanelComponent;
	
	protected static ref ScriptInvoker s_OnJoinGroupRequestSent;
	
		
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuOpen(parentMenu);
		s_PlayerGroupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		m_GroupManager = SCR_GroupsManagerComponent.GetInstance();
		CreateAddGroupButton();
		CreateJoinGroupButton();
		CreateAcceptInviteButton();
		CreateGroupSettingsButton();
		SetupNameChangeButton();
		SetupPrivateChecker();
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (factionManager)
		{
			m_PlayerFaction = SCR_Faction.Cast(factionManager.GetLocalPlayerFaction());
		}
		else
			m_PlayerFaction = null;
		
		Widget chatPanel = parentMenu.GetRootWidget().FindAnyWidget("ChatPanel");
		if (!chatPanel)
			return;
		 m_ChatPanelComponent = SCR_ChatPanel.Cast(chatPanel.FindHandler(SCR_ChatPanel));
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuShow(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuShow(parentMenu);
			
		UpdateGroupsMenu();
						
		if (m_ChatPanelComponent)
		{
			m_ChatPanelComponent.GetOnChatOpen().Insert(OnChatOpen);
			m_ChatPanelComponent.GetOnChatClosed().Insert(OnChatClosed);
			if (m_ChatPanelComponent.IsOpen())
				OnChatOpen();
			
			if (m_ChatPanelComponent.GetFadeOut() == false)
			{
				Widget chatContent = m_ParentMenu.GetRootWidget().FindAnyWidget("ChatContent");
				if (chatContent)
					chatContent.SetVisible(true);
				SCR_FadeInOutAnimator chatAnimator = m_ChatPanelComponent.GetFadeInOutAnimator();
				if (chatAnimator)
					chatAnimator.GetOnStateChanged().Insert(OnAnimatorStateChanged);
			}
		}
		
		m_GroupManager.GetOnPlayableGroupRemoved().Insert(UpdateGroupsMenu);
		m_GroupManager.GetOnPlayableGroupCreated().Insert(UpdateGroupsMenu);
		m_GroupManager.GetOnNewGroupsAllowedChanged().Insert(UpdateGroupsMenu);
		SCR_AIGroup.GetOnPlayerAdded().Insert(UpdateGroupsMenu);
		SCR_AIGroup.GetOnPlayerRemoved().Insert(UpdateGroupsMenu);
		SCR_AIGroup.GetOnPlayerLeaderChanged().Insert(UpdateGroupsMenu);
		SCR_AIGroup.GetOnPrivateGroupChanged().Insert(UpdateGroupsMenu);
		SCR_AIGroup.GetOnCustomNameChanged().Insert(UpdateGroupsMenu);
		SCR_AIGroup.GetOnCustomDescriptionChanged().Insert(UpdateGroupsMenu);
		SCR_AIGroup.GetOnFlagSelected().Insert(UpdateGroupsMenu);
		SCR_AIGroup.GetOnFrequencyChanged().Insert(UpdateGroupsMenu);
		SCR_GroupTileButton.GetOnGroupTileClicked().Insert(UpdateGroupsMenu);
		SetAcceptButtonStatus();
	}	
	
	//------------------------------------------------------------------------------------------------
	static ScriptInvoker GetOnJoingGroupRequestSent()
	{
		if (!s_OnJoinGroupRequestSent)
			s_OnJoinGroupRequestSent = new ScriptInvoker();
		
		return s_OnJoinGroupRequestSent;
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateGroupsMenu()
	{
		m_wGridWidget = m_ParentMenu.GetRootWidget().FindAnyWidget("GroupList");
		if (!m_wGridWidget || !m_AddGroupButton || !m_ButtonLayout || !m_GroupSettingsButton)
			return;
		
		SetAcceptButtonStatus();
		InitGroups(m_wGridWidget, m_AddGroupButton, m_JoinGroupButton, m_GroupSettingsButton, m_ButtonLayout, m_ParentMenu);
		
		SCR_GroupMenu groupMenu = SCR_GroupMenu.Cast(m_ParentMenu.GetRootWidget().FindHandler(SCR_GroupMenu));
		if (!groupMenu)
			return;
		
		groupMenu.UpdateTabs();		
	}
	
	//------------------------------------------------------------------------------------------------
	static void InitGroups(Widget grid, SCR_InputButtonComponent addGroupButton, SCR_InputButtonComponent joinGroupButton, SCR_InputButtonComponent groupSettingsButton, ResourceName buttonWidgetLayout, SCR_SuperMenuBase parentMenu)
	{
		if (!grid)
			return;
		
		GetGame().GetCallqueue().CallLater(UpdateGroups, 1, false, grid, addGroupButton, joinGroupButton, groupSettingsButton, buttonWidgetLayout, parentMenu);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuHide(SCR_SuperMenuBase parentMenu) 
	{ 
		super.OnMenuHide(parentMenu);
		m_GroupManager = SCR_GroupsManagerComponent.GetInstance();
		if (m_GroupManager)
		{
			m_GroupManager.GetOnPlayableGroupRemoved().Remove(UpdateGroupsMenu);
			m_GroupManager.GetOnPlayableGroupCreated().Remove(UpdateGroupsMenu);
			m_GroupManager.GetOnNewGroupsAllowedChanged().Remove(UpdateGroupsMenu);
		}
		
		if (m_ChatPanelComponent)
		{
			m_ChatPanelComponent.GetOnChatOpen().Remove(OnChatOpen);
			m_ChatPanelComponent.GetOnChatClosed().Remove(OnChatClosed);
		}
		
		SCR_AIGroup.GetOnPlayerAdded().Remove(UpdateGroupsMenu);
		SCR_AIGroup.GetOnPlayerRemoved().Remove(UpdateGroupsMenu);
		SCR_AIGroup.GetOnPlayerLeaderChanged().Remove(UpdateGroupsMenu);
		SCR_AIGroup.GetOnPrivateGroupChanged().Remove(UpdateGroupsMenu);		
		SCR_AIGroup.GetOnCustomNameChanged().Remove(UpdateGroupsMenu);
		SCR_AIGroup.GetOnCustomDescriptionChanged().Remove(UpdateGroupsMenu);
		SCR_AIGroup.GetOnFlagSelected().Remove(UpdateGroupsMenu);
		SCR_AIGroup.GetOnFrequencyChanged().Remove(UpdateGroupsMenu);
		SCR_GroupTileButton.GetOnGroupTileClicked().Remove(UpdateGroupsMenu);
		if (s_PlayerGroupController)
			s_PlayerGroupController.GetOnInviteReceived().Remove(SetAcceptButtonStatus);	
		
		GetGame().GetCallqueue().CallLater(m_GroupSettingsButton.SetVisible, 100, false, false, true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected static void UpdateGroups(notnull Widget gridWidget, SCR_InputButtonComponent addGroupbutton, SCR_InputButtonComponent joinGroupButton, SCR_InputButtonComponent groupSettingsButton, ResourceName buttonWidgetLayout, SCR_SuperMenuBase parentMenu)
	{
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return;
		
		SCR_Faction playerFaction = SCR_Faction.Cast(factionManager.GetLocalPlayerFaction());
		if (!playerFaction)
			return;
		
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return;
		
		//iun case the method was called from playerlist while skipping deployscreen try to get the group controller
		if (!s_PlayerGroupController)
		{
			s_PlayerGroupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
			if (!s_PlayerGroupController)
				return;
		}
		
		//no need to check playerFaction for null, because groups are not enabled for players without faction
		addGroupbutton.SetEnabled(groupManager.CanCreateNewGroup(playerFaction));
		Widget children = gridWidget.GetChildren();
		while (children)
		{
			gridWidget.RemoveChild(children);
			children = gridWidget.GetChildren();
		}
				
		SetGroupSettingsButton(groupSettingsButton);
		
		array<SCR_AIGroup> playableGroups = groupManager.GetPlayableGroupsByFaction(playerFaction);
		if (!playableGroups)
			return;
		
		int selectedGroupID = s_PlayerGroupController.GetSelectedGroupID();
		
		if (playableGroups.IsIndexValid(selectedGroupID) && parentMenu.GetRootWidget())
		{
			ImageWidget privateIcon = ImageWidget.Cast(parentMenu.GetRootWidget().FindAnyWidget("PrivateIconDetail"));
			if (privateIcon)
				privateIcon.SetVisible(playableGroups[selectedGroupID].IsPrivate());
		}
		
		int groupCount = playableGroups.Count();
		
		for (int i = 0; i < groupCount; i++)
		{ 
			Widget groupTile = GetGame().GetWorkspace().CreateWidgets(buttonWidgetLayout, gridWidget);	
			if (!groupTile)
				continue;
					
			ButtonWidget buttonWidget = ButtonWidget.Cast(groupTile.FindAnyWidget("Button"));
			if (!buttonWidget)
				continue;
			
			SCR_GroupTileButton buttonComponent = SCR_GroupTileButton.Cast(buttonWidget.FindHandler(SCR_GroupTileButton));
			if (buttonComponent)
			{
				buttonComponent.SetGroupID(playableGroups[i].GetGroupID());
				buttonComponent.SetGroupFaction(playerFaction);
				buttonComponent.SetJoinGroupButton(joinGroupButton);
				buttonComponent.InitiateGroupTile();				
				if (s_PlayerGroupController.GetGroupID() == -1 && i == 0 && s_PlayerGroupController.GetSelectedGroupID() < 0)
					GetGame().GetCallqueue().CallLater(buttonComponent.RefreshPlayers, 1, false);
				if (selectedGroupID < 0 && playableGroups[i].GetGroupID() == s_PlayerGroupController.GetGroupID() )
					GetGame().GetCallqueue().CallLater(buttonComponent.RefreshPlayers, 1, false);
				if (selectedGroupID == playableGroups[i].GetGroupID())
					GetGame().GetCallqueue().CallLater(buttonComponent.RefreshPlayers, 1, false);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateAddGroupButton()
	{
		m_AddGroupButton = CreateNavigationButton("MenuAddGroup", CREATE_GROUP, true);
		if (!m_AddGroupButton)
			return;
		m_AddGroupButton.GetRootWidget().SetZOrder(0);
		m_AddGroupButton.m_OnActivated.Insert(CreateNewGroup);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateJoinGroupButton()
	{
		m_JoinGroupButton = CreateNavigationButton("MenuJoinGroup", JOIN_GROUP, true);
		if (!m_JoinGroupButton)
			return;
		m_JoinGroupButton.GetRootWidget().SetZOrder(0);
		m_JoinGroupButton.m_OnActivated.Insert(JoinSelectedGroup);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateAcceptInviteButton()
	{
		m_AcceptInviteButton = CreateNavigationButton("GroupAcceptInvite", ACCEPT_INVITE, true);
		if (!m_AcceptInviteButton)
			return;
		m_AcceptInviteButton.GetRootWidget().SetZOrder(0);
		m_AcceptInviteButton.m_OnActivated.Insert(AcceptInvite);
		s_PlayerGroupController.GetOnInviteReceived().Insert(SetAcceptButtonStatus);
		SetAcceptButtonStatus()
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateGroupSettingsButton()
	{
		m_GroupSettingsButton = CreateNavigationButton("MenuSettingsGroup", "#AR-Player_Groups_Settings", true);
		if (!m_GroupSettingsButton)
			return;
		m_GroupSettingsButton.GetRootWidget().SetZOrder(0);
		m_GroupSettingsButton.SetVisible(false);
		m_GroupSettingsButton.m_OnActivated.Insert(OpenGroupSettingsDialog);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ChangeGroupPublicState()
	{
		SCR_PlayerControllerGroupComponent playerComponent = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!playerComponent)
			return;
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		SCR_AIGroup playerGroup = groupsManager.FindGroup(playerComponent.GetGroupID());
		playerComponent.RequestPrivateGroupChange(playerComponent.GetPlayerID() , !playerGroup.IsPrivate());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OpenGroupSettingsDialog()
	{
		GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.GroupSettingsDialog);
	}
		
	//------------------------------------------------------------------------------------------------
	protected static void CreateNewGroup()
	{
		if (!s_PlayerGroupController)
			return;
		//we reset the actual group so the menu goes to players actual group, in this case newly created one
		s_PlayerGroupController.SetSelectedGroupID(-1);
		s_PlayerGroupController.RequestCreateGroup();
	}
	
	//------------------------------------------------------------------------------------------------
	static void JoinSelectedGroup()
	{
		if (!s_PlayerGroupController)
			return;	
						
		s_PlayerGroupController.RequestJoinGroup(s_PlayerGroupController.GetSelectedGroupID());
	}	
		
	//------------------------------------------------------------------------------------------------
	static void RequestJoinPrivateGroup()
	{	
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		
		SCR_AIGroup group = groupManager.FindGroup(s_PlayerGroupController.GetSelectedGroupID()); 
		if (!group)
			return;					
		
		if (s_PlayerGroupController)	
		{
			s_PlayerGroupController.PlayerRequestToJoinPrivateGroup(s_PlayerGroupController.GetPlayerID(), Replication.FindId(group));
			SCR_NotificationsComponent.SendToPlayer(s_PlayerGroupController.GetPlayerID(), ENotification.GROUPS_REQUEST_SENT, group.GetGroupID());
			if (s_OnJoinGroupRequestSent)
				GetOnJoingGroupRequestSent().Invoke();
		}	
	}
	
	//------------------------------------------------------------------------------------------------
	static void SetSelectedGroup(int groupID)
	{
		s_PlayerGroupController.SetSelectedGroupID(groupID);
	}
	
	//------------------------------------------------------------------------------------------------
	void AcceptInvite()
	{
		s_PlayerGroupController.AcceptInvite();
		SetAcceptButtonStatus();		
	}
	
	//------------------------------------------------------------------------------------------------
	void SetAcceptButtonStatus()
	{				
		if (s_PlayerGroupController.GetGroupInviteID() == -1)
		{
			m_AcceptInviteButton.SetEnabled(false);
		}
		else
		{
			SCR_AIGroup group = m_GroupManager.FindGroup(s_PlayerGroupController.GetGroupInviteID());
			
			if (!group)
			{
				m_AcceptInviteButton.SetEnabled(false);
				s_PlayerGroupController.SetGroupInviteID(-1);
				return;	
			}		
			m_AcceptInviteButton.SetEnabled(true);	
		}
	}
	
	//------------------------------------------------------------------------------------------------
	private void OnConfirm()
	{
		//dont assign group if player already has one
		if (s_PlayerGroupController.GetGroupID() != -1)
			return;
		
		SCR_AIGroup newPlayerGroup = m_GroupManager.GetFirstNotFullForFaction(m_PlayerFaction, null, true);
		if (!newPlayerGroup)
		{
			//we return here, because creationg of the group automatically moves player to the group
			s_PlayerGroupController.RequestCreateGroup();
			return;
		}
		s_PlayerGroupController.RequestJoinGroup(newPlayerGroup.GetGroupID());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnChatOpen()
	{
		Widget chatContent = m_ParentMenu.GetRootWidget().FindAnyWidget("ChatContent");
		if (chatContent)
			chatContent.SetVisible(true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnChatClosed()
	{
		if (SCR_ChatPanelManager.GetInstance().GetMessages().Count() == 0)
		{
			Widget chatContent = m_ParentMenu.GetRootWidget().FindAnyWidget("ChatContent");
			if (chatContent)
				chatContent.SetVisible(false);
			return;
		}	
		SCR_FadeInOutAnimator chatAnimator = m_ChatPanelComponent.GetFadeInOutAnimator();
		if (!chatAnimator)
			return;
		chatAnimator.GetOnStateChanged().Insert(OnAnimatorStateChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnAnimatorStateChanged(int formerState, int actualState)
	{
		if (formerState == 2 && actualState == 3)
		{
			Widget chatContent = m_ParentMenu.GetRootWidget().FindAnyWidget("ChatContent");
			if (chatContent)
				chatContent.SetVisible(false);
			m_ChatPanelComponent.GetFadeInOutAnimator().GetOnStateChanged().Remove(OnAnimatorStateChanged);
		}	
	}
	
	//------------------------------------------------------------------------------------------------
	protected static void SetGroupSettingsButton(notnull SCR_InputButtonComponent button)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();		
		if (!groupsManager)
			return;
		
		button.SetVisible(s_PlayerGroupController.IsPlayerLeaderOwnGroup() && groupsManager.CanPlayersChangeAttributes());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetupNameChangeButton()
	{
		ButtonWidget nameChangeButton = ButtonWidget.Cast(GetRootWidget().FindAnyWidget("ChangeNameButton"));
		if (!nameChangeButton)
			return;
		
		SCR_ButtonImageComponent buttonComp = SCR_ButtonImageComponent.Cast(nameChangeButton.FindHandler(SCR_ButtonImageComponent));
		if (!buttonComp)
			return;
		
		buttonComp.m_OnClicked.Insert(OpenGroupSettingsDialog);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetupPrivateChecker()
	{
		ButtonWidget privateChecker = ButtonWidget.Cast(GetRootWidget().FindAnyWidget("PrivateChecker"));
		if (!privateChecker)
			return;
		
		SCR_ButtonCheckerComponent buttonComp = SCR_ButtonCheckerComponent.Cast(privateChecker.FindHandler(SCR_ButtonCheckerComponent));
		if (!buttonComp)
			return;
		
		buttonComp.m_OnClicked.Insert(OnPrivateCheckerClicked);		
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPrivateCheckerClicked()
	{
		SCR_AIGroup group = m_GroupManager.FindGroup(s_PlayerGroupController.GetGroupID());
		if (!group)
			return;
		
		s_PlayerGroupController.RequestPrivateGroupChange(s_PlayerGroupController.GetPlayerID() , !group.IsPrivate());
	}
};
