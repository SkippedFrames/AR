class SCR_ConfigHelper
{
	//------------------------------------------------------------------------------------------------
	//! Get BaseContainer from config file path, ensuring the whole thing is valid
	//! returns the found sub-container or null if not found
	//! \param container if null, returns null
	//! \param subChildPath path is case-insensitive, separated by /, spaces can be involved \
	//! examples: "level1/level2/level3", "level1 / level2   /   level3" \
	//! if empty, returns the provided container
	//! \param removeEntryPart if true, removes the last path element \
	//! examples: path/to/entryName → path/to (to target the parent container itself)
	//! \return found child BaseContainer, or null if path is incorrect or if base container is invalid
	static BaseContainer GetBaseContainer(ResourceName configPath, string subChildPath = "", bool removeEntryPart = false)
	{
		if (configPath.IsEmpty())
			return null;

		Resource holder = BaseContainerTools.LoadContainer(configPath);
		if (!holder || !holder.IsValid())
			return null;

		return GetChildBaseContainer(holder.GetResource().ToBaseContainer(), subChildPath, removeEntryPart);
	}

	//------------------------------------------------------------------------------------------------
	//! returns the found sub-container or null if not found
	//! \param container if null, returns null
	//! \param subChildPath path is case-insensitive, separated by /, spaces can be involved \
	//! examples: "level1/level2/level3", "level1 / level2   /   level3" \
	//! if empty, returns the provided container
	//! \param removeEntryPart if true, removes the last path element \
	//! examples: path/to/entryName → path/to (to target the parent container itself)
	//! \return found child BaseContainer, or null if path is incorrect
	protected static BaseContainer GetChildBaseContainer(BaseContainer container, string subChildPath, bool removeEntryPart = false)
	{
		if (!container)
			return null;

		if (subChildPath.Trim().IsEmpty())
			return container;

		array<string> paths = {};
		SplitConfigPath(subChildPath, paths, removeEntryPart);

		return GetChildBaseContainer(container, paths);
	}

	//------------------------------------------------------------------------------------------------
	//! returns the found sub-container or null if not found
	//! \param container if null, returns null
	//! \param paths case-insensitive, items will be trimmed \
	//! if empty, returns the provided container
	//! \return found child BaseContainer, or null if path is incorrect
	// to do: split in smaller methods?
	protected static BaseContainer GetChildBaseContainer(notnull BaseContainer container, array<string> paths)
	{
		if (!container)
			return null;

		if (!paths || paths.IsEmpty())
			return container;

		string path;
		string nextPath;
		BaseContainer prevChild = container;
		BaseContainer child = container;
		BaseContainerList containerList;
		for (int i, cntMinus1 = paths.Count() -1; i <= cntMinus1; i++)
		{
			path = paths[i];
			if (i < cntMinus1)
				nextPath = paths[i+1];
			else
				nextPath = string.Empty;

			prevChild = child;
			child = prevChild.GetObject(path);
			if (child)
				continue; // child is found, we need to go deeper

			if (nextPath.IsEmpty())
				return null;

			containerList = prevChild.GetObjectArray(path);
			if (!containerList)
			{
				return null; // no child, no array of children, get out
			}

			child = GetChildFromList(containerList, nextPath);
			if (!child)
				return null;

			i++; // moved one step ahead with list → child
		}

		return child;
	}

	//------------------------------------------------------------------------------------------------
	//! get BaseContainer from BaseContainerList by its (case-insensitive) name
	//! \param containerList list of containers from which to look
	//! \param childName case-insensitive child name
	//! \return BaseContainer found child or null
	protected static BaseContainer GetChildFromList(BaseContainerList containerList, string childName)
	{
		if (childName.IsEmpty())
			return null;

		string arrayItemName;
		BaseContainer containerListElement;
		for (int i, cnt = containerList.Count(); i < cnt; i++)
		{
			containerListElement = containerList[i];
			arrayItemName = containerListElement.GetName();

			if (arrayItemName.IsEmpty())
				arrayItemName = containerListElement.GetClassName(); // if .et

			if (arrayItemName.IsEmpty())
				continue;

			arrayItemName.ToLower();
			childName.ToLower();
			if (arrayItemName == childName)
			{
				return containerListElement;
			}
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! split "config path" (separator: / ), trims and toLower parts and returns last part if any
	//! \param input the input string
	//! \param output the target output array
	//! \param removeLastPart removes last part if entry path is provided
	//! \return last part (trimmed but not ToLower'ed), whether it has been removed or not
	static string SplitConfigPath(string input, out array<string> output, bool removeLastPart = false)
	{
		input.Split("/", output, true);

		string lastPart;
		if (output.IsEmpty())
			return lastPart;

		int lastIndex = output.Count() -1;
		lastPart = output[lastIndex].Trim();
		if (removeLastPart)
		{
			output.Remove(lastIndex);
		}

		for (int i, cnt = output.Count(); i < cnt; i++)
		{
			output[i] = output[i].Trim();
		}

		return lastPart;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Extract GUID from full resource name.
	\param path Full path
	\return GUID
	*/
	static string GetGUID(ResourceName path)
	{
		"{"; // fix indent
		int guidIndex = path.LastIndexOf("}");
		if (guidIndex >= 0)
			return path.Substring(0, guidIndex + 1);
		else
			return string.Empty;
	}
};

class SCR_ConfigHelperT<Class T>
{
	//------------------------------------------------------------------------------------------------
	//! get typed container instance (or null)
	//! \param configPath
	//! \return T-casted BaseContainer instance or null if not found/wrong type
	static T GetConfigObject(ResourceName configPath)
	{
		if (configPath.IsEmpty())
			return null;

		BaseContainer container = SCR_ConfigHelper.GetBaseContainer(configPath);
		if (!container)
			return null;

		Managed managed = BaseContainerTools.CreateInstanceFromContainer(container);
		return T.Cast(managed);
	}
};
