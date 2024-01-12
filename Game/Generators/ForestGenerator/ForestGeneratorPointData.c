class ForestGeneratorPointData : ShapePointDataScriptBase
{
	[Attribute("1", UIWidgets.CheckBox, "Generate the middle outline along this line?")]
	bool m_bMiddleOutline;

	[Attribute("1", UIWidgets.CheckBox, "Generate the small outline along this line?")]
	bool m_bSmallOutline;
}
