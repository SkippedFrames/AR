[WorkbenchPluginAttribute("Game Mode Setup", "<Description>", "", "", {"WorldEditor"}, "", 0xf6e8)]
class GameModeSetupPlugin: WorkbenchPlugin
{
	[Attribute("{6389BA4D41B187DC}Configs/Workbench/GameModeSetups/GameMaster.conf", desc: "Game mode configuration rules.", params: "conf class=GameModeSetupConfig", category: "Game Mode Template")]
	protected ResourceName m_Template;
	
	//[Attribute(category: "Game Mode Template")] //--- Used only for debugging, game crashes when recompiling Workbench scripts and running the plugin repeatedly.
	protected ref GameModeSetupConfig m_Config;
	
	protected bool m_IsWorldValid;
	protected bool m_IsWorldAttentionNeeded;
	protected bool m_CanAutogenerateWorld;
	protected bool m_IsMissionHeaderValid;
	protected ref array<EGameModeSetupPage> m_PageHistory = {EGameModeSetupPage.INTRO};
	
	protected string m_DialogMessageValidation;
	protected string m_DialogMessageGeneration;
	protected string m_DialogMessageMissionHeader;
	
	protected string CAPTION_INTRO =					"Game Mode Setup";
	protected string CAPTION_VALIDATION =				"Game Mode Setup - World Scan";
	protected string CAPTION_VALIDATION_RESULTS =		"Game Mode Setup - World Scan Results";
	protected string CAPTION_GENERATION =				"Game Mode Setup - World Configuration";
	protected string CAPTION_GENERATION_RESULTS =		"Game Mode Setup - World Configuration Completed";
	protected string CAPTION_MISSION_HEADER =			"Game Mode Setup - Mission Header";
	protected string CAPTION_MISSION_HEADER_RESULTS =	"Game Mode Setup - Mission Header Created";
	protected string CAPTION_OUTRO_GOOD =				"Game Mode Setup - Success";
	protected string CAPTION_OUTRO_BAD =				"Game Mode Setup - Actions Required";
	
	protected string DESCRIPTION_INTRO =					"Welcome to step-by-step setup of a game mode.\nA game mode is a set of rules that define how the world will function.\nWithout it, players won't be able to try your world in game.\nEven opening the world in Game Master requires you to create a Game Master game mode for it.\n\nThe plugin will explain what's needed to get a game mode up an running,\nand offer automatic creation of required configuration.\n\nBefore we start, please select a template of the game mode you wish to set up.\n";
	protected string DESCRIPTION_VALIDATION =				"To know what needs to be configured the plugin will scan all entities currently present in the world.\nDepending on the size of the world, this may take several seconds or minutes.\n\nYou can decide to skip this step, but unless you understand how to set up game mode manually,\nit's better to let the plugin check the current status first.";
	protected string DESCRIPTION_VALIDATION_RESULTS =		"All entities scanned, results are listed below:\n";
	
	protected string DESCRIPTION_GENERATION =				"The plugin can now create required entities in the world.\nThey will be created in the current layer, but you can later move them to any other layer.";
	protected string DESCRIPTION_GENERATION_RESULTS =		"Required entities were created in the current layer.\nTry to re-scan the world to see if nothing is missing anymore.\n";
	protected string DESCRIPTION_MISSION_HEADER =			"For the game mode to appear in main menu's scenario list, it needs a mission header.\nIt's a config file which exists outside of the world, but points to it.\n\nIn this step you can let the plugin create the file automatically.";
	protected string DESCRIPTION_MISSION_HEADER_RESULTS =	"";
	protected string DESCRIPTION_OUTRO_COMPLETE =			"Congratulations, the game mode is configured correctly.\n\nGive it a try and hit that Play button!";
	protected string DESCRIPTION_OUTRO_INCOMPLETE =			"Configuration was not finished.\nTo ensure the game mode is set up correctly, please fix all issues and go through each step again.";
	
	override void Run()
	{		
		if (!Workbench.ScriptDialog(CAPTION_INTRO, DESCRIPTION_INTRO, this))
			return;
		
		//--- Next
		ShowPage(EGameModeSetupPage.VALIDATION);
	}
	protected void RunValidation()
	{
		if (!m_Config)
		{
			string error;
			if (!LoadConfig(error))
			{
				Print(CAPTION_INTRO + ": " + error, LogLevel.WARNING);
				GameModeSetupPluginError errorDialog = new GameModeSetupPluginError();
				Workbench.ScriptDialog(CAPTION_INTRO, error, errorDialog);
				return;
			}
		}
		m_Config.Init();
		
		GameModeSetupPluginValidation dialog = new GameModeSetupPluginValidation();
		if (!Workbench.ScriptDialog(CAPTION_VALIDATION, DESCRIPTION_VALIDATION, dialog))
			return;
		
		m_IsWorldValid = false;
		switch (dialog.m_iResult)
		{
			case EGameModeSetupButton.BACK:
				ShowPrevPage();
				break;
			
			case EGameModeSetupButton.NEXT:
				m_DialogMessageValidation = string.Empty;
				m_CanAutogenerateWorld = true;
				m_IsWorldValid = m_Config.ValidateWorld(m_DialogMessageValidation, m_CanAutogenerateWorld);
				ShowPage(EGameModeSetupPage.VALIDATION_RESULTS);
				break;
			
			case EGameModeSetupButton.SKIP:
				ShowPage(EGameModeSetupPage.GENERATION);
				break;
		}
	}
	protected void RunValidationResults()
	{
		GameModeSetupPluginResults dialog = new GameModeSetupPluginResults();
		if (!Workbench.ScriptDialog(CAPTION_VALIDATION_RESULTS, DESCRIPTION_VALIDATION_RESULTS + m_DialogMessageValidation, dialog))
			return;
		
		switch (dialog.m_iResult)
		{
			case EGameModeSetupButton.BACK:
				ShowPrevPage();
				break;
			
			case EGameModeSetupButton.NEXT:
				if (m_IsWorldValid && m_IsMissionHeaderValid)
					ShowPage(EGameModeSetupPage.OUTRO);
				else if (m_IsWorldValid)
					ShowPage(EGameModeSetupPage.MISSION_HEADER);
				else if (!m_CanAutogenerateWorld)
					ShowPage(EGameModeSetupPage.OUTRO);
				else
					ShowPage(EGameModeSetupPage.GENERATION);
				break;
		}
	}
	protected void RunGeneration()
	{
		GameModeSetupPluginGeneration dialog = new GameModeSetupPluginGeneration();
		if (!Workbench.ScriptDialog(CAPTION_GENERATION, DESCRIPTION_GENERATION, dialog))
			return;
		
		switch (dialog.m_iResult)
		{
			case EGameModeSetupButton.BACK:
				ShowPrevPage();
				break;
			
			case EGameModeSetupButton.NEXT:
				m_DialogMessageGeneration = string.Empty;
				m_IsWorldValid = m_Config.GenerateWorld(m_DialogMessageGeneration);
				ShowPage(EGameModeSetupPage.GENERATION_RESULTS);
				break;
			
			case EGameModeSetupButton.SKIP:
				ShowPage(EGameModeSetupPage.MISSION_HEADER);
				break;
		}
	}
	protected void RunGenerationResults()
	{
		GameModeSetupPluginGenerationResults dialog = new GameModeSetupPluginGenerationResults();
		if (!Workbench.ScriptDialog(CAPTION_GENERATION_RESULTS, DESCRIPTION_GENERATION_RESULTS + m_DialogMessageGeneration, dialog))
			return;
		
		switch (dialog.m_iResult)
		{
			case EGameModeSetupButton.BACK:
				ShowPrevPage();
				break;
			
			case EGameModeSetupButton.NEXT:
				ShowPage(EGameModeSetupPage.MISSION_HEADER);
				break;
			
			case EGameModeSetupButton.VALIDATE:
				ShowPage(EGameModeSetupPage.VALIDATION);
				break;
		}
	}
	protected void RunMissionHeader()
	{
		//--- Mission header already exists, skip this step
		m_IsMissionHeaderValid = m_Config.ValidateMissionHeader(m_DialogMessageMissionHeader);
		if (m_IsMissionHeaderValid)
		{
			m_PageHistory.Resize(m_PageHistory.Count() - 1);
			ShowPage(EGameModeSetupPage.MISSION_HEADER_RESULTS);
			return;
		}
		
		GameModeSetupPluginMissionHeader dialog = new GameModeSetupPluginMissionHeader();
		if (!Workbench.ScriptDialog(CAPTION_MISSION_HEADER, DESCRIPTION_MISSION_HEADER, dialog))
			return;
		
		switch (dialog.m_iResult)
		{
			case EGameModeSetupButton.BACK:
				ShowPrevPage();
				break;
			
			case EGameModeSetupButton.NEXT:
				m_DialogMessageMissionHeader = string.Empty;
				m_IsMissionHeaderValid = m_Config.GenerateMissionHeader(m_Template, m_DialogMessageMissionHeader);
				ShowPage(EGameModeSetupPage.MISSION_HEADER_RESULTS);
				break;
			
			case EGameModeSetupButton.SKIP:
				ShowPage(EGameModeSetupPage.OUTRO);
				break;
		}
	}
	protected void RunMissionHeaderResults()
	{
		GameModeSetupPluginResults dialog = new GameModeSetupPluginResults();
		if (!Workbench.ScriptDialog(CAPTION_MISSION_HEADER_RESULTS, DESCRIPTION_MISSION_HEADER_RESULTS + m_DialogMessageMissionHeader, dialog))
			return;
		
		switch (dialog.m_iResult)
		{
			case EGameModeSetupButton.BACK:
				m_IsMissionHeaderValid = false;
				ShowPrevPage();
				break;
			
			case EGameModeSetupButton.NEXT:
				ShowPage(EGameModeSetupPage.OUTRO);
				break;
		}
	}
	protected void RunOutro()
	{
		GameModeSetupPluginOutro dialog = new GameModeSetupPluginOutro();
		if (m_IsWorldValid && m_IsMissionHeaderValid)
		{
			if (!Workbench.ScriptDialog(CAPTION_OUTRO_GOOD, DESCRIPTION_OUTRO_COMPLETE, dialog))
				return;
		}
		else
		{
			if (!Workbench.ScriptDialog(CAPTION_OUTRO_BAD, DESCRIPTION_OUTRO_INCOMPLETE, dialog))
				return;
		}
		
		switch (dialog.m_iResult)
		{
			case EGameModeSetupButton.BACK:
				ShowPrevPage();
				break;
		}
	}
	
	protected void ShowPage(EGameModeSetupPage page)
	{
		if (!m_PageHistory.Contains(page))
			m_PageHistory.Insert(page);
		
		switch (page)
		{
			case EGameModeSetupPage.INTRO:
				Run();
				break;
			case EGameModeSetupPage.VALIDATION:
				RunValidation();
				break;
			case EGameModeSetupPage.VALIDATION_RESULTS:
				RunValidationResults();
				break;
			case EGameModeSetupPage.GENERATION:
				RunGeneration();
				break;
			case EGameModeSetupPage.GENERATION_RESULTS:
				RunGenerationResults();
				break;
			case EGameModeSetupPage.MISSION_HEADER:
				RunMissionHeader();
				break;
			case EGameModeSetupPage.MISSION_HEADER_RESULTS:
				RunMissionHeaderResults();
				break;
			case EGameModeSetupPage.OUTRO:
				RunOutro();
				break;
		}
	}
	protected void ShowPrevPage()
	{
		m_PageHistory.Resize(m_PageHistory.Count() - 1);
		ShowPage(m_PageHistory[m_PageHistory.Count() - 1]);
	}
	
	protected bool LoadConfig(out string error)
	{
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		WorldEditorAPI api = worldEditor.GetApi();
		
		string worldPath;
		api.GetWorldPath(worldPath);
		if (worldPath.IsEmpty())
		{
			error = "No world is currently loaded, or the current world is not saved!";
			return false;
		}
		
		if (!m_Template)
		{
			error = "No template defined!";
			return false;
		}
				
		Resource templateResource = Resource.Load(m_Template);
		if (!templateResource.IsValid())
		{
			error = "Template config is invalid!";
			return false;
		}
		
		BaseContainer templateContainer = templateResource.GetResource().ToBaseContainer();
		m_Config = GameModeSetupConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(templateContainer));
		if (m_Config)
		{
			return true;
		}
		else
		{
			error = "Failed to open template config!";
			return false;
		}
	}
	
	[ButtonAttribute("Cancel")]
	bool ButtonCancel()
	{
		return false;
	}
	[ButtonAttribute("Next", true)]
	bool ButtonNext()
	{
		return true;
	}
};
class GameModeSetupPluginValidation
{
	int m_iResult;
	
	[ButtonAttribute("Cancel")]
	bool ButtonCancel()
	{
		return false;
	}
	[ButtonAttribute("Skip")]
	bool ButtonSkip()
	{
		m_iResult = EGameModeSetupButton.SKIP;
		return true;
	}
	[ButtonAttribute("Back")]
	bool ButtonBack()
	{
		m_iResult = EGameModeSetupButton.BACK;
		return true;
	}
	[ButtonAttribute("Scan world", true)]
	bool ButtonValidate()
	{
		m_iResult = EGameModeSetupButton.NEXT;
		return true;
	}
};
class GameModeSetupPluginResults
{
	int m_iResult;
	
	[ButtonAttribute("Cancel")]
	bool ButtonCancel()
	{
		return false;
	}
	[ButtonAttribute("Back")]
	bool ButtonBack()
	{
		m_iResult = EGameModeSetupButton.BACK;
		return true;
	}
	[ButtonAttribute("Next", true)]
	bool ButtonNext()
	{
		m_iResult = EGameModeSetupButton.NEXT;
		return true;
	}
};
class GameModeSetupPluginGenerationResults
{
	int m_iResult;
	
	[ButtonAttribute("Cancel")]
	bool ButtonCancel()
	{
		return false;
	}
	[ButtonAttribute("Re-scan")]
	bool ButtonValidate()
	{
		m_iResult = EGameModeSetupButton.VALIDATE;
		return true;
	}
	[ButtonAttribute("Back")]
	bool ButtonBack()
	{
		m_iResult = EGameModeSetupButton.BACK;
		return true;
	}
	[ButtonAttribute("Next", true)]
	bool ButtonNext()
	{
		m_iResult = EGameModeSetupButton.NEXT;
		return true;
	}
};
class GameModeSetupPluginGeneration
{		
	int m_iResult;
	
	[ButtonAttribute("Cancel")]
	bool ButtonCancel()
	{
		return false;
	}
	[ButtonAttribute("Skip")]
	bool ButtonSkip()
	{
		m_iResult = EGameModeSetupButton.SKIP;
		return true;
	}
	[ButtonAttribute("Back")]
	bool ButtonBack()
	{
		m_iResult = EGameModeSetupButton.BACK;
		return true;
	}
	[ButtonAttribute("Create entities", true)]
	bool ButtonGenerate()
	{
		m_iResult = EGameModeSetupButton.NEXT;
		return true;
	}
};
class GameModeSetupPluginMissionHeader
{
	int m_iResult;
	
	[ButtonAttribute("Cancel")]
	bool ButtonCancel()
	{
		return false;
	}
	[ButtonAttribute("Skip")]
	bool ButtonSkip()
	{
		m_iResult = EGameModeSetupButton.SKIP;
		return true;
	}
	[ButtonAttribute("Back")]
	bool ButtonBack()
	{
		m_iResult = EGameModeSetupButton.BACK;
		return true;
	}
	[ButtonAttribute("Create header", true)]
	bool ButtonGenerate()
	{
		m_iResult = EGameModeSetupButton.NEXT;
		return true;
	}
};
class GameModeSetupPluginOutro
{
	int m_iResult;
	
	[ButtonAttribute("Back")]
	bool ButtonBack()
	{
		m_iResult = EGameModeSetupButton.BACK;
		return true;
	}
	[ButtonAttribute("Close", true)]
	bool ButtonClose()
	{
		return false;
	}
};
class GameModeSetupPluginError
{
	[ButtonAttribute("Close", true)]
	bool ButtonGenerate()
	{
		return false;
	}
};
enum EGameModeSetupPage
{
	INTRO,
	VALIDATION,
	VALIDATION_RESULTS,
	GENERATION,
	GENERATION_RESULTS,
	MISSION_HEADER,
	MISSION_HEADER_RESULTS,
	OUTRO
};
enum EGameModeSetupButton
{
	CANCEL,
	BACK,
	NEXT,
	SKIP,
	VALIDATE
};
