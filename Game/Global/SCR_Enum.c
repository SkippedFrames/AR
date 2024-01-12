class SCR_Enum
{
	//------------------------------------------------------------------------------------------------
	/*!
	Check if flags enum contains all required flags.
	\param flags List of flags
	\param condition Queried flag
	\return True if flags contains required flags
	*/
	static bool HasFlag(int flags, int condition)
	{
		return (flags & condition) == condition;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Check if flags enum contains at least one required flag
	\param flags List of flags
	\param condition Queried flag
	\return True if flags contains one of required flags
	*/
	static bool HasPartialFlag(int flags, int condition)
	{
		return (condition & ~flags) != condition;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Adds the given flag to Flags
	\param flags List of flags
	\param flagToSet Flag to add
	\return New flags value
	*/
	static int SetFlag(int flags, int flagToSet)
	{
		flags |= flagToSet;
		return flags;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Removes the given flag to Flags
	\param flags List of flags
	\param flagToRemove Flag to remove
	\return New flags value
	*/
	static int RemoveFlag(int flags, int flagToRemove)
	{
		flags &= ~flagToRemove;
		return flags;
	}

	//------------------------------------------------------------------------------------------------
	//! Convert bit array to integer array
	//! \param Bit value
	//! \param[out] intValues Array of integer values
	//! \return Size of the array
	static int BitToIntArray(int bitValue, out notnull array<int> intValues)
	{
		intValues.Clear();
		int count = 0;
		int i = 1;
		while (i > 0 && i < int.MAX)
		{
			if (bitValue & i)
			{
				intValues.Insert(i);
				count++;
			}
			i *= 2;
		}
		return count;
	}

	//------------------------------------------------------------------------------------------------
	//! Convert integer array to bit array
	//! \param[out] intValues Array of integer values
	//! \return Bit value
	static int IntToBitArray(notnull array<int> intValues)
	{
		int bitValue;
		foreach (int intValue: intValues)
		{
			bitValue |= intValue;
		}
		return bitValue;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get string value of an enum
	\param enumType enum type
	\param enumValue enum value
	\return string enum name
	*/
	static string GetEnumName(typename enumType, int enumValue)
	{
		return typename.EnumToString(enumType, enumValue);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get all value names within an enum.
	\param enumType Type of the enum
	\param[out] stringValues Array of enum values
	\return Size of the array
	*/
	static int GetEnumNames(typename enumType, out notnull array<string> stringValues)
	{
		stringValues.Clear();
		array<int> values = {};
		GetEnumValues(enumType, values);
		for (int i, count = values.Count(); i < count; i++)
		{
			stringValues.Insert(typename.EnumToString(enumType, values[i]));
		}
		return stringValues.Count();
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get all values within an enum.
	\param enumType Type of the enum
	\param[out] intValues Array of enum values
	\return Size of the array
	*/
	static int GetEnumValues(typename enumType, out notnull array<int> intValues)
	{
		int val;
		for (int i, count = enumType.GetVariableCount(); i < count; i++)
		{
			if (enumType.GetVariableValue(null, i, val))
				intValues.Insert(val);
		}
		return intValues.Count();
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get all values within an enum as bitwise flags
	\param enumType Type of the enum
	\return Flags
	*/
	static int GetFlagValues(typename enumType)
	{
		int val, flags;
		for (int i, count = enumType.GetVariableCount(); i < count; i++)
		{
			if (enumType.GetVariableValue(null, i, val))
				flags |= val;
		}
		return flags;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get range of enum values.
	\param enumType Type of the enum
	\param[out] min Minimum value
	\param[out] max Maximum value
	\return True if there are any values
	*/
	static bool GetRange(typename enumType, out int min, out int max)
	{
		min = int.MAX;
		max = int.MIN;
		int val, count = enumType.GetVariableCount();
		for (int i = 0; i < count; i++)
		{
			if (enumType.GetVariableValue(null, i, val))
			{
				min = Math.Min(min, val);
				max = Math.Max(max, val);
			}
		}
		return count > 0;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get enum array with optional custom values.
	\param e Enum type
	\param customValue1 1st custom value added at the beginning of enum array
	\param customValue2 2nd custom value added at the beginning of enum array
	\param customValue3 3rd custom value added at the beginning of enum array
	\return Array of enums to be used in variable attributes after enum: keyword.
	*/
	static ParamEnumArray GetList(typename e, ParamEnum customValue1 = null, ParamEnum customValue2 = null, ParamEnum customValue3 = null)
	{
		ParamEnumArray params = ParamEnumArray.FromEnum(e);

		if (customValue3)
			params.InsertAt(customValue3, 0);
		if (customValue2)
			params.InsertAt(customValue2, 0);
		if (customValue1)
			params.InsertAt(customValue1, 0);

		return params;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get enum array with linear values: 0, 1, 2, 3, ...
	\param names Value names
	\return Array of enums to be used in variable attributes after enum: keyword.
	*/
	static ParamEnumArray GetList(notnull array<string> names)
	{
		ParamEnumArray params = {};
		for (int i, count = names.Count(); i < count; i++)
		{
			params.Insert(new ParamEnum(names[i], i.ToString()));
		}
		return params;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get enum array with flag values: 1, 2, 4, 8, ...
	\param names Value names
	\return Array of enums to be used in variable attributes after enum: keyword.
	*/
	static ParamEnumArray GetFlags(notnull array<string> names)
	{
		ParamEnumArray params = {};
		for (int i, count = names.Count(); i < count; i++)
		{
			params.Insert(new ParamEnum(names[i], Math.Pow(2, i).ToString()));
		}
		return params;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Show flag value names in a string.
	\param e Flags enum type
	\param flags Flag values
	\param delimiter String to be placed between value names
	\param noValue String to be used when the flag has no value, e.g., is 0
	\return Flag value names (e.g., "VALUE1, VALUE2, VALUE3")
	*/
	static string FlagsToString(typename e, int flags, string delimiter = ", ", string noValue = "N/A")
	{
		if (flags <= 0)
			return noValue;

		array<int> outValues = {};

		int count = BitToIntArray(flags, outValues);
		if (count == 0)
			return string.Empty;

		string result = typename.EnumToString(e, outValues[0]);
		for (int i = 1; i < count; i++)
		{
			result += delimiter;
			result += typename.EnumToString(e, outValues[i]);
		}
		return result;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Use enum as a default value in [Attribute()] field
	\param enumValue Value
	\return Compatible default value
	*/
	static string GetDefault(int enumValue)
	{
		return string.Format("%1", enumValue);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Use sum of all flag enum values as a default value in [Attribute()] field
	\param enumType Enum type
	\return Compatible default value
	*/
	static string GetDefault(typename enumType)
	{
		return GetDefault(GetFlagValues(enumType));
	}
};
