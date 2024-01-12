[EntityEditorProps(category: "GameScripted/Power", description: "This is the power pole entity.", color: "0 255 0 255", visible: false, dynamicBox: true)]
class SCR_BranchedJunctionPowerPoleClass: SCR_JunctionPowerPoleClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_BranchedJunctionPowerPole : SCR_JunctionPowerPole
{
	[Attribute("3")]
	protected int m_iBranchSize;

	//------------------------------------------------------------------------------------------------
	//Only for when sameLine == false
	protected int GetClosestBranch(vector point)
	{
		int slotsCount = m_aJunctionSlots.Count();
		int repetitionCount = Math.Floor(slotsCount / m_iBranchSize);
		int selectedBranch = 0;

		float closestDistance = float.MAX;

		float currentDistance;
		for (int i = slotsCount - 1; i >= 0; i--)
		{
			currentDistance += vector.DistanceSq(CoordToParent(m_aJunctionSlots[i].m_vSlotA), point);
			if (i % m_iBranchSize == 0)
			{
				if (currentDistance < closestDistance)
				{
					closestDistance = currentDistance;
					selectedBranch = Math.Floor(i / m_iBranchSize);
				}

				currentDistance = 0;
			}
		}
		/*for (int i = 1; i <= repetitionCount; i++)
		{
			float currentDistance;
			for (int j = 0; j < m_iBranchSize; j++)
			{
				currentDistance += vector.DistanceSq(m_aJunctionSlots[i * m_iBranchSize - j].m_vSlotA, point);
			}

			currentDistance /= m_iBranchSize;
			if (currentDistance < closestDistance)
				selectedBranch = i;
		}*/

		return selectedBranch;
	}

	//------------------------------------------------------------------------------------------------
	override vector TryGetSlot(int index, vector otherSlot, bool sameLine)
	{
		if (sameLine)
			return super.TryGetSlot(index, otherSlot, sameLine);

		index += GetClosestBranch(otherSlot) * m_iBranchSize;

		if (index >= m_aJunctionSlots.Count())
		{
			return m_aJunctionSlots[index % m_aJunctionSlots.Count()].m_vSlotA;
			return vector.Zero;
		}

		SCR_PowerPoleSlot dualSlot = SCR_PowerPoleSlot.Cast(m_aJunctionSlots[index]);
		if (dualSlot)
		{
			if (vector.Distance(otherSlot, CoordToParent(dualSlot.m_vSlotA)) > vector.Distance(otherSlot, CoordToParent(dualSlot.m_vSlotB)))
				return CoordToParent(dualSlot.m_vSlotB);
			else
				return CoordToParent(dualSlot.m_vSlotA);
		}

		SCR_PowerPoleSlotSingle singleSlot = SCR_PowerPoleSlotSingle.Cast(m_aJunctionSlots[index]);
		if (singleSlot)
			return CoordToParent(singleSlot.m_vSlotA);

		return vector.Zero;
	}

	//------------------------------------------------------------------------------------------------
	void SCR_BranchedJunctionPowerPole(IEntitySource src, IEntity parent)
	{
	}
};
