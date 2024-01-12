//------------------------------------------------------------------------------------------------
class SCR_TaskPlayerListEntryHandler : SCR_ButtonBaseComponent
{
	protected SCR_BaseTaskExecutor m_Executor;
	protected Widget m_wMainWidget;
	protected bool m_bIsHovered;
	
	//------------------------------------------------------------------------------------------------
	SCR_BaseTaskExecutor GetExecutor()
	{
		return m_Executor;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetExecutor(SCR_BaseTaskExecutor executor)
	{
		m_Executor = executor;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		super.OnMouseLeave(w, enterW, x, y);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		super.OnMouseEnter(w, x, y);

		if (GetGame().GetInputManager().GetLastUsedInputDevice() != EInputDeviceType.GAMEPAD)
			return false;

		w.GetWorkspace().SetFocusedWidget(w);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		super.OnFocusLost(w, x, y);

		w.FindAnyWidget("Outline").SetVisible(false);
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		super.OnFocus(w, x, y);

		SCR_UITaskManagerComponent uiTaskManager = SCR_UITaskManagerComponent.GetInstance();
		if (!uiTaskManager)
			return false;
		
		SCR_PickAssigneeDialog menu = SCR_PickAssigneeDialog.Cast(GetGame().GetMenuManager().FindMenuByPreset(ChimeraMenuPreset.PickAssignee));
		menu.SelectTaskExecutor(m_Executor);
		w.FindAnyWidget("Outline").SetVisible(true);
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_wMainWidget = w;
	}
};
