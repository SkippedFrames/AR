//------------------------------------------------------------------------------------------------
class SCR_MapTaskListUI : SCR_MapUIBaseComponent
{
	const string TASK_LIST_FRAME = "MapTaskList";
	const string ICON_NAME = "faction";

	protected Widget m_wUI;
	protected OverlayWidget m_wTaskListFrame;
	protected SCR_UITaskManagerComponent m_UITaskManager;
	protected SCR_MapToolMenuUI m_ToolMenu;
	protected SCR_MapToolEntry m_ToolMenuEntry;
	
	protected bool m_bTaskListInvoked;
	protected bool m_bOpened;
	protected bool m_bOnMapClose;
	protected bool m_bMapContextAllowed = true;

	[Attribute("JournalFrame", desc: "Journal frame widget name")]
	protected string m_sJournalFrameName;

	[Attribute("MapTaskList", desc: "Map task list root widget name")]
	protected string m_sMapTaskListRootName;
	
	[Attribute("MapTaskListFrame", desc: "Map task list root frame widget name")]
	protected string m_sMapTaskListRootFrameName;

	[Attribute("exclamationCircle", desc: "Journal Toolmenu imageset quad name")]
	protected string m_sJournalToolMenuIconName;

	//------------------------------------------------------------------------------------------------
	override protected void OnMapOpen(MapConfiguration config)
	{
		m_bOnMapClose = false;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnMapClose(MapConfiguration config)
	{
		if (m_UITaskManager)
		{
			m_bOnMapClose = true;
			m_UITaskManager.Action_TasksClose();
			m_UITaskManager.ClearWidget();
			m_UITaskManager.CreateTaskList();
		}
		
		m_ToolMenuEntry.SetActive(false);
	}

	//------------------------------------------------------------------------------------------------
	override void Init()
	{
		m_UITaskManager = SCR_UITaskManagerComponent.GetInstance();
		if (!m_UITaskManager)
		{
			SetActive(false); // deactivate component
			return;
		}

		m_ToolMenu = SCR_MapToolMenuUI.Cast(m_MapEntity.GetMapUIComponent(SCR_MapToolMenuUI));
		if (m_ToolMenu)
		{
			m_ToolMenuEntry = m_ToolMenu.RegisterToolMenuEntry(SCR_MapToolMenuUI.s_sToolMenuIcons, ICON_NAME, 2);
			m_ToolMenuEntry.m_OnClick.Insert(HandleTaskList);
		}

		SCR_UITaskManagerComponent.s_OnTaskListVisible.Insert(ToggleTaskListInvoker);
		GetGame().OnInputDeviceIsGamepadInvoker().Insert(OnInputDeviceIsGamepad);
		
		//If there is a OverlayWidget like on the DeployMenu we use that instead of the default one
		m_wTaskListFrame = OverlayWidget.Cast(m_RootWidget.FindAnyWidget(TASK_LIST_FRAME));
		if (!m_wTaskListFrame)
			return;

		m_wUI = m_UITaskManager.CreateTaskList(m_wTaskListFrame);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Update(float timeSlice)
	{
		if (m_bOpened && !m_bMapContextAllowed && m_MapEntity && m_MapEntity.IsOpen())
			GetGame().GetInputManager().ActivateContext("TaskListMapContext");
	}
	
	//------------------------------------------------------------------------------------------------
	void SetMapContextAllowed(bool val)
	{
		m_bMapContextAllowed = val;
	}
	
	//------------------------------------------------------------------------------------------------
	void ToggleTaskList()
	{
		if (!m_bOpened)
		{
			m_UITaskManager.ClearWidget();
			if (!m_wTaskListFrame)
				m_wTaskListFrame = OverlayWidget.Cast(m_RootWidget.FindAnyWidget(TASK_LIST_FRAME));
				
			m_wUI = m_UITaskManager.CreateTaskList(m_wTaskListFrame);
			if (m_wUI)
			{
				m_bOpened = true;
				m_bTaskListInvoked = true;
				m_UITaskManager.Action_ShowTasks(m_wUI);
				m_ToolMenuEntry.SetActive(true);
			}
		}
		else
		{
			m_bOpened = false;
			m_bTaskListInvoked = true;
			m_UITaskManager.Action_TasksClose();
			m_ToolMenuEntry.SetActive(false);
		}

		m_bTaskListInvoked = false;
	}
	
	//------------------------------------------------------------------------------------------------
	void ToggleTaskListInvoker(bool isVisible)
	{
		if (m_bOnMapClose)
			return;
		
		if (!m_bTaskListInvoked)
		{
			m_bTaskListInvoked = true;
			HandleTaskList(isVisible);
		}
	}

	//------------------------------------------------------------------------------------------------
	void HandleTaskList(bool isVisible = true)
	{
		if (!m_RootWidget)
			return;
		
		Widget taskListRoot = m_RootWidget.FindAnyWidget(m_sMapTaskListRootName);
		if (!taskListRoot)
			return;
		
		taskListRoot.SetVisible(isVisible);
		Widget taskListRootFrame = m_RootWidget.FindAnyWidget(m_sMapTaskListRootFrameName);
		if (!taskListRootFrame)
		{
			ToggleTaskList();
			return;
		}
		
		taskListRootFrame.SetVisible(isVisible);
		foreach (SCR_MapToolEntry toolEntry : m_ToolMenu.GetMenuEntries())
		{
			if (toolEntry.GetImageSet() != m_sJournalToolMenuIconName)
				continue;

			Widget mapJournalFrame = m_RootWidget.FindAnyWidget(m_sJournalFrameName);
			if (mapJournalFrame && mapJournalFrame.IsVisible())
			{
				mapJournalFrame.SetVisible(false);
				toolEntry.SetActive(false);
			}
		}

		ToggleTaskList();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInputDeviceIsGamepad(bool isGamepad)
	{
		if (isGamepad && m_bOpened)
		{
			m_bMapContextAllowed = false;
			return;
		}
		
		m_bMapContextAllowed = true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (SCR_Global.IsEditMode()) 
			return;

		GetGame().OnInputDeviceIsGamepadInvoker().Remove(OnInputDeviceIsGamepad);
	}
}
