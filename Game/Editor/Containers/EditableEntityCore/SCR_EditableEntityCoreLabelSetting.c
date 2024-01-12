[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityLabel, "m_LabelType")]
class SCR_EditableEntityCoreLabelSetting
{
	[Attribute()]
	protected ref SCR_UIInfo m_Info;
	
	[Attribute("0")]
	private int m_Order;
	
	[Attribute("0", UIWidgets.SearchComboBox, "", enums: ParamEnumArray.FromEnum(EEditableEntityLabel))]
	private EEditableEntityLabel m_LabelType;
	
	[Attribute("0", UIWidgets.SearchComboBox, "", enums: ParamEnumArray.FromEnum(EEditableEntityLabelGroup))]
	private EEditableEntityLabelGroup m_LabelGroupType;
	
	[Attribute("1")]
	private bool m_bFilterEnabled;
	
	SCR_UIInfo GetInfo()
	{
		return m_Info;
	}
	
	int GetOrder()
	{
		return m_Order;
	}
	
	EEditableEntityLabel GetLabelType()
	{
		return m_LabelType;
	}
	EEditableEntityLabelGroup GetLabelGroupType()
	{
		return m_LabelGroupType;
	}
	
	bool GetFilterEnabled()
	{
		return m_bFilterEnabled;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Override function for inherited classes to check if label can be shown
	Is checked when calling GetLabelUIInfoIfValid
	\return true if the label is valid and can be shown
	*/
	bool IsValid(EEditorMode currentMode)
	{
		return true;
	}
};
