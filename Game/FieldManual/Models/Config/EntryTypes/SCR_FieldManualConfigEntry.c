[BaseContainerProps(insertable: false), SCR_BaseContainerLocalizedTitleField("m_sTitle", "Entry: %1")]
class SCR_FieldManualConfigEntry
{
	[Attribute(defvalue: "1")]
	bool m_bEnabled;

	[Attribute(defvalue: SCR_Enum.GetDefault(EFieldManualEntryId.NONE), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EFieldManualEntryId))]
	EFieldManualEntryId m_eId;

	[Attribute(uiwidget: UIWidgets.LocaleEditBox)]
	string m_sTitle;

	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, params: "edds")]
	ResourceName m_Image;

	SCR_FieldManualConfigCategory m_Parent; // no strong ref: if the parent dies, he dies

	[Attribute()]
	ref array<ref SCR_FieldManualPiece> m_aContent;

	//------------------------------------------------------------------------------------------------
	void SCR_FieldManualConfigEntry()
	{
		if (!m_aContent) // can be config-provided
		{
			m_aContent = {};
		}
	}

	//------------------------------------------------------------------------------------------------
	Widget CreateWidget(notnull Widget parent);

	//------------------------------------------------------------------------------------------------
	Widget CreateWidgetFromLayout(ResourceName layout, notnull Widget parent)
	{
		Widget createdWidget = GetGame().GetWorkspace().CreateWidgets(layout, parent);
		if (!createdWidget)
		{
			Print(string.Format("could not create widget from layout \"%1\" | ", layout) + FilePath.StripPath(__FILE__) + ":" + __LINE__, LogLevel.WARNING);
			return createdWidget;
		}
		Widget piecesLayout = SCR_WidgetHelper.GetWidgetOrChild(createdWidget, "piecesLayout");
		if (!piecesLayout || !m_aContent)
		{
			Print("no layout or content found | " + FilePath.StripPath(__FILE__) + ":" + __LINE__, LogLevel.DEBUG);
			return createdWidget;
		}

		foreach (SCR_FieldManualPiece piece : m_aContent)
		{
			piece.CreateWidget(piecesLayout);
		}

		return createdWidget;
	}
};
