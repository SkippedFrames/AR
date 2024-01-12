[EntityEditorProps(category: "GameScripted/Power", description: "This is the power pole entity.", color: "0 255 0 255", visible: false, dynamicBox: true)]
class SCR_PowerPoleClass: PowerPoleEntityClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_PowerPole : PowerPoleEntity
{
	[Attribute(desc: "Slots for connecting with other power poles", category: "Power Cable Slots")]
	protected ref array<ref SCR_PowerPoleSlotBase> m_aSlots;

	[Attribute(desc: "Draw debug shapes?", category: "Debug")]
	protected bool m_bDrawDebugShapes;

	protected ref array<ref Shape> m_aDebugShapes = {};
	protected IEntitySource m_Source;

	//------------------------------------------------------------------------------------------------
	//! \param sameLine is used by SCR_JunctionPowerPole.GetSlotsCount
	int GetSlotsCount(bool sameLine = true)
	{
		return m_aSlots.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! \param sameLine is used by SCR_JunctionPowerPole.GetSlot
	vector GetSlot(int index, bool sameLine)
	{
		if (!m_aSlots.IsIndexValid(index))
			return vector.Zero;

		return m_aSlots[index].m_vSlotA;
	}

	//------------------------------------------------------------------------------------------------
	//! \Tries to find position of the closest compatible slot.
	//! \return vector with world origin of the closest compatible slot.
	//! \param index is the index of the other slot, this method tries to find the slot under the same index.
	//! \param otherSlot is the local position of other slot.
	//! \param sameLine is used by SCR_JunctionPowerPole.TryGetSlot
	vector TryGetSlot(int index, vector otherSlot, bool sameLine)
	{
		if (index < 0)
			return vector.Zero;

		if (index >= m_aSlots.Count())
			return m_aSlots[index % m_aSlots.Count()].m_vSlotA;

		SCR_PowerPoleSlot dualSlot = SCR_PowerPoleSlot.Cast(m_aSlots[index]);
		if (dualSlot)
		{
			if (vector.Distance(otherSlot, CoordToParent(dualSlot.m_vSlotA)) > vector.Distance(otherSlot, CoordToParent(dualSlot.m_vSlotB)))
				return CoordToParent(dualSlot.m_vSlotB);
			else
				return CoordToParent(dualSlot.m_vSlotA);
		}

		SCR_PowerPoleSlotSingle singleSlot = SCR_PowerPoleSlotSingle.Cast(m_aSlots[index]);
		if (singleSlot)
			return CoordToParent(singleSlot.m_vSlotA);

		return vector.Zero;
	}

	#ifdef WORKBENCH
	//-----------------------------------------------------------------------
	override bool _WB_OnKeyChanged(BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		DrawDebugShapes();
		return false;
	}
	#endif

	//------------------------------------------------------------------------------------------------
	protected void DrawDebugShapes()
	{
		m_aDebugShapes.Clear();
		if (!m_bDrawDebugShapes)
			return;

		foreach (SCR_PowerPoleSlotBase slot: m_aSlots)
		{
			slot.DrawDebugShapes(m_aDebugShapes, this);
		}
	}

	//------------------------------------------------------------------------------------------------
	void SCR_PowerPole(IEntitySource src, IEntity parent)
	{
		DrawDebugShapes();

		m_Source = src;
		SetEventMask(EntityEvent.INIT);
	}
};
