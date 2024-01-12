//------------------------------------------------------------------------------------------------
[BaseContainerProps("", "Entry of one command"), SCR_BaseGroupCommandTitleField("m_sCommandName")]
class SCR_BaseGroupCommand
{
	[Attribute("", UIWidgets.EditBox, "Unique name of the command")]
	protected string m_sCommandName;
	
	[Attribute(defvalue: "1", desc: "Will the command be shown to leader?")]
	protected bool m_bShowToLeader;
	
	[Attribute(defvalue: "0", desc: "Will the command be shown to member?")]
	protected bool m_bShowToMember;
	
	[Attribute(defvalue: "0", desc: "Will the command be shown while commanding from map?")]
	protected bool m_bShowOnMap;
	
	[Attribute(defvalue: "0", desc: "Is the command available while player is dead?")]
	protected bool m_bUsableWhileDead;
	
	[Attribute(defvalue: "0", desc: "Is the command available while player is unconscious?")]
	protected bool m_bUsableWhileUncouscious;
	
	[Attribute(defvalue: "0", desc: "ID of the gesture in gesture state machine to be played when command is given out. 0 = no gesture")]
	protected int m_iGestureID;
	
	[Attribute("{2EFEA2AF1F38E7F0}UI/Textures/Icons/icons_wrapperUI-64.imageset", UIWidgets.ResourceNamePicker, "Imageset source for display icon" )]
	protected ResourceName m_sImageset;
	
	[Attribute("", UIWidgets.EditBox, "Name of the icon associated to the command, taken from appropriate imageset set in the radial menu" )]
	protected string m_sIconName;
	
	[Attribute("", UIWidgets.EditBox, "Name of the sound event to be played, leave empty for no event")]
	protected string m_sSoundEventName;
	
	//------------------------------------------------------------------------------------------------
	//! method that will be executed when the command is selected in the menu. 
	// Returns true if the command was executed succesfully, false otherwise.
	bool Execute(IEntity cursorTarget, IEntity target, vector targetPosition, int playerID, bool isClient)
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetCommandName()
	{
		return m_sCommandName;
	}
	
	//------------------------------------------------------------------------------------------------
	bool CanBeShown()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	bool CanShowOnMap()
	{
		return m_bShowOnMap;
	}
	
	//------------------------------------------------------------------------------------------------
	bool CanBePerformed()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	bool CanRoleShow()
	{
		SCR_PlayerControllerGroupComponent groupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!groupController)
			return false;
		
		bool isPlayerLeader = groupController.IsPlayerLeaderOwnGroup();
		if (isPlayerLeader && !m_bShowToLeader)
			return false;
		
		if (!isPlayerLeader && !m_bShowToMember)
			return false;
		
		return true;
	}
	
	//-----------------------------------------------------------------------------------------------
	void PlayAIResponse(IEntity slaveGroup)
	{
		SCR_AIGroup aiGroup = SCR_AIGroup.Cast(slaveGroup);
		if (!slaveGroup)
			return;
		
		AIAgent agent = aiGroup.GetLeaderAgent();
		if (!agent)
			return;
		
		IEntity owner = agent.GetControlledEntity();
		if (!owner)
			return;
		
		SCR_CommunicationSoundComponent soundComponent = SCR_CommunicationSoundComponent.Cast(owner.FindComponent(SCR_CommunicationSoundComponent));
		if (!soundComponent)
			return;
		
		soundComponent.SoundEventPriority(SCR_SoundEvent.SOUND_CP_POSITIVEFEEDBACK, 50);
	}
	
	//------------------------------------------------------------------------------------------------
	string GetSoundEventName()
	{
		return m_sSoundEventName;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetCommandGestureID()
	{
		return m_iGestureID;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetIconName()
	{
		return m_sIconName;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetIconImageset()
	{
		return m_sImageset;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool CanBeShownInCurrentLifeState()
	{
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return false;
		
		//returns true because if there is no controlled entity then player is not dead nor uncoscious.
		IEntity controlledEntity = playerController.GetControlledEntity();
		if (!controlledEntity)
			return true;
		
		//we do not really care if controlled character does or does not have damage component, it is to avoid VME later
		SCR_CharacterControllerComponent characterDamageComp = SCR_CharacterControllerComponent.Cast(controlledEntity.FindComponent(SCR_CharacterControllerComponent));
		if (!characterDamageComp)
			return true;
		
		if ((!m_bUsableWhileDead && characterDamageComp.IsDead()) || (!m_bUsableWhileUncouscious && characterDamageComp.IsUnconscious()))
			return false;
		
		return true;
	}
}
