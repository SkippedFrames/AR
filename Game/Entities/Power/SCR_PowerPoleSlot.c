//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_PowerPoleSlot : SCR_PowerPoleSlotBase
{
	[Attribute(desc: "Visualised by red sphere.", params: "inf inf 0 purpose=coords space=entity")]
	vector m_vSlotB;
	
	bool m_bOccupied = false;
	
	//------------------------------------------------------------------------------------------------
	override void AttachTo(SCR_PowerPole thisPowerPole, SCR_PowerPole otherPowerPole, int index, out vector thisSlot, out vector otherSlot)
	{
		if (vector.Distance(thisPowerPole.CoordToParent(m_vSlotA), otherPowerPole.GetOrigin()) > vector.Distance(thisPowerPole.CoordToParent(m_vSlotB), otherPowerPole.GetOrigin()))
		{
			thisSlot = thisPowerPole.CoordToParent(m_vSlotB);
			otherSlot = otherPowerPole.TryGetSlot(index, thisSlot, true);
		}
		else
		{
			thisSlot = thisPowerPole.CoordToParent(m_vSlotA);
			otherSlot = otherPowerPole.TryGetSlot(index, thisSlot, true);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void DrawDebugShapes(inout array<ref Shape> debugShapes, IEntity parent) 
	{
		Shape shape = Shape.CreateSphere(ARGB(128, 0, 255, 0), ShapeFlags.TRANSP, parent.CoordToParent(m_vSlotA), 0.04);
		debugShapes.Insert(shape);
		shape = Shape.CreateSphere(ARGB(128, 255, 0, 0), ShapeFlags.TRANSP, parent.CoordToParent(m_vSlotB), 0.04);
		debugShapes.Insert(shape);
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_PowerPoleSlotBase : Managed
{
	[Attribute(desc: "Visualised by green sphere.", params: "inf inf 0 purpose=coords space=entity")]
	vector m_vSlotA;
	
	//------------------------------------------------------------------------------------------------
	void AttachTo(SCR_PowerPole thisPowerPole, SCR_PowerPole otherPowerPole, int index, out vector thisSlot, out vector otherSlot)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	void DrawDebugShapes(inout array<ref Shape> debugShapes, IEntity parent)
	{
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_PowerPoleSlotSingle : SCR_PowerPoleSlotBase
{
	//------------------------------------------------------------------------------------------------
	override void AttachTo(SCR_PowerPole thisPowerPole, SCR_PowerPole otherPowerPole, int index, out vector thisSlot, out vector otherSlot)
	{
		thisSlot = thisPowerPole.CoordToParent(m_vSlotA);
		otherSlot = otherPowerPole.TryGetSlot(index, thisSlot, true);
	}
	
	//------------------------------------------------------------------------------------------------
	override void DrawDebugShapes(inout array<ref Shape> debugShapes, IEntity parent) 
	{
		Shape shape = Shape.CreateSphere(ARGB(128, 0, 255, 0), ShapeFlags.TRANSP, parent.CoordToParent(m_vSlotA), 0.04);
		debugShapes.Insert(shape);
	}
};
