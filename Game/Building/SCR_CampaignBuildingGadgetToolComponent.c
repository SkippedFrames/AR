class SCR_CampaignBuildingGadgetToolComponentClass : SCR_GadgetComponentClass
{

}

class SCR_CampaignBuildingGadgetToolComponent : SCR_GadgetComponent
{
	[Attribute(defvalue: "25", desc: "Max distance at which the ghost preview is shown.")]
	protected float m_fDistanceToShowPreview;

	[Attribute(defvalue: "10", desc: "Max distance from which the composition can be build.")]
	protected float m_fDistanceToBuildComposition;

	[Attribute(defvalue: "10", desc: "How much of construction value this tool adds to composition per one action")]
	protected int m_iConstructionValue;

	protected const static float TRACE_DELAY_VALUE = 0.5;

	protected float m_fTraceDelay;

	protected ref array<SCR_CampaignBuildingLayoutComponent> m_aShownPreview = {};
	protected ref array<SCR_CampaignBuildingLayoutComponent> m_aShownPreviewOld = {};

	//protected SCR_CampaignBuildingLayoutComponent m_LayoutComponent;

	//------------------------------------------------------------------------------------------------
	override void Update(float timeSlice)
	{
		m_fTraceDelay += timeSlice;

		if (m_fTraceDelay < TRACE_DELAY_VALUE)
			return;

		TraceCompositionToShowPreview();

		m_fTraceDelay = 0.0;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnToggleActive(bool state)
	{
		m_fTraceDelay = 0.0;
		
		if (state)
			ToolToHand();
		else
			ToolToInventory();
	}
	
	//------------------------------------------------------------------------------------------------
	override EGadgetType GetType()
	{
		return EGadgetType.BUILDING_TOOL;
	}

	//------------------------------------------------------------------------------------------------
	// Perform one build step - add a given build value to a composition player is building.
	/*protected void Build(notnull SCR_CampaignBuildingLayoutComponent layoutComponent)
	{
		layoutComponent.AddBuildingValue(m_iConstructionValue);
	}*/

	//------------------------------------------------------------------------------------------------
	//! Triggered when an operation with gadget is performed - picked up, taken into hands, back to backpack etc.
	//! \param charOwner must have a GadgetManager component
		
	override void ModeSwitch(EGadgetMode mode, IEntity charOwner)
	{
		if (!charOwner || charOwner != EntityUtils.GetPlayer())
		{
			super.ModeSwitch(mode, charOwner);
			return;
		}

		if (SCR_GadgetManagerComponent.GetGadgetManager(charOwner).GetHeldGadgetComponent() == this)
			ToolToHand();
		else
			ToolToInventory();

		super.ModeSwitch(mode, charOwner);
	}


	//------------------------------------------------------------------------------------------------
	//! Building tool taken to hand - show preview etc.
	protected void ToolToHand()
	{
		// Disabled for future
		/*EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(m_CharacterOwner.FindComponent(EventHandlerManagerComponent));
		if (eventHandlerManager)
			eventHandlerManager.RegisterScriptHandler("OnADSChanged", this, TraceCompositionToBuild);	*/

		//ToDo: Later the frame has to run on server too because of tracing the composition to build
		if (!System.IsConsoleApp())
			ConnectToGadgetsSystem();
	}

	//------------------------------------------------------------------------------------------------
	//! Building tool out of hands - show preview.
	protected void ToolToInventory()
	{
		//ToDo: Later the frame has to run on server too because of tracing the composition to build
		if (!System.IsConsoleApp())
		{
			RemovePreviews();
			DisconnectFromGadgetsSystem();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void RemovePreviews()
	{
		foreach (SCR_CampaignBuildingLayoutComponent layoutComponent : m_aShownPreviewOld)
		{
			if (layoutComponent)
				layoutComponent.DeletePreview();
		}

		m_aShownPreviewOld.Clear();
	}

	//------------------------------------------------------------------------------------------------
	// Trace a composition to build commented out for future
	/*void TraceCompositionToBuild()
	{
		if (!m_CharacterOwner)
			return;

		ChimeraCharacter character = ChimeraCharacter.Cast(m_CharacterOwner);

		TraceParam param = new TraceParam();
		param.Start = character.EyePosition();
		param.End = param.Start + GetPlayersDirection() * m_fDistanceToBuildComposition;

		param.Flags =TraceFlags.ENTS;
		param.Exclude = character;
		param.LayerMask = EPhysicsLayerPresets.Interaction;
		BaseWorld world = GetOwner().GetWorld();
		float traceDistance = world.TraceMove(param, EvaluateBuildEntity);

		// This has to be done after the trace finished, because spawning / deleting or moving with the entity while trace is running can cause an issues.
		if (m_LayoutComponent)
			{
				Build(m_LayoutComponent);
				m_LayoutComponent = null;
			}

	}*/

	//------------------------------------------------------------------------------------------------
	//! Trace the possible previews
	protected bool TraceCompositionToShowPreview()
	{
		BaseWorld world = GetOwner().GetWorld();
		if (!world)
			return false;
				
		world.QueryEntitiesBySphere(GetOwner().GetOrigin(), m_fDistanceToShowPreview, EvaluatePreviewEntity);

		// Delete those preview which are no more on the list of traced one and then copy the array for next iteration cycle.
		foreach (SCR_CampaignBuildingLayoutComponent component : m_aShownPreviewOld)
		{
			if (component && !m_aShownPreview.Contains(component))
				component.DeletePreview();
		}
				
		foreach (SCR_CampaignBuildingLayoutComponent component : m_aShownPreview)
		{
			if (component && !m_aShownPreviewOld.Contains(component))
				component.SpawnPreview();
		}
		
		m_aShownPreviewOld.Copy(m_aShownPreview);
		m_aShownPreview.Clear();

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool EvaluatePreviewEntity(IEntity ent)
	{
		if (!ent)
			return true;

		SCR_CampaignBuildingLayoutComponent layoutComponent = SCR_CampaignBuildingLayoutComponent.Cast(ent.FindComponent(SCR_CampaignBuildingLayoutComponent));
		if (!layoutComponent)
			return true;

		m_aShownPreview.Insert(layoutComponent);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the value how much this tool adds to a building value with one action
	int GetToolConstructionValue()
	{
		return m_iConstructionValue;
	}

	//------------------------------------------------------------------------------------------------
	// Disabled for future
	/*bool EvaluateBuildEntity(IEntity ent, vector start = "0 0 0", vector dir = "0 0 0")
	{
		IEntity parent = ent.GetParent();
		if (!parent)
			return true;

		SCR_CampaignBuildingLayoutComponent layoutComponent = SCR_CampaignBuildingLayoutComponent.Cast(parent.FindComponent(SCR_CampaignBuildingLayoutComponent));
		if (!layoutComponent)
			return true;

		m_LayoutComponent = layoutComponent;
		return false;
	}*/

	//------------------------------------------------------------------------------------------------
	//! Returns the direction the player is looking Disabled debug
	/*private vector GetPlayersDirection()
	{
		vector aimMat[4];
		ChimeraCharacter character = ChimeraCharacter.Cast(m_CharacterOwner);
		Math3D.AnglesToMatrix(CharacterControllerComponent.Cast(character.FindComponent(CharacterControllerComponent)).GetInputContext().GetAimingAngles() * Math.RAD2DEG, aimMat);
		return aimMat[2];
	}*/
}

