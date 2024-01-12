class SCR_AdvancedActionRowComponent : SCR_ScriptedWidgetComponent
{
	protected Widget m_wParentWidget;

	protected int m_iKeybindIndex;

	//------------------------------------------------------------------------------------------------
	void Init(string actionName, string actionPreset, int index, SCR_SettingsManagerKeybindModule settingsKeybindModule)
	{
		RichTextWidget rowRichText = RichTextWidget.Cast(m_wRoot.FindAnyWidget("ActionBindRichText"));
		if (!rowRichText)
			return;

		SetKeybindIndex(index);

		rowRichText.SetText(string.Format("<action name='%1' preset='%2' index='%3'/>", actionName, actionPreset, index));

		RichTextWidget indexRichText = RichTextWidget.Cast(m_wRoot.FindAnyWidget("IndexRichtext"));
		if (!indexRichText)
			return;

		//offset index by 1 so it's comfortable for humans
		int order = m_iKeybindIndex + 1;

		//todo: check how localization deals with the dot: Updated with Wlib - removing "." 
		indexRichText.SetText("" + order + "");

		//check for conflicts and deal with them if they exist
		array<SCR_KeyBindingEntry> conflictedActions = {};
		if (!settingsKeybindModule.IsActionConflicted(actionName, conflictedActions, m_iKeybindIndex, actionPreset))
			return;

		rowRichText.SetColor(Color.DarkRed);

		RichTextWidget warningRichText = RichTextWidget.Cast(m_wRoot.FindAnyWidget("WarningText"));
		if (!warningRichText)
			return;

		RichTextWidget actionsRichText = RichTextWidget.Cast(m_wRoot.FindAnyWidget("WarningActions"));
		if (!actionsRichText)
			return;

		warningRichText.SetOpacity(1);
		actionsRichText.SetOpacity(1);

		string actionNames = conflictedActions.Get(0).m_sDisplayName;
		
		for (int i = 1, count = conflictedActions.Count(); i < count; i++)
			actionNames = actionNames + ", " + conflictedActions.Get(i).m_sDisplayName;

		actionsRichText.SetText(actionNames);
	}

	//------------------------------------------------------------------------------------------------
	void SetKeybindIndex(int index)
	{
		m_iKeybindIndex = index;
	}

	//------------------------------------------------------------------------------------------------
	int GetKeybindIndex()
	{
		return m_iKeybindIndex;
	}
};
