[ComponentEditorProps(category: "GameScripted/Tasks", description: "")]
class SCR_TextsTaskManagerComponentClass: ScriptComponentClass
{
	[Attribute()]
	protected ref array<ref SCR_TextsTaskManagerEntry> m_aEntries;
	
	int GetTexts(ETaskTextType type, out notnull array<ref SCR_UIDescription> outInfos)
	{
		foreach (SCR_TextsTaskManagerEntry entry: m_aEntries)
		{
			if (entry.m_TextType == type)
			{
				int count = entry.m_aTexts.Count();
				for (int i; i < count; i++)
				{
					outInfos.Insert(entry.m_aTexts[i]);
				}
				return count;
			}
		}
		return 0;
	}
	SCR_UIDescription GetText(ETaskTextType type, int index)
	{
		foreach (SCR_TextsTaskManagerEntry entry: m_aEntries)
		{
			if (entry.m_TextType == type)
			{
				return entry.m_aTexts[index];
			}
		}
		return null;
	}
};
class SCR_TextsTaskManagerComponent: ScriptComponent
{
	/*!
	Get instance of this class.
	\return SCR_TextsTaskManagerComponent attached to SCR_BaseTaskManager
	*/
	static SCR_TextsTaskManagerComponent GetInstance()
	{
		SCR_BaseTaskManager taskManager = GetTaskManager();
		if (taskManager)
			return SCR_TextsTaskManagerComponent.Cast(taskManager.FindComponent(SCR_TextsTaskManagerComponent));
		else
			return null;
	}
	/*!
	Get all texts for given text type.
	\param type Text type
	\param[out] outInfos Array to be filled with UI infos
	*/
	int GetTexts(ETaskTextType type, out notnull array<ref SCR_UIDescription> outInfos)
	{
		SCR_TextsTaskManagerComponentClass prefabData = SCR_TextsTaskManagerComponentClass.Cast(GetComponentData(GetOwner()));
		if (prefabData)
			return prefabData.GetTexts(type, outInfos);
		else
			return 0;
	}
	/*!
	Get specific text for given type.
	\param type Text type
	\param index Index of text entry
	\return UI info
	*/
	SCR_UIDescription GetText(ETaskTextType type, int index)
	{
		SCR_TextsTaskManagerComponentClass prefabData = SCR_TextsTaskManagerComponentClass.Cast(GetComponentData(GetOwner()));
		if (prefabData)
			return prefabData.GetText(type, index);
		else
			return null;
	}
};

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ETaskTextType, "m_TextType")]
class SCR_TextsTaskManagerEntry
{
	[Attribute(SCR_Enum.GetDefault(ETaskTextType.NONE), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ETaskTextType))]
	ETaskTextType m_TextType;
	
	[Attribute()]
	ref array<ref SCR_UIDescription> m_aTexts;
};

enum ETaskTextType
{
	NONE,
	CUSTOM,
	MOVE,
	DEFEND,
	ATTACK
};
