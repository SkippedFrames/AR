class SCR_ArrayHelper
{
	//------------------------------------------------------------------------------------------------
	/**
		\brief Given array of weights, get a random index based on the weight values
		i.e in an array [80, 15, 5], 80% of the time, we get index 0, 15% index 1 and 5% index 2, weight sum does not have to add up to any particular value,
		ie [100, 100, 100, 100] gives equal 25% distribution(provided random distribution) for all 4 indices
		\param weights \p array with weights. If empty, the method returns -1
		\param value \p a value between [0..1], can be random, perlin, or any other. if outside these boundaries, will be clamped
		\return \p int index selected based on weight
		@code
			array<float> weights = {};
			weights.Insert(80);
			weights.Insert(15);
			weights.Insert(5);

			float rand01;
			int index;

			rand01 = Math.RandomFloat01();
			index = SCR_ArrayHelper.GetWeightedIndex(weights, rand01);
			Print(index);
			rand01 = Math.RandomFloat01();
			index = SCR_ArrayHelper.GetWeightedIndex(weights, rand01);
			Print(index);
			rand01 = Math.RandomFloat01();
			index = SCR_ArrayHelper.GetWeightedIndex(weights, rand01);
			Print(index);

			>> 'index = 0'
			>> 'index = 1'
			>> 'index = 0'
		@endcode
	*/
	static int GetWeightedIndex(notnull array<float> weights, float value01)
	{
		if (weights.IsEmpty())
			return -1;

		if (value01 < 0 || value01 > 1)
			value01 = Math.Mod(value01, 1);

		float probabilitySum;
		for (int i, count = weights.Count(); i < count; i++)
		{
			probabilitySum += weights[i];
		}

		float add = 0;
		probabilitySum *= value01;

		for (int i, count = weights.Count(); i < count; i++)
		{
			add += weights[i];
			if (add > probabilitySum)
				return i;
		}

		return 0;
	}
}

//! T being the array ITEM class, not the full array<item>
//! In the case of references, simply use SCR_ArrayHelperT<OtherClass>, not SCR_ArrayHelperT<ref OtherClass>
//! Some methods may seem duplicated but they are actually variants for <T> and <ref T>
class SCR_ArrayHelperT<Class T>
{
	//------------------------------------------------------------------------------------------------
	static void CopyReferencesFromTo(notnull array<ref T> source, notnull array<ref T> destination)
	{
		destination.Clear();
		foreach (T sourceRef : source)
		{
			destination.Insert(sourceRef);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Gets all items that are common to the two arrays
	// NATIVE version
	static void Intersect(notnull array<T> array1, notnull array<T> array2, notnull out array<T> result/*, bool unique = true */)
	{
		result.Clear();
		array<T> smallArray;
		array<T> bigArray;
		if (array1.Count() > array2.Count())
		{
			smallArray = array2;
			bigArray = array1;
		}
		else
		{
			bigArray = array1;
			smallArray = array2;
		}

		for (int i = smallArray.Count() - 1; i >= 0; i--)
		{
			if (bigArray.Contains(smallArray[i]))
				result.Insert(smallArray[i]);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Gets all items that are common to the two arrays
	// REFERENCE version
	static void Intersect(notnull array<ref T> array1, notnull array<ref T> array2, notnull out array<ref T> result/*, bool unique = true */)
	{
		result.Clear();
		array<ref T> smallArray;
		array<ref T> bigArray;
		if (array1.Count() > array2.Count())
		{
			smallArray = array2;
			bigArray = array1;
		}
		else
		{
			bigArray = array1;
			smallArray = array2;
		}

		for (int i = smallArray.Count() - 1; i >= 0; i--)
		{
			if (bigArray.Contains(smallArray[i]))
				result.Insert(smallArray[i]);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Reverse item orders
	//! OBSOLETE when array.Reverse appears
	//! @code
	//! array<int> values = { 2, 42, 33, 0 };
	//! SCR_ArrayHelperT<int>.Reverse(values);
	//! Print(values); // outputs { 0, 33, 42, 2 }
	//! @endcode
	// NATIVE version
	static void Reverse(notnull inout array<T> items)
	{
		int itemsCount = items.Count();
		if (itemsCount < 2)
			return;

		int flooredMiddle = itemsCount * 0.5;
		itemsCount--;

		// 4 indices
		// flooredMiddle = 2
		// 0 <-> 3
		// 1 <-> 2
		// { 0 1 2 3 }
		// { 3 2 1 0 }

		// 5 indices
		// flooredMiddle = 2
		// 0 <-> 4
		// 1 <-> 3
		// 2 is central
		// { 0 1 2 3 4 }
		// { 4 3 2 1 0 }

		for (int i; i < flooredMiddle; i++)
		{
			items.SwapItems(i, itemsCount - i);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Reverse item orders
	//! OBSOLETE when array.Reverse appears
	//! @code
	//! array<int> values = { 2, 42, 33, 0 };
	//! SCR_ArrayHelperT<int>.Reverse(values);
	//! Print(values); // outputs { 0, 33, 42, 2 }
	//! @endcode
	// REFERENCE version
	static void Reverse(notnull inout array<ref T> items)
	{
		int itemsCount = items.Count();
		if (itemsCount < 2)
			return;

		int flooredMiddle = itemsCount * 0.5;
		itemsCount--;

		// 4 indices
		// flooredMiddle = 2
		// 0 <-> 3
		// 1 <-> 2
		// { 0 1 2 3 }
		// { 3 2 1 0 }

		// 5 indices
		// flooredMiddle = 2
		// 0 <-> 4
		// 1 <-> 3
		// 2 is central
		// { 0 1 2 3 4 }
		// { 4 3 2 1 0 }

		for (int i; i < flooredMiddle; i++)
		{
			items.SwapItems(i, itemsCount - i);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Shuffle an array
	//! \param items
	//! \param shuffles number of shuffle passes to happen. min 1, max 10
	//! @code
	//! array<int> values = { 1, 2, 3, 4, 5 };
	//! SCR_ArrayHelperT<int>.Shuffle(values);
	//! Print(values); // outputs e.g { 4, 1, 5, 2, 3 }
	//! @endcode
	// NATIVE version
	static void Shuffle(notnull inout array<T> items, int shuffles = 1)
	{
		if (items.Count() < 2)
			return;

		// two checks are faster than Math.ClampInt
		if (shuffles < 1)
			shuffles = 1;

		if (shuffles > 10)
			shuffles = 10;

		while (shuffles > 0)
		{
			for (int i = 0, count = items.Count(); i < count; i++)
			{
				int index1 = Math.RandomInt(0, count);
				int index2 = Math.RandomInt(0, count);
				if (index1 != index2)
					items.SwapItems(index1, index2);
			}

			shuffles--;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Shuffle an array of references
	//! \param items
	//! \param shuffles number of shuffle passes to happen. min 1, max 10
	//! @code
	//! array<ref MyClass> values = { new MyClass(1), new MyClass(2), new MyClass(3), new MyClass(4), new MyClass(5) };
	//! SCR_ArrayHelperT<MyClass>.Shuffle(values);
	//! Print(values); // outputs e.g { MyClass(4), MyClass(1), MyClass(5), MyClass(2), MyClass(3) }
	//! @endcode
	// REFERENCE version
	static void Shuffle(notnull inout array<ref T> items, int shuffles = 1)
	{
		if (items.Count() < 2)
			return;

		// two checks are faster than Math.ClampInt
		if (shuffles < 1)
			shuffles = 1;

		if (shuffles > 10)
			shuffles = 10;

		while (shuffles > 0)
		{
			for (int i = 0, count = items.Count(); i < count; i++)
			{
				int index1 = Math.RandomInt(0, count);
				int index2 = Math.RandomInt(0, count);
				if (index1 != index2)
					items.SwapItems(index1, index2);
			}

			shuffles--;
		}
	}
/*
	//------------------------------------------------------------------------------------------------
	// NATIVE version
	static void ArrayToSet(notnull array<T> toConvert, notnull set<T> result)
	{
		result.Clear();
		foreach (T item : toConvert)
		{
			result.Insert(item);
		}
	}

	//------------------------------------------------------------------------------------------------
	// REFERENCE version
	static void ArrayToSet(notnull array<ref T> toConvert, notnull set<ref T> result)
	{
		result.Clear();
		foreach (T item : toConvert)
		{
			result.Insert(item);
		}
	}

	//------------------------------------------------------------------------------------------------
	// NATIVE version
	static void SetToArray(notnull set<T> toConvert, notnull array<T> result)
	{
		result.Clear();
		foreach (T item : toConvert)
		{
			result.Insert(item);
		}
	}

	//------------------------------------------------------------------------------------------------
	// REFERENCE version
	static void SetToArray(notnull set<ref T> toConvert, notnull array<ref T> result)
	{
		result.Clear();
		foreach (T item : toConvert)
		{
			result.Insert(item);
		}
	}
// */
}
