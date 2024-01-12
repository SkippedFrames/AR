[EntityEditorProps(category: "GameScripted/Power", description: "This is the power pole entity.", color: "0 255 0 255", visible: false, dynamicBox: true)]
class SCR_JunctionPowerPoleClass: SCR_PowerPoleClass
{
}

//------------------------------------------------------------------------------------------------
class SCR_JunctionPowerPole : SCR_PowerPole
{
	[Attribute(desc: "Slots for connecting with other power poles in a junction", category: "Power Cable Slots")]
	protected ref array<ref SCR_PowerPoleSlotBase> m_aJunctionSlots;

	//------------------------------------------------------------------------------------------------
	override void DrawDebugShapes()
	{
		super.DrawDebugShapes();

		if (!m_bDrawDebugShapes)
			return;

		foreach (SCR_PowerPoleSlotBase slot: m_aJunctionSlots)
		{
			slot.DrawDebugShapes(m_aDebugShapes, this);
		}
	}

	//------------------------------------------------------------------------------------------------
	override vector GetSlot(int index, bool sameLine)
	{
		if (sameLine)
			return super.GetSlot(index, sameLine);
		else
			return m_aJunctionSlots[index].m_vSlotA;
	}

	//------------------------------------------------------------------------------------------------
	override int GetSlotsCount(bool sameLine = true)
	{
		if (sameLine)
			return m_aSlots.Count();
		else
			return m_aJunctionSlots.Count();
	}

	//------------------------------------------------------------------------------------------------
	override vector TryGetSlot(int index, vector otherSlot, bool sameLine)
	{
		if (sameLine)
			return super.TryGetSlot(index, otherSlot, sameLine);

		int junctionCount = m_aJunctionSlots.Count();
		if (index >= junctionCount)
			index = index % junctionCount;

		SCR_PowerPoleSlot dualSlot = SCR_PowerPoleSlot.Cast(m_aJunctionSlots[index]);
		if (dualSlot)
		{
			vector avgSideA;
			vector avgSideB;
			SCR_PowerPoleSlot powerPoleJunction;
			for (int i; i < junctionCount; i++)
			{
				avgSideA += m_aJunctionSlots[i].m_vSlotA;
				powerPoleJunction = SCR_PowerPoleSlot.Cast(m_aJunctionSlots[i]);
				if (powerPoleJunction)
					avgSideB += powerPoleJunction.m_vSlotB;
			}

			avgSideA /= (float)junctionCount;
			avgSideB /= (float)junctionCount;

			if (avgSideB == vector.Zero || vector.Distance(otherSlot, CoordToParent(avgSideA)) <= vector.Distance(otherSlot, CoordToParent(avgSideB)))
				return CoordToParent(dualSlot.m_vSlotA);
			else
				return CoordToParent(dualSlot.m_vSlotB);
		}

		SCR_PowerPoleSlotSingle singleSlot = SCR_PowerPoleSlotSingle.Cast(m_aJunctionSlots[index]);
		if (singleSlot)
			return CoordToParent(singleSlot.m_vSlotA);

		return vector.Zero;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_JunctionPowerPole(IEntitySource src, IEntity parent)
	{
	}
}
