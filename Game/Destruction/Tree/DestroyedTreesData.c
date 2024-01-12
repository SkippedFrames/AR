class SCR_DestroyedTreesData : Managed
{
	EntityID treeID;
	ref array<int> m_aDestroyedPartsIndexes = new ref array<int>();
	ref array<vector> m_aPositions = new ref array<vector>();
	ref array<float> m_aRotations = new ref array<float>();
	ref array<bool> m_aIsTreePartDynamic = new ref array<bool>();
	ref array<bool> m_aIsParented = new ref array<bool>();
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_DestroyedTreesData()
	{
		m_aDestroyedPartsIndexes = null;
		m_aPositions = null;
		m_aRotations = null;
		m_aIsTreePartDynamic = null;
		m_aIsParented = null;
	}
};
