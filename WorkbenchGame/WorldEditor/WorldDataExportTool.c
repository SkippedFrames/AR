enum EMapDataExportType
{
	Geometry2D,
	Soundmap,
	Rasterization
};

/*!
Exports data of the current map based on selected type.
*/

//----------------------------------------------------------------------------------------------
[WorkbenchToolAttribute("Export Map data", "Export data of various types from the current world", "", awesomeFontCode: 0xf0ac)]
class WorldMapExportTool: WorldEditorTool
{
	[Attribute(defvalue: "", desc: "Folder where the file will be saved", params: "unregFolders", enums: NULL, category: "Export file")]
	ResourceName destinationPath;
	string exportPath;
	
	[Attribute("0", UIWidgets.ComboBox, "Export type", "", ParamEnumArray.FromEnum(EMapDataExportType), category: "Export file")]
	int type;

	[Attribute("1", UIWidgets.CheckBox, "2D Geometry: Generation of Roads' data", category: "Geometry 2D")]
	bool generateRoads;
	[Attribute("1", UIWidgets.CheckBox, "2D Geometry: Save Roads' data to exported file", category: "Geometry 2D")]
	bool saveRoads;
	
	[Attribute("1", UIWidgets.CheckBox, "2D Geometry: Generation of Powerlines' data", category: "Geometry 2D")]
	bool generatePowerlines;
	[Attribute("1", UIWidgets.CheckBox, "2D Geometry: Save Powerlines' data to exported file", category: "Geometry 2D")]
	bool savePowerlines;
	
	[Attribute("1", UIWidgets.CheckBox, "2D Geometry: Generation of Buildings' data", category: "Geometry 2D")]
	bool generateBuildings;
	[Attribute("1", UIWidgets.CheckBox, "2D Geometry: Save Buildings' data to exported file", category: "Geometry 2D")]
	bool saveBuildings;
	
	[Attribute("1", UIWidgets.CheckBox, "2D Geometry: Generation of Areas' data", category: "Geometry 2D")]
	bool generateAreas;
	[Attribute("1", UIWidgets.CheckBox, "2D Geometry: Ignore Generator entities during generation", category: "Geometry 2D")]
	bool ignoreGeneratorAreas;
	[Attribute("1", UIWidgets.CheckBox, "2D Geometry: Save Areas' data to exported file", category: "Geometry 2D")]
	bool saveAreas;
	
	[Attribute("1", UIWidgets.CheckBox, "2D Geometry: Generation of Water Bodies' data", category: "Geometry 2D")]
	bool generateWaterBodies;
	[Attribute("1", UIWidgets.CheckBox, "2D Geometry: Save Water Bodies' data to exported file", category: "Geometry 2D")]
	bool saveWaterBodies;
	
	[Attribute("1", UIWidgets.CheckBox, "2D Geometry: Generation of Hills' data", category: "Geometry 2D")]
	bool generateHills;
	[Attribute("50.0", UIWidgets.Slider, "Minimum hill height", "30 155 5", category: "Geometry 2D")]
	float hillMinimumHeight;
	[Attribute("1", UIWidgets.CheckBox, "2D Geometry: Save Hills' data to exported file", category: "Geometry 2D")]
	bool saveHills;

	[Attribute("1.0 1.0 1.0 1.0", UIWidgets.ColorPicker, "Rasterization: Land color Bright", category: "Rasterization")]
	ref Color landColorBright;
	[Attribute("0.5 0.5 0.5 1.0", UIWidgets.ColorPicker, "Rasterization: Land color Dark", category: "Rasterization")]
	ref Color landColorDark;
	[Attribute("0.863 0.980 1.0 1.0", UIWidgets.ColorPicker, "Rasterization: Ocean color Bright", category: "Rasterization")]
	ref Color oceanColorBright;
	[Attribute("0.757 0.91 0.929 1.0", UIWidgets.ColorPicker, "Rasterization: Ocean color Dark", category: "Rasterization")]
	ref Color oceanColorDark;
	[Attribute("2.5", UIWidgets.Slider, "Rasterization: Land scale factor", "0 10 0.1", category: "Rasterization")]
	float scaleLand;
	[Attribute("1.2", UIWidgets.Slider, "Rasterization: Ocean scale factor", "0 10 0.1", category: "Rasterization")]
	float scaleOcean;
	[Attribute("500.0", UIWidgets.Slider, "Rasterization: Height scale factor", "-1000 1000 1.0", category: "Rasterization")]
	float heightScale;
	[Attribute("60.0", UIWidgets.Slider, "Rasterization: Depth scale factor", "-1000 1000 1.0", category: "Rasterization")]
	float depthScale;
	[Attribute("-50.0", UIWidgets.Slider, "Rasterization: Depth Lerp meters", "-1000 1000 1.0", category: "Rasterization")]
	float depthLerpMeters;
	[Attribute("0.5", UIWidgets.Slider, "Rasterization: Shade intensity factor", "0.5 15 0.01", category: "Rasterization")]
	float shadeIntensity;
	[Attribute("1.8", UIWidgets.Slider, "Rasterization: Height intensity factor", "0.5 15 0.01", category: "Rasterization")]
	float heightIntensity;
	[Attribute("1", UIWidgets.CheckBox, "Rasterization: Include Areas defined by Generators into Rasterization", category: "Rasterization")]
	bool includeGeneratorAreas;
	[Attribute("0.608 0.784 0.529 1.0", UIWidgets.ColorPicker, "Rasterization: Forest area color", category: "Rasterization")]
	ref Color forestAreaColor;
	[Attribute("1.25", UIWidgets.Slider, "Rasterization: Forest area intensity factor", "0.5 15 0.01", category: "Rasterization")]
	float forestAreaIntensity;
	[Attribute("0.745 0.745 0.745 1.0", UIWidgets.ColorPicker, "Rasterization: Other area color", category: "Rasterization")]
	ref Color otherAreaColor;
	[Attribute("1.0", UIWidgets.Slider, "Rasterization: Other area intensity factor", "0.5 15 0.01", category: "Rasterization")]
	float otherAreaIntensity;

	[ButtonAttribute("Export")]
	void Execute()
	{
		Export();
	}
	
	ref MapDataExporter dataExporter = new MapDataExporter();
	
	// Report error
	string ReportMessage( DataExportErrorType eCode ) 
	{
		string reason = "";
		switch(eCode)
		{
			case DataExportErrorType.DataExportErrorNoGame: reason = "Game not found"; break;
			case DataExportErrorType.DataExportErrorNoDataType: reason = "Data type not found"; break;
			case DataExportErrorType.DataExportErrorNoMapEntity: reason = "Map Entity not found"; break;
			case DataExportErrorType.DataExportErrorNoWorld: reason = "World not found"; break;
			case DataExportErrorType.DataExportErrorNoWorldPath: reason = "World path empty"; break;
			case DataExportErrorType.DataExportErrorNoOutputFile: reason = "Could not open output file"; break;
			case DataExportErrorType.DataExportErrorNoCompatibleWorld: reason = "World is not compatible"; break;
			case DataExportErrorType.DataExportErrorNoSoundWorld: reason = "Sound World not found"; break;
			case DataExportErrorType.DataExportErrorNotSavingToFile: reason = "Not saving to file, Default Map created"; break;
			case DataExportErrorType.DataExportErrorInvalidSoundmap: reason = "Invalid Soundmap"; break;
			case DataExportErrorType.DataExportErrorNoLandOrOcean: reason = "No Land or Ocean color found"; break;
			case DataExportErrorType.DataExportErrorNoTerrain: reason = "Terrain not found"; break;
			default: reason = "Unkown, code=" + eCode; break;
		}
		return reason;
	}	
		
	// Export
	bool Export()
	{
		WorldEditor we = Workbench.GetModule(WorldEditor);
		WorldEditorAPI api = we.GetApi();
		string worldPath;
		api.GetWorldPath(worldPath);

		WBProgressDialog progress = new WBProgressDialog("Processing", we);


		exportPath = destinationPath.GetPath();

		if (type == EMapDataExportType.Geometry2D)
		{
			Print("Exporting map 2D map data...");
			ExcludeGenerateFlags generateFlags = ExcludeGenerateFlags.ExcludeGenerateNone;
			if(!generateRoads) generateFlags = generateFlags | ExcludeGenerateFlags.ExcludeGenerateRoads;
			if(!generatePowerlines) generateFlags = generateFlags | ExcludeGenerateFlags.ExcludeGeneratePowerlines;
			if(!generateBuildings) generateFlags = generateFlags | ExcludeGenerateFlags.ExcludeGenerateBuildings;
			if(!generateAreas) generateFlags = generateFlags | ExcludeGenerateFlags.ExcludeGenerateAreas;
			if(!generateWaterBodies) generateFlags = generateFlags | ExcludeGenerateFlags.ExcludeGenerateWaterBodies;
			if(!generateHills) generateFlags = generateFlags | ExcludeGenerateFlags.ExcludeGenerateHills;
			ExcludeSaveFlags saveFlags = ExcludeSaveFlags.ExcludeSaveNone;
			if(!saveRoads) saveFlags = saveFlags | ExcludeSaveFlags.ExcludeSaveRoads;
			if(!savePowerlines) saveFlags = saveFlags | ExcludeSaveFlags.ExcludeSavePowerlines;
			if(!saveBuildings) saveFlags = saveFlags | ExcludeSaveFlags.ExcludeSaveBuildings;
			if(!saveAreas) saveFlags = saveFlags | ExcludeSaveFlags.ExcludeSaveAreas;
			if(!saveWaterBodies) saveFlags = saveFlags | ExcludeSaveFlags.ExcludeSaveWaterBodies;
			if(!saveHills) saveFlags = saveFlags | ExcludeSaveFlags.ExcludeSaveHills;

			DataExportErrorType result = dataExporter.ExportData(EMapDataType.Geometry2D, exportPath, worldPath, hillMinimumHeight, ignoreGeneratorAreas, generateFlags, saveFlags);
			if(result != DataExportErrorType.DataExportErrorNone)
			{
				string reason = ReportMessage(result);
				Print(string.Format("Could not export 2D map data. (%1)", reason), LogLevel.ERROR);
				return false;
			}
		}
		else if (type == EMapDataExportType.Soundmap)
		{
			Print("Exporting Soundmap...");
			DataExportErrorType result = dataExporter.ExportData(EMapDataType.Soundmap, exportPath, worldPath);
			if(result != DataExportErrorType.DataExportErrorNone)
			{
				string reason = ReportMessage(result);
				Print(string.Format("Could not export Soundmap data. (%1)", reason), LogLevel.ERROR);
				return false;
			}
		}
		else if (type == EMapDataExportType.Rasterization)
		{
			Print("Exporting map rasterization...");
			DataExportErrorType result = dataExporter.SetupColors(landColorBright, landColorDark, oceanColorBright, oceanColorDark, forestAreaColor, otherAreaColor);
			if(result == DataExportErrorType.DataExportErrorNone)
				result = dataExporter.ExportRasterization(exportPath, worldPath, scaleLand, scaleOcean, heightScale, depthScale, depthLerpMeters, shadeIntensity, heightIntensity, includeGeneratorAreas, forestAreaIntensity, otherAreaIntensity);
			if(result != DataExportErrorType.DataExportErrorNone)
			{
				string reason = ReportMessage(result);
				Print(string.Format("Could not export map rasterization. (%1)", reason), LogLevel.ERROR);
				return false;
			}
		}
		return true;
	}
};

//----------------------------------------------------------------------------------------------
[WorkbenchPluginAttribute("World Data Export Plugin", "Open world and export defined data type")]
class WorldDataExport: WorkbenchPlugin
{

	ref MapDataExporter dataExporter = new MapDataExporter();

	// Report error
	string ReportMessage( DataExportErrorType eCode ) 
	{
		string reason = "";
		switch(eCode)
		{
			case DataExportErrorType.DataExportErrorNoGame: reason = "Game not found"; break;
			case DataExportErrorType.DataExportErrorNoDataType: reason = "Data type not found"; break;
			case DataExportErrorType.DataExportErrorNoMapEntity: reason = "Map Entity not found"; break;
			case DataExportErrorType.DataExportErrorNoWorld: reason = "World not found"; break;
			case DataExportErrorType.DataExportErrorNoWorldPath: reason = "World path empty"; break;
			case DataExportErrorType.DataExportErrorNoOutputFile: reason = "Could not open output file"; break;
			case DataExportErrorType.DataExportErrorNoCompatibleWorld: reason = "World is not compatible"; break;
			case DataExportErrorType.DataExportErrorNoSoundWorld: reason = "Sound World not found"; break;
			case DataExportErrorType.DataExportErrorNotSavingToFile: reason = "Not saving to file, Default Map created"; break;
			case DataExportErrorType.DataExportErrorInvalidSoundmap: reason = "Invalid Soundmap"; break;
			case DataExportErrorType.DataExportErrorNoLandOrOcean: reason = "No Land or Ocean color found"; break;
			case DataExportErrorType.DataExportErrorNoTerrain: reason = "Terrain not found"; break;
			default: reason = "Unkown, code=" + eCode; break;
		}
		return reason;
	}

	// CLI arguments
	override void RunCommandline()
	{
		ResourceManager rb = Workbench.GetModule(ResourceManager);
		string path;
		rb.GetCmdLine("-world", path);
		string type;
		rb.GetCmdLine("-type", type);
		string dir;
		rb.GetCmdLine("-saveDir", dir);
		
		if (ExportWorldData(path, type, dir))
			Workbench.Exit(0);
		else
			Workbench.Exit(-1);
	}

	// Export
	bool ExportWorldData(string path, string type, string dir)
	{
		bool success = false;
		Print("WorldDataExport: opening world "+ path, LogLevel.VERBOSE);

		Workbench.OpenModule(WorldEditor);
		WorldEditor we = Workbench.GetModule(WorldEditor);
		if (we.SetOpenedResource(path))
		{
			Sleep(300);

			bool exportMap = true;
			bool exportSound = true;
			bool exportRasterization = true;
			if (type.Length() > 0)
			{
				if (type.StartsWith("2DMap"))
				{
					exportSound = false;
					exportRasterization = false;
				}
				else if (type.StartsWith("SoundMap"))
				{
					exportMap = false;
					exportRasterization = false;
				}
				else if (type.StartsWith("2DRasterization"))
				{
					exportSound = false;
					exportMap = false;
				}
			}

			WorldEditorAPI api = we.GetApi();
			string worldPath;
			api.GetWorldPath(worldPath);

			WBProgressDialog progress = new WBProgressDialog("Processing", we);
			success = true;
			if (exportMap)
			{
				Print("Exporting map 2D map data...");
				DataExportErrorType result = dataExporter.ExportData(EMapDataType.Geometry2D, dir, worldPath, 50, true, ExcludeGenerateFlags.ExcludeGenerateNone, ExcludeSaveFlags.ExcludeSaveNone);
				if(result != DataExportErrorType.DataExportErrorNone)
				{
					string reason = ReportMessage(result);
					Print(string.Format("Could not export 2D map data. (%1)", reason), LogLevel.ERROR);
					success = false;
				}
			}
			if (exportSound)
			{
				Print("Exporting Soundmap...");
				DataExportErrorType result = dataExporter.ExportData(EMapDataType.Soundmap, dir, worldPath);
				if(result != DataExportErrorType.DataExportErrorNone)
				{
					string reason = ReportMessage(result);
					Print(string.Format("Could not export Soundmap data. (%1)", reason), LogLevel.ERROR);
					success = false;
				}
			}
			if (exportRasterization)
			{
				Print("Exporting map rasterization...");
				//TODO: change to user defined values (along with WorldMapExportTool)
				Color landColorBright = new Color(1.0, 1.0, 1.0, 1.0);
				Color landColorDark = new Color(0.5, 0.5, 0.5, 1.0);
				Color oceanColorBright = new Color(0.863, 0.980, 1.0, 1.0);
				Color oceanColorDark = new Color(0.757, 0.91, 0.929, 1.0);
				float scaleLand = 2.5;
				float scaleOcean = 1.2;
				float heightScale = 500.0;
				float depthScale = 60.0;
				float depthLerpMeters = -50.0;
				float shadeIntensity = 0.5;
				float heightIntensity = 1.8;
				bool includeGeneratorAreas = true;
				Color forestAreaColor = new Color(0.608, 0.784, 0.529, 1.0);
				float forestAreaIntensity = 1.25;
				Color otherAreaColor = new Color(0.745, 0.745, 0.745, 1.0);
				float otherAreaIntensity = 1.0;

				DataExportErrorType result = dataExporter.SetupColors(landColorBright, landColorDark, oceanColorBright, oceanColorDark, forestAreaColor, otherAreaColor);
				if(result == DataExportErrorType.DataExportErrorNone)
					result = dataExporter.ExportRasterization(dir, worldPath, scaleLand, scaleOcean, heightScale, depthScale, depthLerpMeters, shadeIntensity, heightIntensity, includeGeneratorAreas, forestAreaIntensity, otherAreaIntensity);
				if(result != DataExportErrorType.DataExportErrorNone)
				{
					string reason = ReportMessage(result);
					Print(string.Format("Could not export map rasterization. (%1)", reason), LogLevel.ERROR);
					success = false;
				}
			}

			if (success)
				Print("WorldDataExport: export successful!", LogLevel.VERBOSE);

			Sleep(100);
			we.Close();
		}

		return success;
	}
};
