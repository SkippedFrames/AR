//------------------------------------------------------------------------------------------------
class SCR_DebriefingScreenMenu : SCR_WelcomeScreenMenu
{
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		m_GameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		m_PlayerManager = PlayerManager.Cast(GetGame().GetPlayerManager());
		m_MapEntity = SCR_MapEntity.GetMapInstance();

		SCR_DebriefingScreenComponent debriefingScreen = SCR_DebriefingScreenComponent.Cast(m_GameMode.FindComponent(SCR_DebriefingScreenComponent));
		if (!debriefingScreen)
		{
			Close();
			return;
		}

		RichTextWidget scenarioTitle = RichTextWidget.Cast(GetRootWidget().FindAnyWidget("DeploySetup"));
		if (scenarioTitle)
			//scenarioTitle.SetText(debriefingScreen.GetHeaderTitle());
			scenarioTitle.SetText("#AR-DebriefingScreen_Debriefing");

		/*RichTextWidget scenarioSubtitle = RichTextWidget.Cast(GetRootWidget().FindAnyWidget("HeaderSubtitle"));
		if (scenarioSubtitle)
			scenarioSubtitle.SetText(debriefingScreen.GetHeaderSubtitle());*/

		SCR_MissionHeader header = SCR_MissionHeader.Cast(GetGame().GetMissionHeader());
		if (header)
			m_iMaxPlayerCount = header.m_iPlayerCount;

		m_wPlayerCount = RichTextWidget.Cast(GetRootWidget().FindAnyWidget("PlayerCount"));
		UpdatePlayerCount(0);

		m_wScenarioTimeElapsed = RichTextWidget.Cast(GetRootWidget().FindAnyWidget("TimeElapsed"));

		Widget continueBtn = GetRootWidget().FindAnyWidget("CloseButton");
		if (continueBtn)
		{
			SCR_InputButtonComponent continueButton = SCR_InputButtonComponent.Cast(continueBtn.FindHandler(SCR_InputButtonComponent));
			if (continueButton)
				continueButton.m_OnActivated.Insert(ReturnToMenu);
			
			RichTextWidget continueBtnBackToMenu = RichTextWidget.Cast(continueBtn.FindAnyWidget("Text"));
			if (continueBtnBackToMenu)
				continueBtnBackToMenu.SetText("#AR-PauseMenu_Exit");
		}
		
		Widget pauseMenuBtn = GetRootWidget().FindAnyWidget("PauseButton");
		if (pauseMenuBtn)
		{
			SCR_InputButtonComponent pauseMenuButton = SCR_InputButtonComponent.Cast(pauseMenuBtn.FindHandler(SCR_InputButtonComponent));
			if (pauseMenuButton)
				pauseMenuButton.m_OnActivated.Insert(OpenPauseMenu);
		}
		
		Widget chat = GetRootWidget().FindAnyWidget("ChatPanel");
		if (chat)
			m_ChatPanel = SCR_ChatPanel.Cast(chat.FindHandler(SCR_ChatPanel));

		m_ChatButton = SCR_InputButtonComponent.GetInputButtonComponent("ChatButton", GetRootWidget());
		if (m_ChatButton)
			m_ChatButton.m_OnActivated.Insert(OnChatToggle);

		UpdateElapsedTime();
		SCR_DeployMenuBaseScreenLayout baseLayout = debriefingScreen.GetBaseLayout();
		if (!baseLayout)
			return;

		baseLayout.InitContent(this);

		//InitMap();
	}
	
	//------------------------------------------------------------------------------------------------
	void ReturnToMenu()
	{
		SCR_ConfigurableDialogUi dlg = SCR_CommonDialogs.CreateDialog("scenario_exit");
		if (!dlg)
			return;
		
		dlg.m_OnConfirm.Insert(BackToMainMenuPopupConfirm);
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void BackToMainMenuPopupConfirm()
	{
		GetRootWidget().RemoveFromHierarchy();
		GameStateTransitions.RequestGameplayEndTransition();
	}
	
	//------------------------------------------------------------------------------------------------
	void OpenDebriefingScreenMenu()
	{
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.DebriefingScreenMenu);
	}

	//------------------------------------------------------------------------------------------------
	void CloseDebriefingScreenMenu()
	{
		GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.DebriefingScreenMenu);
	}
};
