[BaseContainerProps()]
class SCR_SimpleMessageUIInfo: SCR_UIInfo
{
	[Attribute("1 1 1 1", UIWidgets.ColorPicker)]
	protected ref Color m_ImageColor;
	
	[Attribute("false")]
	protected bool m_bDisplayLoading;
	
	/*!
	Get given color
	\return Color to set image to
	*/
	Color GetImageColor()
	{
		return m_ImageColor;
	}
	
	bool GetDisplayLoading()
	{
		return m_bDisplayLoading;
	}
};
