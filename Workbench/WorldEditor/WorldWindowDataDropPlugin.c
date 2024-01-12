const string DataType_EntityClassName = "WorldEditor/EntityType";		//entity class name type
const string DataType_ResourceFiles = "Workbench/ResourceFiles";		//registered resource files type

[WorkbenchPluginAttribute("WorldWindowDataDropPlugin", "Allows to handle dropped data into a world edit window and completely reimplement default editor functionality", "", "", {"WorldEditor"})]
class WorldWindowDataDropPlugin : WorldEditorPlugin
{
//-------------------------------------------------------------------------------
	override bool OnWorldEditWindowDataDropped(int windowType, int posX, int posY, string dataType, array<string> data)
	{
		if(dataType == DataType_EntityClassName)
		{
			string className = data[0];
			
			//check whether the particular dropped class is interesting for us
			if(className == "SomeNotExistingClassAsExample")
			{
				//if so then we can spawn it from here with any additional customizations
				SpawnCustomizedEntityAsExample(windowType, posX, posY, className);
				return true;	//event is processed. Editor's default implementation won't be done
			}
		}
		else if(dataType == DataType_ResourceFiles)
		{
			//iterate over dropped resource files
			for(int n = 0; n < data.Count(); n++)
			{
				string resName = data[n];
				
				string ext;
				FilePath.StripExtension(resName, ext);
				ext.ToLower();
				
				//check whether the particular dropped resoure file is interesting for us
				if(ext == "ptc")
				{
					//if so then we can spawn anything custom that uses the dropped data
					SpawnParticleEffect(windowType, posX, posY, resName);
					return true;	//means event is processed. Editor's default implementation won't be done
				}
			}
		}

		return false;	//event not processed. Let work editor's default implementation
	}

//-------------------------------------------------------------------------------
	void SpawnParticleEffect(int windowType, int posX, int posY, ResourceName resName)
	{
		WorldEditorAPI api = GetEditorApi();
		if(api.BeginEntityAction("Creating entity")) 
		{
			api.ClearEntitySelection();
			
			//use the same creating funtion as editor does because it solves correct placing etc.
			IEntity ent = api.CreateEntityInWindowEx(windowType, posX, posY, "ParticleEffectEntity", "", api.GetCurrentEntityLayerId());
			if(ent)
			{
				IEntitySource entSrc = api.EntityToSource(ent);
				api.ModifyEntityKey(ent, "EffectPath", resName);
				api.AddToEntitySelection(api.SourceToEntity(entSrc));
			}
			
			api.EndEntityAction();
		}
	}
	
//-------------------------------------------------------------------------------
	void SpawnCustomizedEntityAsExample(int windowType, int posX, int posY, string className)
	{
		//this is an example how entity must be created.
		WorldEditorAPI api = GetEditorApi();
		if(api.BeginEntityAction("Creating entity")) 
		{
			api.ClearEntitySelection();
			
			//use the same creating funtion as editor does because it solves correct placing etc.
			IEntity ent = api.CreateEntityInWindowEx(windowType, posX, posY, className, "", api.GetCurrentEntityLayerId());
			if(ent)
				api.AddToEntitySelection(ent);
			
			api.EndEntityAction();
		}
	}
	
//-------------------------------------------------------------------------------
	WorldEditorAPI GetEditorApi()
	{
		WorldEditor mod = Workbench.GetModule(WorldEditor);
		if (!mod)
		{
			Print("World editor module is not available.", LogLevel.ERROR);
			return null;
		}

		return mod.GetApi();		
	}
	
//-------------------------------------------------------------------------------
	private void WorldWindowDataDropPlugin()
	{
	}
}
