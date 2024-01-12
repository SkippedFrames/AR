/**
Attribute base for Name, icon and float value for other attributes to inherent from
*/
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_BaseFloatValueHolderEditorAttribute: SCR_BaseEditorAttribute
{
	[Attribute(desc: "Values")]
	protected ref array<ref SCR_EditorAttributeFloatStringValueHolder> m_aValues;
	
	override int GetEntries(notnull array<ref SCR_BaseEditorAttributeEntry> outEntries)
	{
		outEntries.Insert(new SCR_BaseEditorAttributeFloatStringValues(m_aValues));
		return outEntries.Count();
	}
};

[BaseContainerProps(), BaseContainerCustomTitleField("m_sEntryName")]
class SCR_EditorAttributeFloatStringValueHolder
{	
	[Attribute()]
	protected ResourceName m_Icon;
	
	[Attribute(uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sEntryName;
	
	[Attribute(uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sEntryDescription;
	
	[Attribute()]
	protected float m_fEntryFloatValue;	
	
	void SetIcon(ResourceName newIcon)
	{
		m_Icon = newIcon;
	}
	
	ResourceName GetIcon()
	{
		return m_Icon;
	}
	
	void SetName(string newName)
	{
		m_sEntryName = newName;
	}
	
	string GetName()
	{
		return m_sEntryName;
	}
	
	void SetDescription(string newDescription)
	{
		m_sEntryDescription = newDescription;
	}
	
	string GetDescription()
	{
		return m_sEntryDescription;
	}
	
	void SetFloatValue(float newValue)
	{
		m_fEntryFloatValue = newValue;
	}
	
	float GetFloatValue()
	{
		return m_fEntryFloatValue;
	}
	
	void SetWithUIInfo(SCR_UIInfo info, float value)
	{
		SetName(info.GetName());
		SetIcon(info.GetIconPath());
		SetFloatValue(value);
	}
	
	void SetWithUIInfo(UIInfo info, float value)
	{
		SetName(info.GetName());
		SetIcon(info.GetIconPath());
		SetFloatValue(value);
	}
};
