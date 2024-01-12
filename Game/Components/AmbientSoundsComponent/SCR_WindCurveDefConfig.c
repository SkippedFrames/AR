/*
Array has to have the same size as EWindCurve count
*/
[BaseContainerProps(configRoot: true)]
class SCR_WindCurveDefConfig
{
	[Attribute("", UIWidgets.Object, "Wind modifier curve definition")]
	ref array<ref SCR_WindCurveDef> m_aWindModifier;
}
