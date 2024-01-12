class DownloadConfigCallback : BackendCallback
{
	ServerConfigMeta config;
	
	override void OnSuccess(int code)
	{
		Print("config downloaded");
		
		GameStateTransitions.RequestServerConfigChange(config);
	}
	
	override void OnError(int code, int restCode, int apiCode)
	{
		Print("config download error");
	}
	
}
// -------------------------------------------------------------------------
class SCR_RCONCommander: RCONCommander
{
	ref DownloadConfigCallback callback = new DownloadConfigCallback;
	
	override void ProcessCommand(string sCommand, int iRequestId)
	{	
		ref array<string> commandSegments = new array<string>;
		sCommand.Split(" ", commandSegments, true);
		
		if (commandSegments.Count() != 2)
		{
			Print("Invalid command format", LogLevel.ERROR);
		}
		else
		{
			switch(commandSegments[0])
			{
				case "config": { 
					
					Print("config command received");
					
					string configId = commandSegments[1];
					
					ServerConfigApi ca = GetGame().GetBackendApi().GetServerConfigApi();
					callback.config = ca.Download(configId, callback);
					
					break;
				}
				
				default: Print("unknown command"); break;
			}
		}
		
		
	}
}
