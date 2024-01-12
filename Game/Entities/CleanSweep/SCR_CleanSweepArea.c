[EntityEditorProps(category: "GameScripted/CleanSweep", description: "This is clean sweep area entity.", color: "0 0 255 255")]
class SCR_CleanSweepAreaClass: GenericEntityClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CleanSweepArea : GenericEntity
{
	[Attribute("400", UIWidgets.EditBox, "How many groups are spawned in the location")]
	float m_Range;
	
	[Attribute("0", UIWidgets.CheckBox, "Can this location be selected in the mission")]
	bool m_Active;
	
	static ref array<SCR_CleanSweepArea> s_aInstances = new ref array<SCR_CleanSweepArea>;
	
	#ifdef WORKBENCH
	override void _WB_AfterWorldUpdate(float timeSlice)
	{
		int color;
		const int alpha = 128;
		if (m_Active)
			color = ARGB(alpha,0,255,0);
		else
			color = ARGB(alpha,0, 0, 0);
		
		ref auto pShape = Shape.CreateSphere(color, ShapeFlags.ONCE | ShapeFlags.DOUBLESIDE | ShapeFlags.TRANSP, GetOrigin(), m_Range);
	}
	#endif
	
	void SCR_CleanSweepArea(IEntitySource src, IEntity parent)
	{
		if (s_aInstances)
			s_aInstances.Insert(this);
	}
	
	void ~SCR_CleanSweepArea()
	{
		if (s_aInstances)
			s_aInstances.RemoveItem(this);
	}
};
