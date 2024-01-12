class SCR_SortedArray<Class TValue>: Managed
{
	protected ref array<int> m_aOrders = new array<int>();
	protected ref array<TValue> m_aValues = new array<TValue>();
	
	/*!
	\return Value at index [n]
	\param n Index
	\return Value
	*/
	TValue Get(int n)
	{
		return m_aValues.Get(n);
	}
	/*!
	Sets value of element at index [n]
	\param n Index
	\param value New value
	*/
	void Set(int n, TValue value)
	{
		m_aValues.Set(n, value);
	}
	/*!
	Insert new value with given order.
	Values are ordered from lowest to highest.
	\param order Order in array
	\param value Value to be inserted
	*/
	void Insert(int order, TValue value)
	{
		int index = Count();
		for (int i = 0; i < index; i++)
		{
			if (order < m_aOrders[i])
			{
				index = i;
				break;
			}
		}
		m_aOrders.InsertAt(order, index);
		m_aValues.InsertAt(value, index);
	}
	/*!
	Remove value on given index.
	\param Index
	*/
	void Remove(int i)
	{
		m_aOrders.RemoveOrdered(i);
		m_aValues.RemoveOrdered(i);
	}
	/*!
	Remove all entries with given order number.
	\param order Order number to be removed
	*/
	void RemoveOrders(int order)
	{
		for (int i = Count() - 1; i >= 0; i--)
		{
			if (m_aOrders[i] == order)
			{
				m_aOrders.RemoveOrdered(i);
				m_aValues.RemoveOrdered(i);
			}
		}
	}
	/*!
	Remove all entries with given value.
	\param value Value to be removed
	*/
	void RemoveValues(TValue value)
	{
		for (int i = Count() - 1; i >= 0; i--)
		{
			if (m_aValues[i] == value)
			{
				m_aOrders.RemoveOrdered(i);
				m_aValues.RemoveOrdered(i);
			}
		}
	}
	/*!
	\param Index
	\return Order number on given index
	*/
	int GetOrder(int i)
	{
		return m_aOrders[i];
	}
	/*!
	\param Index
	\return Value on given index
	*/
	TValue GetValue(int i)
	{
		return m_aValues[i];
	}
	/*!
	\return Number of elements in the array
	*/
	int Count()
	{
		return m_aOrders.Count();
	}
	/*!
	\return True if the array size is 0, false otherwise
	*/
	bool IsEmpty()
	{
		return m_aOrders.IsEmpty();
	}
	/*!
	\return True if the array contains given value
	*/
	bool Contains(TValue value)
	{
		return m_aValues.Contains(value);
	}
	/*!
	Try to find given value in the array.
	\param value Searched value
	\return Index of first occurance, -1 when not found
	*/
	int Find(TValue value)
	{
		return m_aValues.Find(value);
	}
	/*!
	Copy data from another sorted array.
	\param from Source array
	\return Number of items in array
	*/
	int CopyFrom(notnull SCR_SortedArray<TValue> from)
	{
		Clear();
		int count = from.Count();
		for (int i = 0; i < count; i++)
		{
			m_aOrders.Insert(from.m_aOrders[i]);
			m_aValues.Insert(from.m_aValues[i]);
		}
		return count;
	}
	/*!
	Destroys all elements of the array and sets the Count to 0.
	*/
	void Clear()
	{
		m_aOrders.Clear();
		m_aValues.Clear();
	}
	/*!
	Fills normal array with items in sorted order.
	\param[out] outArray Target array
	\return Number of items
	*/
	int ToArray(out notnull array<TValue> outArray)
	{
		return outArray.Copy(m_aValues);
	}
	/*!
	Print array values to log.
	*/
	void Debug()
	{
		PrintFormat("SCR_SortedArray count: %1", Count());
		for (int i, count = Count(); i < count; i++)
		{
			PrintFormat("[%1] => %2: %3", i, m_aOrders[i], m_aValues[i]);
		}
	}
};
