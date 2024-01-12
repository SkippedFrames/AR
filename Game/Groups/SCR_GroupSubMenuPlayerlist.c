class SCR_GroupSubMenuPlayerlist : SCR_SubMenuBase
{
	[Attribute("6", params: "1 100 1")]
	protected int m_iMaxColumnNumber;
	
	[Attribute()]
	protected ResourceName m_ButtonLayout;
	
	[Attribute("#AR-PauseMenu_Continue", UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sContinueButtonText;
	
	protected Widget m_wGridWidget;
	protected SCR_InputButtonComponent m_AddGroupButton;
	protected SCR_InputButtonComponent m_JoinGroupButton;
	protected SCR_InputButtonComponent m_AcceptInviteButton;
	protected SCR_InputButtonComponent m_GroupSettingsButton;
	protected SCR_InputButtonComponent m_ViewProfileButton;
	protected SCR_GroupsManagerComponent m_GroupManager;
	protected SCR_PlayerControllerGroupComponent m_PlayerGroupController;
	
	protected const string CREATE_GROUP = "#AR_DeployMenu_AddNewGroup";
	protected const string JOIN_GROUP = "#AR-DeployMenu_JoinGroup";
	protected const string ACCEPT_INVITE = "#AR-DeployMenu_AcceptInvite";
	
	protected int m_iLastSelectedPlayerId;
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(SCR_SuperMenuBase parentMenu, float tDelta)
	{
		GetGame().GetInputManager().ActivateContext("GroupMenuContext");
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuOpen(parentMenu);
		m_GroupManager = SCR_GroupsManagerComponent.GetInstance();
		m_PlayerGroupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!m_PlayerGroupController)
			return;
		if (!m_ParentMenu)
			return;
	
		CreateAddGroupButton();
		CreateJoinGroupButton();
		CreateAcceptInviteButton();
		CreateGroupSettingsButton();
		CreateViewProfileButton();
		SetupNameChangeButton();
		SetupPrivateChecker();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuShow(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuShow(parentMenu);
	
		UpdateViewProfileButton(true);
		
		//todo:mku this is a temporary solution because of how playerlist is implemented right now
		OverlayWidget header = OverlayWidget.Cast(m_ParentMenu.GetRootWidget().FindAnyWidget("SortHeader"));
		if (header)
			header.SetVisible(false);
		ScrollLayoutWidget scrollWidget = ScrollLayoutWidget.Cast(m_ParentMenu.GetRootWidget().FindAnyWidget("ScrollLayout0"));
		if (scrollWidget)
			scrollWidget.SetVisible(false);
		HorizontalLayoutWidget footerLeft = HorizontalLayoutWidget.Cast(m_ParentMenu.GetRootWidget().FindAnyWidget("FooterLeft"));
		if (footerLeft)
			footerLeft.SetVisible(false);
		
		UpdateGroupsMenu();
		
		if (m_GroupManager)
		{
			m_GroupManager.GetOnPlayableGroupRemoved().Insert(UpdateGroupsMenu);
			m_GroupManager.GetOnPlayableGroupCreated().Insert(UpdateGroupsMenu);
		}
		
		SCR_AIGroup.GetOnPlayerAdded().Insert(UpdateGroupsMenu);
		SCR_AIGroup.GetOnPlayerRemoved().Insert(UpdateGroupsMenu);
		SCR_AIGroup.GetOnPlayerLeaderChanged().Insert(UpdateGroupsMenu);
		SCR_AIGroup.GetOnPrivateGroupChanged().Insert(UpdateGroupsMenu);
		SCR_AIGroup.GetOnCustomNameChanged().Insert(UpdateGroupsMenu);
		SCR_AIGroup.GetOnFlagSelected().Insert(UpdateGroupsMenu);
		SCR_AIGroup.GetOnCustomDescriptionChanged().Insert(UpdateGroupsMenu);
		SCR_GroupTileButton.GetOnGroupTileClicked().Insert(UpdateGroupsMenu);
		SCR_GroupTileButton.GetOnPlayerTileFocus().Insert(OnPlayerTileFocus);
		SCR_GroupTileButton.GetOnPlayerTileFocusLost().Insert(OnPlayerTileFocusLost);
		SetAcceptButtonStatus();
	}	
		
	//------------------------------------------------------------------------------------------------
	void UpdateGroupsMenu()
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController || !m_ParentMenu)
			return;
		
		SetAcceptButtonStatus();
		
		m_wGridWidget = m_ParentMenu.GetRootWidget().FindAnyWidget("GroupList");
		SCR_GroupSubMenu.InitGroups(m_wGridWidget, m_AddGroupButton, m_JoinGroupButton, m_GroupSettingsButton, m_ButtonLayout, m_ParentMenu);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuHide(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuHide(parentMenu);
	
		SCR_GroupTileButton.GetOnPlayerTileFocus().Remove(OnPlayerTileFocus);
		SCR_GroupTileButton.GetOnPlayerTileFocusLost().Remove(OnPlayerTileFocusLost);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuClose(parentMenu);
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (m_GroupManager)
		{
			m_GroupManager.GetOnPlayableGroupRemoved().Remove(UpdateGroupsMenu);
			m_GroupManager.GetOnPlayableGroupCreated().Remove(UpdateGroupsMenu);
		}
		
		SCR_AIGroup.GetOnPlayerAdded().Remove(UpdateGroupsMenu);
		SCR_AIGroup.GetOnPlayerRemoved().Remove(UpdateGroupsMenu);
		SCR_AIGroup.GetOnPlayerLeaderChanged().Remove(UpdateGroupsMenu);
		SCR_AIGroup.GetOnCustomNameChanged().Remove(UpdateGroupsMenu);
		SCR_AIGroup.GetOnFlagSelected().Remove(UpdateGroupsMenu);
		SCR_AIGroup.GetOnCustomDescriptionChanged().Remove(UpdateGroupsMenu);
		m_PlayerGroupController.GetOnInviteReceived().Remove(SetAcceptButtonStatus);
		
		//todo:mku this is a temporary solution because of how playerlist is implemented right now
		OverlayWidget header = OverlayWidget.Cast(m_ParentMenu.GetRootWidget().FindAnyWidget("SortHeader"));
		if (header)
			header.SetVisible(true);
		ScrollLayoutWidget scrollWidget = ScrollLayoutWidget.Cast(m_ParentMenu.GetRootWidget().FindAnyWidget("ScrollLayout0"));
		if (scrollWidget)
			scrollWidget.SetVisible(true);
		HorizontalLayoutWidget footerLeft = HorizontalLayoutWidget.Cast(m_ParentMenu.GetRootWidget().FindAnyWidget("FooterLeft"));
		if (footerLeft)
			footerLeft.SetVisible(true);
		
		SCR_GroupTileButton.GetOnPlayerTileFocus().Remove(OnPlayerTileFocus);
		SCR_GroupTileButton.GetOnPlayerTileFocusLost().Remove(OnPlayerTileFocusLost);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateNewGroup()
	{
		if (!m_PlayerGroupController)
			return;
		//we reset the selected group so the menu goes to players actual group, in this case newly created one
		m_PlayerGroupController.SetSelectedGroupID(-1);
		m_PlayerGroupController.RequestCreateGroup();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void JoinSelectedGroup()
	{
		if (!m_PlayerGroupController)
			return;	
		m_PlayerGroupController.RequestJoinGroup(m_PlayerGroupController.GetSelectedGroupID());
	}
	
	//------------------------------------------------------------------------------------------------
	void AcceptInvite()
	{
		m_PlayerGroupController.AcceptInvite();
		SetAcceptButtonStatus();
	}
	
	//------------------------------------------------------------------------------------------------
	void SetAcceptButtonStatus()
	{
		if (!m_AcceptInviteButton)
			return;
		
		if (m_PlayerGroupController.GetGroupInviteID() == -1)
		{
			m_AcceptInviteButton.SetEnabled(false);
		}
		else
		{
			SCR_AIGroup group = m_GroupManager.FindGroup(m_PlayerGroupController.GetGroupInviteID());
			
			if (!group)
			{
				m_AcceptInviteButton.SetEnabled(false);
				m_PlayerGroupController.SetGroupInviteID(-1);
				return;	
			}		
			m_AcceptInviteButton.SetEnabled(true);	
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
		m_PlayerGroupController.GetOnInviteReceived().Insert(SetAcceptButtonStatus);
		SetAcceptButtonStatus();
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
	protected void CreateViewProfileButton()
	{
		m_ViewProfileButton = CreateNavigationButton("MenuViewProfile", "", true);
		if (!m_ViewProfileButton)
			return;
		
		// Dynamically add the component to update the button label dpending on platform. TODO: allow sub menus to create different layouts of buttons
		SCR_ViewProfileButtonComponent handler = new SCR_ViewProfileButtonComponent();
		if (!handler)
			return;

		m_ViewProfileButton.GetRootWidget().AddHandler(handler);
		handler.Init();
		
		UpdateViewProfileButton(true);
		
		m_ViewProfileButton.GetRootWidget().SetZOrder(0);
		m_ViewProfileButton.m_OnActivated.Insert(OnViewProfile);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateViewProfileButton(bool forceHidden = false)
	{
		if (!m_ViewProfileButton)
			return;

		m_ViewProfileButton.SetVisible(!forceHidden && GetGame().GetPlayerManager().IsUserProfileAvailable(m_iLastSelectedPlayerId), false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlayerTileFocus(int id)
	{
		m_iLastSelectedPlayerId = id;
		UpdateViewProfileButton();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlayerTileFocusLost(int id)
	{
		UpdateViewProfileButton(true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnViewProfile()
	{
		GetGame().GetPlayerManager().ShowUserProfile(m_iLastSelectedPlayerId);
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
		SCR_AIGroup group = m_GroupManager.FindGroup(m_PlayerGroupController.GetGroupID());
		if (!group)
			return;
		
		m_PlayerGroupController.RequestPrivateGroupChange(m_PlayerGroupController.GetPlayerID() , !group.IsPrivate());
	}
};
