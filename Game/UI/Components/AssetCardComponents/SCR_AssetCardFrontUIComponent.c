/** @ingroup Editor_UI Editor_UI_Components
Content Browser Card
*/
class SCR_AssetCardFrontUIComponent : ScriptedWidgetComponent
{
	//ToDo: Replace many of these values with automated solutions
    [Attribute()]
	private string m_sEntityNameWidgetName;
	
	//[Attribute()]
	//private string m_sTitleBackgroundWidgetName;
	
	[Attribute()]
	private string m_sEnityImageWidgetName;
	
	[Attribute()]
	protected string m_sIconSlotWidgetName;
	
	[Attribute("ModOverlay")]
	protected string m_sModIndicatorWidgetName;
	
	[Attribute("ModIndicatorIcon")]
	protected string m_sModIndicatorIconName;
	
	[Attribute("NoBudget")]
	protected string m_sExceedBudgetLayoutName;
	
	[Attribute("BudgetIcon")]
	protected string m_sExceedBudgetIconName;
	
	[Attribute("CompositionCost")]
	protected string m_sBudgetCostLayoutName;
	
	[Attribute("CostValue")]
	protected string m_sBudgetCostTextName;
	
	[Attribute("ImageOverlay")]
	protected string m_sImageOverlayName;
	
	[Attribute("")]
	protected string m_sAreaName;
	
	[Attribute()]
	protected ref array<string> m_aFactionColorNames;
	
	[Attribute(uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EEditableEntityLabelGroup))]
	protected ref array<EEditableEntityLabelGroup> m_sTraitLabelGroups;
	
	[Attribute()]
	protected ref array<string> m_sTraitNames;
	
	[Attribute()]
	private ref Color m_DisabledColor;
	
	[Attribute(params: "edds")]
	protected ref array<ResourceName> m_ImageOverlayTextures;
	
	protected ref ImageWidget m_EntityImageWidget;
	
	protected Widget m_BudgetCostLayout;
	protected Widget m_ExceedBudgetLayout;
	protected ref ScriptInvoker Event_OnCardInit = new ScriptInvoker();
	
	//Widgets
	private Widget m_Widget;
	//protected Widget m_Background;
	
	//Grid
	protected int m_iPrefabIndex;
	
	ref ScriptInvoker m_OnHover = new ref ScriptInvoker();
	ref ScriptInvoker m_OnFocus = new ref ScriptInvoker();
	
	//Info Ref
	private ref SCR_UIInfo m_Info;
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnCardInit()
	{		
		return Event_OnCardInit;
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateBudgetCost(SCR_EntityBudgetValue entityBudgetCost = null)
	{
		m_BudgetCostLayout = m_Widget.FindAnyWidget(m_sBudgetCostLayoutName);
		if (!m_BudgetCostLayout)
			return;
		
		if (!entityBudgetCost)
			m_BudgetCostLayout.SetVisible(false);
		else
		{
			m_BudgetCostLayout.SetVisible(true);
			TextWidget budgetCostTextWidget = TextWidget.Cast(m_BudgetCostLayout.FindAnyWidget(m_sBudgetCostTextName));
			if (budgetCostTextWidget)
				budgetCostTextWidget.SetText(entityBudgetCost.GetBudgetValue().ToString());
		}
	}
	
	void UpdateBlockingBudget(SCR_UIInfo blockingBudgetInfo = null)
	{
		if (!m_ExceedBudgetLayout)
			return;
		
		bool canPlace = blockingBudgetInfo == null;
		
		m_EntityImageWidget.SetSaturation(canPlace);
		m_ExceedBudgetLayout.SetVisible(!canPlace);
		
		ImageWidget exceedBudgetIcon = ImageWidget.Cast(m_ExceedBudgetLayout.FindWidget(m_sExceedBudgetIconName));
		if (blockingBudgetInfo)
			blockingBudgetInfo.SetIconTo(exceedBudgetIcon);
	}
	
	/*!
	Init card
	\param info
	\param prefab
	*/
	void InitCard(int prefabID, SCR_UIInfo info, ResourceName prefab, SCR_UIInfo blockingBudgetInfo = null)
	{
		if (!m_Widget)
			return;
		
		m_iPrefabIndex = prefabID;
		
		TextWidget entityNameWidget = TextWidget.Cast(m_Widget.FindAnyWidget(m_sEntityNameWidgetName));
		//m_Background = m_Widget.FindAnyWidget(m_sBackgroundsName);
		
		if (info)
		{
			m_Info = info;			
			//Set the entity name
			info.SetNameTo(entityNameWidget);

			SCR_EditableEntityUIInfo infoCard = SCR_EditableEntityUIInfo.Cast(info);
			if (infoCard)
			{
				//--- Set faction color (all widgets marked by 'Inherit Color' will use it as well)
				Faction faction = infoCard.GetFaction();
				if (faction)
				{
					Widget factionColor;
					foreach (string factionColorName: m_aFactionColorNames)
					{
						factionColor = m_Widget.FindAnyWidget(factionColorName);
						if (factionColor)
							factionColor.SetColor(faction.GetFactionColor());
					}
				}
				
				//--- Set icon
				Widget iconSlotWidget = m_Widget.FindAnyWidget(m_sIconSlotWidgetName);
				if (iconSlotWidget)
				{
					SCR_EditableEntityBaseSlotUIComponent iconSlot = SCR_EditableEntityBaseSlotUIComponent.Cast(iconSlotWidget.FindHandler(SCR_EditableEntityBaseSlotUIComponent));
					if (iconSlot)
					{
						SCR_EditableEntityUIConfig entityUIConfig = SCR_EditableEntityUIConfig.GetConfig();
						if (entityUIConfig)
						{
							array<ref SCR_EntitiesEditorUIRule> entityRules = entityUIConfig.GetRules();
							foreach (SCR_EntitiesEditorUIRule rule: entityRules)
							{
								if (rule.GetStates() & EEditableEntityState.RENDERED)
								{
									iconSlot.CreateWidget(infoCard, rule);
								}
							}
						}
					}
				}
				
				//--- Set entity image
				m_EntityImageWidget = ImageWidget.Cast(m_Widget.FindAnyWidget(m_sEnityImageWidgetName));
				if (m_EntityImageWidget) 
					infoCard.SetAssetImageTo(m_EntityImageWidget);
				
				ImageWidget imageOverlayWidget = ImageWidget.Cast(m_Widget.FindAnyWidget(m_sImageOverlayName));
				if (imageOverlayWidget && !m_ImageOverlayTextures.IsEmpty()) 
				{
					int textureIndex = m_iPrefabIndex % m_ImageOverlayTextures.Count();
					imageOverlayWidget.LoadImageTexture(0, m_ImageOverlayTextures[textureIndex]);
				}
				
				m_ExceedBudgetLayout = m_Widget.FindAnyWidget(m_sExceedBudgetLayoutName);
				
				UpdateBudgetCost();	
				UpdateBlockingBudget(blockingBudgetInfo);
				
				//--- Set faction flag
				int traitIndex;
				Widget traitWidget;
				ImageWidget traitIcon;
				if (faction)
				{
					SCR_ContentBrowserEditorComponent contentBrowser = SCR_ContentBrowserEditorComponent.Cast(SCR_ContentBrowserEditorComponent.GetInstance(SCR_ContentBrowserEditorComponent, false, true));
					if (!contentBrowser || contentBrowser.AreFactionsShownOnContentCards())
					{
						if (FindTraitIcon(traitIndex, traitWidget, traitIcon))
						{
							traitWidget.SetVisible(true);
							traitIcon.LoadImageTexture(0, faction.GetUIInfo().GetIconPath());
						}
					}
				}
				
				EEditorMode editorMode = 0;
				SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
				if (editorManager) 
				{
					SCR_EditorModeEntity modeEntity = editorManager.GetCurrentModeEntity();
					if (modeEntity)
						editorMode = modeEntity.GetModeType();
				}		
						
				//--- Find traits in labels
				SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
				if (core)
				{
					EEditableEntityLabelGroup labelGroup;
					array<EEditableEntityLabel> labels = {};
					SCR_UIInfo labelInfo;
					for (int i, count = infoCard.GetEntityLabels(labels); i < count; i++)
					{
						if (!core.GetLabelUIInfoIfValid(labels[i], editorMode, labelInfo))
							continue;
						
						if (core.GetLabelGroupType(labels[i], labelGroup) && m_sTraitLabelGroups.Contains(labelGroup) && labelInfo.HasIcon())
						{
							if (FindTraitIcon(traitIndex, traitWidget, traitIcon))
							{
								traitWidget.SetVisible(true);
								labelInfo.SetIconTo(traitIcon);
								
								SCR_LinkTooltipTargetEditorUIComponent tooltip = SCR_LinkTooltipTargetEditorUIComponent.Cast(traitWidget.FindHandler(SCR_LinkTooltipTargetEditorUIComponent));
								if (tooltip)
									tooltip.SetInfo(labelInfo);
							}
							else
							{
								//--- Out of trait icons
								break;
							}
						}
					}
				}
				
				//--- Show area image (only for systems)
				if (m_sAreaName)
				{
					Widget areaWidget = m_Widget.FindAnyWidget(m_sAreaName);
					if (areaWidget)
						areaWidget.SetVisible(infoCard.HasEntityFlag(EEditableEntityFlag.HAS_AREA));
				}
			}
		}
		else
		{
			if (entityNameWidget && prefab)
			{
				string path = prefab.GetPath();
				array<string> pathNames = new array<string>;
				path.Split("/", pathNames, true);
				entityNameWidget.SetText(pathNames[pathNames.Count() - 1]);
			}
		}
		
		//Check if asset is modded
		Widget modIndicator = m_Widget.FindAnyWidget(m_sModIndicatorWidgetName);
		if (modIndicator)
		{
			array<string> modList = SCR_AddonTool.GetResourceAddons(prefab, true);
			if (modList)
				modIndicator.SetVisible(modList.Count() > 0);
			else
				modIndicator.SetVisible(false);
			
			//~todo: Get Addon Icon. Use m_sModIndicatorIconName to get the ImageWidget
		}
		
		Event_OnCardInit.Invoke(m_iPrefabIndex);
	}
	protected bool FindTraitIcon(out int outIndex, out Widget traitWidget, out ImageWidget traitIcon)
	{
		if (m_sTraitNames && outIndex < m_sTraitNames.Count())
		{
			traitWidget = m_Widget.FindAnyWidget(m_sTraitNames[outIndex]);
			if (traitWidget)
			{
				traitIcon = ImageWidget.Cast(traitWidget.FindAnyWidget("Trait"));
				outIndex++;
				return true;
			}
		}
		outIndex = int.MAX;
		return false;
	}
	/*!
	Get Card info
	\return SCR_UIInfo
	*/
	SCR_UIInfo GetInfo()
	{
		return m_Info;
	} 
	
	/*!
	Set faction color of specific widgets
	\param newColor
	* /
	void SetColor(Color newColor)
	{
		if (m_Background)
			m_Background.SetColor(newColor);
	}
	*/
	
	/*!
	Get Button Widget
	\return Widget button
	*/
	Widget GetButtonWidget()
	{
		return m_Widget;
	}
	
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if (m_Widget)
			m_OnHover.Invoke(m_Widget, m_iPrefabIndex, true);
		return false;
	}
		
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		if (m_Widget)
			m_OnHover.Invoke(m_Widget, m_iPrefabIndex, false);
		
		return false;
	}
	
	/*!
	Get ScriptInvoker onHover
	\return ScriptInvoker
	*/
	ScriptInvoker GetOnHover()
	{
		return m_OnHover;
	}
	
	override bool OnFocus(Widget w, int x, int y)
	{
		if (m_Widget)
			m_OnFocus.Invoke(m_Widget, m_iPrefabIndex, true);
		return false;
	}
		
	override bool OnFocusLost(Widget w, int x, int y)
	{
		if (m_Widget)
			m_OnFocus.Invoke(m_Widget, m_iPrefabIndex, false);
		
		return false;
	}
	
	/*!
	Get ScriptInvoker onFocus
	\return ScriptInvoker
	*/
	ScriptInvoker GetOnFocus()
	{
		return m_OnFocus;
	}
	
	/*!
	\return Prefab index from the list of placeable prefabs.
	*/
	int GetPrefabIndex()
	{
		return m_iPrefabIndex;
	}	
	
	override void HandlerAttached(Widget w)
    {
		m_Widget = w;
	}
};
