class SCR_ResourceGridCell : Managed
{
	protected int m_iIndex;
	protected int m_iGridUpdateId = int.MIN;
	protected ref array<SCR_ResourceComponent> m_aStaticItems = {};
	protected ref array<SCR_ResourceComponent> m_aDynamicItems = {};
	
	//------------------------------------------------------------------------------------------------
	int GetGridUpdateId()
	{
		return m_iGridUpdateId;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetIndex()
	{
		return m_iIndex;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetX()
	{
		return m_iIndex >> SCR_ResourceGrid.CELL_POWER;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetZ()
	{
		return m_iIndex & SCR_ResourceGrid.CELL_SIZE_MINUS_1;
	}
	
	//------------------------------------------------------------------------------------------------
	int Get(int axis)
	{
		if (axis == 0)
			return m_iIndex >> SCR_ResourceGrid.CELL_POWER;
		
		if (axis == 2)
			return m_iIndex & SCR_ResourceGrid.CELL_SIZE_MINUS_1;
		
		return 0;
	}
	
	//------------------------------------------------------------------------------------------------
	array<SCR_ResourceComponent> GetResourceDynamicItems()
	{
		//! Sanitization.
		while (m_aDynamicItems.RemoveItem(null));
		
		return m_aDynamicItems;
	}
	
	//------------------------------------------------------------------------------------------------
	array<SCR_ResourceComponent> GetResourceStaticItems()
	{
		//! Sanitization.
		while (m_aStaticItems.RemoveItemOrdered(null));
		
		return m_aStaticItems;
	}
	
	//------------------------------------------------------------------------------------------------
	/*
	array<SCR_ResourceGridSubscription> GetSubscriptions()
	{
		return m_aSubscriptions;
	}
	*/
	
	//------------------------------------------------------------------------------------------------
	void SetGridUpdateId(int gridUpdateId)
	{
		if (gridUpdateId <= m_iGridUpdateId)
			return;
		
		m_iGridUpdateId = gridUpdateId;
	}
	
	//------------------------------------------------------------------------------------------------
	int RegisterDynamicItem(notnull SCR_ResourceComponent item, int gridUpdateId)
	{
		if (gridUpdateId > m_iGridUpdateId)
			m_iGridUpdateId = gridUpdateId;
		
		return m_aDynamicItems.Insert(item);
	}
	
	//------------------------------------------------------------------------------------------------
	bool UnregisterDynamicItem(notnull SCR_ResourceComponent item, int gridUpdateId)
	{
		if (gridUpdateId > m_iGridUpdateId)
			m_iGridUpdateId = gridUpdateId;
		
		return m_aDynamicItems.RemoveItem(item);
	}
	
	//------------------------------------------------------------------------------------------------
	int RegisterStaticItem(notnull SCR_ResourceComponent item, int gridUpdateId)
	{
		if (gridUpdateId > m_iGridUpdateId)
			m_iGridUpdateId = gridUpdateId;
		
		return m_aStaticItems.Insert(item);
	}
	
	//------------------------------------------------------------------------------------------------
	bool UnregisterStaticItem(notnull SCR_ResourceComponent item, int gridUpdateId)
	{
		if (gridUpdateId > m_iGridUpdateId)
			m_iGridUpdateId = gridUpdateId;
		
		return m_aStaticItems.RemoveItemOrdered(item);
	}
	
	//------------------------------------------------------------------------------------------------
	void MoveStaticItem(notnull SCR_ResourceComponent item, int gridUpdateId)
	{
		if (gridUpdateId > m_iGridUpdateId)
			m_iGridUpdateId = gridUpdateId;
		
		m_aStaticItems.RemoveItemOrdered(item);
		m_aStaticItems.Insert(item);
	}
	
	//------------------------------------------------------------------------------------------------
	void MoveDynamicItem(notnull SCR_ResourceComponent item, int gridUpdateId)
	{
		if (gridUpdateId > m_iGridUpdateId)
			m_iGridUpdateId = gridUpdateId;
	}
	
	/*
	//------------------------------------------------------------------------------------------------
	int Subscribe(notnull SCR_ResourceGridSubscription subscription)
	{
		return m_aSubscriptions.Insert(subscription);
	}
	
	//------------------------------------------------------------------------------------------------
	bool Unsubscribe(notnull SCR_ResourceGridSubscription subscription)
	{
		return m_aSubscriptions.RemoveItem(subscription);
	}
	*/
	
	//------------------------------------------------------------------------------------------------
	void PromoteResourceItemToDynamic(notnull SCR_ResourceComponent item)
	{
		m_aStaticItems.RemoveItemOrdered(item);
		m_aDynamicItems.Insert(item);
	}
	
	//------------------------------------------------------------------------------------------------
	void PromoteResourceItemToStatic(notnull SCR_ResourceComponent item)
	{
		m_aDynamicItems.RemoveItem(item);
		m_aStaticItems.Insert(item);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_ResourceGridCell(int index)
	{
		m_iIndex = index;
	}
}

class SCR_ResourceGrid
{
	static const int MAX_FRAME_BUDGET	= 20;
	static const int CELL_POWER			= 4;
	static const int CELL_SIZE_MINUS_1	= (1 << CELL_POWER) - 1;
	
	protected ref map<int, ref SCR_ResourceGridCell> m_Cells;
	protected ref array<SCR_ResourceComponent> m_aFlaggedItems = {};
	protected ref set<SCR_ResourceContainer> m_aQueriedContainers = new set<SCR_ResourceContainer>();
	protected int m_iGridUpdateId = int.MIN;
	protected int m_iGridPerimeter;
	protected int m_iGridPerimeterMinusOne;
	protected int m_iFrameBudget;
	
	//------------------------------------------------------------------------------------------------
	int GetGridUpdateId()
	{
		return m_iGridUpdateId;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetFrameBudget()
	{
		return m_iFrameBudget;
	}
	
	//------------------------------------------------------------------------------------------------
	void ResetFrameBudget()
	{
		m_iFrameBudget = SCR_ResourceGrid.MAX_FRAME_BUDGET;
	}
	
	//------------------------------------------------------------------------------------------------
	int ComputeIndex(int x, int z)
	{
		return x | z << CELL_POWER;
	}
	
	//------------------------------------------------------------------------------------------------
	int ComputeIndex(int x, int y, int z)
	{
		return x | z << CELL_POWER;
	}
	
	//------------------------------------------------------------------------------------------------
	int ComputeIndex(vector position)
	{
		return	((int)position[0] >> CELL_POWER) | ((int)position[2] >> CELL_POWER << CELL_POWER);
	}
	
	//------------------------------------------------------------------------------------------------
	int IncreaseGridUpdateId()
	{
		if (++m_iGridUpdateId == int.MAX)
			Debug.Error2("SCR_ResourceGrid", "Maximum grid update id has been reached.");
		
		return m_iGridUpdateId;
	}
	
	//------------------------------------------------------------------------------------------------
	void FlagResourceItem(notnull SCR_ResourceComponent item)
	{
		m_aFlaggedItems.Insert(item);
	}
	
	//------------------------------------------------------------------------------------------------
	void UnflagResourceItem(notnull SCR_ResourceComponent item)
	{
		m_aFlaggedItems.RemoveItem(item);
	}
	
	//------------------------------------------------------------------------------------------------
	void ProcessFlaggedItems()
	{
		if (m_aFlaggedItems.IsEmpty())
			return;
		
		IncreaseGridUpdateId();
		
		foreach (SCR_ResourceComponent item: m_aFlaggedItems)
		{
			if (item)
				ProcessResourceItem(item);
		}
		
		m_aFlaggedItems.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateResourceStaticItem(notnull SCR_ResourceComponent item)
	{
		int previousMins, previousMaxs;
		
		//! Get the previous grid bounds.
		item.GetGridContainersBounds(previousMins, previousMaxs);
		
		//! Cancel the operation if any of the previous grid bounds is invalid.
		if (previousMins == 0xFFFFFFFF || previousMaxs == 0xFFFFFFFF)
			return;
		
		//! Extract grid bounds's items from the previous grid bounds.
		int prevMinX = previousMins & CELL_SIZE_MINUS_1;
		int prevMinY = previousMins >> CELL_POWER;
		int prevMaxX = previousMaxs & CELL_SIZE_MINUS_1;
		int prevMaxY = previousMaxs >> CELL_POWER;
		
		vector boundsMins, boundsMaxs;
		
		item.GetGridContainersWorldBoundingBox(boundsMins, boundsMaxs);
			
		int minX = (int)boundsMins[0] >> CELL_POWER;
		int minY = (int)boundsMins[2] >> CELL_POWER;
		int maxX = (int)boundsMaxs[0] >> CELL_POWER;
		int maxY = (int)boundsMaxs[2] >> CELL_POWER;
		
		SCR_ResourceGridCell gridCell;
		
		for (int y = prevMinY; y <= prevMaxY; ++y)
		{
			for (int x = prevMinX; x <= prevMaxX; ++x)
			{
				gridCell = m_Cells[y << CELL_POWER | x];
				
				if (!gridCell)
					continue;
				
				if ((x >= minX && x <= maxX) && (y >= minY && y <= maxY))
					gridCell.MoveStaticItem(item, m_iGridUpdateId);
				else
					gridCell.UnregisterStaticItem(item, m_iGridUpdateId);
			}
		}
		
		int currentCellPosition;
		
		for (int y = minY; y <= maxY; ++y)
		{
			for (int x = minX; x <= maxX; ++x)
			{
				if (x >= prevMinX && x <= prevMaxX && y >= prevMinY && y <= prevMaxY)
				{
					x = prevMaxX;
					
					continue;
				}
				
				currentCellPosition = y << CELL_POWER | x;
				gridCell = m_Cells[currentCellPosition];
				
				if (!gridCell)
					m_Cells[currentCellPosition] = new SCR_ResourceGridCell(currentCellPosition);
				
				m_Cells[currentCellPosition].RegisterStaticItem(item, m_iGridUpdateId);
			}
		}
		
		item.SetGridContainersBounds(minY << CELL_POWER | minX, maxY << CELL_POWER | maxX);
		item.SetGridUpdateId(m_iGridUpdateId);
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateResourceDynamicItem(notnull SCR_ResourceComponent item)
	{
		int previousMins, previousMaxs;
		
		//! Get the previous grid bounds.
		item.GetGridContainersBounds(previousMins, previousMaxs);
		
		//! Cancel the operation if any of the previous grid bounds is invalid.
		if (previousMins == 0xFFFFFFFF || previousMaxs == 0xFFFFFFFF)
			return;
		
		//! Extract grid bounds's items from the previous grid bounds.
		int prevMinX = previousMins & CELL_SIZE_MINUS_1;
		int prevMinY = previousMins >> CELL_POWER;
		int prevMaxX = previousMaxs & CELL_SIZE_MINUS_1;
		int prevMaxY = previousMaxs >> CELL_POWER;
		
		vector boundsMins, boundsMaxs;
		
		item.GetGridContainersWorldBoundingBox(boundsMins, boundsMaxs);
			
		int minX = (int)boundsMins[0] >> CELL_POWER;
		int minY = (int)boundsMins[2] >> CELL_POWER;
		int maxX = (int)boundsMaxs[0] >> CELL_POWER;
		int maxY = (int)boundsMaxs[2] >> CELL_POWER;
		
		SCR_ResourceGridCell gridCell;
		
		for (int y = prevMinY; y <= prevMaxY; ++y)
		{
			for (int x = prevMinX; x <= prevMaxX; ++x)
			{
				gridCell = m_Cells[y << CELL_POWER | x];
				
				if (!gridCell)
					continue;
				
				if (x >= minX && x <= maxX && y >= minY && y <= maxY)
					gridCell.MoveDynamicItem(item, m_iGridUpdateId);
				else
					gridCell.UnregisterDynamicItem(item, m_iGridUpdateId);
			}
		}
		
		int currentCellPosition;
		
		for (int y = minY; y <= maxY; ++y)
		{
			for (int x = minX; x <= maxX; ++x)
			{
				if (x >= prevMinX && x <= prevMaxX && y >= prevMinY && y <= prevMaxY)
				{
					x = prevMaxX;
					
					continue;
				}
				
				currentCellPosition = y << CELL_POWER | x;
				gridCell = m_Cells[currentCellPosition];
				
				if (!gridCell)
					m_Cells[currentCellPosition] = new SCR_ResourceGridCell(currentCellPosition);
				
				m_Cells[currentCellPosition].RegisterDynamicItem(item, m_iGridUpdateId);
			}
		}
		
		item.SetGridContainersBounds(minY << CELL_POWER | minX, maxY << CELL_POWER | maxX);
		item.SetGridUpdateId(m_iGridUpdateId);
	}
	
	//------------------------------------------------------------------------------------------------
	void PromoteResourceItemToDynamic(notnull SCR_ResourceComponent item)
	{
		array<SCR_ResourceGridCell> cells = {};
		int mins, maxs;
		
		item.GetGridContainersBounds(mins, maxs);
		QueryGridCells(mins, maxs, cells);
		
		foreach (SCR_ResourceGridCell cell: cells)
		{
			cell.PromoteResourceItemToDynamic(item);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void PromoteResourceItemToStatic(notnull SCR_ResourceComponent item)
	{
		array<SCR_ResourceGridCell> cells = {};
		int mins, maxs;
		
		item.GetGridContainersBounds(mins, maxs);
		QueryGridCells(mins, maxs, cells);
		
		foreach (SCR_ResourceGridCell cell: cells)
		{
			cell.PromoteResourceItemToStatic(item);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void QueryGridCells(int mins, int maxs, inout notnull array<SCR_ResourceGridCell> gridCells)
	{
		//! Extract grid bounds's components from the previous grid bounds.
		int minX = mins & CELL_SIZE_MINUS_1;
		int minY = mins >> CELL_POWER;
		int maxX = maxs & CELL_SIZE_MINUS_1;
		int maxY = maxs >> CELL_POWER;
		
		SCR_ResourceGridCell gridCell;
		
		for (int y = minY; y <= maxY; ++y)
		{
			for (int x = minX; x <= maxX; ++x)
			{
				gridCell = m_Cells[y << CELL_POWER | x];
				
				if (!gridCell)
					continue;
				
				gridCells.Insert(gridCell);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void QueryContainers(vector boundsMins, vector boundsMaxs, EResourceType resourceType)
	{
		//! Extract grid bounds's components from the previous grid bounds.
		int minX = (int)boundsMins[0] >> CELL_POWER;
		int minY = (int)boundsMins[2] >> CELL_POWER;
		int maxX = (int)boundsMaxs[0] >> CELL_POWER;
		int maxY = (int)boundsMaxs[2] >> CELL_POWER;
		
		SCR_ResourceGridCell gridCell;
		
		for (int y = minY; y <= maxY; ++y)
		{
			for (int x = minX; x <= maxX; ++x)
			{
				gridCell = m_Cells[y << CELL_POWER | x];
			
				if (!gridCell)
					continue;
				
				foreach (SCR_ResourceComponent item: gridCell.GetResourceStaticItems())
				{
					m_aQueriedContainers.Insert(item.GetContainer(resourceType));
				}
				
				foreach (SCR_ResourceComponent item: gridCell.GetResourceDynamicItems())
				{
					m_aQueriedContainers.Insert(item.GetContainer(resourceType));
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void QueryContainers(int mins, int maxs, EResourceType resourceType)
	{
		//! Extract grid bounds's components from the previous grid bounds.
		int minX = mins & CELL_SIZE_MINUS_1;
		int minY = mins >> CELL_POWER;
		int maxX = maxs & CELL_SIZE_MINUS_1;
		int maxY = maxs >> CELL_POWER;
		
		SCR_ResourceGridCell gridCell;
		
		for (int y = minY; y <= maxY; ++y)
		{
			for (int x = minX; x <= maxX; ++x)
			{
				gridCell = m_Cells[y << CELL_POWER | x];
			
				if (!gridCell)
					continue;
				
				foreach (SCR_ResourceComponent item: gridCell.GetResourceStaticItems())
				{
					m_aQueriedContainers.Insert(item.GetContainer(resourceType));
				}
				
				foreach (SCR_ResourceComponent item: gridCell.GetResourceDynamicItems())
				{
					m_aQueriedContainers.Insert(item.GetContainer(resourceType));
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void QueryContainers(vector boundsMins, vector boundsMaxs, EResourceType resourceType, int gridUpdateId)
	{	
		//! Extract grid bounds's components from the previous grid bounds.
		int minX = (int)boundsMins[0] >> CELL_POWER;
		int minY = (int)boundsMins[2] >> CELL_POWER;
		int maxX = (int)boundsMaxs[0] >> CELL_POWER;
		int maxY = (int)boundsMaxs[2] >> CELL_POWER;
		
		SCR_ResourceGridCell gridCell;
		array<SCR_ResourceComponent> gridCellItems;
		SCR_ResourceComponent item;
		
		for (int y = minY; y <= maxY; ++y)
		{
			for (int x = minX; x <= maxX; ++x)
			{
				gridCell = m_Cells[y << CELL_POWER | x];
			
				if (!gridCell || gridCell.GetGridUpdateId() <= gridUpdateId)
					continue;
				
				gridCellItems = gridCell.GetResourceStaticItems();
				
				for (int idx = gridCellItems.Count() - 1; idx >= 0; --idx)
				{
					item = gridCellItems[idx];
					
					if (item.IsGridUpdateIdGreaterThan(gridUpdateId))
						m_aQueriedContainers.Insert(item.GetContainer(resourceType));
					else
						break;
				}
				
				gridCellItems = gridCell.GetResourceDynamicItems();
				
				for (int idx = gridCellItems.Count() - 1; idx >= 0; --idx)
				{
					item = gridCellItems[idx];
					
					if (item.IsGridUpdateIdGreaterThan(gridUpdateId))
						m_aQueriedContainers.Insert(item.GetContainer(resourceType));
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void QueryContainers(int mins, int maxs, EResourceType resourceType, int gridUpdateId)
	{	
		//! Extract grid bounds's components from the previous grid bounds.
		int minX = mins & CELL_SIZE_MINUS_1;
		int minY = mins >> CELL_POWER;
		int maxX = maxs & CELL_SIZE_MINUS_1;
		int maxY = maxs >> CELL_POWER;
		
		SCR_ResourceGridCell gridCell;
		array<SCR_ResourceComponent> gridCellItems;
		SCR_ResourceComponent item;
		
		for (int y = minY; y <= maxY; ++y)
		{
			for (int x = minX; x <= maxX; ++x)
			{
				gridCell = m_Cells[y << CELL_POWER | x];
			
				if (!gridCell || gridCell.GetGridUpdateId() <= gridUpdateId)
					continue;
				
				gridCellItems = gridCell.GetResourceStaticItems();
				
				for (int idx = gridCellItems.Count() - 1; idx >= 0; --idx)
				{
					item = gridCellItems[idx];
					
					if (item.IsGridUpdateIdGreaterThan(gridUpdateId))
						m_aQueriedContainers.Insert(item.GetContainer(resourceType));
					else
						break;
				}
				
				gridCellItems = gridCell.GetResourceDynamicItems();
				
				for (int idx = gridCellItems.Count() - 1; idx >= 0; --idx)
				{
					item = gridCellItems[idx];
					
					if (item.IsGridUpdateIdGreaterThan(gridUpdateId))
						m_aQueriedContainers.Insert(item.GetContainer(resourceType));
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateInteractor(notnull SCR_ResourceInteractor interactor, bool useFrameBudget = false)
	{	
		int gridUpdateId = interactor.GetGridUpdateId();
		vector interactorOrigin	= interactor.GetOwnerOrigin();
		SCR_ResourceComponent interactorItem = interactor.GetComponent();
		bool hasInteractorMoved = vector.DistanceSq(interactorOrigin, interactor.GetLastPosition()) > SCR_ResourceComponent.UPDATE_DISTANCE_TRESHOLD_SQUARE;
		
		if (!hasInteractorMoved && gridUpdateId == m_iGridUpdateId || interactor.IsIsolated())
			return;
		
		float resourceGridRange	= interactor.GetResourceGridRange();
		vector boundsOffset		= vector.One * resourceGridRange;
		EResourceType resourceType = interactor.GetResourceType();
		SCR_ResourceContainerQueueBase containerQueue = interactor.GetContainerQueue();
		SCR_ResourceContainer container;
		
		if (hasInteractorMoved)
		{
			QueryContainers(interactorOrigin - boundsOffset, interactorOrigin + boundsOffset, resourceType);
			
			for (int idx = containerQueue.GetContainerCount() - 1; idx >= 0; --idx)
			{
				container = containerQueue.GetContainerAt(idx);
				
				if (!container || container.IsIsolated())
					continue;
				
				m_aQueriedContainers.Insert(container);
			}
		}
		else
		{
			QueryContainers(interactorOrigin - boundsOffset, interactorOrigin + boundsOffset, resourceType, gridUpdateId);
			
			for (int idx = containerQueue.GetContainerCount() - 1; idx >= 0; --idx)
			{
				container = containerQueue.GetContainerAt(idx);
				
				if (!container || container.IsIsolated())
					continue;
				
				if (container.IsGridUpdateIdGreaterThan(gridUpdateId))
					m_aQueriedContainers.Insert(container);
			}
		}
		
		m_aQueriedContainers.RemoveItem(null);
		
		int containerCount	= m_aQueriedContainers.Count();
		
		if (useFrameBudget)
			m_iFrameBudget	-= containerCount;
		
		for (int idx = containerCount - 1; idx >= 0; --idx)
		{
			container = m_aQueriedContainers[idx];
			
			if (interactor.CanInteractWith(container) && container.IsInRange(interactorOrigin, resourceGridRange))
			{
				if (!container.IsInteractorLinked(interactor))
					interactor.RegisterContainerForced(container);
			}
			else
				interactor.UnregisterContainer(container);
		}
		
		interactor.SetGridUpdateId(m_iGridUpdateId);
		interactor.UpdateLastPosition();
		m_aQueriedContainers.Clear();
		interactorItem.Replicate();
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	NOTE: Expensive, used only for specific cases and/or debugging.
	*/
	array<SCR_ResourceGenerator> QueryGenerators(float range, vector origin)
	{
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	NOTE: Expensive, used only for specific cases and/or debugging.
	*/
	array<SCR_ResourceGenerator> QueryGenerators(vector mins, vector maxs, vector transform[4])
	{
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	bool ProcessResourceItem(notnull SCR_ResourceComponent item)
	{
		//! TODO: Remove this hack.
		if (item.GetOwner().GetOrigin() == vector.Zero)
			return false;
		
		vector boundsMins, boundsMaxs;
		
		item.GetGridContainersWorldBoundingBox(boundsMins, boundsMaxs);
			
		SCR_ResourceGridCell currentCell;
		int currentCellPosition;
		int minX = (int)boundsMins[0] >> CELL_POWER;
		int minY = (int)boundsMins[2] >> CELL_POWER;
		int maxX = (int)boundsMaxs[0] >> CELL_POWER;
		int maxY = (int)boundsMaxs[2] >> CELL_POWER;
		
		for (int y = minY; y <= maxY; ++y)
		{
			for (int x = minX; x <= maxX; ++x)
			{
				currentCellPosition = y << CELL_POWER | x;
				currentCell = m_Cells[currentCellPosition];
				
				if (!currentCell)
				{
					m_Cells[currentCellPosition] = new SCR_ResourceGridCell(currentCellPosition);
					currentCell = m_Cells[currentCellPosition];
				}
				
				currentCell.RegisterStaticItem(item, m_iGridUpdateId);
			}
		}
		
		item.SetGridContainersBounds(minY << CELL_POWER | minX, maxY << CELL_POWER | maxX);
		item.SetGridUpdateId(m_iGridUpdateId);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void UnregisterResourceItem(notnull SCR_ResourceComponent item)
	{
		SCR_ResourceGridCell gridCell;
		int previousMins, previousMaxs;
		
		//! Get the previous grid bounds.
		item.GetGridContainersBounds(previousMins, previousMaxs);
		
		//! Cancel the operation if any of the previous grid bounds is invalid.
		if (previousMins == 0xFFFFFFFF || previousMaxs == 0xFFFFFFFF)
			return;
		
		//! Extract grid bounds's items from the previous grid bounds.
		int prevMinX = previousMins & CELL_SIZE_MINUS_1;
		int prevMinY = previousMins >> CELL_POWER;
		int prevMaxX = previousMaxs & CELL_SIZE_MINUS_1;
		int prevMaxY = previousMaxs >> CELL_POWER;
		
		//! Unregister item from every grid cell.
		for (int y = prevMinY; y <= prevMaxY; ++y)
		{
			for (int x = prevMinX; x <= prevMaxX; ++x)
			{
				gridCell = m_Cells[y << CELL_POWER | x];
				
				if (!gridCell)
					continue;
				
				if (!gridCell.UnregisterStaticItem(item, m_iGridUpdateId))
					gridCell.UnregisterDynamicItem(item, m_iGridUpdateId)
			}
		}
		
		//! Set the item grid bounds to be invalid.
		item.SetGridContainersBounds(0xFFFFFFFF, 0xFFFFFFFF);
		item.SetGridUpdateId(int.MIN);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_ResourceGrid()
	{
		vector mins; 
		vector maxs; 
		vector coordinateSizes;	
		BaseWorld world = GetGame().GetWorld();
		m_Cells = new map<int, ref SCR_ResourceGridCell>();
		
		if (world)
		{
			world.GetBoundBox(mins, maxs);
			
			coordinateSizes		= maxs - mins;
			m_iGridPerimeter	= Math.Max(coordinateSizes[0], coordinateSizes[2]);
			m_iGridPerimeter	= (m_iGridPerimeter >> CELL_POWER) + 1;
		}
		else
			m_iGridPerimeter = (32000 >> CELL_POWER) + 1;
		
		m_iGridPerimeterMinusOne = m_iGridPerimeter - 1;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_ResourceGrid()
	{		
		m_Cells.Clear();
	}
}
