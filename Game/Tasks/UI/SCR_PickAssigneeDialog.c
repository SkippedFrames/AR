//------------------------------------------------------------------------------------------------
class SCR_PickAssigneeDialog : ChimeraMenuBase
{
	protected SCR_InputButtonComponent m_CloseButton;
	protected SCR_InputButtonComponent m_PickAssigneeButton;
	protected SCR_BaseTaskExecutor m_SelectedTaskExecutor;
	
	//------------------------------------------------------------------------------------------------
	SCR_BaseTaskExecutor GetSelectedExecutor()
	{
		return m_SelectedTaskExecutor;
	}
	
	//------------------------------------------------------------------------------------------------
	void SelectTaskExecutor(SCR_BaseTaskExecutor executor)
	{
		m_SelectedTaskExecutor = executor;
	}
	
	//------------------------------------------------------------------------------------------------
	void CloseDialog()
	{
		SCR_UITaskManagerComponent ui = SCR_UITaskManagerComponent.GetInstance();
		if (ui)
			ui.HidePickAssignee();
	}

	void PickAssignee()
	{
		SCR_UITaskManagerComponent ui = SCR_UITaskManagerComponent.GetInstance();
		if (ui)
			ui.Action_PickAssignee();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		m_SelectedTaskExecutor = null;

		m_CloseButton = SCR_InputButtonComponent.GetInputButtonComponent("CloseButton", GetRootWidget());
		if (m_CloseButton)
			m_CloseButton.m_OnActivated.Insert(CloseDialog);
		m_PickAssigneeButton = SCR_InputButtonComponent.GetInputButtonComponent("PickAssigneeButton", GetRootWidget());
		if (m_PickAssigneeButton)
			m_PickAssigneeButton.m_OnActivated.Insert(PickAssignee);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_PickAssigneeDialog()
	{
	}
};
