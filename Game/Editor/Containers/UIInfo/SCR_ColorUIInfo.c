[BaseContainerProps(), SCR_BaseContainerLocalizedTitleField("Name")]
class SCR_ColorUIInfo : SCR_UIInfo
{
	[Attribute("1.0 1.0 1.0 1.0")]
	protected ref Color m_cColor;
	
	/*!
	Get Color
	\return UI Info Color
	*/
	Color GetColor()
	{
		return m_cColor;
	}
	
	//~ Protected, to be overriden and/or made public by inherited classes
	override protected void CopyFrom(SCR_UIName source)
	{
		SCR_ColorUIInfo sourceInfo = SCR_ColorUIInfo.Cast(source);
		if (sourceInfo)
		{
			Icon = sourceInfo.Icon;
			IconSetName = sourceInfo.IconSetName;
			m_cColor = sourceInfo.m_cColor;
		}
		
		super.CopyFrom(source);
	}
};
