//------------------------------------------------------------------------------------------------
class SCR_ContainerMedicalActionTitle : BaseContainerCustomTitle
{
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		title = source.GetClassName();
		title.Replace("SCR_ScenarioFrameworkMedicalAction", "");
		string sOriginal = title;
		SplitStringByUpperCase(sOriginal, title);
		return true;
	}

	protected void SplitStringByUpperCase(string input, out string output)
	{
		output = "";
		bool wasPreviousUpperCase;
		int asciiChar;
		for (int i, count = input.Length(); i < count; i++)
		{
			asciiChar = input.ToAscii(i);
			bool isLowerCase = (asciiChar > 96 && asciiChar < 123);
			if (i > 0 && !wasPreviousUpperCase && !isLowerCase)
			{
				output += " " + asciiChar.AsciiToString();
				wasPreviousUpperCase = true;
			}
			else
			{
				 if (isLowerCase)
					wasPreviousUpperCase = false;
				output += asciiChar.AsciiToString();
			}
		}
	}
}

//------------------------------------------------------------------------------------------------
//! Medical actions that will be executed on target entity
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionMedical : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Target entity for Medical Action")];
	protected ref SCR_ScenarioFrameworkGet m_Getter;

	[Attribute(desc: "Medical actions that will be executed on target entity")];
	protected ref array<ref SCR_ScenarioFrameworkMedicalAction> m_aMedicalActions;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!m_Getter || !CanActivate())
			return;

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
			return;

		IEntity entity = IEntity.Cast(entityWrapper.GetValue());
		if (!entity)
			return;

		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(entity);
		if (!character)
			return;

		foreach (SCR_ScenarioFrameworkMedicalAction medicalAction : m_aMedicalActions)
		{
			medicalAction.Init(character);
		}
	}
}

//------------------------------------------------------------------------------------------------
//! Medical action base class
[BaseContainerProps(), SCR_ContainerMedicalActionTitle()]
class SCR_ScenarioFrameworkMedicalAction
{
	protected SCR_CharacterDamageManagerComponent m_DamageManager;

	//------------------------------------------------------------------------------------------------
	void Init(SCR_ChimeraCharacter character)
	{
		m_DamageManager = SCR_CharacterDamageManagerComponent.Cast(character.GetDamageManager());
		if (!m_DamageManager)
			return;

		OnActivate();
	}

	//------------------------------------------------------------------------------------------------
	void OnActivate();
}

//------------------------------------------------------------------------------------------------
//! Sets blood hit zone value
[BaseContainerProps(), SCR_ContainerMedicalActionTitle()]
class SCR_ScenarioFrameworkMedicalActionSetBlood : SCR_ScenarioFrameworkMedicalAction
{
	[Attribute(defvalue: "6000", uiwidget: UIWidgets.Slider, desc: "Character blood hit zone value", params: "0 6000 0.001")]
	protected float m_fBloodValue;

	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		HitZone bloodHitZone = m_DamageManager.GetBloodHitZone();
		if (bloodHitZone)
			bloodHitZone.SetHealth(m_fBloodValue);
	}
}

//------------------------------------------------------------------------------------------------
//! Sets resilience hit zone value
[BaseContainerProps(), SCR_ContainerMedicalActionTitle()]
class SCR_ScenarioFrameworkMedicalActionSetResilience : SCR_ScenarioFrameworkMedicalAction
{
	[Attribute(defvalue: "100", uiwidget: UIWidgets.Slider, desc: "Character resilience hit zone value", params: "0 100 0.001")]
	protected float m_fResilienceValue;

	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		HitZone resilienceHitZone = m_DamageManager.GetResilienceHitZone();
		if (resilienceHitZone)
			resilienceHitZone.SetHealth(m_fResilienceValue);
	}
}

//------------------------------------------------------------------------------------------------
//! Removes all bleedings
[BaseContainerProps(), SCR_ContainerMedicalActionTitle()]
class SCR_ScenarioFrameworkMedicalActionRemoveAllBleedings : SCR_ScenarioFrameworkMedicalAction
{
	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		m_DamageManager.RemoveAllBleedings();
	}
}

//------------------------------------------------------------------------------------------------
//! Removes all bleedings from specific hit zone group
[BaseContainerProps(), SCR_ContainerMedicalActionTitle()]
class SCR_ScenarioFrameworkMedicalActionRemoveGroupBleeding : SCR_ScenarioFrameworkMedicalAction
{
	[Attribute("10", UIWidgets.ComboBox, "Select Character hit zone group to stop bleeding from", "", ParamEnumArray.FromEnum(ECharacterHitZoneGroup))]
	protected ECharacterHitZoneGroup	m_eCharacterHitZoneGroup;

	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		m_DamageManager.RemoveGroupBleeding(m_eCharacterHitZoneGroup);
	}
}

//------------------------------------------------------------------------------------------------
//! Adds particular bleeding to selected hit zone
[BaseContainerProps(), SCR_ContainerMedicalActionTitle()]
class SCR_ScenarioFrameworkMedicalActionAddParticularBleeding : SCR_ScenarioFrameworkMedicalAction
{
	[Attribute(uiwidget: UIWidgets.EditBox, desc: "Which hit zone will start bleeding")];
	protected string m_sHitZoneName;

	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		m_DamageManager.AddParticularBleeding(m_sHitZoneName);
	}
}

//------------------------------------------------------------------------------------------------
//! Adds random bleeding
[BaseContainerProps(), SCR_ContainerMedicalActionTitle()]
class SCR_ScenarioFrameworkMedicalActionAddRandomBleeding : SCR_ScenarioFrameworkMedicalAction
{
	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		m_DamageManager.AddRandomBleeding();
	}
}

//------------------------------------------------------------------------------------------------
//! Sets Character hit zone group to add/remove tourniquets
[BaseContainerProps(), SCR_ContainerMedicalActionTitle()]
class SCR_ScenarioFrameworkMedicalActionSetTourniquettedGroup : SCR_ScenarioFrameworkMedicalAction
{
	[Attribute("10", UIWidgets.ComboBox, "Select Character hit zone group to add/remove tourniquets", "", ParamEnumArray.FromEnum(ECharacterHitZoneGroup))]
	protected ECharacterHitZoneGroup	m_eCharacterHitZoneGroup;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Whether target hit zone group is tourniquetted or not")]
	protected bool m_bTourniquetted;

	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		m_DamageManager.SetTourniquettedGroup(m_eCharacterHitZoneGroup, m_bTourniquetted);
	}
}

//------------------------------------------------------------------------------------------------
//! Sets Character hit zone group to add/remove saline bag
[BaseContainerProps(), SCR_ContainerMedicalActionTitle()]
class SCR_ScenarioFrameworkMedicalActionSetSalineBaggedGroup : SCR_ScenarioFrameworkMedicalAction
{
	[Attribute("10", UIWidgets.ComboBox, "Select Character hit zone group to add/remove saline bag", "", ParamEnumArray.FromEnum(ECharacterHitZoneGroup))]
	protected ECharacterHitZoneGroup	m_eCharacterHitZoneGroup;

	[Attribute(defvalue: "true", uiwidget: UIWidgets.CheckBox, desc: "Whether target hit zone group is saline bagged or not")]
	protected bool m_bSalineBagged;

	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		m_DamageManager.SetSalineBaggedGroup(m_eCharacterHitZoneGroup, m_bSalineBagged);
	}
}

//------------------------------------------------------------------------------------------------
//! Sets whether unconsciousness is allowed for this particular character
[BaseContainerProps(), SCR_ContainerMedicalActionTitle()]
class SCR_ScenarioFrameworkMedicalActionSetPermitUnconsciousness : SCR_ScenarioFrameworkMedicalAction
{
	[Attribute(defvalue: "true", uiwidget: UIWidgets.CheckBox, desc: "Whether unconsciousness is allowed for this particular character")]
	protected bool m_bPermitUnconsciousness;

	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		m_DamageManager.SetPermitUnconsciousness(m_bPermitUnconsciousness, true);
	}
}

//------------------------------------------------------------------------------------------------
//! Sets character bleeding rate multiplier for this particular character
[BaseContainerProps(), SCR_ContainerMedicalActionTitle()]
class SCR_ScenarioFrameworkMedicalActionSetBleedingRate : SCR_ScenarioFrameworkMedicalAction
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.Slider, desc: "Character bleeding rate multiplier", params: "0 5 0.001")]
	protected float m_fBleedingRate;

	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		m_DamageManager.SetDOTScale(m_fBleedingRate, true);
	}
}

//------------------------------------------------------------------------------------------------
//! Sets character regeneration rate multiplier for this particular character
[BaseContainerProps(), SCR_ContainerMedicalActionTitle()]
class SCR_ScenarioFrameworkMedicalActionSetRegenerationRate : SCR_ScenarioFrameworkMedicalAction
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.Slider, desc: "Character regeneration rate multiplier", params: "0 5 0.001")]
	protected float m_fRegeneration;

	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		m_DamageManager.SetRegenScale(m_fRegeneration, true);
	}
}
