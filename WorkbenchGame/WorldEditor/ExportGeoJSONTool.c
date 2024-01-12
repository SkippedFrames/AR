static string ROAD_GENERATOR_ENTITY = "RoadGeneratorEntity";
static string WALL_GENERATOR_ENTITY = "WallGeneratorEntity";
static string PREFAB_GENERATOR_ENTITY = "PrefabGeneratorEntity";
static string FOREST_GENERATOR_ENTITY = "ForestGeneratorEntity";
static string LAKE_GENERATOR_ENTITY = "LakeGeneratorEntity";
static string POLYLINE_GENERATOR_ENTITY = "PolylineShapeEntity";
static string SPLINE_GENERATOR_ENTITY = "SplineShapeEntity";

static string IS_CLOSED_PROPERTY = "IsClosed";
static string LINE_COLOR_PROPERTY = "LineColor";
static string ROAD_WIDTH_PROPERTY = "RoadWidth";

class PointData
{
	vector position;
	vector inTangent;
	vector outTangent;

	void PointData(vector p, vector inT = "0 0 0", vector outT = "0 0 0")
	{
		position = p;
		inTangent = inT;
		outTangent = outT;
	}
};

enum EGeoExportType
{
	GeoJSON,
	SVG,
};

class GeoProperty
{
	string name;
	string value;
	
	void GeoProperty(string n, string v)
	{
		name = n;
		value = v;
	}
};

class GeoExporter
{
	string output;
	float x_shift;
	float y_shift;
	
	ref FileHandle file;
	
	void GeoExporter(string path, int x, int y)
	{
		output = path;
		x_shift = x;
		y_shift = y;
	}
	
	bool Init()
	{
		file = FileIO.OpenFile(output, FileMode.WRITE);
		if (!file)
		{
			Print("Unable to open file for writting " + output, LogLevel.ERROR);
			return false;
		}
		
		return true;
	}
	
	void Close()
	{
		if (file)
		{
			file.Close();
		}
	}
	
	string GetPath()
	{
		return output;
	}
	
	void Export(string line)
	{
		file.WriteLine(line);
	}
	
	void BeginExport() {}
	void FeatureExport(string name, GeoShapeType shape, array<ref PointData> points, array<ref GeoProperty> props) {}
	void EndExport() {}
};

class GeoSVGExporter : GeoExporter
{
	float max_x = 20000;
	float max_y = 20000;
	
	override void BeginExport()
	{
		WorldEditor we = Workbench.GetModule(WorldEditor);

		vector min, max;
		we.GetTerrainBounds(min, max);
		max_x = max[0];
		max_y = max[2];
		
		Export("<?xml version=\"1.0\" encoding=\"utf-8\"?>");
		Export("<!-- Generator: Enfusion, SVG Export Plug-In  -->");
		
		Export("<svg version=\"1.1\" id=\"WE\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\" width=\""+ (int)max_x + "\" height=\"" + (int)max_y + "\" viewBox=\"0 0 " + (int)max_x + " " + (int)max_y + "\">");
	}
	
	override void FeatureExport(string name, GeoShapeType shape, array<ref PointData> points, array<ref GeoProperty> props)
	{
		if (shape == GeoShapeType.POINT)
		{
			vector pos;
			for (int pIndex = 0; pIndex < points.Count(); pIndex++)
			{
				pos = points.Get(pIndex).position;
				Export(string.Format("<path d=\"M%1,%2\"/>", pos[0] + x_shift, pos[2] + y_shift));
			}
		}
		else if (shape == GeoShapeType.POLYLINE)
		{
			int width = 1;
						
			string polyline = "<polyline fill=\"none\" stroke=\"#000000\" stroke-width=\"" + width + "\" stroke-miterlimit=\"10\" ";
			string pointsAttr = "points=\"";
			vector pos;

			for (int pIndex = 0; pIndex < points.Count(); pIndex++)
			{
				if (pIndex > 0)
				 pointsAttr += " ";
				pos = points.Get(pIndex).position;
				pointsAttr += string.Format("%1,%2", pos[0] + x_shift, pos[2] + y_shift);
			}
			
			polyline += pointsAttr;
			polyline += "\"/>";
			
			Export(polyline);
		}
		else if (shape == GeoShapeType.POLYGON)
		{
			string polyline = "<polygon ";
			string pointsAttr = "points=\"";
			vector pos;

			for (int pIndex = 0; pIndex < points.Count() - 1; pIndex++)
			{
				if (pIndex > 0)
				 pointsAttr += " ";
				pos = points.Get(pIndex).position;
				pointsAttr += string.Format("%1,%2", pos[0] + x_shift, pos[2] + y_shift);
			}
			
			polyline += pointsAttr;
			polyline += "\"/>";
			
			Export(polyline);
		}
		else if (shape == GeoShapeType.UNKNOWN) // Unknown is Spline for us atm
		{
			int width = 1;
			for (int i = 0; i < props.Count(); i++)
			{
				GeoProperty geo = props.Get(i);
				
				if (geo.name == ROAD_WIDTH_PROPERTY && name == ROAD_GENERATOR_ENTITY)
				{
					width = geo.value.ToInt();
				}
			}
			
			string polyline = "<path fill=\"none\" stroke=\"#000000\" stroke-width=\"" + width + "\" ";
			string dAttr = "d=\"";
			vector pos, nextPos;
			vector inTangent, outTangent;

			// https://developer.mozilla.org/en-US/docs/Web/SVG/Tutorial/Paths
			for (int pIndex = 0; pIndex < points.Count() - 1; pIndex++)
			{
				PointData point = points.Get(pIndex);
				PointData nextPoint = points.Get(pIndex + 1);

				// Negate pos.z and add max_y because our Z maps to SVG Y and SVG has Y down but we have Z up.
				// We need to mirror the Z(Y) axis. Also negate the tangent Y coordinate for the same reason.
				pos = point.position;
				pos[0] = pos[0] + x_shift;
				pos[2] = -pos[2] + max_y + y_shift;
				vector ot = point.outTangent;
				ot[2] = -ot[2];
				// Divide by 3 because Enfusion uses cubic Hermite splines, SVG uses cubic Bezier splines
				// Division by 3 converts these two representations https://en.wikipedia.org/wiki/Cubic_Hermite_spline#Representations
				outTangent = pos + ot / 3;

				nextPos = nextPoint.position;
				nextPos[0] = nextPos[0] + x_shift;
				nextPos[2] = -nextPos[2] + max_y + y_shift;
				vector it = nextPoint.inTangent;
				it[2] = -it[2];
				// In tangent needs to be subtracted, in Enfusion we represent tangents always as "forward"
				inTangent = nextPos - it / 3;

				if (pIndex == 0)
				{
					dAttr += string.Format("M %1 %2", pos[0], pos[2]);
				}
	
				dAttr += string.Format(" C %1 %2, %3 %4, %5 %6", outTangent[0], outTangent[2], inTangent[0], inTangent[2], nextPos[0], nextPos[2]);
			}
			
			polyline += dAttr;
			polyline += "\"/>";
			
			Export(polyline);
		}
		
	}
	
	override void EndExport()
	{
		Export("</svg>");
	}
};

class GeoJSONExporter : GeoExporter
{
	int index;
	override void BeginExport()
	{
		Export("{\"type\": \"FeatureCollection\", \"features\": [");
	}
		
	override void FeatureExport(string name, GeoShapeType shape, array<ref PointData> points, array<ref GeoProperty> props)
	{
		string type = "Point";
			
			
		if (shape == GeoShapeType.UNKNOWN) // Spline is polyline for us
			shape = GeoShapeType.POLYLINE;
			
		if (shape == GeoShapeType.POLYLINE)
			type = "LineString";
		else if (shape == GeoShapeType.POLYGON)
			type = "Polygon";
		
		if (index > 0)
			Export(","); // properties end
		
		Export("{\"type\": \"Feature\",\"properties\":{"); // properties
		Export("\"name\": \"" + name + "\"");
				
		for (int i = 0; i < props.Count(); i++)
		{
			GeoProperty geo = props.Get(i);
			Export(",\"" + geo.name + "\": \"" + geo.value + "\"");
		}

		Export("},\"geometry\":{\"type\": \"" + type + "\",\"coordinates\":"); // geometry/coordinates
			
		if (shape == GeoShapeType.POLYGON)
			Export("[["); // polygons can have holes
		if (shape == GeoShapeType.POLYLINE)
			Export("["); // polygons can have holes

		vector pos;
		for (int pIndex = 0; pIndex < points.Count(); pIndex++)
		{
			pos = points.Get(pIndex).position;
					
			if (pIndex > 0)
				Export(",");

			Export(string.Format("[%1,%2]", pos[0] + x_shift, pos[2] + y_shift));
		}
		
		if (shape == GeoShapeType.POLYGON)
			Export("]]"); // polygons can have holes end
		if (shape == GeoShapeType.POLYLINE)
			Export( "]"); // polygons can have holes end
				
		Export("}"); // coordinates/geometry end
		
		Export("}"); // properties end
		
		index++;
	}
	
	override void EndExport()
	{
		Export("]}");
	}
};

/*!
Exports selected entity as a GeoJSON or SVG file.
It support exporting PolylineShapeEntity and SplineShapeEntity (both closed and unclosed). Exporting Generators are also supported, but they needs to have a shape entity as a parent.
Other entities are exported as a single points.
*/

//[WorkbenchPluginAttribute("Export Geographic data", "Export vector data as Geographical data", "", "", {"WorldEditor"})]
//class ExportGeoJSONTool: WorkbenchPlugin

[WorkbenchToolAttribute("Export Geographic data", "Export vector data as Geographical data", "5", awesomeFontCode: 0xf56e)]
class ExportGeoJSONTool: WorldEditorTool
{
	[Attribute("$profile:export", UIWidgets.FileNamePicker, "Where to save exported file. Do not use file suffix, it will be created automatically.")]
	string exportPath;
	
	[Attribute("0", UIWidgets.ComboBox, "Export type", "", ParamEnumArray.FromEnum(EGeoExportType) )]
	int type;
	
	[Attribute("0.0", UIWidgets.EditBox)]
	float x_shift;
	
	[Attribute("0.0", UIWidgets.EditBox)]
	float y_shift;

// you can uncomment this so this functionality can be used as a plugin, not tool
/*	
	//----------------------------------------------------------------------------------------------
	override void Configure()
	{
		Workbench.ScriptDialog("Configure Export Geographic data tool", "", this);
	}
	

	//----------------------------------------------------------------------------------------------
	override void Run()
	{
		Export();
	}
*/
	
	[ButtonAttribute("Export")]
	void Execute()
	{
		Export();
	}
	
	//----------------------------------------------------------------------------------------------
	bool Export()
	{
		if (exportPath.Length() == 0)
		{
			Print("Export file must be set.", LogLevel.ERROR);
			return false;
		}
	
		WorldEditor we = Workbench.GetModule(WorldEditor);
		WorldEditorAPI api = we.GetApi();
		WBProgressDialog progress = new WBProgressDialog("Processing", we);

		int selectedCount = api.GetSelectedEntitiesCount();
		if (selectedCount == 0)
		{
			Print("You need to select at least one entity.", LogLevel.ERROR);
			return false;
		};
		
		GeoExporter exporter;
		
		if (type == EGeoExportType.SVG)
		{
			exporter = new GeoSVGExporter(exportPath + ".svg", x_shift, y_shift);
		}
		else
		{
			exporter = new GeoJSONExporter(exportPath + ".json", x_shift, y_shift);
		}
		
		if (!exporter.Init())
		{
			Print("Unable to initialize the exporter.", LogLevel.ERROR);
			return false;
		}

		Print("GeoExport: exporting " + selectedCount + " entities...");
		exporter.BeginExport();
			
		for (int i = 0; i < selectedCount; i++)
		{
			progress.SetProgress(i / selectedCount);
			
			IEntity e = api.GetSelectedEntity(i);
			IEntitySource src = api.EntityToSource(e);

			string name;
			int shapeCount;
			
			ref array<ref GeoProperty> properties = new array<ref GeoProperty>();
				
			if (src.GetClassName() == ROAD_GENERATOR_ENTITY)
			{
				name = src.GetClassName();

				float roadWidth;
				src.Get(ROAD_WIDTH_PROPERTY, roadWidth);
				
				properties.Insert(new GeoProperty(ROAD_WIDTH_PROPERTY, roadWidth.ToString()));
					
				src = src.GetParent();
			}
				
			if (src.GetClassName() == WALL_GENERATOR_ENTITY)
			{
				name = src.GetClassName();
				src = src.GetParent();
			}

			if (src.GetClassName() == PREFAB_GENERATOR_ENTITY)
			{
				name = src.GetClassName();
				src = src.GetParent();
			}
				
			if (src.GetClassName() == FOREST_GENERATOR_ENTITY)
			{
				name = src.GetClassName();
				src = src.GetParent();
			}
				
			if (src.GetClassName() == LAKE_GENERATOR_ENTITY)
			{
				name = src.GetClassName();
				src = src.GetParent();
			}
				
			if (src && src.GetClassName() == POLYLINE_GENERATOR_ENTITY || src.GetClassName() == SPLINE_GENERATOR_ENTITY)
			{
				if (!name)
					name = src.GetClassName();
					
				bool isClosed;
				src.Get(IS_CLOSED_PROPERTY, isClosed);
				
				GeoShapeType shape = GeoShapeType.UNKNOWN; // INFO: Spline will be unknown for us, right now closed splines are not supported, but we can propage it thru properties or change FeatureExport signature
				if (src.GetClassName() == POLYLINE_GENERATOR_ENTITY)
				{
					if (isClosed)
						shape = GeoShapeType.POLYGON;
					else
						shape = GeoShapeType.POLYLINE;
				}
				
				Color color;
				src.Get(LINE_COLOR_PROPERTY, color);
				string colorProperty = string.Format("%1,%2,%3,%4", Math.Round(color.R() * 255), Math.Round(color.G() * 255), Math.Round(color.B() * 255), Math.Round(color.A() * 255));
				properties.Insert(new GeoProperty(LINE_COLOR_PROPERTY, colorProperty));
				// we can add isClosed to the properties so we can react in the exporters
				
				ShapeEntity shapeEntity = ShapeEntity.Cast(api.SourceToEntity(src)); // can't use e, since parant can be changed
				SplineShapeEntity splineEntity = SplineShapeEntity.Cast(shapeEntity);
				
				array<vector> positions = new array<vector>();
				shapeEntity.GetPointsPositions(positions);
				
				ref array<ref PointData> pointdata = new array<ref PointData>();
				
				for (int j = 0; j < positions.Count(); j++)
				{
					vector pos = shapeEntity.CoordToParent(positions.Get(j));
					vector inTangent = "0 0 0";
					vector outTangent = "0 0 0";
					
					if (splineEntity)
					{
						splineEntity.GetTangents(j, inTangent, outTangent); // tangents are relative to entity
					}
					
					pointdata.Insert(new PointData(pos, inTangent, outTangent));
				}
				
				if (isClosed && pointdata.Count() >= 3)
				{
					vector first = pointdata.Get(0).position;
					vector last = pointdata.Get(pointdata.Count() - 1).position;
					if (first != last)
					{
						pointdata.Insert(new PointData(first));
					}
				}

				exporter.FeatureExport(name, shape, pointdata, properties);
			}
			else
			{
				name = src.GetClassName();
				ref array<ref PointData> pointdata = new array<ref PointData>();
				pointdata.Insert(new PointData(e.GetOrigin()));

				exporter.FeatureExport(name, GeoShapeType.POINT, pointdata, properties);
			}
		}

		PrintFormat("GeoExport: entities export done to %1.", exporter.GetPath());	
		
		exporter.EndExport();
		exporter.Close();
		delete exporter;
		
		return true;
	}
	
};
