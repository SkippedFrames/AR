/*!
Behavior for soldier to look at something.
Looking lasts for some time (duration).
There is also a timeout value, after which the action will fail even if never started.
!!! This is a base class and should not be used alone.
!!! Derive a class from it and initialize duration, radius, timeout and priority!
*/
class SCR_AIObservePositionBehavior : SCR_AIBehaviorBase
{
	ref SCR_BTParam<vector> m_vPosition = new SCR_BTParam<vector>("Position");
	ref SCR_BTParam<float> m_fDuration = new SCR_BTParam<float>("Duration");	// Initialize in derived class
	ref SCR_BTParam<float> m_fRadius = new SCR_BTParam<float>("Radius");		// Initialize in derived class
	ref SCR_BTParam<bool> m_bUseBinoculars = new SCR_BTParam<bool>("UseBinoculars"); // Initialize in derived class
	ref SCR_BTParam<float> m_fDelay = new SCR_BTParam<float>("Delay"); // Initialize in derived class
	
	float m_fDeleteActionTime_ms;	// Initialize in derived class by InitTimeout()
	
	//------------------------------------------------------------------------------------------------------------------------
	void InitParameters(vector position)
	{
		m_vPosition.Init(this, position);
		m_fDuration.Init(this, 0);
		m_fRadius.Init(this, 0);
		m_bUseBinoculars.Init(this, false);
		m_fDelay.Init(this, 0.0);
	}
	
	// posWorld - position to observe
	void SCR_AIObservePositionBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, vector posWorld)
	{
		InitParameters(posWorld);
		if (!utility)
			return;
				
		m_sBehaviorTree = "{AD1A56AE2A7ADFE8}AI/BehaviorTrees/Chimera/Soldier/ObservePositionBehavior.bt";
		m_bAllowLook = false; // Disable standard looking
		m_bResetLook = true;
		SetIsUniqueInActionQueue(true);
	}
	
	override float CustomEvaluate()
	{
		// Fail action if timeout has been reached
		float currentTime_ms = GetGame().GetWorld().GetWorldTime();
		if (currentTime_ms > m_fDeleteActionTime_ms)
		{
			Fail();
			return 0;
		}
		return GetPriority();
	}
	
	void InitTimeout(float timeout_s)
	{
		float currentTime_ms = GetGame().GetWorld().GetWorldTime(); // Milliseconds!
		m_fDeleteActionTime_ms = currentTime_ms + 1000 * timeout_s;
	}
};

/*!
Behavior to observe supposed location of where gunshot came from.
*/
class SCR_AIObserveUnknownFireBehavior : SCR_AIObservePositionBehavior
{
	protected const float TIMEOUT_S = 16.0;
	protected const float DURATION_MIN_S = 3.0;			// Min duration of behavior
	protected const float DIRECTION_SPAN_DEG = 32.0;	
	protected const float DURATION_S_PER_METER = 0.1;	// How duration depends on distance
	protected const float USE_BINOCULARS_DISTANCE_THRESHOLD = 70;
	
	protected const float DELAY_MIN_S = 0.15;			// Min delay before we start looking at the position
	protected const float DELAY_S_PER_METER = 0.0015;	// How the delay increases depending on distance
	
	void SCR_AIObserveUnknownFireBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity,
		vector posWorld, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		m_fThreat = 1.01 * SCR_AIThreatSystem.VIGILANT_THRESHOLD;
		SetPriority(SCR_AIActionBase.PRIORITY_BEHAVIOR_OBSERVE_UNKNOWN_FIRE);
		m_fPriorityLevel.m_Value = priorityLevel;
		
		if (!utility || !utility.GetAIAgent())
			return;
		
		
		// Calculate duration depending on distance
		IEntity controlledEntity = utility.GetAIAgent().GetControlledEntity();
		float distance;
		if (controlledEntity)
			distance = vector.Distance(controlledEntity.GetOrigin(), posWorld);
		InitTiming(distance);
		
		if (controlledEntity)
		{
			float radius = distance * Math.Tan(Math.DEG2RAD * DIRECTION_SPAN_DEG);
			m_fRadius.m_Value = radius;
			
			m_bUseBinoculars.m_Value = distance > USE_BINOCULARS_DISTANCE_THRESHOLD;
		}
	}
	
	void InitTiming(float distance)
	{
		float duration_s = Math.Max(DURATION_MIN_S, DURATION_S_PER_METER * distance);	// Linearly increase with distance
		duration_s = Math.RandomFloat(0.7*duration_s, 1.3*duration_s);	
		m_fDuration.m_Value = duration_s;
		
		float timeout_s = Math.Max(TIMEOUT_S, duration_s);	// Timeout is quite big, but it should be smaller than duration
		InitTimeout(timeout_s);
		
		float delay_s = Math.Max(DELAY_MIN_S, DELAY_S_PER_METER * distance); // Linearly increase with distance
		delay_s = Math.RandomFloat(0.7*delay_s, 1.3*delay_s);
		m_fDelay.m_Value = delay_s;
	}
	
	override void OnActionSelected()
	{
		super.OnActionSelected();
		
		if (Math.RandomFloat01() < 0.2)
		{
			if (!m_Utility.m_CommsHandler.CanBypass())
			{
				SCR_AITalkRequest rq = new SCR_AITalkRequest(ECommunicationType.REPORT_UNDER_FIRE, null, vector.Zero, 0, false, SCR_EAITalkRequestPreset.IRRELEVANT);
				m_Utility.m_CommsHandler.AddRequest(rq);
			}
		}
	}
	
	static bool IsNewPositionMoreRelevant(vector myWorldPos, vector oldWorldPos, vector newWorldPos)
	{
		vector vDirOld = vector.Direction(myWorldPos, oldWorldPos);
		vector vDirNew = vector.Direction(myWorldPos, newWorldPos);
		float cosAngle = vector.Dot(vDirOld, vDirNew);
		
		return cosAngle < 0.707; // cos 45 deg
	}
};

class SCR_AIGetObservePositionBehaviorParameters: SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIObservePositionBehavior(null, null,	vector.Zero)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	override bool VisibleInPalette() { return true; }
};
