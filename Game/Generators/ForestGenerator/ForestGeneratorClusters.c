[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class ForestGeneratorCluster
{
	[Attribute(defvalue: "", uiwidget: UIWidgets.Object, desc: "Define which tree groups should spawn in this cluster.")]
	ref array<ref SmallForestGeneratorClusterObject> m_aObjects;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.SpinBox, desc: "The minimum density of the filling in number of clusters for hectare.")]
	float m_fMinCDENSHA;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.SpinBox, desc: "The maximum density of the filling in number of clusters for hectare.")]
	float m_fMaxCDENSHA;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Generate this cluster?")]
	bool m_bGenerate;

	SCR_EForestGeneratorClusterType m_Type;
	float m_fRadius;

	void ForestGeneratorCluster()
	{
		if (!m_aObjects)
			return;

		foreach (SmallForestGeneratorClusterObject object : m_aObjects)
		{
			if (object.m_fMaxRadius > m_fRadius)
				m_fRadius = object.m_fMaxRadius;

			if (object.m_fMinRadius > m_fRadius)
				m_fRadius = object.m_fMinRadius;
		}
	}
}

[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class ForestGeneratorCircleCluster : ForestGeneratorCluster
{
	//------------------------------------------------------------------------------------------------
	void ForestGeneratorCircleCluster()
	{
		m_Type = SCR_EForestGeneratorClusterType.CIRCLE;
	}
}

[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class ForestGeneratorStripCluster : ForestGeneratorCluster
{
	[Attribute(defvalue: "10", uiwidget: UIWidgets.SpinBox, desc: "Offset that is applied to generated objects to avoid seeing the exact shape of the curve. [m]")]
	float m_fMaxXOffset;

	[Attribute(defvalue: "10", uiwidget: UIWidgets.SpinBox, desc: "Offset that is applied to generated objects to avoid seeing the exact shape of the curve. [m]")]
	float m_fMaxYOffset;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.SpinBox, desc: "Defines how many times should the curve repeat for this strip.")]
	float m_fFrequency;

	[Attribute(defvalue: "10", uiwidget: UIWidgets.SpinBox, desc: "Defines the maximum extent of the curve. [m]")]
	float m_fAmplitude;

	//------------------------------------------------------------------------------------------------
	void ForestGeneratorStripCluster()
	{
		m_Type = SCR_EForestGeneratorClusterType.STRIP;
	}
}

[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class WideForestGeneratorClusterObject : SmallForestGeneratorClusterObject
{
	[Attribute(desc: "Capsule start, defines offset from the pivot of the object, serves for collision detection")]
	vector m_CapsuleStartInEditor;

	[ForestGeneratorCapsuleStartAttribute()]
	vector m_CapsuleStart;

	[Attribute(desc: "Capsule end, defines offset from the pivot of the object, serves for collision detection")]
	vector m_CapsuleEndInEditor;

	[ForestGeneratorCapsuleEndAttribute()]
	vector m_CapsuleEnd;

	[Attribute(defvalue: "1", desc: "This overrides the setting from template library!")]
	bool m_bAlignToNormal;

	float m_fYaw;
	protected float m_fMinDistanceFromLine = -1;

	//------------------------------------------------------------------------------------------------
	void Rotate()
	{
		m_CapsuleStart = Rotate2D(m_CapsuleStartInEditor * m_fScale, Math.DEG2RAD * m_fYaw);
		m_CapsuleEnd = Rotate2D(m_CapsuleEndInEditor * m_fScale, Math.DEG2RAD * m_fYaw);
	}

	//------------------------------------------------------------------------------------------------
	vector Rotate2D(vector vec, float rads)
	{
		float sin = Math.Sin(rads);
		float cos = Math.Cos(rads);

		return Vector(
			vec[0] * cos - vec[2] * sin,
			0,
			vec[0] * sin + vec[2] * cos);
	}

	//------------------------------------------------------------------------------------------------
	float GetMinDistanceFromLine()
	{
		if (m_fMinDistanceFromLine != -1)
			return m_fMinDistanceFromLine;

		float distance = m_CapsuleStart.Length();
		if (distance > m_fMinDistanceFromLine)
			m_fMinDistanceFromLine = distance;

		distance = m_CapsuleEnd.Length();
		if (distance > m_fMinDistanceFromLine)
			m_fMinDistanceFromLine = distance;

		return m_fMinDistanceFromLine;
	}
}

[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class SmallForestGeneratorClusterObject : SCR_ForestGeneratorTreeBase
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.SpinBox, desc: "Minimum count of this object to spawn")];
	int m_iMinCount;

	[Attribute(defvalue: "2", uiwidget: UIWidgets.SpinBox, desc: "Maximum count of this object to spawn")];
	int m_iMaxCount;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.SpinBox, desc: "Minimum distance of this object from the center of this cluster")]
	float m_fMinRadius;

	[Attribute(defvalue: "2", uiwidget: UIWidgets.SpinBox, desc: "Maximum distance of this object from the center of this cluster")]
	float m_fMaxRadius;

	//------------------------------------------------------------------------------------------------
	override void AdjustScale()
	{
		m_fBotDistance *= m_fScale;
	}
}

[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class MiddleForestGeneratorClusterObject : SmallForestGeneratorClusterObject
{
	[Attribute(defvalue: "5", uiwidget: UIWidgets.SpinBox, "Minimum required radius in the middle layer for this object to spawn"), ForestGeneratorDistaceAttribute(DistanceType.MID)]
	float m_fMidDistance;

	//------------------------------------------------------------------------------------------------
	override void AdjustScale()
	{
		m_fBotDistance *= m_fScale;
		m_fMidDistance *= m_fScale;
	}
}

[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class BigForestGeneratorClusterObject : MiddleForestGeneratorClusterObject
{
	[Attribute(defvalue: "10", uiwidget: UIWidgets.SpinBox, "Minimum required radius in the top layer for this object to spawn"), ForestGeneratorDistaceAttribute(DistanceType.TOP)]
	float m_fTopDistance;

	//------------------------------------------------------------------------------------------------
	override void AdjustScale()
	{
		m_fBotDistance *= m_fScale;
		m_fMidDistance *= m_fScale;
		m_fTopDistance *= m_fScale;
	}
}

enum SCR_EForestGeneratorClusterType
{
	CIRCLE,
	STRIP,
}

