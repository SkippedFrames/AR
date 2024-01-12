/*
Config which specifies the appearence of the chat messasge.
*/

[BaseContainerProps(configRoot: true)]
class SCR_ChatMessageStyle : Managed
{
	[Attribute("", UIWidgets.ColorPicker, "Color")]
	ref Color m_Color;
	
	[Attribute("", UIWidgets.CheckBox, "Badge is colored")]
	bool m_bColoredBadge;
	
	[Attribute("", UIWidgets.CheckBox, "Icon is colored")]
	bool m_bColoredIcon;
	
	[Attribute("", UIWidgets.CheckBox, "Background is colored")]
	bool m_bColoredBackground;
	
	[Attribute("", UIWidgets.CheckBox, "Player name is colored")]
	bool m_bColoredPlayerName;
	
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Imageset for the icon", params: "imageset")]
	ResourceName m_IconImageset;
	
	[Attribute("", UIWidgets.EditBox, "Image name in the imageset")]
	string m_sIconName;
	
	[Attribute("<Name>", UIWidgets.EditBox, "Channel name")]
	string m_sName;
	
	[Attribute("<name>", UIWidgets.EditBox, "Channel name, all lowercase")]
	string m_sNameLower;
	
	[Attribute("/0", UIWidgets.EditBox, "Prefix to quickly send a message to that channel")]
	string m_sPrefix;
};
