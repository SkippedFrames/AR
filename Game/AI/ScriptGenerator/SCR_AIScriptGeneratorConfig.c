[BaseContainerProps(configRoot: true)]
class SCR_AIScriptGeneratorConfig
{
	[Attribute("", UIWidgets.Auto, "Script files which will be parsed. Remember to specify file system, for example \"$ArmaReforger:\"")]
	ref array<string> m_aInputFiles;
	
	[Attribute("SendGoalMessageAutogenerated.c", UIWidgets.EditBox, "File for SendGoalMessage generated classes. Remember to specify file system!")]
	string m_sSendGoalMessageOutputFile;
	
	[Attribute("SendInfoMessageAutogenerated.c", UIWidgets.EditBox, "File for SendInfoMessage generated classes. Remember to specify file system!")]
	string m_sSendInfoMessageOutputFile;
	
	[Attribute("SendOrderAutogenerated.c", UIWidgets.EditBox, "File for SendOrder generated classes. Remember to specify file system!")]
	string m_sSendOrderOutputFile;
}