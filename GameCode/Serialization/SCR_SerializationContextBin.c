
// Convenience Save/Load contexts to work with binary format

/*-----------------------------------------------------------
 Context to save data to given serliazation container
*/
class SCR_BinSaveContext : ContainerSerializationSaveContext
{
	private ref BinSaveContainer container;

	void SCR_BinSaveContext(bool skipEmptyObjects = true)
	{
		container = new BinSaveContainer();
		SetContainer(container);
	}

	void ~SCR_BinSaveContext()
	{
	}

	bool SaveToFile(string filePath)
	{
		return container.SaveToFile(filePath);
	}
};

/*-----------------------------------------------------------
 Context to load data from given serliazation container
*/
class SCR_BinLoadContext : ContainerSerializationLoadContext
{
	private ref BinLoadContainer container;
	
	void SCR_BinLoadContext(bool skipEmptyObjects = true)
	{
		container = new BinLoadContainer();
		SetContainer(container);
	}

	void ~SCR_BinLoadContext()
	{
	}
	
	bool LoadFromFile(string fileName)
	{
		return container.LoadFromFile(fileName);
	}
	
};
