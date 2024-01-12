//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_DamagePassRuleContainerTitle()]
class SCR_DamagePassRule
{
	[Attribute(uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EDamageState))]
	ref array<EDamageState> m_aDamageStates;

	[Attribute(uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EDamageType))]
	ref array<EDamageType> m_aSourceDamageTypes;

	[Attribute(uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EDamageType))]
	EDamageType m_eOutputDamageType;

	[Attribute()]
	float m_fMultiplier;

	[Attribute(uiwidget: UIWidgets.CheckBox)]
	bool m_bAllowDOT;

	[Attribute(uiwidget: UIWidgets.CheckBox)]
	bool m_bPassToRoot;

	[Attribute(uiwidget: UIWidgets.CheckBox)]
	bool m_bPassToParent;	
	
	[Attribute(uiwidget: UIWidgets.CheckBox)]
	bool m_bPassToDefaultHitZone;
}

//------------------------------------------------------------------------------------------------
class SCR_DamagePassRuleContainerTitle : BaseContainerCustomTitle
{
	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		title = string.Empty;

		int type;
		if (source.Get("m_eOutputDamageType", type))
			title = "Pass as " + SCR_Enum.GetEnumName(EDamageType, type);

		float multiplier;
		if (source.Get("m_fMultiplier", multiplier))
			title += " x " + multiplier.ToString(-1, 3);

		bool dot;
		if (source.Get("m_bAllowDOT", dot) && dot)
			title += ", DOT";

		bool root;
		if (source.Get("m_bPassToRoot", root) && root)
			title += ", root";

		bool parent;
		if (source.Get("m_bPassToParent", parent) && parent)
			title += ", parent";		
		
		bool defaultHitZone;
		if (source.Get("m_bPassToDefaultHitZone", defaultHitZone) && defaultHitZone)
			title += ", defaultHitZone";

		return true;
	}
}

