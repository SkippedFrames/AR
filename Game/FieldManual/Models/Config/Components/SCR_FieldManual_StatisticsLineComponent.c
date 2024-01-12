class SCR_FieldManual_StatisticsLineComponent
{
	protected Widget m_wRootWidget;
	protected TextWidget m_wValue;

	//------------------------------------------------------------------------------------------------
	void SCR_FieldManual_StatisticsLineComponent(notnull Widget rootWidget)
	{
		m_wRootWidget = rootWidget;
		m_wValue = TextWidget.Cast(rootWidget.FindAnyWidget("Value"));
	}

	//------------------------------------------------------------------------------------------------
	void SetValue(string value)
	{
		if (m_wValue)
			m_wValue.SetText(value);
	}

	//------------------------------------------------------------------------------------------------
	void RemoveWidgetFromHierarchy()
	{
		m_wRootWidget.RemoveFromHierarchy();
	}

	//------------------------------------------------------------------------------------------------
	void SetTranslatedValue(string value, string parameter1 = "")
	{
		m_wValue.SetTextFormat(value, parameter1);
	}

	//------------------------------------------------------------------------------------------------
	static SCR_FieldManual_StatisticsLineComponent GetComponent(notnull Widget parentWidget, string widgetName)
	{
		Widget result = parentWidget.FindAnyWidget(widgetName);
		if (!result)
			return null;

		return GetComponent(result);
	}

	//------------------------------------------------------------------------------------------------
	static SCR_FieldManual_StatisticsLineComponent GetComponent(notnull Widget widget)
	{
		return new SCR_FieldManual_StatisticsLineComponent(widget);
	}
};
