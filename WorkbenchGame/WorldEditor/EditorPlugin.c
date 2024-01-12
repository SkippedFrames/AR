[WorkbenchPluginAttribute(name: "Game Master Settings", icon: "WBData/EntityEditorProps/entityEditor.png", wbModules: {"WorldEditor"})]
class EditorPlugin : WorldEditorPlugin
{
	[Attribute(desc: "When enabled, Game Master camera will start on the position of World Editor camera.", category: "Game Master")]
	protected bool m_bStartOnWorldEditorCamera;
	
	[Attribute(desc: "When enabled, Game Master interface will start unpaused.", category: "Game Master")]
	protected bool m_bStartUnpaused;
	
	protected void Unpause()
	{
		SCR_EditorManagerEntity.GetInstance().GetOnOpened().Remove(Unpause);
		ChimeraWorld world = GetGame().GetWorld();
		world.PauseGameTime(false);
	}
	
	override void OnGameModeStarted(string worldName, string gameMode, bool playFromCameraPos, vector cameraPosition, vector cameraAngles)
	{
		//--- Player is spawned at camera position; initialize, but don't open the editor
		//--- Disabled, was showing error and editor was initialized either way
		if (playFromCameraPos)
		{
#ifdef WORKBENCH
			SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
			if (core)
				core.OnPlayFromCameraPos();
#endif
		}
		
		if (m_bStartOnWorldEditorCamera)
		{
			SCR_CameraEditorComponent cameraComponent = SCR_CameraEditorComponent.Cast(SCR_CameraEditorComponent.GetInstance(SCR_CameraEditorComponent));
			if (cameraComponent)
			{
				//--- Set initial camera position and rotation
				vector transform[4];
				Math3D.AnglesToMatrix(Vector(cameraAngles[1], cameraAngles[0], cameraAngles[2]), transform);
				transform[3] = cameraPosition;
				
				cameraComponent.SetInitTransform(transform);
			}
		}
		
		if (m_bStartUnpaused)
		{
			SCR_PauseGameTimeEditorComponent pauseComponent = SCR_PauseGameTimeEditorComponent.Cast(SCR_PauseGameTimeEditorComponent.GetInstance(SCR_PauseGameTimeEditorComponent));
			if (pauseComponent)
			{
				pauseComponent.SetPauseOnOpen(false);
				
				SCR_EditorManagerEntity editorManager = pauseComponent.GetManager();
				if (editorManager.IsOpened())
				{
					//--- Editor already opened, unpause the game
					Unpause();
				}
				else if (editorManager.IsInTransition())
				{
					//--- Editor is being opened (e.g., when playing from character in editor world), unpause the game once it's opened
					editorManager.GetOnOpened().Insert(Unpause);
				}
			}
		}
	}
	
	//--- Play the editor directly
	//--- Example: -plugin=EditorPlugin
	override void RunCommandline() 
	{
		Workbench.OpenModule(WorldEditor);
		WorldEditor we = Workbench.GetModule(WorldEditor);
		if (!we) return;
		
		we.SetOpenedResource("{25E6D4AEC3F45872}worlds/Editor/Test/TestGameMaster.ent");
		we.SwitchToGameMode();
	}
	
	override void Configure()
	{
		Workbench.ScriptDialog("Game Master Settings", "Configuration of Game Master when running in World Editor.", this);
	}
	
	[ButtonAttribute("Close")]
	void ButtonClose();
};
