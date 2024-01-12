//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_GraphLinesData
{
	[Attribute("0.3", UIWidgets.Slider, "Default alpha of the lines.", "0, 1, 0.1")]
	protected float m_iAlphaDefault;
	
	[Attribute("1", UIWidgets.Slider, "Alpha of the lines, when highlighted or selected.", "0, 1, 0.1")]
	protected float m_iAlphaHighlight;
	
	//------------------------------------------------------------------------------------------------
	float GetDefaultAlpha()
	{
		return m_iAlphaDefault;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetHighlightedAlpha()
	{
		return m_iAlphaHighlight;
	}
};
