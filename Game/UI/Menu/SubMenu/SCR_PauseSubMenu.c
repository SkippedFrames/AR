//------------------------------------------------------------------------------------------------
class SCR_PauseSubMenu: SCR_SubMenuBase
{
	InputManager m_InputManager;
	
	protected Widget m_wEditorOpen;
	protected Widget m_wEditorClose;
	protected TextWidget m_wVersion;

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_SuperMenuBase parentMenu)
	{
		SCR_ButtonComponent comp;
		super.OnMenuOpen(parentMenu);

		// Continue
		comp = GetButtonComponent("Continue");
		if (comp)
		{
			comp.m_OnClicked.Insert(CloseParent);
			GetGame().GetWorkspace().SetFocusedWidget(comp.GetRootWidget());
		}

		// Restart 
		comp = GetButtonComponent("Restart");
		if (comp)
		{
			bool enabledRestart = !Replication.IsRunning();
			comp.GetRootWidget().SetEnabled(enabledRestart);
			comp.m_OnClicked.Insert(OnRestart);
		}

		// Exit
		comp = GetButtonComponent("Exit");
		if (comp)
			comp.m_OnClicked.Insert(OnExit);

		// Camera
		comp = GetButtonComponent("Camera");
		if (comp)
			comp.m_OnClicked.Insert(OnCamera);
		
		// Settings
		comp = GetButtonComponent("Settings");
		if (comp)
			comp.m_OnClicked.Insert(OnSettings);
		
		// Version
		m_wVersion = TextWidget.Cast(m_wRoot.FindAnyWidget("Version"));
		if (m_wVersion)
			m_wVersion.SetText(GetGame().GetBuildVersion());

		// EditorOpen / EditorClose
		comp = GetButtonComponent("EditorOpen");
		if (comp)
		{
			comp.m_OnClicked.Insert(OnEditorOpen);
			m_wEditorOpen = comp.GetRootWidget();
		}
		
		comp = GetButtonComponent("EditorClose");
		if (comp)
		{
			comp.m_OnClicked.Insert(OnEditorClose);
			m_wEditorClose = comp.GetRootWidget();
		}
		
		comp = GetButtonComponent("Feedback");
		if (comp)
			comp.m_OnClicked.Insert(OnFeedback);

		// Handle navigation buttons
		SCR_InputButtonComponent navigation = CreateNavigationButton("MenuBack", "Back");
		if (navigation)
			navigation.m_OnActivated.Insert(CloseParent);
		
		m_InputManager = GetGame().GetInputManager();
		m_InputManager.AddActionListener("MenuOpen", EActionTrigger.PRESSED, CloseParent);
		#ifdef WORKBENCH
			m_InputManager.AddActionListener("MenuOpenWB", EActionTrigger.PRESSED, CloseParent);
			m_InputManager.AddActionListener("MenuBackWB", EActionTrigger.PRESSED, CloseParent);
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuShow(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuShow(parentMenu);
		
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager && m_wEditorOpen && m_wEditorClose)
		{
			editorManager.GetOnOpened().Insert(CloseParent);
			editorManager.GetOnClosed().Insert(CloseParent);
		
			m_wEditorOpen.SetVisible(!editorManager.IsOpened());
			m_wEditorOpen.SetEnabled(editorManager.CanOpen());
			m_wEditorClose.SetVisible(editorManager.IsOpened());
			m_wEditorClose.SetEnabled(editorManager.CanClose());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuHide(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuHide(parentMenu);
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
		{
			editorManager.GetOnOpened().Insert(CloseParent);
			editorManager.GetOnClosed().Insert(CloseParent);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuClose(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuClose(parentMenu);
	}
	
	//------------------------------------------------------------------------------------------------
	private void OnSettings()
	{
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.SettingsSuperMenu);
	}
	
	//------------------------------------------------------------------------------------------------
	private void OnExit()
	{
		GameStateTransitions.RequestGameplayEndTransition();
		CloseParent();
	}
	
	//------------------------------------------------------------------------------------------------
	private void OnEditorOpen()
	{
		SCR_EditorManagerEntity.OpenInstance();
	}

	//------------------------------------------------------------------------------------------------
	private void OnEditorClose()
	{
		SCR_EditorManagerEntity.CloseInstance();
	}
	
	//------------------------------------------------------------------------------------------------
	private void OnFeedback()
	{
		FeedbackDialogUI.OpenFeedbackDialog();
	}
	
	//------------------------------------------------------------------------------------------------
	private void OnRestart()
	{
		GetGame().GetMenuManager().CloseAllMenus();
		GameStateTransitions.RequestScenarioRestart();
	}
	
	//------------------------------------------------------------------------------------------------
	private void OnCamera()
	{
		SCR_DebugCameraCore cameraCore = SCR_DebugCameraCore.Cast(SCR_DebugCameraCore.GetInstance(SCR_DebugCameraCore));
		if (cameraCore) 
			cameraCore.CreateCamera();
		CloseParent();
	}
};
