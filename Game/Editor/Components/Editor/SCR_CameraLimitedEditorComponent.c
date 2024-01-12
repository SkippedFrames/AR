#ifdef WORKBENCH
	#define SHOW_DISCLAIMER
#endif

[ComponentEditorProps(category: "GameScripted/Editor", description: "Limited camera for in-game editor. Works only with SCR_EditorBaseEntity!", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_CameraLimitedEditorComponentClass: SCR_CameraEditorComponentClass
{
};

/** @ingroup Editor_Components
*/
/*!
Limited version of camera manager.

Contains two prefabs - one used only when all editor modes have also limited camera, and the other when there is at least one unlimited camera.
*/
class SCR_CameraLimitedEditorComponent : SCR_CameraEditorComponent
{
	[Attribute("", UIWidgets.ResourceNamePicker, "Prefab of class SCR_ManualCamera", "et", category: "Camera")]
	private ResourceName m_LimitedCameraPrefab;
	
#ifdef SHOW_DISCLAIMER
	private ref DebugTextScreenSpace m_Disclaimer;
	private float m_fDisclaimerCountdown;
#endif
	
	override protected ResourceName GetCameraPrefab()
	{
		SCR_EditorManagerEntity manager = SCR_EditorManagerEntity.GetInstance();
		if (!manager) return ResourceName.Empty;
		
		
		bool isLimited = manager.IsLimited() && !DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_MANUAL_CAMERA_UNLIMITED);
		
#ifdef SHOW_DISCLAIMER
		if (isLimited)
		{
			m_Disclaimer = DebugTextScreenSpace.Create(GetGame().GetWorld(), "", 0, 50, 50, 10, ARGBF(1, 1, 1, 1), ARGBF(1, 0, 0, 0));
			m_fDisclaimerCountdown = 15;
		}
#endif

		if (isLimited)
			return m_LimitedCameraPrefab;
		else
			return super.GetCameraPrefab();
	}
#ifdef SHOW_DISCLAIMER
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);
		
		if (m_Disclaimer)
		{
			m_fDisclaimerCountdown -= timeSlice;
			m_Disclaimer.SetText(string.Format("Armavision camera is restricted around player when no other editor modes are available.\n\nTo unlock it for testing, activate: Debug Menu > Manual Camera > Unlimited Movement\nRestart the editor afterwards.\n\nThis message will disappear in %1 s.", Math.Ceil(m_fDisclaimerCountdown)));
			if (m_fDisclaimerCountdown < 0) delete m_Disclaimer;
		}
	}
#endif
	override protected void EOnEditorActivate()
	{
		super.EOnEditorActivate();
		
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager) editorManager.GetOnLimitedChange().Insert(ReplaceCamera);
		
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_MANUAL_CAMERA_UNLIMITED, "", "Unlimited Movement", "Manual Camera");
	}
	override protected void EOnEditorDeactivate()
	{
		super.EOnEditorDeactivate();
		
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager) editorManager.GetOnLimitedChange().Remove(ReplaceCamera);
		
		//DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_MANUAL_CAMERA_UNLIMITED);
		
#ifdef SHOW_DISCLAIMER
		delete m_Disclaimer;
#endif
	}
};
