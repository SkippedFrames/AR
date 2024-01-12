[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class ForestGeneratorLevel
{
	[Attribute(uiwidget: UIWidgets.Object, desc: "Tree groups to spawn in this forest generator level")]
	ref array<ref TreeGroupClass> m_aTreeGroups;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Generate this level?")]
	bool m_bGenerate;

	[Attribute(defvalue: "1", desc: "How many trees per hectare should be generated", uiwidget: UIWidgets.SpinBox)]
	float m_fDensity;

	[Attribute(defvalue: "", category: "Forest", desc: "Curve defining level's outline scaling; from inside (left, forest core) to outside (right, forest outline)", uiwidget: UIWidgets.GraphDialog, params: "100 " + (SCALE_CURVE_MAX_VALUE - SCALE_CURVE_MIN_VALUE) + " 0 " + SCALE_CURVE_MIN_VALUE)]
	ref Curve m_aOutlineScaleCurve;

	[Attribute(defvalue: "0", category: "Forest", desc: "Distance from shape over which the scaling occurs", uiwidget: UIWidgets.Slider, params: "0 100 0.1")]
	float m_fOutlineScaleCurveDistance;

	SCR_EForestGeneratorLevelType m_eType;
	ref array<float> m_aGroupProbas = {};

	static const float SCALE_CURVE_MAX_VALUE = 1;
	static const float SCALE_CURVE_MIN_VALUE = 0;
}

[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class ForestGeneratorOutline : ForestGeneratorLevel
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.SpinBox, desc: "How separated are the tree groups?")]
	float m_fClusterStrength;

	[Attribute(defvalue: "15", uiwidget: UIWidgets.SpinBox, desc: "In what radius should trees around be taken into count for clusters?")]
	float m_fClusterRadius;

	[Attribute("0", UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_EForestGeneratorOutlineType), desc: "Which type of outline is this?")]
	SCR_EForestGeneratorOutlineType m_eOutlineType;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.SpinBox, desc: "Minimal distance from spline")]
	float m_fMinDistance;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.SpinBox, desc: "Maximal distance from spline")]
	float m_fMaxDistance;

	//------------------------------------------------------------------------------------------------
	void ForestGeneratorOutline()
	{
		m_eType = SCR_EForestGeneratorLevelType.OUTLINE;
	}
}

[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class ForestGeneratorTopLevel : ForestGeneratorLevel
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.SpinBox, desc: "How separated are the tree groups?")]
	float m_fClusterStrength;

	[Attribute(defvalue: "15", uiwidget: UIWidgets.SpinBox, desc: "In what radius should trees around be taken into count for clusters?")]
	float m_fClusterRadius;

	//------------------------------------------------------------------------------------------------
	void ForestGeneratorTopLevel()
	{
		m_eType = SCR_EForestGeneratorLevelType.TOP;
	}
}

[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class ForestGeneratorBottomLevel : ForestGeneratorLevel
{
	void ForestGeneratorBottomLevel()
	{
		m_eType = SCR_EForestGeneratorLevelType.BOTTOM;
	}
}

enum SCR_EForestGeneratorLevelType
{
	TOP,
	BOTTOM,
	OUTLINE,
}

enum SCR_EForestGeneratorOutlineType
{
	SMALL,
	MIDDLE,
}
