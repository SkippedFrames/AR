//------------------------------------------------------------------------------------------------
class SCR_CommunitySubMenu : SCR_SubMenuBase
{
	[Attribute("{6FFF0601A48C7FEE}UI/layouts/Menus/MainMenu/CommunityNotificationButton.layout", UIWidgets.ResourceNamePicker, "", "layout")]
	protected ResourceName m_sNotificationsLayout;

	[Attribute("Send")]
	protected string m_sSendButtonName;
	[Attribute("Feedback")]
	protected string m_sFeedbackEditboxName;
	[Attribute("Address")]
	protected string m_sMailEditboxName;
	[Attribute("Notifications")]
	protected string m_sNotificationName;
	[Attribute("Type")]
	protected string m_sTypeComboBoxName;
	[Attribute("Category")]
	protected string m_sCategoryComboBoxName;

	protected SCR_ButtonBaseComponent m_Send;
	protected SCR_EditBoxComponent m_Feedback;
	protected SCR_ComboBoxComponent m_FeedbackType;
	protected SCR_ComboBoxComponent m_FeedbackCategory;

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_SuperMenuBase parentMenu)
	{
		super.OnMenuOpen(parentMenu);

		SetupNotifications();

		m_Send = SCR_ButtonBaseComponent.GetButtonBase(m_sSendButtonName, m_wRoot);
		if (m_Send)
		{
			m_Send.m_OnClicked.Insert(OnSendFeedback);
			m_Send.SetEnabled(false, false);
		}

		m_Feedback = SCR_EditBoxComponent.GetEditBoxComponent(m_sFeedbackEditboxName, m_wRoot);

		SCR_InputButtonComponent tos = SCR_InputButtonComponent.GetInputButtonComponent("ToS",parentMenu.GetRootWidget());
		if (tos)
			tos.SetVisible(true);

		SetupCategories();
	}

	// Setup type and category combos according to FeedbackDialogUI constants
	//------------------------------------------------------------------------------------------------
	protected void SetupCategories()
	{
		m_FeedbackCategory = SCR_ComboBoxComponent.GetComboBoxComponent(m_sCategoryComboBoxName, m_wRoot);
		m_FeedbackType = SCR_ComboBoxComponent.GetComboBoxComponent(m_sTypeComboBoxName, m_wRoot);
		if (!m_FeedbackCategory || !m_FeedbackType)
			return;

		m_FeedbackCategory.ClearAll();
		m_FeedbackType.ClearAll();


		foreach (string s : FeedbackDialogUI.CATEGORY_NAMES)
		{
			m_FeedbackCategory.AddItem(s);
		}

		foreach (string s : FeedbackDialogUI.TYPE_NAMES)
		{
			m_FeedbackType.AddItem(s);
		}

		m_FeedbackType.SetCurrentItem(0);
		m_FeedbackCategory.SetCurrentItem(0);
	}

	//------------------------------------------------------------------------------------------------
	void SetupNotifications()
	{
		Widget notifications = m_wRoot.FindAnyWidget(m_sNotificationName);
		if (!notifications)
			return;

		array<ref SCR_NewsEntry> entries = {};
		MainMenuUI.GetNotificationEntries(entries);
		foreach (SCR_NewsEntry entry : entries)
		{
			Widget w = GetGame().GetWorkspace().CreateWidgets(m_sNotificationsLayout, notifications);
			if (!w)
				continue;

			SCR_NewsTileComponent tile = SCR_NewsTileComponent.Cast(w.FindHandler(SCR_NewsTileComponent));
			if (!tile)
				continue;

			tile.ShowTile(entry);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(SCR_SuperMenuBase parentMenu, float tDelta)
	{
		if (m_Send)
			m_Send.SetEnabled(m_Feedback.GetValue() != string.Empty && FeedbackDialogUI.CanSendFeedback());
	}

	// Copy code from feedback dialog
	//------------------------------------------------------------------------------------------------
	void OnSendFeedback()
	{
		if (!m_Feedback || !m_FeedbackType || !m_FeedbackCategory)
			return;

		string content = m_Feedback.GetValue();

		// Clear feedback window
		m_Feedback.SetValue(string.Empty);

		// Send feedback
		FeedbackDialogUI.SendFeedback(content, m_FeedbackType.GetCurrentIndex(), m_FeedbackCategory.GetCurrentIndex());
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose(SCR_SuperMenuBase parentMenu)
	{
		FeedbackDialogUI.ClearFeedback();
		SCR_InputButtonComponent tos = SCR_InputButtonComponent.GetInputButtonComponent("ToS",parentMenu.GetRootWidget());
		if (tos)
			tos.SetVisible(false);
	}
};
