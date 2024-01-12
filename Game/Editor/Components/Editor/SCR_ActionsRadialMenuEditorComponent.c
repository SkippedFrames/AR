/*!
\addtogroup EditorRadialMenu
\{
*/

[ComponentEditorProps(category: "GameScripted/Editor", description: "Responsible for controlling radial menus used within GameMaster", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_ActionsRadialMenuEditorComponentClass : SCR_BaseEditorComponentClass
{
};

/*!
Responsible for controlling and managing radial menus used within GameMaster.
*/
class SCR_ActionsRadialMenuEditorComponent : SCR_BaseEditorComponent
{
	[Attribute()]
	protected ref SCR_RadialMenuController m_ActionsMenuController;

	[Attribute()]
	protected ref SCR_RadialMenuController m_CommandsMenuController;

	[Attribute()]
	protected ref SCR_RadialMenuController m_AddCommandsMenuController;

	protected SCR_MenuLayoutEditorComponent m_EditorMenuComponent;
	protected ref array<ref SCR_EditorActionSelectionMenuEntry> m_aMenuEntries = {};

	[Attribute()]
	protected string m_sActionName;

	[Attribute()]
	protected string m_sCommandActionName;

	[Attribute()]
	protected string m_sAddCommandActionName;

	//------------------------------------------------------------------------------------------------
	void OnInputDeviceChanged(EInputDeviceType oldDevice, EInputDeviceType newDevice)
	{	
		if (newDevice != EInputDeviceType.GAMEPAD)
		{
			CleanMenu();

			if (m_ActionsMenuController)
			{
				m_ActionsMenuController.SetEnableControl(false);
				m_ActionsMenuController.StopControl();
			}
			
			if (m_CommandsMenuController)
			{
				m_CommandsMenuController.SetEnableControl(false);
				m_CommandsMenuController.StopControl();
			}

			if (m_AddCommandsMenuController)
			{
				m_AddCommandsMenuController.SetEnableControl(false);
				m_AddCommandsMenuController.StopControl();
			}
		}
		else
		{
			if (m_ActionsMenuController)
				m_ActionsMenuController.SetEnableControl(true);
		
			if (m_CommandsMenuController)
				m_CommandsMenuController.SetEnableControl(true);

			if (m_AddCommandsMenuController)
				m_AddCommandsMenuController.SetEnableControl(true);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBeforeMenuOpen(notnull SCR_RadialMenuController menuController, notnull SCR_BaseActionsEditorComponent actionsComponent, int flags = 0)
	{
		CleanMenu();

		if (GetGame().GetInputManager().GetLastUsedInputDevice() != EInputDeviceType.GAMEPAD)
			return;

		SCR_MenuLayoutEditorComponent editorMenuLayout = SCR_MenuLayoutEditorComponent.Cast(SCR_MenuLayoutEditorComponent.GetInstance(SCR_MenuLayoutEditorComponent, true));
		if (!editorMenuLayout)
			return;

		SCR_RadialMenu radialMenu = menuController.GetRadialMenu();
		if (!radialMenu)
			return;

		vector cursorWorldPosition;
		if (!editorMenuLayout.GetCursorWorldPos(cursorWorldPosition))
			return;

		array<ref SCR_EditorActionData> filteredActions = {};
		actionsComponent.GetAndEvaluateActions(cursorWorldPosition, filteredActions, flags);

		// Close menu if there is nothing to display.
		if (filteredActions.IsEmpty())
		{
			CleanMenu();
			return;
		}

		array<ref SCR_SelectionMenuEntry> radialMenuEntries = {};
		foreach (SCR_EditorActionData actionData : filteredActions)
		{
			SCR_BaseEditorAction action = actionData.GetAction();
			if (!action)
				continue;

			SCR_UIInfo info = action.GetInfo();
			if (!info)
				continue;

			SCR_EditorActionSelectionMenuEntry menuEntry = new SCR_EditorActionSelectionMenuEntry(action, actionsComponent, cursorWorldPosition, flags);

			menuEntry.Enable(true);
			menuEntry.SetName(info.GetName());
			menuEntry.SetIcon(info.GetIconPath());
			menuEntry.SetDescription(info.GetDescription());
			menuEntry.SetInputAction(action.GetShortcut());

			radialMenuEntries.Insert(menuEntry);
		}

		radialMenu.GetOnClose().Insert(CleanMenu);
		radialMenu.AddEntries(radialMenuEntries, true);
			
		menuController.OnInputOpen();
	}

	//------------------------------------------------------------------------------------------------
	protected void CleanMenu()
	{
		m_aMenuEntries.Clear();
		SCR_RadialMenu radialMenu = null;

		if (m_ActionsMenuController)
			radialMenu = m_ActionsMenuController.GetRadialMenu();

		if (radialMenu)
		{
			radialMenu.ClearEntries();
			radialMenu = null;
		}

		if (m_CommandsMenuController)
			radialMenu = m_CommandsMenuController.GetRadialMenu();

		if (radialMenu)
		{
			radialMenu.ClearEntries();
			radialMenu = null;
		}

		if (m_AddCommandsMenuController)
			radialMenu = m_AddCommandsMenuController.GetRadialMenu();

		if (radialMenu)
		{
			radialMenu.ClearEntries();
			radialMenu = null;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnActionRadialMenuOpen()
	{
		if (!m_ActionsMenuController)
			return;

		m_ActionsMenuController.Control(GetOwner());

		SCR_ContextActionsEditorComponent editorActionsComponent = SCR_ContextActionsEditorComponent.Cast(SCR_ContextActionsEditorComponent.GetInstance(SCR_ContextActionsEditorComponent, true));
		if (!editorActionsComponent)
			return;

		OnBeforeMenuOpen(m_ActionsMenuController, editorActionsComponent);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCommandsRadialMenu()
	{
		if (!m_CommandsMenuController)
			return;
		
		m_CommandsMenuController.Control(GetOwner());
			
		SCR_CommandActionsEditorComponent editorCommandsComponent = SCR_CommandActionsEditorComponent.Cast(SCR_CommandActionsEditorComponent.GetInstance(SCR_CommandActionsEditorComponent, true));
		if (!editorCommandsComponent)
			return;

		OnBeforeMenuOpen(m_CommandsMenuController, editorCommandsComponent, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAddCommandsRadialMenu()
	{
		if (!m_AddCommandsMenuController)
			return;

		m_AddCommandsMenuController.Control(GetOwner());

		SCR_CommandActionsEditorComponent editorCommandsComponent = SCR_CommandActionsEditorComponent.Cast(SCR_CommandActionsEditorComponent.GetInstance(SCR_CommandActionsEditorComponent, true));
		if (!editorCommandsComponent)
			return;

		OnBeforeMenuOpen(m_AddCommandsMenuController, editorCommandsComponent, EEditorCommandActionFlags.IS_QUEUE);
	}

	//------------------------------------------------------------------------------------------------
	protected override void EOnEditorInit()
	{
#ifdef WORKBENCH
		if (SCR_Global.IsEditMode())
			return;
#endif
		GetGame().OnInputDeviceUserChangedInvoker().Insert(OnInputDeviceChanged);

		GetGame().GetInputManager().AddActionListener(m_sActionName, EActionTrigger.DOWN, OnActionRadialMenuOpen);
		GetGame().GetInputManager().AddActionListener(m_sCommandActionName, EActionTrigger.DOWN, OnCommandsRadialMenu);
		GetGame().GetInputManager().AddActionListener(m_sAddCommandActionName, EActionTrigger.DOWN, OnAddCommandsRadialMenu);
	}

	//------------------------------------------------------------------------------------------------
	protected override void EOnEditorDelete()
	{
#ifdef WORKBENCH
		if (SCR_Global.IsEditMode())
			return;
#endif

		if (m_ActionsMenuController)
			m_ActionsMenuController.GetOnInputOpen().Remove(OnActionRadialMenuOpen);

		if (m_CommandsMenuController)
			m_CommandsMenuController.GetOnInputOpen().Remove(OnCommandsRadialMenu);

		if (m_AddCommandsMenuController)
			m_AddCommandsMenuController.GetOnInputOpen().Remove(OnAddCommandsRadialMenu);
	}
};
/*!
\}
*/
