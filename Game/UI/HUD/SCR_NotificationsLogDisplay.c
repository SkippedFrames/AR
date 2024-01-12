/*
Handles the SCR_NotificationsLogComponent when Player is initialized.
*/
class SCR_NotificationsLogDisplay : SCR_InfoDisplayExtended
{
	[Attribute(desc: "Should the Notifications go from top to bottom instead of bottom to top?")]
	protected bool m_bInsertFromTop;

	protected SCR_NotificationsLogComponent m_NotificationsHud;
	protected SCR_InfoDisplaySlotHandler m_slotHandler;
	/*
	Sets the height of 1 notification to the available spac in the slot can be set correctly. 
	!CANNOT BE 0!
	*/
	protected const int HEIGHT_DIVIDER = 50;

	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		if (!m_wRoot)
			return;

		m_NotificationsHud = SCR_NotificationsLogComponent.Cast(m_wRoot.FindHandler(SCR_NotificationsLogComponent));

		m_slotHandler = SCR_InfoDisplaySlotHandler.Cast(GetHandler(SCR_InfoDisplaySlotHandler));
		if (!m_slotHandler)
			return;

		m_slotHandler.GetSlotUIComponent().GetOnResize().Insert(OnSlotUIResize);
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayStopDraw(IEntity owner)
	{
		if (m_slotHandler)
			m_slotHandler.GetSlotUIComponent().GetOnResize().Remove(OnSlotUIResize);
	}

	//------------------------------------------------------------------------------------------------
	//! Calculate the amount of lines that can be displayed dependent on how much space the Layout has
	void OnSlotUIResize()
	{
		int maxNotifications = (int)m_slotHandler.GetSlotUIComponent().GetHeight() / HEIGHT_DIVIDER;
		if (maxNotifications < 1)
			maxNotifications = 1;

		m_NotificationsHud.OnSlotResize(maxNotifications);
		m_NotificationsHud.ChangeInsertOrder(m_bInsertFromTop);
	}
}
