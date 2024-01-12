[EntityEditorProps(category: "GameLib/Scripted/Generator", description: "Track Generator", dynamicBox: true, visible: false)]
class SCR_TrackGeneratorEntityClass : SCR_GeneratorBaseEntityClass
{
}

class SCR_TrackGeneratorEntity : SCR_GeneratorBaseEntity
{
	[Attribute(defvalue: "0", category: "Track", desc: "Distance around the line that should be left empty by other generators", uiwidget: UIWidgets.Slider, params: "0 100 1")]
	protected float m_fClearance;

	//------------------------------------------------------------------------------------------------
	void SCR_TrackGeneratorEntity(IEntitySource src, IEntity parent)
	{
	}
}
