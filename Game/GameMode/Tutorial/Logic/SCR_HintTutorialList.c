[BaseContainerProps(configRoot: true)]
class SCR_HintTutorialList
{
	[Attribute()]
	protected ref array<ref SCR_HintTutorial> m_aHints;
	
	SCR_HintUIInfo GetHint(SCR_ECampaignTutorialArlandStage stage, string hintname = "")
	{
		foreach (SCR_HintTutorial hint:m_aHints)
		{
			if (stage == hint.GetEnum())
			{
				if (!hintname.IsEmpty())
				{
					if (hintname == hint.GetBaseString())
						return hint.GetHint();
					continue;
				}
				return hint.GetHint();
			}
		}
		return null;
	}
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(SCR_ECampaignTutorialArlandStage, "m_eStage")]
class SCR_HintTutorial
{
	[Attribute()]
	protected ref SCR_HintUIInfo m_Hint;	
	
	[Attribute(defvalue: "none")]
	protected string m_sBaseOverviewName;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_ECampaignTutorialArlandStage))]
	protected SCR_ECampaignTutorialArlandStage m_eStage;
	
	SCR_ECampaignTutorialArlandStage GetEnum()
	{
		return m_eStage;
	}
	
	SCR_HintUIInfo GetHint()
	{
		return m_Hint;
	}
	
	string GetBaseString()
	{
		return m_sBaseOverviewName;
	}
};
