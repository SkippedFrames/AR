[WorkbenchPluginAttribute("Image Set Generator", "", "", "", {"ResourceManager"},"", 0xf302)]
class ImageSetGenerator: WorkbenchPlugin
{
	const string TAB = " ";
	
	//[Attribute("{717ED53394FC3C27}UI/Textures/Editor/Concept/editor_mockup_sheet.edds", UIWidgets.ResourceNamePicker, "Path to the source image", "edds")]
	//private ResourceName image;
	
	[Attribute("generatedImageSet", UIWidgets.Auto, "File name of the image set")]
	private string fileName;
	
	[Attribute("$ArmaReforger:UI/imagesets/", UIWidgets.Auto, "Directory at which the image set will be saved")]
	private string directory;
	
	[Attribute("512", UIWidgets.Auto, "Width of the image set")]
	private int imageWidth;
	
    [Attribute("512", UIWidgets.Auto, "Height of the image set")]
	private int imageHeight;
	
	[Attribute("", UIWidgets.Auto, "Independent tile areas. There can be multiple within one image set, e.g., left half containing 32x32 tiles and the right one 64x64 tiles")]
	private ref array<ref ImageSetGeneratorArea> areas;
	
	private string tabs;
	private int tabsCount;
	private ref FileHandle file;
	
	void SetTabs(int tabsCountDelta)
	{
		tabsCount += tabsCountDelta;
		tabs = "";
		for (int i = 0; i < tabsCount; i++) tabs += TAB;
	}
	void AddLine(string text)
	{
		file.WriteLine(tabs + text);
	}
	void Generate()
	{
		
		if (!directory.EndsWith("/")) directory += "/";
		string fullPath = directory + fileName + ".imageset";
		file = FileIO.OpenFile(fullPath, FileMode.WRITE);
		Print(file);
		if (!file) return;

		//--- Header
		AddLine("ImageSetClass {");
		SetTabs(1);
			
		AddLine(string.Format("RefSize %1 %2", imageWidth, imageHeight));
		
		/*
		AddLine("Textures {");
		SetTabs(1);
				
		AddLine("ImageSetTextureClass {");
		SetTabs(1);
				
		AddLine("mpix 1");
		AddLine(string.Format("directory \"%1\"", image.GetPath()));
					
		SetTabs(-1);
		AddLine("}");

		SetTabs(-1);
		AddLine("}");
		*/
			
		//--- Groups
		AddLine("Groups {");
		SetTabs(1);
				
		for (int i = 0; i < areas.Count(); i++)
		{
			ImageSetGeneratorArea area = areas[i];
			string groupName = string.Format("%1x%2", area.tileWidth, area.tileHeight);
					
			AddLine(string.Format("ImageSetGroupClass \"%1\" {", groupName));
			SetTabs(1);	
			AddLine(string.Format("Name \"%1\"", groupName));
			AddLine("Images {");
			SetTabs(1);

			//--- Images
			int widthFull = area.tileWidth + area.marginWidth;
			int heightFull = area.tileHeight + area.marginHeight;
			int columns = area.areaWidth / widthFull;
			int rows = area.areaHeight / heightFull;
			for (int r = 0; r < rows; r++)
			{
				for (int c = 0; c < columns; c++)
				{
					string tileName = string.Format("%1-%2", c, r);
					AddLine(string.Format("ImageSetDefClass %1 {", groupName));
					SetTabs(1);
											
					//--- Image
					AddLine(string.Format("Name \"%1\"", tileName));
					AddLine(string.Format("Pos %1 %2", area.areaX + c * widthFull, area.areaY + r * heightFull));
					AddLine(string.Format("Size %1 %2", area.tileWidth, area.tileHeight));
					AddLine("Flags 0");

					SetTabs(-1);
					AddLine("}");
				}
			}

			SetTabs(-1);
			AddLine("}");
						
			SetTabs(-1);
			AddLine("}");
		}
	
				
		SetTabs(-1);
		AddLine("}");
			
		SetTabs(-1);
		AddLine("}");
		
		file.Close();
		
		Print(string.Format("Image set generated at %1", fullPath), LogLevel.DEBUG);
	}
	
	override void Run()
	{
		if (Workbench.ScriptDialog("Image Set Generator", "Generate an image set template with pre-define grids.\nOnce generated, you can simply map the texture and rename existing tiles,\ninstead of having to configure them manually one by one.", this))
		{			
			Generate();
		}
	}
	
	[ButtonAttribute("OK")]
	void ButtonOK()
	{
	}
	
	[ButtonAttribute("Cancel")]
	bool ButtonCancel()
	{
		return false;
	}
};

[BaseContainerProps()]
class ImageSetGeneratorArea
{
    [Attribute("0", UIWidgets.Auto, "Position of left edge")]
	int areaX;
	
    [Attribute("0", UIWidgets.Auto, "Position of top edge")]
	int areaY;
	
    [Attribute("512", UIWidgets.Auto, "Area width")]
	int areaWidth;
	
    [Attribute("512", UIWidgets.Auto, "Area height")]
	int areaHeight;
	
    [Attribute("32", UIWidgets.Auto, "Width of individual tile")]
	int tileWidth;
	
    [Attribute("32", UIWidgets.Auto, "Height of individual tile")]
	int tileHeight;
	
    [Attribute("0", UIWidgets.Auto, "Width of a margin between tiles")]
	int marginWidth;
	
    [Attribute("0", UIWidgets.Auto, "Height of a margin between tiles")]
	int marginHeight;
};
