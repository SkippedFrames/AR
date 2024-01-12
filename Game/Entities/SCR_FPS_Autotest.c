// TODO(koudelkaluk): clean up this mess, get rid of unnecessary files saved for Confluence upload
[EntityEditorProps(category: "GameScripted/Utility", description: "Modified Heatmap autotest", dynamicBox: true)]
class SCR_HeatMap_AutotestClass: GenericEntityClass
{
};

class HeatmapData
{
	float m_fps;
	float m_x;
	float m_y;
	float m_z;

	void HeatmapData(float fps, float x, float y, float z)
	{
		m_fps = fps;
		m_x = x;
		m_y = y;
		m_z = z;
	}
};

//------------------------------------------------------------------------------------------------
// Handles closing the game after the data is sent to the heatmap database
class SCR_RestCallbackExample: RestCallback
{
	//------------------------------------------------------------------------------------------------
    override void OnError(int errorCode)
    {
        Print("Sending data has failed", LogLevel.WARNING);
		GetGame().RequestClose();
    };
 
	//------------------------------------------------------------------------------------------------
    override void OnTimeout()
    {
        Print("Sending data has timed out", LogLevel.WARNING);
		GetGame().RequestClose();		
    };
 
	//------------------------------------------------------------------------------------------------
    override void OnSuccess(string data, int dataSize)
    {
        Print("Data sent successfully. Size: " + dataSize);
		GetGame().RequestClose();
    };
}; 

class SCR_HeatMap_Autotest: GenericEntity
{
	[Attribute("ScriptCamera1", UIWidgets.EditBox, "Name of camera entity", "")]
	private string m_cameraEntityName;

	[Attribute("", UIWidgets.EditBox, "Name of Confluence page", "")]
	private string m_pageName;

	[Attribute("Unnamed test", UIWidgets.EditBox, "Title of particular test", "")]
	private string m_testName;

	[Attribute("Heatmap title", UIWidgets.EditBox, "Title of heatmap", "")]
	private string m_heatmapName;

	[Attribute("300", UIWidgets.Slider, "Measurements step", "10 2000 10")]
	private int m_step;

	[Attribute("1", UIWidgets.Slider, "Wait time", "0 20 0.1")]
	private float m_stepWaitTime;

	[Attribute("80", UIWidgets.Slider, "Camera height", "1 300 10")]
	private float m_cameraHeight;

	[Attribute("10", UIWidgets.Slider, "Number of slowest points for lowest FPS graph", "10 50 1")]
	private int m_lowFrameCount;

	[Attribute("", UIWidgets.EditBox)]
	private string mapName;

	[Attribute("0", UIWidgets.Auto)]
	private int startX;

	[Attribute("0", UIWidgets.Auto)]
	private int startZ;

	[Attribute("0", UIWidgets.Auto)]
	private int endX;

	[Attribute("0", UIWidgets.Auto)]
	private int endZ;

	[Attribute("120", UIWidgets.EditBox)]
	private int fpsRange;

	[Attribute("14", UIWidgets.EditBox)]
	private float timeOfTheDay;
	
	[Attribute("", UIWidgets.Auto)]
	private string mapID;
	
	private ref map<EPlatform, string> platforms = new map<EPlatform, string>();
	
	private ref SCR_RestCallbackExample m_jsonCallback = new SCR_RestCallbackExample();
	
	private const string HEATMAP_URL = "https://ar-heatmap.bistudio.com/";
	
	protected bool m_bDoneGenerating = false;	//< Used to stop the benchmark after processing and sending data
	
	protected float m_timer; 			//< Used for counting how long camera stays on one place

	private IEntity m_camera;			//< Entity used as camera, this one is auto moved by autotest

	private int m_x = startX;			//< Next camera x position
	private int m_y;					//< Camera y position
	private int m_z = startZ;			//< Next camera y position

	private int m_worldSize;			//< How big is world we are scanning
	private int m_worldSizeX;
	private int m_worldSizeZ;

	protected ref MeasurementFile m_heatmapfile; //< Heatmap output data

	private ref array<ref HeatmapData> m_geojson_data = new array<ref HeatmapData>();
	private ref array<int> image_file = new array<int>();

	private float min_fps = 1000;
	private float max_fps = 0;
	private float avg_fps = 0;

	protected ref AutotestRegister m_register;

	private bool area_only; // do we want to measure only specified area?

	private float fps_four_directions[4];
	private int prev_dir = -1;

	private int colors[6] = {
		0x0000ff,
		0x00ffff,
		0x00ff00,
		0xffff00,
		0xff0000,
		0xff0000
	};

	void SCR_HeatMap_Autotest(IEntitySource src, IEntity parent)
	{
		m_timer = -5;
		SetEventMask(EntityEvent.INIT | EntityEvent.FRAME);
		SetFlags(EntityFlags.ACTIVE, true);
    }

	override void EOnInit(IEntity owner)
	{
		ChimeraWorld world = GetGame().GetWorld();
		if (world.GetTimeAndWeatherManager())
		{
			world.GetTimeAndWeatherManager().SetTimeOfTheDay(timeOfTheDay);
			world.GetTimeAndWeatherManager().SetIsDayAutoAdvanced(false);
		}

		area_only = (endX > 0 || endZ > 0 || startX > 0 || startZ > 0);
		//! Find camera entity and move it to world origin m_cameraHeight above ground
		m_camera = GetGame().FindEntity(m_cameraEntityName);

		if (m_camera)
		{
			float groundY = m_camera.GetWorld().GetSurfaceY(m_x, m_z);
			m_camera.SetOrigin(Vector(m_x, groundY + m_cameraHeight, m_z));
		}

		//! Detect world size
		if (GetGame().GetWorldEntity())
		{
			vector min, max;
			GetGame().GetWorldEntity().GetWorldBounds(min, max);
			m_worldSize = Math.Max(max[0] - min[0], max[2] - min[2]);

			if (area_only)
			{
				m_worldSizeX = endX;
				m_worldSizeZ = endZ;
			}
			else
			{
				m_worldSizeX = m_worldSize;
				m_worldSizeZ = m_worldSize;

				endX = m_worldSizeX;
				endZ = m_worldSizeZ;
			}
		}
		
		platforms.Insert(EPlatform.WINDOWS, "62e0e57519098fc05e1b28fe");
		platforms.Insert(EPlatform.XBOX_ONE, "62e0e58f19098fc05e1b28ff");
		platforms.Insert(EPlatform.XBOX_SERIES_X, "632863b8211b59093a5afb79");
		platforms.Insert(EPlatform.XBOX_SERIES_S, "632863cb211b59093a5afb7a");
	}

	override void EOnFrame(IEntity owner, float timeSlice)
	{
		m_timer += timeSlice;
		if (m_timer < 0)
		{
			return;
		}
		
		if (m_bDoneGenerating)
		{
			return;
		}
		
		if (!m_camera)
		{
			Print("Camera entity cannot be found!", LogLevel.ERROR);
			GetGame().RequestClose();
			return;
		}

		int cur_fps = System.GetFPS();
		int cur_dir = ((m_timer / m_stepWaitTime - Math.Floor(m_timer / m_stepWaitTime)) * 4);

		if (cur_dir != prev_dir)
		{
			vector new_angles = m_camera.GetAngles(); // rotate the camera in 4 directions
			new_angles[1] = new_angles[1] + 90;
			m_camera.SetAngles(new_angles);
			prev_dir = cur_dir;
		}

		fps_four_directions[cur_dir] = cur_fps;

		if (m_timer > m_stepWaitTime) //< When camera was still long enough
		{
			m_timer -= m_stepWaitTime;
			if (m_camera)
			{
				//! Move camera
				float groundY = m_camera.GetWorld().GetSurfaceY(m_x, m_z);
				m_y = groundY + m_cameraHeight;
				m_camera.SetOrigin(Vector(m_x, groundY + m_cameraHeight, m_z));
			}

			if (!m_register)
			{
				m_register = new AutotestRegister();
				m_register.Init(m_pageName);
			}

			if (!m_heatmapfile)
			{
				ref MeasurementFile descrFile = m_register.OpenMeasurementFile("0_fps_test_descr", "", MeasurementType.HTML);
				descrFile.AddData(string.Format("<h1>" + m_heatmapName + "</h1><p>Camera step: %1m<br/>Step wait time: %2s<br/>Camera height above ground: %3m<br/>Number of worst FPS places: %4</p>", m_step, m_stepWaitTime, m_cameraHeight, m_lowFrameCount));

				m_heatmapfile = m_register.OpenMeasurementFile("1_fps_heatmap", m_testName, MeasurementType.HeatMap);

			}

			int fps = (fps_four_directions[0] + fps_four_directions[1] + fps_four_directions[2] + fps_four_directions[3]) / 4;

			image_file.Insert(fps);
			m_heatmapfile.AddData(string.Format("%1,%2,%3", m_x, m_z, fps));  //< output new data to heatmap

			if (m_x + m_step <= m_worldSizeX)
			{
				HeatmapData new_geojson_value = new HeatmapData(fps, m_x, m_y, m_z);

				m_geojson_data.Insert(new_geojson_value);
				if (fps > max_fps)
					max_fps = fps;
				if (fps < min_fps)
					min_fps = fps;
				//! Prepare position for next camera move
				m_x = m_x + m_step;
			}
			else
			{
				m_x = startX;
				if (m_z + m_step <= m_worldSizeZ - m_step)
				{
					m_z = m_z + m_step;
				}
				else
				{
					CalculateAvgFPS();
					GenerateDDS();
					GenerateGeoJSON();
					GenerateCSV();
					m_bDoneGenerating = true;
				}
			}
		}
	}

	private void CalculateAvgFPS()
	{
		for (int i = 0; i < m_geojson_data.Count(); ++i)
		{
			avg_fps += m_geojson_data[i].m_fps;
		}

		if (m_geojson_data.Count() == 0)
			return;

		avg_fps /= m_geojson_data.Count();
		Print("Avg FPS: " + avg_fps);
	}

	private void ReverseArray(inout array<int> arr)
	{
		array<int> temp = new array<int>();
		for (int i = arr.Count()-1; i >= 0; --i)
		{
			temp.Insert(arr[i]);
		}
		arr = temp;
	}

	private void GenerateDDS()
	{
#ifdef WORKBENCH
		int width = (endX - startX) / m_step + 1;//2;
		int height = (endZ - startZ) / m_step;//2;

		int offset_x = startX / m_step;
		int offset_z = startZ / m_step;

		int image_size = m_worldSize / m_step;

		int id = 0;
		int id_z = 0;

		array <int> image = new array <int>;

		for (int i = 0; i < image_size * image_size; ++i)
		{
			image.Insert(ARGB(255, 0, 0, 0));
		}

		for (int y = 0; y < height; ++y)
		{
			array <int> row = new array <int>;
			for (int i = 0; i < image_size; ++i)
			{
				row.Insert(ARGB(255, 0, 0, 0));
			}

			int id_x = 0;

			for (int x = 0; x < width; ++x)
			{
				float range;
				int col;

				int fps = image_file[id];

				float level = Math.Clamp(fps / fpsRange, 0, 1);
				int phase = Math.Floor(level / 0.25);

				range = Math.Clamp((level - phase * 0.25) / 0.25, 0, 1);

				int from = colors[phase];
				int to = colors[phase+1];

				Color col1 = Color.FromInt(from);
				Color col2 = Color.FromInt(to);

				col1.Lerp(col2, range);
				col = col1.PackToInt();

				int index = offset_x + x;

				if (index < row.Count())
					row[index] = col;

				id_x++;
				id++;
			}

			ReverseArray(row);
			for (int i = 0; i < row.Count(); ++i)
			{
				int index = (offset_z + id_z) * image_size + i;
				if (index < image.Count())
					image[index] = row[i];
			}
			id_z++;
		}

		ReverseArray(image);
		TexTools.SaveImageData("$logs:heatmap.dds", image_size, image_size, image);
#endif
	}

	private string GetGeoJSONLine(HeatmapData val) //vector val)
	{
		string new_value = string.Format(
			"{ \"properties\": { \"FPS\": \"%1\" }, \"geometry\": { \"coordinates\": [ %2, %3, %4] } }",
			Math.Floor(val.m_fps), val.m_x, val.m_z, val.m_y);

		return new_value;
	}

	private string GetCSVLine(HeatmapData val) //vector val)
	{
		string new_value = string.Format(
			"%1,%2,%3",
			val.m_x, val.m_z, Math.Floor(val.m_fps) );

		return new_value;
	}

	private void GenerateGeoJSON()
	{
		FileHandle file;
		
		Print("Creating a geojson file");
		
		FileIO.MakeDirectory("$logs:/" + mapName);
		string filename = FormatTimestamp() + ".geojson";
		file = FileIO.OpenFile("$logs:/" + mapName + "/" + filename, FileMode.WRITE);

		WriteDataGJSON(file);
		file.Close();		
		file = FileIO.OpenFile("$logs:/" + mapName + "/" + filename, FileMode.READ);
		
		string data;
		file.Read(data, file.GetLength());
		file.Close();
		
		RestContext restContext = GetGame().GetRestApi().GetContext(HEATMAP_URL);
		
		if (!restContext)
		{
			Print("Failed to get RestAPI context. Unable to send data", LogLevel.ERROR);
			return;
		}
		
		restContext.SetHeaders("Content-Type,application/json");
		restContext.POST(m_jsonCallback, "api/fps-statistics", data);
		
		restContext = null;
		
		Print("Creating a json done");
	}

	private void GenerateCSV()
	{
		FileHandle file;
		
		Print("Creating a CSV file");
		
		FileIO.MakeDirectory("$logs:/" + mapName);
		string filename = FormatTimestamp() + ".csv";
		file = FileIO.OpenFile("$logs:/" + mapName + "/" + filename, FileMode.WRITE);

		WriteDataCSV(file);
		file.Close();
		
		Print("Creating a CSV done");
	}

	private void WriteDataGJSON(FileHandle file)
	{
		Print("Adding data to geojson");
		
		string platform;
		platforms.Find(System.GetPlatform(), platform);
		
		if (!platform)
		{
			platform = "Invalid";
		
			Print("SCR_FPS_Autotest::Invalid platform, GJSON file will fail to post to database", LogLevel.ERROR);
		}
		
		file.WriteLine("{");
		file.WriteLine(string.Format("\"date\": \"%1\",", SCR_DateTimeHelper.GetDateTimeLocal()));
		file.WriteLine(string.Format("\"platform\": \"%1\",", platform));
		file.WriteLine(string.Format("\"map\": \"%1\",", mapID));
		file.WriteLine("\"featurecollection\": {");
		file.WriteLine("\"type\": \"FeatureCollection\",");
		file.WriteLine("\"name\": \"fps\",");
		file.WriteLine(string.Format("\"minfps\": %1,", Math.Floor(min_fps)));
		file.WriteLine(string.Format("\"maxfps\": %1,", Math.Floor(max_fps)));
		file.WriteLine(string.Format("\"avgfps\": %1,", Math.Floor(avg_fps)));
		file.WriteLine("\"crs\": { \"type\": \"name\", \"properties\": { \"name\": \"urn:ogc:def:crs:EPSG::32632\" } },");
		file.WriteLine("\"features\": [");

		for (int i = 0; i < m_geojson_data.Count(); ++i)
		{
			string line_end = ",";
			if (i == m_geojson_data.Count()-1)
				line_end = string.Empty;
			
			file.WriteLine(GetGeoJSONLine(m_geojson_data[i]) + line_end);
		}

		file.WriteLine("]\n}\n}");
	}

	private void WriteDataCSV(FileHandle file)
	{
		Print("Adding data to CSV");
		
		file.WriteLine("x,z,fps");

		for (int i = 0; i < m_geojson_data.Count(); ++i)
			file.WriteLine(GetCSVLine(m_geojson_data[i]));

	}
	private string FormatTimestamp()
	{
		int year, month, day;
		System.GetYearMonthDay(year, month, day);
		string smonth, sday;
		if (month < 10)
			smonth = string.Format("0%1", month);
		else
			smonth = string.Format("%1", month);

		if (day < 10)
			sday = string.Format("0%1", day);
		else
			sday = string.Format("%1", day);

		return string.Format("%1%2%3", year, smonth, sday);
	}
};
