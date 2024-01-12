//------------------------------------------------------------------------------------------------
class FeedbackData extends JsonApiStruct
{
	//! Type of feedback: issue, feedback..
	protected string m_sTypeTag;
	protected string m_sTypeName;
	//! Category of feedback: character, vehicle, UI..
	protected string m_sCategoryTag;
	protected string m_sCategoryName;

	//! User provided content
	protected string m_sContent;
	
	//! User data
	protected string m_sUserName;
	
	//! Version information
	//! Game build time
	protected string m_sBuildTime;
	//! Game build version
	protected string m_sBuildVersion;
	
	//! Game information
	//! Current world file
	protected string m_sWorldFile;
	//! When feedback was submit
	//! Formatted as YYYYY-MM-DD HH-MM-SS
	protected string m_sSubmitTime;
	
	//! Current world position taken from camera if any
	protected vector m_vWorldPosition;
	//! Current camera orientation taken from camera if any
	protected vector m_vWorldRotation;

	//! Message string for Slack (not part of JSON)
	protected string m_sMessage;
	
	//------------------------------------------------------------------------------------------------
	string FormatMessage()
	{
		const string altsep = "-----------------------------\n";
		string res = "\n";
		
		res += string.Format("Type: %1\n", GetFeedbackType());
		res += string.Format("Category: %1\n", GetFeedbackCategory());
		res += string.Format("User: %1\n", GetUserName());

		// assembly link to WE
		res += string.Format("<enfusion://WorldEditor/%1;%2,%3,%4;%5,%6,%7|Enfusion Link>\n",
		 GetWorldFile(),
		  m_vWorldPosition[0].ToString(),
		   m_vWorldPosition[1].ToString(),
		    m_vWorldPosition[2].ToString(),
			 m_vWorldRotation[1].ToString(),
			  m_vWorldRotation[2].ToString(),
			   m_vWorldRotation[0].ToString());

		res += string.Format("Build Version: %1\n", GetBuildVersion());
		res += string.Format("Submit Time: %1\n", GetFeedbackSubmitTime());
		res += string.Format("Build Time: %1\n", GetBuildTime());
		res += string.Format("World File: %1\n", GetWorldFile());

		// position && message
		res += string.Format("Position: %1\n", m_vWorldPosition.ToString());
		res += string.Format("Rotation: %1\n", m_vWorldRotation.ToString());

		res += altsep;
		res += "FEEDBACK USER DATA:\n";
		res += m_sContent;
		
	
		m_sMessage = res;
		
		return m_sMessage;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetWorldFile()
	{
		return m_sWorldFile;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetUserName()
	{
		return m_sUserName;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetBuildTime()
	{
		return m_sBuildTime;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetBuildVersion()
	{
		return m_sBuildVersion;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetFeedbackSubmitTime()
	{
		return m_sSubmitTime;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetFeedbackType()
	{
		return m_sTypeName;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetFeedbackCategory()
	{
		return m_sCategoryName;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetFeedbackContent()
	{
		return m_sContent;
	}
	
	//------------------------------------------------------------------------------------------------
	protected string GetCurrentTimestamp()
	{
		int year, month, day, hour, minute, sec;
		System.GetYearMonthDay(year, month, day);
		System.GetHourMinuteSecond(hour, minute, sec);
		string smonth, sday, shour, sminute, ssec;
		if(month < 10)
			smonth = string.Format("0%1", month);
		else
			smonth = string.Format("%1", month);

		if(day < 10)
			sday = string.Format("0%1", day);
		else
			sday = string.Format("%1", day);
		
		if (hour < 10)
			shour = string.Format("0%1", hour);
		else
			shour = string.Format("%1", hour);
		
		if (minute < 10)
			sminute = string.Format("0%1", minute);
		else
			sminute = string.Format("%1", minute);
		
		if (sec < 10)
			ssec = string.Format("0%1", sec);
		else
			ssec = string.Format("%1", sec);

		return string.Format("%1-%2-%3 %4:%5:%6", year, smonth, sday, shour, sminute, ssec);
	}
	
		//------------------------------------------------------------------------------------------------
	protected bool FetchCoords()
	{
		ArmaReforgerScripted game = GetGame();
		if(game)
		{
			BaseWorld world = game.GetWorld();
			if (world)
			{
				int cameraId = world.GetCurrentCameraId();
				// get current engine camera transformation
				vector mat[4];
				world.GetCamera(cameraId, mat);
				vector yawPitchRoll = Math3D.MatrixToAngles(mat);
				m_vWorldPosition = mat[3];
				
				// yawpitchroll->pitchyawroll
				m_vWorldRotation[0] = yawPitchRoll[1];
				m_vWorldRotation[1] = yawPitchRoll[0];
				m_vWorldRotation[2] = yawPitchRoll[2];				
				return true;
			}
		}
		
		// Plausible defaults
		m_vWorldPosition = Vector(0,0,0);
		m_vWorldRotation = Vector(0,0,0);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Creates feedback data container
	void FeedbackData( string typeTag, string typeName, string catTag, string catName, string content)
	{
		// ...
		// data below are structured, send as JSON!
		RegV("m_sTypeName");
		RegV("m_sCategoryName");
		RegV("m_sContent");
		RegV("m_sUserName");

		RegV("m_sBuildTime");
		RegV("m_sBuildVersion");

		RegV("m_sWorldFile");
		RegV("m_sSubmitTime");

		RegV("m_vWorldPosition");
		RegV("m_vWorldRotation");
		// ...
		

		m_sTypeTag = typeTag;
		m_sCategoryTag = catTag;

		m_sTypeName = typeName;
		m_sCategoryName = catName;
		
		m_sContent = content;
		
		ArmaReforgerScripted game = GetGame();
		if (!game)
			return;
		
		// TODO: Not sure if the user name should be directly sent. Same goes for platform specific UID
		m_sUserName = SCR_Global.GetProfileName();
		m_sBuildTime = game.GetBuildTime();
		m_sBuildVersion = game.GetBuildVersion();
		
		m_sWorldFile = game.GetWorldFile();
		m_sSubmitTime = GetCurrentTimestamp();
		
		// Fetch coords data
		FetchCoords();
	}
	
};

//------------------------------------------------------------------------------------------------
class FeedbackDialogUI: DialogUI
{
	//! Available types of feedback
	static const int TYPE_NAMES_COUNT = 2;
	static const string TYPE_TAGS[TYPE_NAMES_COUNT] = 
	{
		"feedback",
		"issue"
	};
	static const LocalizedString TYPE_NAMES[TYPE_NAMES_COUNT] = 
	{
		"#AR-MainMenu_Feedback_Name",
		"#AR-Feedback_Issue"
	};
	
	//! Available categories of feedback
	static const int CATEGORY_NAMES_COUNT = 8;
	static const string CATEGORY_TAGS[CATEGORY_NAMES_COUNT] = 
	{
		"general",
		"ui",
		"editor",
		"character",
		"vehicles",
		"weapons",
		"multiplayer",
		"conflict"
	};
	static const LocalizedString CATEGORY_NAMES[CATEGORY_NAMES_COUNT] = 
	{
		"#AR-Feedback_General",
		"#AR-Feedback_UI",
		"#AR-MainMenu_Editor_Name",
		"#AR-Feedback_Character",
		"#AR-Feedback_Vehicles",
		"#AR-Feedback_Weapons",
		"#AR-MainMenu_Multiplayer_Name",
		"#AR-MainMenu_Conflict_Name"
	};
	
	protected SCR_EditBoxComponent m_EditBox;	
	protected SCR_ComboBoxComponent m_Type;
	protected SCR_ComboBoxComponent m_Category;
	
	protected static ref FeedbackData m_LastFeedback;
	protected static float m_fLastFeedbackTime = -float.MAX;
	protected static const float FEEDBACK_SEND_TIMEOUT = 5000; // ms
	
	SCR_InputButtonComponent m_CreateAccount;	
	protected bool m_bShouldEnableConfirmButton;
	
	//------------------------------------------------------------------------------------------------
	override protected void OnConfirm()
	{
		if (!m_Type || !m_Category || !m_EditBox)
			return;
		
		// Fetch feedback data
		int type = m_Type.GetCurrentIndex();
		int category = m_Category.GetCurrentIndex();
		string content = m_EditBox.GetValue();
		SendFeedback(content, type, category);
		
		super.OnConfirm();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuShow()
	{
		if (m_Type)
			GetGame().GetWorkspace().SetFocusedWidget(m_Type.GetRootWidget());
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);
		
		GetGame().GetInputManager().ActivateContext("InteractableDialogContext");
	}
	
	//------------------------------------------------------------------------------------------------
	static FeedbackDialogUI OpenFeedbackDialog()
	{
		ArmaReforgerScripted game = GetGame();
		if (!game)
			return null;

		MenuManager menuManager = game.GetMenuManager();
		if (!menuManager)
			return null;

		MenuBase dialog = menuManager.FindMenuByPreset(ChimeraMenuPreset.FeedbackDialog);
		if (!dialog)
			dialog = menuManager.OpenDialog(ChimeraMenuPreset.FeedbackDialog, DialogPriority.INFORMATIVE, 0, true);

		return FeedbackDialogUI.Cast(dialog);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool CanSendFeedback()
	{
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return false;
		
		float timeSinceLast = world.GetWorldTime() - m_fLastFeedbackTime;
		return timeSinceLast > FEEDBACK_SEND_TIMEOUT;
	}
	
	//------------------------------------------------------------------------------------------------
	static void SendFeedback(string content, int type, int category)
	{
		// Create container for our feedback
		m_LastFeedback = NULL;
		m_LastFeedback = new FeedbackData(TYPE_TAGS[type], TYPE_NAMES[type], CATEGORY_TAGS[category], CATEGORY_NAMES[category], content);
		string summary = m_LastFeedback.FormatMessage();
		
		GetGame().GetBackendApi().FeedbackMessage(null,m_LastFeedback,summary);
		m_fLastFeedbackTime = GetGame().GetWorld().GetWorldTime();
	}
	
	//------------------------------------------------------------------------------------------------
	static void ClearFeedback()
	{
		m_LastFeedback = null;
	}
	
	//------------------------------------------------------------------------------------------------
	private void OnCreateAccount()
	{
		GetGame().GetPlatformService().OpenBrowser(GetGame().GetBackendApi().GetLinkItem("Link_PrivacyPolicy"));
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		m_Confirm.SetEnabled(false, false);
		Widget w = GetRootWidget();
		m_EditBox = SCR_EditBoxComponent.GetEditBoxComponent("EditBox", w);

		m_Type = SCR_ComboBoxComponent.GetComboBoxComponent("FeedbackType", w);
		if (m_Type)
		{
			for (int i = 0; i < TYPE_NAMES_COUNT; i++)
			{
				m_Type.AddItem(TYPE_NAMES[i]);
			}
			m_Type.SetCurrentItem(0);
		}
		
		m_Category = SCR_ComboBoxComponent.GetComboBoxComponent("FeedbackCategory", w);
		if (m_Category)
		{
			for (int i = 0; i < CATEGORY_NAMES_COUNT; i++)
			{
				m_Category.AddItem(CATEGORY_NAMES[i]);
			}
			m_Category.SetCurrentItem(0);
		}
		
		m_CreateAccount = SCR_InputButtonComponent.GetInputButtonComponent("CreateAccount", w);
		if (m_CreateAccount)
			m_CreateAccount.m_OnActivated.Insert(OnCreateAccount);
		
		if(m_EditBox)
		{
			m_EditBox.m_OnWriteModeLeave.Insert(OnWriteModeLeave);
			m_EditBox.m_OnTextChange.Insert(OnTextChange);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnWriteModeLeave(string text)
	{
		OnTextChange(text);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnTextChange(string text)
	{
		if (!m_Confirm)
			return;
		
		m_bShouldEnableConfirmButton = m_EditBox.GetValue() != string.Empty && FeedbackDialogUI.CanSendFeedback();

		m_Confirm.SetEnabled(m_bShouldEnableConfirmButton);
	}
};
