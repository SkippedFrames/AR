class SCR_AISmartActionSentinelComponentClass: SCR_AISmartActionComponentClass
{
};

enum ELeaningType
{
	LEFT = -1,
	NONE = 0,
	RIGHT = 1,
};

class SCR_AISmartActionSentinelComponent : SCR_AISmartActionComponent
{	
	
	[Attribute("0 0 0", UIWidgets.EditBox, desc: "Position where AI will look from action offset (in local coords of the object entity)", params: "inf inf 0 purpose=coords space=entity")]
	protected vector m_vLookPosition;
	
	[Attribute("180", UIWidgets.Coords, desc: "Range of rotation within which AI will restrict their observing")]
	protected float m_fLookDirectionRange;
	
	[Attribute("0", uiwidget: UIWidgets.ComboBox, "Desired stance the AI will use upon reaching the observationpost", "", ParamEnumArray.FromEnum(ECharacterStance))]
	protected int m_iCharacterStance;
	
	[Attribute("0", uiwidget: UIWidgets.CheckBox, "Whether the AI will be using binoculars during observation", "")]
	protected bool m_bUseBinoculars;
	
	[Attribute("0", uiwidget: UIWidgets.ComboBox, "Whether the AI will occasionally lean to side", "", ParamEnumArray.FromEnum(ELeaningType))]
	protected ELeaningType m_eLeaningType;
	
	//------------------------------------------------------------------------------------------------
	vector GetLookPosition()
	{
		return m_vLookPosition;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetLookDirectionRange()
	{
		return m_fLookDirectionRange;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetDesiredStance()
	{
		return m_iCharacterStance;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetUseBinoculars()
	{
		return m_bUseBinoculars;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetUseBinoculars(bool useBinoculars)
	{
		m_bUseBinoculars = useBinoculars;
	}
	
	//------------------------------------------------------------------------------------------------
	ELeaningType GetLeaningType()
	{
		return m_eLeaningType;
	}
};

