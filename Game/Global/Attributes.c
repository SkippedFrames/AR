/*!
	\brief Attribute to manually set a static title.
	@code
	[BaseContainerProps(), SCR_BaseContainerHandMadeTitleField("My Super Title")]
	class TestConfigClass
	{
	}
	@endcode
*/
class SCR_BaseContainerStaticTitleField : BaseContainerCustomTitle
{
	protected string m_sCustomTitle;

	//------------------------------------------------------------------------------------------------
	void SCR_BaseContainerStaticTitleField(string customTitle = "")
	{
		customTitle.Trim();
		m_sCustomTitle = customTitle;
	}

	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		// if (m_sCustomTitle.IsEmpty())
		// 	return false;

		title = m_sCustomTitle;
		return true;
	}
};

/*!
	\brief Attribute for setting any string property as custom title.
	@code
	[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sDisplayName", "Title is %1")]
	class TestConfigClass
	{
		[Attribute()]
		protected string m_sDisplayName;
	}
	@endcode
*/
class SCR_BaseContainerCustomTitleField : BaseContainerCustomTitle
{
	protected string m_sPropertyName;
	protected string m_sFormat;

	//------------------------------------------------------------------------------------------------
	void SCR_BaseContainerCustomTitleField(string propertyName, string format = "%1")
	{
		m_sPropertyName = propertyName;
		m_sFormat = format;
	}

	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		if (!source.Get(m_sPropertyName, title))
			return false;

		title = string.Format(m_sFormat, title);
		return true;
	}
};

/*!
	\brief allow to define multiple fields - up to 9 elements
	@code
	[BaseContainerProps(), SCR_BaseContainerCustomTitleFields({ "m_sTitle", "m_sIcon" }, "[%2] %1")]
	class TestConfigClass
	{
	}
	@endcode
*/
class SCR_BaseContainerCustomTitleFields : BaseContainerCustomTitle
{
	protected ref array<string> m_aPropertyNames;
	protected string m_sFormat;

	//------------------------------------------------------------------------------------------------
	void SCR_BaseContainerCustomTitleFields(array<string> propertyNames, string format = "%1")
	{
		m_aPropertyNames = propertyNames;
		m_sFormat = format;
	}

	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		if (!m_aPropertyNames)
		{
			title = m_sFormat;
			return false;
		}

		int count = m_aPropertyNames.Count();
		array<string> arguments = {};
		arguments.Resize(count); // needed here

		for (int i = 0; i < count; i++)
		{
			title = "<°))))-<";
			if (!source.Get(m_aPropertyNames[i], title) || title == "<°))))-<")
			{
				ResourceName tempResourceName;
				if (source.Get(m_aPropertyNames[i], tempResourceName))
					title = FilePath.StripPath(tempResourceName);
				else
					title = "x";
			}

			arguments[i] = title;
		}

		title = SCR_StringHelper.Format(m_sFormat, arguments);

		return true;
	}
};

/*!
	\brief Attribute to manually set a LOCALIZED (translated) title.
	@code
	[BaseContainerProps(), SCR_BaseContainerLocalizedTitleField("m_sTitle")]
	class TestConfigClass
	{
	}
	@endcode
*/
class SCR_BaseContainerLocalizedTitleField : BaseContainerCustomTitle
{
	protected string m_sPropertyName;
	protected string m_sFormat;

	//------------------------------------------------------------------------------------------------
	void SCR_BaseContainerLocalizedTitleField(string propertyName, string format = "%1")
	{
		m_sPropertyName = propertyName;
		m_sFormat = format;
	}

	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		if (!source.Get(m_sPropertyName, title))
			return false;

		title = string.Format(m_sFormat, WidgetManager.Translate(title));
		return true;
	}
};

/*!
	\brief allow to define multiple fields - up to 9 elements
	@code
	[BaseContainerProps(), SCR_BaseContainerLocalizedTitleFields({ "m_sTitle", "m_sIcon" }, "[%2] %1")]
	class TestConfigClass
	{
	}
	@endcode
*/
class SCR_BaseContainerLocalizedTitleFields : BaseContainerCustomTitle
{
	protected ref array<string> m_aPropertyNames;
	protected string m_sFormat;

	//------------------------------------------------------------------------------------------------
	void SCR_BaseContainerMultipleParametersTitleField(array<string> propertyNames, string format = "%1")
	{
		m_aPropertyNames = propertyNames;
		m_sFormat = format;
	}

	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		if (!m_aPropertyNames)
		{
			title = m_sFormat;
			return false;
		}

		int count = m_aPropertyNames.Count();
		array<string> arguments = {};
		arguments.Resize(count); // needed here

		for (int i = 0; i < count; i++)
		{
			title = "<°))))-<"; // looks fishy hey?
			if (!source.Get(m_aPropertyNames[i], title) || title == "<°))))-<")
			{
				ResourceName tempResourceName;
				if (source.Get(m_aPropertyNames[i], tempResourceName) && title != "<°))))-<")
					title = FilePath.StripPath(tempResourceName);
				else
					title = "x";
			}

			arguments[i] = title;
		}

		title = SCR_StringHelper.Translate(m_sFormat, arguments);

		return true;
	}
};
/*!
	\brief Attribute to use a ResourceName filename
	@code
	[BaseContainerProps(), SCR_BaseContainerResourceTitleField("m_sPrefab", "Prefab: %1")]
	class TestConfigClass
	{
		[Attribute()]
		protected ResourceName m_sPrefab;
	};
	@endcode
*/
class SCR_BaseContainerResourceTitleField : BaseContainerCustomTitle
{
	protected string m_sPropertyName;
	protected string m_sFormat;

	//------------------------------------------------------------------------------------------------
	void SCR_BaseContainerResourceTitleField(string propertyName, string format = "%1")
	{
		m_sPropertyName = propertyName;
		m_sFormat = format;
	}

	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		ResourceName resourceName;
		if (!source.Get(m_sPropertyName, resourceName))
			return false;

		title = string.Format(m_sFormat, FilePath.StripPath(resourceName));
		return true;
	}
};

/*!
	\brief Attribute for setting any enum property as custom title.
	@code
	[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditorMode, "m_Mode")]
	class TestConfigClass
	{
		[Attribute()]
		protected EEditorMode m_Mode;
	}
	@endcode
*/
class SCR_BaseContainerCustomTitleEnum : BaseContainerCustomTitle
{
	protected typename m_EnumType;
	protected string m_PropertyName;
	protected string m_sFormat;

	//------------------------------------------------------------------------------------------------
	void SCR_BaseContainerCustomTitleEnum(typename enumType, string propertyName, string format = "%1")
	{
		m_EnumType = enumType;
		m_PropertyName = propertyName;
		m_sFormat = format;
	}

	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		int enumValue;
		if (!source.Get(m_PropertyName, enumValue))
		{
			return false;
		}

		title = string.Format(m_sFormat, typename.EnumToString(m_EnumType, enumValue));
		return true;
	}
};

/*!
	\brief Attribute for setting any flags enum property as custom title.
	@code
	[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditorMode, "m_Modes")]
	class TestConfigClass
	{
		[Attribute()]
		protected EEditorMode m_Modes;
	}
	@endcode
*/
class SCR_BaseContainerCustomTitleFlags : BaseContainerCustomTitle
{
	protected typename m_EnumType;
	protected string m_PropertyName;
	protected string m_sFormat;

	//------------------------------------------------------------------------------------------------
	void SCR_BaseContainerCustomTitleFlags(typename enumType, string propertyName, string format = "%1")
	{
		m_EnumType = enumType;
		m_PropertyName = propertyName;
		m_sFormat = format;
	}

	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		int enumValue;
		if (!source.Get(m_PropertyName, enumValue))
			return false;

		array<int> values = {};
		for (int i = 0, count = SCR_Enum.BitToIntArray(enumValue, values); i < count; i++)
		{
			if (i > 0)
				title += " | ";

			title += typename.EnumToString(m_EnumType, values[i]);
		}

		title = string.Format(m_sFormat, title);
		return true;
	}
};

/*!
	\brief Attribute for setting any ResourceName path property as custom title.
	@code
	[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_Path")]
	class TestConfigClass
	{
		[Attribute()]
		protected ResourceName m_Path;
	}
	@endcode
*/
class SCR_BaseContainerCustomTitleResourceName : BaseContainerCustomTitle
{
	protected string m_sPropertyName;
	protected bool m_bFileNameOnly;
	protected string m_sFormat;

	//------------------------------------------------------------------------------------------------
	void SCR_BaseContainerCustomTitleResourceName(string propertyName, bool fileNameOnly = false, string format = "%1")
	{
		m_sPropertyName = propertyName;
		m_bFileNameOnly = fileNameOnly;
		m_sFormat = format;
	}

	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		ResourceName path;
		if (!source.Get(m_sPropertyName, path))
			return false;

		title = path.GetPath();
		if (m_bFileNameOnly)
			title = string.Format(m_sFormat, FilePath.StripPath(title));

		return true;
	}
};

/*!
	\brief Attribute for setting any object classname property as custom title.
	@code
	[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_Variable")]
	class TestConfigClass
	{
		[Attribute()]
		protected SCR_SomeClass m_Variable;
	}
	@endcode
*/
class SCR_BaseContainerCustomTitleObject : BaseContainerCustomTitle
{
	protected string m_sPropertyName;
	protected string m_sFormat;

	//------------------------------------------------------------------------------------------------
	void SCR_BaseContainerCustomTitleObject(string propertyName, string format = "%1")
	{
		m_sPropertyName = propertyName;
		m_sFormat = format;
	}

	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		BaseContainer object = source.GetObject(m_sPropertyName);
		if (!object)
			return false;

		title = string.Format(m_sFormat, object.GetClassName());
		return true;
	}
};

/*!
	\brief Attribute for setting a custom string as title
	@code
	[BaseContainerProps(), BaseContainerCustomStringTitleField("Title")]
	class TestConfigClass
	{
	}
	@endcode
*/
class BaseContainerCustomStringTitleField : BaseContainerCustomTitle
{
	string m_Title;

	//------------------------------------------------------------------------------------------------
	void BaseContainerCustomStringTitleField(string title)
	{
		m_Title = title;
	}

	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		title = m_Title;
		return !title.IsEmpty();
	}
};

/*!
	\brief Attribute for setting two string property as custom title.
	@code
	[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sDisplayName1", m_sDisplayName2, "Title is %1")]
	class TestConfigClass
	{
		[Attribute()]
		protected string m_sDisplayName;
	}
	@endcode
*/
class BaseContainerCustomDoubleTitleField : BaseContainerCustomTitle
{
	protected string m_sPropertyName1;
	protected string m_sPropertyName2;
	protected string m_sFormat;

	//------------------------------------------------------------------------------------------------
	void BaseContainerCustomDoubleTitleField(string propertyName1, string propertyName2, string format = "%1: %2")
	{
		m_sPropertyName1 = propertyName1;
		m_sPropertyName2 = propertyName2;
		m_sFormat = format;
	}

	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		string title1, title2;

		if (!source.Get(m_sPropertyName1, title1))
			return false;

		if (!source.Get(m_sPropertyName2, title2))
			return false;

		title = string.Format(m_sFormat, title1, title2);
		return true;
	}
};

/*!
	\brief Attribute for setting a custom format if the given checkVar is equal to checkVarEqual. If true sets propertyName as title with format else just shows propertyName as title
	@code
	[BaseContainerProps(), BaseContainerCustomDoubleCheckTitleField("m_bEnabled", "m_sDisplayName", "Test", "%1", "(Disabled) %1")]
	class TestConfigClass
	{
		[Attribute()]
		protected bool m_bEnabled;

		[Attribute()]
		protected string m_sDisplayName;

	}
	@endcode
*/
class BaseContainerCustomDoubleCheckTitleField : BaseContainerCustomTitle
{
	protected string m_sCheckVar;
	protected string m_sPropertyName;
	protected string m_sFormatTrue;
	protected string m_sFormatFalse;
	protected string m_sCheckVarEqual;

	//------------------------------------------------------------------------------------------------
	void BaseContainerCustomDoubleCheckTitleField(string checkVar, string propertyName, string checkVarEqual = "1", string formatTrue = "%1", string formatFalse = "EXAMPLE FORMAT - %1")
	{
		m_sCheckVar = checkVar;
		m_sPropertyName = propertyName;
		m_sFormatTrue = formatTrue;
		m_sFormatFalse = formatFalse;
		m_sCheckVarEqual = checkVarEqual;
	}

	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		string checkVar, titleName;

		if (!source.Get(m_sCheckVar, checkVar))
			return false;

		if (!source.Get(m_sPropertyName, titleName))
			return false;

		if (checkVar == m_sCheckVarEqual)
			title = string.Format(m_sFormatTrue, titleName);
		else
			title = string.Format(m_sFormatFalse, titleName);

		return true;
	}
};

/*!
	\brief Attribute for setting a custom format if the given checkVar is equal to checkVarEqual. If true sets propertyName ResourceName as title with format else just shows ResourceName as title
	@code
	[BaseContainerProps(), BaseContainerCustomDoubleCheckResourceNameTitleField("m_bEnabled", "m_sPrefabResourceName", 0, "%1", "(Disabled) %1", true)]
	class TestConfigClass
	{
		[Attribute()]
		protected bool m_bEnabled;

		[Attribute()]
		protected ResourceName m_sPrefabResourceName;

	}
	@endcode
*/
class BaseContainerCustomDoubleCheckIntResourceNameTitleField : BaseContainerCustomTitle
{
	protected string m_sCheckVar;
	protected string m_sPropertyName;
	protected string m_sFormatTrue;
	protected string m_sFormatFalse;
	protected float m_iCheckVarEqual;
	protected bool m_bFileNameOnly;

	//------------------------------------------------------------------------------------------------
	void BaseContainerCustomDoubleCheckIntResourceNameTitleField(string checkVar, string propertyName, int checkVarEqual, string formatTrue = "%1", string formatFalse = "EXAMPLE FORMAT - %1", bool fileNameOnly = true)
	{
		m_sCheckVar = checkVar;
		m_sPropertyName = propertyName;
		m_sFormatTrue = formatTrue;
		m_sFormatFalse = formatFalse;
		m_iCheckVarEqual = checkVarEqual;
		m_bFileNameOnly = fileNameOnly;
	}

	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		int checkVar;
		string pathString;

		if (!source.Get(m_sCheckVar, checkVar))
			return false;

		ResourceName path;
		if (!source.Get(m_sPropertyName, path))
			return false;

		if (path.IsEmpty())
			pathString = "NO PREFAB";
		else
			pathString = path.GetPath();

		if (checkVar == m_iCheckVarEqual)
		{
			if (!m_bFileNameOnly || path.IsEmpty())
				title = string.Format(m_sFormatTrue, pathString);
			else
				title = string.Format(m_sFormatTrue, FilePath.StripPath(pathString));
		}
		else
		{
			if (!m_bFileNameOnly || path.IsEmpty())
				title = string.Format(m_sFormatFalse, pathString);
			else
				title = string.Format(m_sFormatFalse, FilePath.StripPath(pathString));
		}

		return true;
	}
};

/*!
	\brief Attribute for setting a custom format if the given checkVar is equal to checkVarEqual. If condition met just shows default TitleField, if false it show default within given format
	@code
	[BaseContainerProps(), BaseContainerCustomCheckIntTitleField("m_bEnabled", "Enabled test", "disabled test", 1)]
	class TestConfigClass
	{
		[Attribute()]
		protected bool m_bEnabled;
	}
	@endcode
*/
class BaseContainerCustomCheckIntTitleField : BaseContainerCustomTitle
{
	protected string m_sCheckVar;
	protected string m_sConditionTrueText;
	protected string m_sConditionFalseText;
	protected int m_iCheckVarEqual;

	//------------------------------------------------------------------------------------------------
	void BaseContainerCustomCheckIntTitleField(string checkVar, string conditionTrueText, string conditionFalseText, int checkVarEqual)
	{
		m_sCheckVar = checkVar;
		m_sConditionTrueText = conditionTrueText;
		m_sConditionFalseText = conditionFalseText;
		m_iCheckVarEqual = checkVarEqual;
	}

	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		int checkVar;

		if (!source.Get(m_sCheckVar, checkVar))
			return false;

		if (checkVar == m_iCheckVarEqual)
			title = m_sConditionTrueText;
		else
			title = m_sConditionFalseText;

		return true;
	}
};

/*!
	\brief Attribute for setting UIInfo's name property as (Localized) custom title.
	@code
	[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
	class TestConfigClass
	{
		[Attribute()]
		protected SCR_UIInfo m_Info;
	}
	@endcode
*/
class SCR_BaseContainerCustomTitleUIInfo : BaseContainerCustomTitle
{
	protected string m_sPropertyName;
	protected string m_sFormat;

	//------------------------------------------------------------------------------------------------
	void SCR_BaseContainerCustomTitleUIInfo(string propertyName, string format = "%1")
	{
		m_sPropertyName = propertyName;
		m_sFormat = format;
	}

	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		BaseContainer info = source.GetObject(m_sPropertyName);
		if (!info || !info.Get("Name", title))
			return false;

		title = string.Format(m_sFormat, WidgetManager.Translate(title));
		return true;
	}
};
