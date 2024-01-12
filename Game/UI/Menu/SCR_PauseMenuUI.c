//------------------------------------------------------------------------------------------------
class PauseMenuUI: ChimeraMenuBase
{
	InputManager m_InputManager;

	protected TextWidget m_wVersion;
	protected Widget m_wRoot;
	protected Widget m_wFade;
	protected Widget m_wSystemTime;
	protected bool m_bFocused = true;

	//Editor and Photo mode Specific
	protected SCR_ButtonTextComponent m_EditorUnlimitedOpenButton;
	protected SCR_ButtonTextComponent m_EditorUnlimitedCloseButton;
	protected SCR_ButtonTextComponent m_EditorPhotoOpenButton;
	protected SCR_ButtonTextComponent m_EditorPhotoCloseButton;
	
	protected SCR_SaveLoadComponent m_SavingComponent;
	
	const string EXIT_SAVE = "#AR-PauseMenu_ReturnSaveTitle";
	const string EXIT_NO_SAVE = "#AR-PauseMenu_ReturnTitle";
	
	const ResourceName ACTIONS_IMAGESET = "{2EFEA2AF1F38E7F0}UI/Textures/Icons/icons_wrapperUI-64.imageset";
	const string EXIT_MESSAGE = "#AR-PauseMenu_ReturnText";
	const string EXIT_TITLE = "#AR-PauseMenu_ReturnTitle";
	const string EXIT_IMAGE = "exit";
	
	const string RESTART_MESSAGE = "#AR-PauseMenu_RestartText";
	const string RESTART_TITLE = "#AR-PauseMenu_Restart";
	const string RESTART_IMAGE = "restart";

	const string LOAD_MESSAGE = "#AR-PauseMenu_LoadText";
	const string LOAD_TITLE = "#AR-PauseMenu_Load";
	const string LOAD_IMAGE = "up";
		
	static ref ScriptInvoker m_OnPauseMenuOpened = new ScriptInvoker();
	static ref ScriptInvoker m_OnPauseMenuClosed = new ScriptInvoker();
	
	protected static PauseMenuUI s_Instance;

	//------------------------------------------------------------------------------------------------
	// If the pause menu was open, reinitialize it so it sits on top
	static void MoveToTop()
	{
		if (!s_Instance)
			return;
		
		s_Instance.Close();
		
		// Must be called the next frame for the menu to be reopened immediately after closing
		GetGame().GetCallqueue().CallLater(OpenMenuOnTop);
		
		// TODO: tried setting the ZOrder but it does not work: ask for Enfusion API
	}
	
	//------------------------------------------------------------------------------------------------
	// Opens the pause menu with settings for it to be on top of other menus
	static void OpenMenuOnTop()
	{
		GetGame().OpenPauseMenu(false, true);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		s_Instance = this;
		
		m_SavingComponent = SCR_SaveLoadComponent.GetInstance();

		m_wRoot = GetRootWidget();
		m_wFade = m_wRoot.FindAnyWidget("BackgroundFade");
		m_wSystemTime = m_wRoot.FindAnyWidget("SystemTime");
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		SCR_ButtonTextComponent comp;

		// Continue
		comp = SCR_ButtonTextComponent.GetButtonText("Continue", m_wRoot);
		if (comp)
		{
			GetGame().GetWorkspace().SetFocusedWidget(comp.GetRootWidget());
			comp.m_OnClicked.Insert(Close);
		}

		// Restart
		comp = SCR_ButtonTextComponent.GetButtonText("Restart", m_wRoot);
		if (comp)
		{
			bool enabledRestart = !Replication.IsRunning();
			comp.GetRootWidget().SetVisible(enabledRestart);
			comp.m_OnClicked.Insert(OnRestart);
		}

		// Respawn
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.Cast(GetGame().GetGameMode());
		
		comp = SCR_ButtonTextComponent.GetButtonText("Respawn", m_wRoot);
		if (comp)
		{
			if (campaign && campaign.IsTutorial())
			{
				comp.SetVisible(false)
			}
			else
			{
				bool canRespawn;
				BaseGameMode gameMode = GetGame().GetGameMode();
				if (gameMode)
				{
					RespawnSystemComponent respawn = RespawnSystemComponent.Cast(gameMode.FindComponent(RespawnSystemComponent));
					canRespawn = (respawn != null);
				}

				comp.GetRootWidget().SetVisible(canRespawn);
				comp.m_OnClicked.Insert(OnRespawn);
			}
		}
		
		// Leave faction
		comp = SCR_ButtonTextComponent.GetButtonText("LeaveFaction", m_wRoot);
		if (comp)
		{
			bool factionLeaveAllowed;
			SCR_BaseGameMode gm = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
			if (gm)
				factionLeaveAllowed = gm.IsFactionChangeAllowed();

			SCR_FactionManager fm = SCR_FactionManager.Cast(GetGame().GetFactionManager());
			if (fm)
			{
				array<Faction> factions = {};
				fm.GetFactionsList(factions);
				int playableFactionCount = 0;
				foreach (Faction f : factions)
				{
					SCR_Faction scriptedFaction = SCR_Faction.Cast(f);
					if (scriptedFaction && scriptedFaction.IsPlayable())
						playableFactionCount++;
				}

				factionLeaveAllowed = factionLeaveAllowed && (playableFactionCount > 1);
			}

			comp.GetRootWidget().SetVisible(factionLeaveAllowed);
			if (factionLeaveAllowed)
			{			
				PlayerController pc = GetGame().GetPlayerController();
				SCR_PlayerFactionAffiliationComponent factionComp = SCR_PlayerFactionAffiliationComponent.Cast(pc.FindComponent(SCR_PlayerFactionAffiliationComponent));
				
				if (factionComp)
					comp.SetEnabled(factionComp.GetAffiliatedFaction() != null);
			
				comp.m_OnClicked.Insert(OnLeaveFaction)
			}
		}

		// Exit
		comp = SCR_ButtonTextComponent.GetButtonText("Exit", m_wRoot);
		if (comp)
		{
			comp.m_OnClicked.Insert(OnExit);
			if (IsSavingOnExit())
				comp.SetText(EXIT_SAVE);
			else
				comp.SetText(EXIT_NO_SAVE);
		}

		// Rewind
		comp = SCR_ButtonTextComponent.GetButtonText("Rewind", m_wRoot);
		if (comp)
		{
			SCR_RewindComponent rewindManager = SCR_RewindComponent.GetInstance();
			comp.GetRootWidget().SetVisible(rewindManager != null); //--- Hide the button when rewinding is not configured for the mission
			comp.GetRootWidget().SetEnabled(rewindManager && rewindManager.HasRewindPoint()); //--- Disable the button when the rewind point does not exist
			
			comp.m_OnClicked.Insert(OnRewind);
		}
		
		// Tutorial HUB
		comp = SCR_ButtonTextComponent.GetButtonText("ReturnHUB", m_wRoot);
		
		if (comp)
			if (!campaign)
				comp.SetVisible(false);
			else 
				if (campaign.IsTutorial())
				{
					comp.SetVisible(true);
					comp.m_OnClicked.Insert(OnReturnToHub);
				}
				else
					comp.SetVisible(false);

		// Camera
		comp = SCR_ButtonTextComponent.GetButtonText("Camera", m_wRoot);
		if (comp)
		{
			comp.m_OnClicked.Insert(OnCamera);
			comp.SetEnabled(editorManager && !editorManager.IsOpened());
			comp.GetRootWidget().SetVisible(Game.IsDev());
		}

		// Settings
		comp = SCR_ButtonTextComponent.GetButtonText("Settings", m_wRoot);
		if (comp)
			comp.m_OnClicked.Insert(OnSettings);

		// Field Manual
		comp = SCR_ButtonTextComponent.GetButtonText("FieldManual", m_wRoot);
		if (comp)
		{
			comp.m_OnClicked.Insert(OnFieldManual);
		}

		// Players
		comp = SCR_ButtonTextComponent.GetButtonText("Players", m_wRoot);
		if (comp)
		{
			comp.GetRootWidget().SetVisible(Replication.IsRunning());
			comp.m_OnClicked.Insert(OnPlayers);
		}

		// Invite friends
		comp = SCR_ButtonTextComponent.GetButtonText("InviteFriend", m_wRoot);
		if (comp)
		{
			bool canInvite = Replication.IsRunning() && GetGame().GetPlayerManager().IsMultiplayerActivityInviteAvailable();
			comp.GetRootWidget().SetVisible(canInvite);
			
			if (canInvite)
				comp.m_OnClicked.Insert(OnInviteFriends);
		}
		
		// Version
		m_wVersion = TextWidget.Cast(m_wRoot.FindAnyWidget("Version"));
		if (m_wVersion)
			m_wVersion.SetText(GetGame().GetBuildVersion());

		// Unlimited editor (Game Master)
		m_EditorUnlimitedOpenButton = SCR_ButtonTextComponent.GetButtonText("EditorUnlimitedOpen",m_wRoot);
		if (m_EditorUnlimitedOpenButton)		
			m_EditorUnlimitedOpenButton.m_OnClicked.Insert(OnEditorUnlimited);

		m_EditorUnlimitedCloseButton = SCR_ButtonTextComponent.GetButtonText("EditorUnlimitedClose",m_wRoot);
		if (m_EditorUnlimitedCloseButton)		
			m_EditorUnlimitedCloseButton.m_OnClicked.Insert(OnEditorUnlimited);
		
		//--- Photo mode
		m_EditorPhotoOpenButton = SCR_ButtonTextComponent.GetButtonText("EditorPhotoOpen",m_wRoot);
		if (m_EditorPhotoOpenButton)
			m_EditorPhotoOpenButton.m_OnClicked.Insert(OnEditorPhoto);
		m_EditorPhotoCloseButton = SCR_ButtonTextComponent.GetButtonText("EditorPhotoClose",m_wRoot);
		if (m_EditorPhotoCloseButton)
			m_EditorPhotoCloseButton.m_OnClicked.Insert(OnEditorPhoto);
		
		SetEditorUnlimitedButton(editorManager);
		SetEditorPhotoButton(editorManager);
		
		if (editorManager)
		{
			editorManager.GetOnModeAdd().Insert(OnEditorModeChanged);
			editorManager.GetOnModeRemove().Insert(OnEditorModeChanged);
		}

		comp = SCR_ButtonTextComponent.GetButtonText("Feedback", m_wRoot);
		if (comp)
		{
			comp.m_OnClicked.Insert(OnFeedback);
		}

		m_InputManager = GetGame().GetInputManager();
		
		m_OnPauseMenuOpened.Invoke();
		
		SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_FE_HUD_PAUSE_MENU_OPEN);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuShow()
	{
		//--- Close pause menu when editor is opened or closed
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
		{
			editorManager.GetOnOpened().Insert(Close);
			editorManager.GetOnClosed().Insert(Close);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuHide()
	{
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
		{
			editorManager.GetOnOpened().Insert(Close);
			editorManager.GetOnClosed().Insert(Close);
		}
		
		if (m_wFade)
			m_wFade.SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		s_Instance = null;
		
		SCR_HUDManagerComponent hud = GetGame().GetHUDManager();
		if (hud)
			hud.SetVisible(true);
		
		m_OnPauseMenuClosed.Invoke();
		
		SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_FE_HUD_PAUSE_MENU_CLOSE);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusLost()
	{
		m_bFocused = false;
		m_InputManager.RemoveActionListener("MenuOpen", EActionTrigger.DOWN, Close);
		m_InputManager.RemoveActionListener("MenuBack", EActionTrigger.DOWN, Close);
		#ifdef WORKBENCH
			m_InputManager.RemoveActionListener("MenuOpenWB", EActionTrigger.DOWN, Close);
			m_InputManager.RemoveActionListener("MenuBackWB", EActionTrigger.DOWN, Close);
		#endif
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusGained()
	{
		m_bFocused = true;
		m_InputManager.AddActionListener("MenuOpen", EActionTrigger.DOWN, Close);
		m_InputManager.AddActionListener("MenuBack", EActionTrigger.DOWN, Close);
		#ifdef WORKBENCH
			m_InputManager.AddActionListener("MenuOpenWB", EActionTrigger.DOWN, Close);
			m_InputManager.AddActionListener("MenuBackWB", EActionTrigger.DOWN, Close);
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	void FadeBackground(bool fade, bool animate = true)
	{
		if (!m_wFade)
			return;
		
		m_wFade.SetVisible(fade);
		m_wFade.SetOpacity(0);
		if (fade && animate)
			AnimateWidget.Opacity(m_wFade, 1, UIConstants.FADE_RATE_FAST, true);
	}

	//------------------------------------------------------------------------------------------------
	private void OnSettings()
	{
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.SettingsSuperMenu);
	}

	//------------------------------------------------------------------------------------------------
	private void OnFieldManual()
	{
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.FieldManualDialog);
	}

	//------------------------------------------------------------------------------------------------
	private void OnReturnToHub()
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.Cast(GetGame().GetGameMode());
		
		if (!campaign)
			return;
		
		SCR_CampaignTutorialArlandComponent component = SCR_CampaignTutorialArlandComponent.Cast(campaign.FindComponent(SCR_CampaignTutorialArlandComponent));
		
		component.SetActiveConfig(0);
	}
	
	//------------------------------------------------------------------------------------------------
	private void OnExit()
	{
		// Create exit dialog
		SCR_ConfigurableDialogUi dialog = SCR_CommonDialogs.CreateDialog("scenario_exit");
		if (!dialog)
			return;
		
		dialog.m_OnConfirm.Insert(OnExitConfirm);
		
	}
	
	//------------------------------------------------------------------------------------------------
	private void OnRewind()
	{
		new SCR_RewindDialog(ESaveType.EDITOR, string.Empty);
	}
	
	//------------------------------------------------------------------------------------------------
	private void OnExitConfirm()
	{
		if (IsSavingOnExit())
		{
			//--- Close only after the save file was created
			GetGame().GetSaveManager().GetOnSaved().Insert(OnSaved);
			GetGame().GetSaveManager().Save(ESaveType.AUTO);
		}
		else
		{
			//--- Close instantly
			Close();
			GameStateTransitions.RequestGameplayEndTransition();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSaved(ESaveType type, string fileName)
	{
		GetGame().GetSaveManager().GetOnSaved().Remove(OnSaved);
		Close();
		GameStateTransitions.RequestGameplayEndTransition();
	}

	//------------------------------------------------------------------------------------------------
	private void OnEditorUnlimited()
	{
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
		{
			if (!editorManager.IsOpened() || editorManager.GetCurrentModeEntity().IsLimited())
			{
				editorManager.SetCurrentMode(false);
				editorManager.Open();
			}
			else
			{
				editorManager.Close();
			}
		}
		Close();
	}

	//Update Editor Mode Button text and if Enabled
	//------------------------------------------------------------------------------------------------
	private void SetEditorUnlimitedButton(SCR_EditorManagerEntity editorManager)
	{
		if (!m_EditorUnlimitedOpenButton || !m_EditorUnlimitedCloseButton) return;
		
		if (!editorManager || editorManager.IsLimited())
		{
			//--- LIMITED EDITOR
			
			//--- Show disabled "Open Unlimited Editor" button when editor is *legal* in the mission
			bool isUnlimitedEditorLegal;
			SCR_EditorSettingsEntity settingsEntity = SCR_EditorSettingsEntity.GetInstance();
			if (settingsEntity)
				isUnlimitedEditorLegal = settingsEntity.IsUnlimitedEditorLegal();
			
			m_EditorUnlimitedOpenButton.GetRootWidget().SetVisible(isUnlimitedEditorLegal);
			m_EditorUnlimitedOpenButton.SetEnabled(false);
			
			//--- Don't show the "Close Unlimited Editor" button
			m_EditorUnlimitedCloseButton.GetRootWidget().SetVisible(false);
			
			//--- Don't show the real-time clock
			m_wSystemTime.SetVisible(false);
		}
		else
		{
			//--- UNLIMITED EDITOR
			if (!editorManager.IsOpened() || editorManager.GetCurrentModeEntity().IsLimited())
			{
				//--- Current mode is limited, show the "Open Unlimited Editor" button
				m_EditorUnlimitedOpenButton.GetRootWidget().SetVisible(true);
				m_EditorUnlimitedCloseButton.GetRootWidget().SetVisible(false);
			}
			else
			{
				//--- Current mode is unlimited, show the "Close Unlimited Editor" button
				m_EditorUnlimitedOpenButton.GetRootWidget().SetVisible(false);
				m_EditorUnlimitedCloseButton.GetRootWidget().SetVisible(true);
				m_EditorUnlimitedCloseButton.SetEnabled(editorManager.CanClose());
			}
			
			//--- Show the real-time clock
			m_wSystemTime.SetVisible(true);
		}
	}

	//Updates Editor and Photomode button if Rights changed
	//------------------------------------------------------------------------------------------------
	protected void OnEditorModeChanged(SCR_EditorModeEntity modeEntity)
	{
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		SetEditorUnlimitedButton(editorManager);
		SetEditorPhotoButton(editorManager);
	}

	//------------------------------------------------------------------------------------------------
	private void OnEditorPhoto()
	{
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
		{
			if (!editorManager.IsOpened() || editorManager.GetCurrentMode() != EEditorMode.PHOTO)
			{
				editorManager.SetCurrentMode(EEditorMode.PHOTO);
				editorManager.Open();
			}
			else
			{
				editorManager.Close();
			}
		}
		Close();
	}

	//Update Photo Mode Button text and if Enabled
	//------------------------------------------------------------------------------------------------
	private void SetEditorPhotoButton(SCR_EditorManagerEntity editorManager)
	{		
		if (!m_EditorPhotoOpenButton || !m_EditorPhotoCloseButton) return;
		
		if (!editorManager || !editorManager.HasMode(EEditorMode.PHOTO))
		{
			m_EditorPhotoOpenButton.GetRootWidget().SetVisible(true);
			m_EditorPhotoOpenButton.SetEnabled(false);
			m_EditorPhotoCloseButton.GetRootWidget().SetVisible(false);
		}
		else
		{			
			if (!editorManager.IsOpened() || editorManager.GetCurrentMode() != EEditorMode.PHOTO)
			{
				m_EditorPhotoOpenButton.GetRootWidget().SetVisible(true);
				m_EditorPhotoCloseButton.GetRootWidget().SetVisible(false);
				
				//Set enabled
				m_EditorPhotoOpenButton.SetEnabled(!editorManager.IsLimited() || GetGame().GetPlayerController().GetControlledEntity());
			}
			else
			{
				m_EditorPhotoOpenButton.GetRootWidget().SetVisible(false);
				m_EditorPhotoCloseButton.GetRootWidget().SetVisible(true);
				m_EditorPhotoCloseButton.SetEnabled(editorManager.CanClose());
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	private void OnFeedback()
	{
		FeedbackDialogUI.OpenFeedbackDialog();
	}

	//------------------------------------------------------------------------------------------------
	private void OnRestart()
	{
		// Create dialog
		SCR_ConfigurableDialogUi dialog = SCR_CommonDialogs.CreateDialog("scenario_restart");
		if (!dialog)
			return;
		
		dialog.m_OnConfirm.Insert(OnRestartConfirm);
	}
	
	//------------------------------------------------------------------------------------------------
	private void OnRestartConfirm()
	{
		GetGame().GetMenuManager().CloseAllMenus();
		GameStateTransitions.RequestScenarioRestart();
	}

	//------------------------------------------------------------------------------------------------
	private void OnPlayers()
	{
		ArmaReforgerScripted.OpenPlayerList();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInviteFriends()
	{
		GetGame().GetPlayerManager().ShowMultiplayerActivityInvite();
	}
	
	//------------------------------------------------------------------------------------------------
	private void OnRespawn()
	{
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;

		SCR_RespawnComponent respawn = SCR_RespawnComponent.Cast(playerController.FindComponent(SCR_RespawnComponent));
		if (!respawn)
			return;

		respawn.RequestPlayerSuicide();
		Close();
	}

	//------------------------------------------------------------------------------------------------	
	private void OnLeaveFaction()
	{
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return;
		
		SCR_PlayerFactionAffiliationComponent factionComp = SCR_PlayerFactionAffiliationComponent.Cast(pc.FindComponent(SCR_PlayerFactionAffiliationComponent));
		if (!factionComp)
			return;

		SCR_RespawnComponent rc = SCR_RespawnComponent.Cast(pc.FindComponent(SCR_RespawnComponent));
		if (!rc)
			return;

		factionComp.RequestFaction(null);
		rc.RequestPlayerSuicide();

		Close();
	}

	//------------------------------------------------------------------------------------------------
	private void OnCamera()
	{
		SCR_DebugCameraCore cameraCore = SCR_DebugCameraCore.Cast(SCR_DebugCameraCore.GetInstance(SCR_DebugCameraCore));
		if (cameraCore)
			cameraCore.ToggleCamera();
		Close();
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);

		//Remove Editor modes listener
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		
		if (editorManager)
		{
			editorManager.GetOnModeAdd().Remove(OnEditorModeChanged);
			editorManager.GetOnModeRemove().Remove(OnEditorModeChanged);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsSavingOnExit()
	{
		return !Replication.IsRunning() && GetGame().GetSaveManager().CanSave(ESaveType.AUTO) && m_SavingComponent && m_SavingComponent.CanSaveOnExit();
	}
};



