[WorkbenchToolAttribute("Coords Tool", "Tool that allows you to navigate to coordinates or copy current ones easily.\n\nThis tool provides two options of input:\n1) Enter coordinates into Position and Rotation field manually as\n X,Y,Z and Pitch,Yaw,Roll in degrees then click on the Goto Coords button.\n\n2) Enter enfusion protocol link or string in the following format\n \"x,y,z;pitch,yaw,roll\" into the String field and press the Goto StringLink button.\nNote: This does NOT change current world to one in link if provided.\n\n\nYou can always copy current coordinates by pressing the Copy Link button.", awesomeFontCode : 0xf5a0)]
class CoordsTool : WorldEditorTool
{
	[Attribute("0 0 0", UIWidgets.Auto, "VectorCoordinates: Position in world space to set the camera position to.", category: "Vector Coordinates")]
	protected vector m_vPosition;

	[Attribute("0 0 0", UIWidgets.Auto, "VectorCoordinates: Angles as pitch, yaw, roll to set the camera rotation to.", category: "Vector Coordinates")]
	protected vector m_vRotation;

	[Attribute("", UIWidgets.EditBox, "StringCoordinates: *Enfusion protocol link* or coordinates string as \"x,y,z;pitch,yaw,roll\".", category: "String Coordinates")]
	protected string m_sLink;

	[Attribute(defvalue: "0", desc: "Prefix the link with https prefix (" + WEB_PREFIX + ")", category: "Options")]
	protected bool m_bUseWebPrefix;

	protected static const string WEB_PREFIX = "https://enfusionengine.com/api/redirect?to=";

	//------------------------------------------------------------------------------------------------
	/*!
		Return enfusion protocol link for world editor.
		\param transformation The transformation to create the link at.
	*/
	static string GetWorldEditorLink(vector transformation[4], bool useWebPrefix = false)
	{
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		if (!worldEditor)
			return string.Empty;

		WorldEditorAPI editorApi = worldEditor.GetApi();
		if (!editorApi)
			return string.Empty;

		string fullLink;
		editorApi.GetWorldPath(fullLink);
		if (fullLink.IsEmpty())
			return string.Empty;

		// Fetch position
		vector position = transformation[3];

		// Fetch angles
		vector angles = Math3D.MatrixToAngles(transformation);
		if (angles == vector.Zero) // prevents 0 0 -0
			angles = vector.Zero;

		// We want to substring only /worlds/...
		// to prevent exposing local folders, etc.
		int begin = fullLink.IndexOf("worlds\\");
		if (begin == -1)
			begin = fullLink.IndexOf("worlds/");

		if (begin == -1)
			return string.Empty;

		string worldPath = fullLink.Substring(begin, fullLink.Length() - begin);

		// Have consistent link
		worldPath.Replace("\\", "/");

		// Create link
		string link = string.Format(
			"enfusion://WorldEditor/%1;%2,%3,%4;%5,%6,%7",
			worldPath,
			position[0],
			position[1],
			position[2],
			angles[1],
			angles[0],
			angles[2]);

		if (useWebPrefix)
			link = WEB_PREFIX + link;

		// Print it to console
		return link;
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Sets world editor camera position and rotation.
		\param pos Position in world space.
		\param rot Rotation as pitch, yaw, roll in degrees.
	*/
	protected void SetCamera(vector pos, vector rot)
	{
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		if (!worldEditor)
			return;

		WorldEditorAPI editorApi = worldEditor.GetApi();
		if (!editorApi)
			return;

		vector yawPitchRoll = { rot[1], rot[0], rot[2] };
		vector lookDirection = yawPitchRoll.AnglesToVector();
		editorApi.SetCamera(pos, lookDirection);
		Print(string.Format("Camera set to:\nPosition= %1\nRotation= %2", pos, rot));
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Sets world editor camera position and rotation.
		\param pos Position in world space.
		\param rot Rotation as pitch, yaw, roll in degrees.
	*/
	protected bool GetCamera(out vector transform[4])
	{
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		if (!worldEditor)
			return false;

		WorldEditorAPI editorApi = worldEditor.GetApi();
		if (!editorApi)
			return false;

		BaseWorld world = editorApi.GetWorld();
		if (!world)
			return false;

		world.GetCurrentCamera(transform);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Parses string for coordinates and returns true in case of success, false otherwise.
	*/
	protected bool ParseString(string coordsLink, out vector pos, out vector rot)
	{
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(coordsLink))
			return false;

		if (coordsLink.StartsWith(WEB_PREFIX))
			coordsLink = coordsLink.Substring(WEB_PREFIX.Length(), coordsLink.Length() - WEB_PREFIX.Length());

		// Try parse as enflink to get coords
		// enf link is world;pos;rot
		if (coordsLink.Contains(";") && coordsLink.StartsWith("enfusion"))
		{
			int separatorIndex = coordsLink.IndexOf(";");
			coordsLink = coordsLink.Substring(separatorIndex +1, coordsLink.Length() - separatorIndex -1);
		}

		// split coords into two
		array<string> values = {};
		coordsLink.Split(";", values, true);

		// check validity
		int length = values.Count();
		if (length < 1)
			return false;

		// parse values
		string posString = values[0];
		posString.Replace(",", " ");
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(posString))
			return false;

		vector tempPos = posString.ToVector();
		vector tempRot = vector.Zero;

		if (length > 1)
		{
			string rotString = values[1];
			if (!rotString.IsEmpty())
			{
				rotString.Replace(",", " ");
				tempRot = rotString.ToVector();
			}
		}

		pos = tempPos;
		rot = tempRot;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Copies current transform and generates enfusion link, which is copied to clipboard.
	*/
	[ButtonAttribute("Copy link")]
	protected void CopyLink()
	{
		vector transform[4];
		if (GetCamera(transform))
		{
			string link = GetWorldEditorLink(transform, m_bUseWebPrefix);
			if (!link.IsEmpty())
			{
				System.ExportToClipboard(link);
				Print("Link copied to clipboard:\n" + link);
				return;
			}
		}

		Print("Link could not be copied, an error occurred.", LogLevel.WARNING);
	}

	//------------------------------------------------------------------------------------------------
	/*!
		When clicked by the user, sets the camera to desired position.
	*/
	[ButtonAttribute("Goto Coords")]
	protected void NavigateToCoords()
	{
		SetCamera(m_vPosition, m_vRotation);
	}

	[ButtonAttribute("Goto StringLink")]
	protected void NavigateToStringLink()
	{
		vector pos;
		vector rot;
		if (ParseString(m_sLink, pos, rot))
			SetCamera(pos, rot);
		else
			Print("String is in invalid format. Expected enfusion protocol link or in following format: \"x,y,z;pitch,yaw,roll\"!");
	}
};
