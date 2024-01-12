class SCR_GameModeHealthSettingsClass : ScriptComponentClass
{}

class SCR_GameModeHealthSettings : ScriptComponent
{	
	[Attribute(defvalue: "1", uiwidget: UIWidgets.Slider, desc: "Character bleeding rate multiplier", params: "0 5 0.001", precision: 3, category: "Game Mode")]
	protected float m_fDOTScale;	
	
	[Attribute(defvalue: "1", uiwidget: UIWidgets.Slider, desc: "Character regeneration rate multiplier", params: "0 5 0.001", precision: 3, category: "Game Mode")]
	protected float m_fRegenScale;
	
	[Attribute(defvalue: "true", uiwidget: UIWidgets.CheckBox, desc: "Whether unconsciousness is allowed", category: "Game Mode")]
	protected bool m_bPermitUnconsciousness;

	[Attribute(defvalue: "false", uiwidget: UIWidgets.CheckBox, desc: "Whether using VON during unconsciousness is allowed", category: "Game Mode")]
	protected bool m_bPermitUnconsciousVON;
	
	[Attribute(defvalue: "0.75", uiwidget: UIWidgets.Slider, desc: "How much will the character be slowed down when having tourniquetted leg", params: "0 1 0.001", precision: 3, category: "Game Mode")]
	protected float m_fTourniquettedLegMovementSlowdown;
	
	[Attribute(defvalue: "0.1", uiwidget: UIWidgets.Slider, desc: "Affects how much the bleeding is reduced", params: "0 1 0.001", precision: 3, category: "Game Mode")]
	private float m_fTourniquetStrengthMultiplier;
	
	[Attribute(defvalue: "1", uiwidget: UIWidgets.Slider, desc: "How fast will character regenerate when situated in medical compartment", params: "0 5 0.001", precision: 3, category: "Game Mode")]
	protected float m_fCompartmentRegenerationRateMultiplier;
	
	[Attribute(defvalue: "10", uiwidget: UIWidgets.Slider, desc: "Time without receiving damage or bleeding to start regeneration [s]", params: "0 600 1", category: "Game Mode")]
	protected float m_fRegenerationDelay;
	
	[Attribute(defvalue: "200", uiwidget: UIWidgets.Slider, desc: "Time to fully regenerate resilience hitzone [s]", params: "0 600 1", category: "Game Mode")]
	protected float m_fFullRegenerationTime;
	
	[Attribute(defvalue: "0.75", uiwidget: UIWidgets.Slider, desc: "Minimum amount of stamina needed to regenerate character hitzones", params: "0 1 0.001", precision: 3, category: "Game Mode")]
	protected float m_fRegenerationMinStaminaLevel;
	
	[Attribute(defvalue: "40", uiwidget: UIWidgets.Slider, desc: "Maximal weight of all the items character can carry to regenerate character hitzones", params: "0 150 1", category: "Game Mode")]
	protected float m_fRegenerationMaxLoadoutWeight;
	
	[Attribute(defvalue: "2", uiwidget: UIWidgets.Slider, desc: "Maximal character movement speed to regenerate character hitzones", params: "0 25 0.001", precision: 3, category: "Game Mode")]
	protected float m_fRegenerationMaxMovementSpeed;
	
	[Attribute(defvalue: "1.333", uiwidget: UIWidgets.Slider, desc: "Character hitzone regeneration speed when in crouch", params: "0 5 0.001", precision: 3, category: "Game Mode")]
	protected float m_fRegenerationSpeedCrouch;
	
	[Attribute(defvalue: "1.666", uiwidget: UIWidgets.Slider, desc: "Character hitzone regeneration speed when in prone", params: "0 5 0.001", precision: 3, category: "Game Mode")]
	protected float m_fRegenerationSpeedProne;	

	//------------------------------------------------------------------------------------------------
	/*!
	Get rate of the attribute.
	\return rate - (0.0-5.00) Higher numbers result in faster bleeding
	*/
	float GetBleedingScale()
	{
		return m_fDOTScale;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Set rate of the attribute.
	\param rate - (0.0-5.00) Higher numbers result in faster bleeding
	*/
	void SetBleedingScale(float rate)
	{
		m_fDOTScale = rate;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get rate of the attribute.
	\return rate - (0.0-5.00) Higher numbers result in faster regeneration
	*/
	float GetRegenScale()
	{
		return m_fRegenScale;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Set rate of the attribute.
	\param rate - (0.0-5.00) Higher numbers result in faster regeneration
	*/
	void SetRegenScale(float rate)
	{
		m_fRegenScale = rate;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get rate of the attribute.
	\return permitted - (true/false) True means that unconsciousness is enabled.
	*/
	bool IsUnconsciousnessPermitted()
	{
		return m_bPermitUnconsciousness;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Set rate of the attribute.
	\param permitted - (true/false) True means that unconsciousness is enabled.
	*/
	void SetUnconsciousnessPermitted(bool permitted)
	{
		m_bPermitUnconsciousness = permitted;
	}	

	//------------------------------------------------------------------------------------------------
	/*!
	Get rate of the attribute.
	\return permitted - (true/false) True means that using VON while unconsciousness is enabled.
	*/
	bool IsUnconsciousVONPermitted()
	{
		return m_bPermitUnconsciousVON;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Set rate of the attribute.
	\param permitted - (true/false) True means that VON is enabled while unconscious.
	*/
	void SetUnconsciousVONPermitted(bool permitted)
	{
		m_bPermitUnconsciousVON = permitted;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get rate of the attribute.
	\return rate - (0.0-1.00) Higher numbers result in higher slowdown
	*/
	float GetTourniquettedLegMovementSlowdown()
	{
		return m_fTourniquettedLegMovementSlowdown;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Set rate of the attribute.
	\param rate - (0.0-1.00) Higher numbers result in higher slowdown
	*/
	void SetTourniquettedLegMovementSlowdown(float rate)
	{
		m_fTourniquettedLegMovementSlowdown = rate;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get rate of the attribute.
	\return rate - (0.0-1.00) Lower numbers result in slower bleeding when tourniquet is on
	*/
	float GetTourniquettrengthMultiplier()
	{
		return m_fTourniquetStrengthMultiplier;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Set rate of the attribute.
	\param rate - (0.0-1.00) Lower numbers result in slower bleeding when tourniquet is on
	*/
	void SetTourniquettrengthMultiplier(float rate)
	{
		m_fTourniquetStrengthMultiplier = rate;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get rate of the attribute.
	\return rate - (0.0-5.00) Higher numbers result in faster regeneration when in certain medical compartment
	*/
	float GetCompartmentRegenRateMultiplier()
	{
		return m_fCompartmentRegenerationRateMultiplier;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Set rate of the attribute.
	\param rate - (0.0-5.00) Higher numbers result in faster regeneration when in certain medical compartment
	*/
	void SetCompartmentRegenRateMultiplier(float rate)
	{
		m_fCompartmentRegenerationRateMultiplier = rate;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get rate of the attribute.
	\return rate - (0.0-60.00) Higher numbers result in higher delay the regeneration cooldown
	*/
	float GetRegenerationDelay()
	{
		return m_fRegenerationDelay;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Set rate of the attribute.
	\param rate - (0.0-60.00) Higher numbers result in higher delay the regeneration cooldown
	*/
	void SetRegenerationDelay(float rate)
	{
		m_fRegenerationDelay = rate;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get rate of the attribute.
	\return rate - (0.0-1000.00) Higher numbers result in slower regeneration of resilience
	*/
	float GetResilienceHzRegenTime()
	{
		return m_fFullRegenerationTime;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Set rate of the attribute.
	\param rate - (0.0-1000.00) Higher numbers result in slower regeneration of resilience
	*/
	void SetResilienceHzRegenTime(float rate)
	{
		m_fFullRegenerationTime = rate;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get rate of the attribute.
	\return rate - (0.0-1.00) Minimum amount of stamina needed to regenerate character hitzones.
	*/
	float GetMinStaminaLevelForRegeneration()
	{
		return m_fRegenerationMinStaminaLevel;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Set rate of the attribute.
	\param rate - (0.0-1.00) Minimum amount of stamina needed to regenerate character hitzones.
	*/
	void SetMinStaminaLevelForRegeneration(float rate)
	{
		m_fRegenerationMinStaminaLevel = rate;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get rate of the attribute.
	\return rate - (0.0-100.00) Maximal weight of all the items character can carry to regenerate character hitzones
	*/
	float GetMaxCharWeightForRegeneration()
	{
		return m_fRegenerationMaxLoadoutWeight;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Set rate of the attribute.
	\param rate - (0.0-100.00) Maximal weight of all the items character can carry to regenerate character hitzon
	*/
	void SetMaxCharWeightForRegeneration(float rate)
	{
		m_fRegenerationMaxLoadoutWeight = rate;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get rate of the attribute.
	\return rate - (0.0-5.50) Maximal character movement speed to regenerate character hitzones
	*/
	float GetMaxCharMovementSpeedForRegeneration()
	{
		return m_fRegenerationMaxMovementSpeed;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Set rate of the attribute.
	\param rate - (0.0-5.50) Maximal character movement speed to regenerate character hitzones
	*/
	void SetMaxCharMovementSpeedForRegeneration(float rate)
	{
		m_fRegenerationMaxMovementSpeed = rate;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get rate of the attribute.
	\return rate - (0.0-5.00) Character hitzone regeneration speed when in crouch. Higher numbers result in higher regeneration speed
	*/
	float GetRegenerationSpeedCrouc()
	{
		return m_fRegenerationSpeedCrouch;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Set rate of the attribute.
	\param rate - (0.0-5.00) Character hitzone regeneration speed when in crouch. Higher numbers result in higher regeneration speed
	*/
	void SetRegenerationSpeedCrouc(float rate)
	{
		m_fRegenerationSpeedCrouch = rate;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get rate of the attribute.
	\return rate - (0.0-5.00) Character hitzone regeneration speed when in prone. Higher numbers result in higher regeneration speed
	*/
	float GetRegenerationSpeedProne()
	{
		return m_fRegenerationSpeedProne;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Set rate of the attribute.
	\param rate - (0.0-5.00) Character hitzone regeneration speed when in prone. Higher numbers result in higher regeneration speed
	*/
	void SetRegenerationSpeedProne(float rate)
	{
		m_fRegenerationSpeedProne = rate;
	}
}
