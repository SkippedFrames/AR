class SCR_FieldManualEntryScriptedWidgetEventHandler : ScriptedWidgetEventHandler
{
	protected SCR_FieldManualUI m_UI;

	void SCR_FieldManualEntryScriptedWidgetEventHandler(notnull SCR_FieldManualUI ui)
	{
		m_UI = ui;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (m_UI)
			m_UI.OnTileClicked(w);

		return m_UI != null;
	}
};
