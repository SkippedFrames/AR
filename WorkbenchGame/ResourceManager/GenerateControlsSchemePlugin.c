enum GenerateControlsSchemeType
{
	KEYBOARD,
	MOUSE,
	GAMEPAD
};

class GenerateControlsSchemeGamepadLabel
{
	protected string m_sAnchor;
	protected float m_fTextX;
	protected float m_fTextY;
	protected float m_fLineX1;
	protected float m_fLineX2;
	protected float m_fLineY;
	protected float m_fTargetX;
	protected float m_fTargetY;
	protected string m_sSymbol;
	
	void GenerateControlsSchemeGamepadLabel(string anchor, float textX, float textY, float lineX1, float lineX2, float lineY, float targetX, float targetY, string symbol = "")
	{
		m_sAnchor = anchor;
		m_fTextX = textX;
		m_fTextY = textY;
		m_fLineX1 = lineX1;
		m_fLineX2 = lineX2;
		m_fLineY = lineY;
		m_fTargetX = targetX;
		m_fTargetY = targetY;
		m_sSymbol = symbol;
	}
	
	void AddText(out string text, string binding)
	{
		if (m_sSymbol != "")
		{
			if (m_sAnchor == "end")
				binding = binding + " " +  m_sSymbol;
			else
				binding = m_sSymbol + " " + binding;
		}
		text += string.Format("<text x='%3' y='%4' text-anchor='%2'>%1</text>\n", binding, m_sAnchor, m_fTextX, m_fTextY);
	}
	void AddLines(out string text)
	{
		//text += string.Format("<text x='%3' y='%4' text-anchor='%2'>%1</text>\n", binding, m_sAnchor, m_fTextX, m_fTextY);
		text += string.Format("<line x1='%1' x2='%2' y1='%3' y2='%3' />\n", m_fLineX1, m_fLineX2, m_fLineY, m_fTargetX, m_fTargetY);
		text += string.Format("<line x1='%2' x2='%4' y1='%3' y2='%5' />\n", m_fLineX1, m_fLineX2, m_fLineY, m_fTargetX, m_fTargetY);
	}
};

[WorkbenchPluginAttribute("Generate Controls Scheme", "", "", "", {"ResourceManager"},"",0xf11b)]
class GenerateControlsSchemePlugin : WorkbenchPlugin
{
	[Attribute("", UIWidgets.Auto, "A list of contexts to be exported. Actions from all contexts will be merged into one image.")]
	private ref array<string> contextNames;
	
	[Attribute("$profile:GenerateControlsScheme/", UIWidgets.Auto, "Directory at which the scheme will be saved")]
	private string directory;
	
	//[Attribute("2", UIWidgets.ComboBox, "", enums: ParamEnumArray.FromEnum(GenerateControlsSchemeType))]
	private GenerateControlsSchemeType type = 2;
	
	[Attribute("1", UIWidgets.Auto, "Font size multiplier. Use a smaller font when action names are too long and overflow from the image.")]
	private float fontSize;
	
	[Attribute("1", UIWidgets.Auto, "True to label actions that are not used in given contexts. Unassigned actions will be shown in faded color.")]
	private bool showUnassigned;
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Controller
	bool ProcessBinding_Gamepad(map<string, ref array<string>> allBindings, string fullPath, string contextTitle)
	{
		bool didCopy = FileIO.CopyFile("scripts/WorkbenchGame/GenerateControlsScheme_Gamepad.txt", fullPath);
		if (!didCopy)
		{
			Print(string.Format("Cannot copy header to '%1'!", fullPath), LogLevel.ERROR);
			return false;
		};
		
		FileHandle file = FileIO.OpenFile(fullPath, FileMode.APPEND);
		if (!file)
		{
			Print(string.Format("Cannot open file at '%1'!", fullPath), LogLevel.ERROR);
			return false;
		};

		float tL1 = 280;
		float lL1 = 290;
		float lL2 = 370;
		
		float tR1 = 800;
		float lR1 = 790;
		float lR2 = 710;
		
		map<string, ref GenerateControlsSchemeGamepadLabel> labels = new map<string, ref GenerateControlsSchemeGamepadLabel>;		
		labels.Insert("gamepad0:view", 						new GenerateControlsSchemeGamepadLabel("end", 	500, 040, 510, 510, 035, 510, 245));
		labels.Insert("gamepad0:menu", 						new GenerateControlsSchemeGamepadLabel("start", 580, 040, 572, 572, 035, 572, 245));
		
		labels.Insert("gamepad0:left_trigger", 				new GenerateControlsSchemeGamepadLabel("end", 	tL1, 070, lL1, lL2, 065, 450, 171));
		labels.Insert("gamepad0:shoulder_left", 			new GenerateControlsSchemeGamepadLabel("end", 	tL1, 130, lL1, lL2, 125, 420, 185));
		labels.Insert("gamepad0:left_thumb_horizontal", 	new GenerateControlsSchemeGamepadLabel("end", 	tL1, 250, lL1, lL2, 275, 420, 250, "⭤"));
		labels.Insert("gamepad0:left_thumb_vertical", 		new GenerateControlsSchemeGamepadLabel("end", 	tL1, 280, lL1, lL2, 275, 420, 250, "⭥"));
		labels.Insert("gamepad0:thumb_left", 				new GenerateControlsSchemeGamepadLabel("end", 	tL1, 310, lL1, lL2, 275, 420, 250, "●"));
		labels.Insert("gamepad0:pad_up", 					new GenerateControlsSchemeGamepadLabel("end", 	tL1, 360, lL1, lL2, 400, 460, 320, "▲"));//355, 480, 295, "▲"));
		labels.Insert("gamepad0:pad_left", 					new GenerateControlsSchemeGamepadLabel("end", 	tL1, 390, lL1, lL2, 400, 460, 320, "◄"));//385, 460, 325, "◄"));
		labels.Insert("gamepad0:pad_right", 				new GenerateControlsSchemeGamepadLabel("end", 	tL1, 420, lL1, lL2, 400, 460, 320, "►"));//415, 505, 320, "►"));
		labels.Insert("gamepad0:pad_down", 					new GenerateControlsSchemeGamepadLabel("end", 	tL1, 450, lL1, lL2, 400, 460, 320, "▼"));//445, 490, 345, "▼"));
		
		labels.Insert("gamepad0:right_trigger", 			new GenerateControlsSchemeGamepadLabel("start", tR1, 070, lR1, lR2, 065, 630, 171));
		labels.Insert("gamepad0:shoulder_right", 			new GenerateControlsSchemeGamepadLabel("start", tR1, 130, lR1, lR2, 125, 660, 185));
		labels.Insert("gamepad0:y", 						new GenerateControlsSchemeGamepadLabel("start", tR1, 190, lR1, lR2, 185, 660, 220));
		labels.Insert("gamepad0:x", 						new GenerateControlsSchemeGamepadLabel("start", tR1, 220, lR1, lR2, 215, 630, 250));
		labels.Insert("gamepad0:b", 						new GenerateControlsSchemeGamepadLabel("start", tR1, 250, lR1, lR2, 245, 690, 250));
		labels.Insert("gamepad0:a", 						new GenerateControlsSchemeGamepadLabel("start", tR1, 280, lR1, lR2, 275, 660, 280));
		labels.Insert("gamepad0:right_thumb_horizontal", 	new GenerateControlsSchemeGamepadLabel("start", tR1, 360, lR1, lR2, 385, 610, 320, "⭤"));
		labels.Insert("gamepad0:right_thumb_vertical", 		new GenerateControlsSchemeGamepadLabel("start", tR1, 390, lR1, lR2, 385, 610, 320, "⭥"));
		labels.Insert("gamepad0:thumb_right", 				new GenerateControlsSchemeGamepadLabel("start", tR1, 420, lR1, lR2, 385, 610, 320, "●"));

		string output;
		string texts = string.Format("<text x='20' y='570' text-anchor='start' font-size='25' font-weight='bold'>%1</text>\n", contextTitle);
		string lines;
		string textsDisabled;
		string linesDisabled;
		
		for (int l = 0; l < labels.Count(); l++)
		{
			GenerateControlsSchemeGamepadLabel label= labels.GetElement(l);
			ref array<string> bindings;
			if (allBindings.Find(labels.GetKey(l), bindings))
			{
				label.AddText(texts, bindings[0]);
				label.AddLines(lines);
			}
			else if (showUnassigned)
			{
				label.AddText(textsDisabled, "N/A");
				label.AddLines(linesDisabled);
			}
		}
		
		file.WriteLine(string.Format("<g font-family='Roboto' font-size='%1' fill='#ccc'>", 20 * fontSize));
		file.WriteLine(textsDisabled);
		file.WriteLine("</g>");
		
		file.WriteLine(string.Format("<g font-family='Roboto' font-size='%1' fill='black'>", 20 * fontSize));
		file.WriteLine(texts);
		file.WriteLine("</g>");
		
		file.WriteLine("<g stroke='black' stroke-width='2' opacity='0.1' transform='translate(0.5,0.5)'>");
		file.WriteLine(linesDisabled);
		file.WriteLine("</g>");
		
		file.WriteLine("<g stroke='black' stroke-width='2' opacity='1' transform='translate(0.5,0.5)'>");
		file.WriteLine(lines);
		file.WriteLine("</g>");
		
		file.WriteLine("</svg>");
		
		file.Close();
		
		return true;
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Keyboard
	bool ProcessBinding_Keyboard(map<string, ref array<string>> allBindings, string fileName, string contextTitle)
	{
		return false;
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Mouse
	bool ProcessBinding_Mouse(map<string, ref array<string>> allBindings, string fileName, string contextTitle)
	{
		return false;
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Find all relevant actions and generate the image
	void Generate()
	{
		if (!contextNames) return;
		InputBinding input = GetGame().GetInputManager().CreateUserBinding();
		
		//--- Whitelist specific keys based on selected input type
		ref array<string> whitelist = new array<string>();
		switch (type)
		{
			case GenerateControlsSchemeType.KEYBOARD:
				break;
			case GenerateControlsSchemeType.MOUSE:
				break;
			case GenerateControlsSchemeType.GAMEPAD:
				whitelist.Insert("gamepad0:menu");
				whitelist.Insert("gamepad0:view");
				whitelist.Insert("gamepad0:a");
				whitelist.Insert("gamepad0:b");
				whitelist.Insert("gamepad0:x");
				whitelist.Insert("gamepad0:y");
				whitelist.Insert("gamepad0:pad_up");
				whitelist.Insert("gamepad0:pad_down");
				whitelist.Insert("gamepad0:pad_left");
				whitelist.Insert("gamepad0:pad_right");
				whitelist.Insert("gamepad0:shoulder_left");
				whitelist.Insert("gamepad0:shoulder_right");
				whitelist.Insert("gamepad0:thumb_left");
				whitelist.Insert("gamepad0:thumb_right");
				whitelist.Insert("gamepad0:left_thumb_horizontal");
				whitelist.Insert("gamepad0:left_thumb_vertical");
				whitelist.Insert("gamepad0:right_thumb_horizontal");
				whitelist.Insert("gamepad0:right_thumb_vertical");
				whitelist.Insert("gamepad0:left_trigger");
				whitelist.Insert("gamepad0:right_trigger");
				break;
		}
		
		//--- Find relevant actions
		map<string, ref array<string>> allBindings = new map <string, ref array<string>>();
		string contextTitle;
		for (int c = 0; c < contextNames.Count(); c++)
		{
			BaseContainer context = input.FindContext(contextNames[c]);
			if (!context)
			{
				Print(string.Format("Context '%1' not found!", contextNames[c]), LogLevel.ERROR);
				continue;
			}
			ref array<string> actionNames = new array<string>();
			if (!context.Get("ActionRefs", actionNames)) continue;
		
			for (int a = 0; a < actionNames.Count(); a++)
			{
				string actionName = actionNames[a];
				array<string> bindings = {};
				if (!input.GetBindings(actionName, bindings, EInputDeviceType.MOUSE)) continue;
				if (!input.GetBindings(actionName, bindings, EInputDeviceType.KEYBOARD)) continue;
				if (!input.GetBindings(actionName, bindings, EInputDeviceType.GAMEPAD)) continue;
				if (!input.GetBindings(actionName, bindings, EInputDeviceType.JOYSTICK)) continue;
				
				for (int b = 0; b < bindings.Count(); b++)
				{
					string binding = bindings[b];
					if (whitelist.Find(binding) < 0) continue;
					
					ref array<string> currentBindings;
					allBindings.Find(binding, currentBindings);
					if (!currentBindings) currentBindings = new array<string>();
					currentBindings.Insert(actionName);
					allBindings.Insert(binding, currentBindings);
				}
			}
			if (contextTitle != "") contextTitle += ", ";
			contextTitle += contextNames[c];
		}
		
		
		string fileName;
		for (int c = 0; c < contextNames.Count(); c++) fileName += "_" + contextNames[c];
		string directoryLocal = directory;
		if (!directoryLocal.EndsWith("/")) directoryLocal += "/";
		
		string fullPath;
		bool succeeded;
		switch (type)
		{
			case GenerateControlsSchemeType.KEYBOARD:
			{
				fullPath = directoryLocal + "keyboard" + fileName + ".svg";
				succeeded = ProcessBinding_Keyboard(allBindings, fullPath, contextTitle);
				break;
			}
			case GenerateControlsSchemeType.MOUSE:
			{
				fullPath = directoryLocal + "mouse" + fileName + ".svg";
				succeeded = ProcessBinding_Mouse(allBindings, fullPath, contextTitle);
				break;
			}
			case GenerateControlsSchemeType.GAMEPAD:
			{
				fullPath = directoryLocal + "gamepad" + fileName + ".svg";
				succeeded = ProcessBinding_Gamepad(allBindings, fullPath, contextTitle);
				break;
			}
		}
		if (succeeded)
		{
			Print(string.Format("Controls scheme generated in '%1'", fullPath), LogLevel.DEBUG);
		}
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Default functions
	override void Run()
	{
		if (Workbench.ScriptDialog("Generate Controls Scheme", "Generate a *.svg file which lists all actions in given contexts.\nThe file can be attached to Confluence page to showcase a control scheme.", this))
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
