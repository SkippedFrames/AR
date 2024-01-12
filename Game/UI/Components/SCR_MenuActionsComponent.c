/*!
Handles activating action listeners when the menu is focused. Requires the menu to be handled by a child of MenuRootBase, and the actions to be in an active context
*/
void ScriptInvokerActionMethod(string name, float multiplier);
typedef func ScriptInvokerActionMethod;
typedef ScriptInvokerBase<ScriptInvokerActionMethod> ScriptInvokerAction;

//------------------------------------------------------------------------------------------------
class SCR_MenuActionsComponent : MenuRootSubComponent
{
	[Attribute(desc: "Actions to be shown in the Tooltip")]
	protected ref array<ref SCR_MenuActionPreset> m_aActions;

	[Attribute("400", desc: "Action listeners activation delay")]
	protected int m_iDelay;

	protected bool m_bHasActionListeners;

	protected ref ScriptInvokerAction m_OnAction;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		AddActionListenersDelayed(m_iDelay);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttachedScripted(Widget w)
	{
		super.HandlerAttachedScripted(w);

		AddActionListenersDelayed(m_iDelay);

		GetMenu().GetOnMenuFocusGained().Insert(OnMenuFocusGained);
		GetMenu().GetOnMenuFocusLost().Insert(OnMenuFocusLost);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);

		RemoveActionListeners();
	}


	//------------------------------------------------------------------------------------------------
	// Owner Menu Events
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	protected void OnMenuFocusGained()
	{
		AddActionListenersDelayed(m_iDelay);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMenuFocusLost()
	{
		RemoveActionListeners();
	}


	//------------------------------------------------------------------------------------------------
	// Events
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	//! Bind this in your menu
	protected void OnAction(float multiplier)
	{
		//! We might be in a hidden tab or menu, in which case we don't want the invoker to trigger
		//! TODO: perhaps remove the action listeners when invisible, using OnUpdate()?
		if (!GetWidget().IsVisible())
			return;

		InputManager inputManager = GetGame().GetInputManager();
		foreach (SCR_MenuActionPreset action : m_aActions)
		{
			if (inputManager.GetActionTriggered(action.m_sActionName) && m_OnAction)
				m_OnAction.Invoke(action.m_sActionName, multiplier);
		}
	}


	//------------------------------------------------------------------------------------------------
	// Action Listeners
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	//! Activate / deactivate in your menu. Tried calling this on focus gained / lost, but if the menu is not focused the component gets deactivated
	//! As of now, if a button press closes a menu and that button's action is active in the menu below, it may get triggered immediatly, thus the delayed activation. This is already being investigated by the Enfusion guys
	protected void AddActionListenersDelayed(int delay)
	{
		if (m_bHasActionListeners)
			return;

		GetGame().GetCallqueue().CallLater(AddActionListeners, delay);
	}

	//------------------------------------------------------------------------------------------------
	protected void AddActionListeners()
	{
		GetGame().GetCallqueue().Remove(AddActionListeners);

		if (m_bHasActionListeners)
			return;

		m_bHasActionListeners = true;

		InputManager inputManager = GetGame().GetInputManager();
		string name;
		EActionTrigger trigger;
		foreach (SCR_MenuActionPreset action : m_aActions)
		{
			name = action.m_sActionName;
			trigger = action.m_eActionTrigger;
			inputManager.AddActionListener(name, trigger, OnAction);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveActionListeners()
	{
		GetGame().GetCallqueue().Remove(AddActionListeners);

		if (!m_bHasActionListeners)
			return;

		m_bHasActionListeners = false;

		InputManager inputManager = GetGame().GetInputManager();
		string name;
		EActionTrigger trigger;
		foreach (SCR_MenuActionPreset action : m_aActions)
		{
			name = action.m_sActionName;
			trigger = action.m_eActionTrigger;
			inputManager.RemoveActionListener(name, trigger, OnAction);
		}
	}

	//------------------------------------------------------------------------------------------------
	void ActivateActions()
	{
		AddActionListenersDelayed(m_iDelay);
	}

	//------------------------------------------------------------------------------------------------
	void DeactivateActions()
	{
		RemoveActionListeners();
	}


	//------------------------------------------------------------------------------------------------
	// Utility
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	array<ref SCR_MenuActionPreset> GetActions()
	{
		return m_aActions;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerAction GetOnAction()
	{
		if (!m_OnAction)
			m_OnAction = new ScriptInvokerAction();

		return m_OnAction;
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_MenuActionsComponent FindComponent(Widget w)
	{
		return SCR_MenuActionsComponent.Cast(w.FindHandler(SCR_MenuActionsComponent));
	}
}

//------------------------------------------------------------------------------------------------
//! Configuration for an action
[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sActionName")]
class SCR_MenuActionPreset
{
	[Attribute()]
	string m_sActionName;

	[Attribute("4", UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EActionTrigger))]
	EActionTrigger m_eActionTrigger;
}

