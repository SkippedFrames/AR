[ComponentEditorProps(category: "GameScripted/Camera", description: "")]
class SCR_PostProcessCameraComponentClass: SCR_BaseCameraComponentClass
{
};

/*!
Post-process effect of scripted camera.
*/
class SCR_PostProcessCameraComponent : SCR_BaseCameraComponent
{
	[Attribute()]
	private ref array<ref SCR_CameraPostProcessEffect> m_Effects;
	
	private SCR_CameraBase m_Camera;
	
	/*!
	Get effect of given type.
	\param type Post-process effect type
	\return Effect
	*/
	SCR_CameraPostProcessEffect FindEffect(PostProcessEffectType type)
	{
		foreach (SCR_CameraPostProcessEffect effect: m_Effects)
		{
			if (type == effect.GetType()) return effect;
		}
		return null;
	}
	
	protected void OnCameraActivate()
	{		
		foreach (SCR_CameraPostProcessEffect effect: m_Effects)
		{
			effect.CreateEffect(m_Camera.GetCameraIndex());
		}
	}
	protected void OnCameraDeactivate()
	{
		foreach (SCR_CameraPostProcessEffect effect: m_Effects)
		{
			effect.DeleteEffect();
		}
	}
	
	void SCR_PostProcessCameraComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_Camera = SCR_CameraBase.Cast(ent);
		if (!m_Camera) return;
		
		m_Camera.GetOnCameraActivate().Insert(OnCameraActivate);
		m_Camera.GetOnCameraDeactivate().Insert(OnCameraDeactivate);
	}
	void ~SCR_PostProcessCameraComponent()
	{
		if (!m_Camera) return;
		m_Camera.GetOnCameraActivate().Remove(OnCameraActivate);
		m_Camera.GetOnCameraDeactivate().Remove(OnCameraDeactivate);
	}
};

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(PostProcessEffectType, "m_Type")]
class SCR_CameraPostProcessEffect
{
	[Attribute(defvalue: "0", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(PostProcessEffectType))]
	private PostProcessEffectType m_Type;

	[Attribute(uiwidget: UIWidgets.Slider, params: "0 19 1")]
	private int m_iPriority;
	
	[Attribute(params: "emat")]
	private ResourceName m_MaterialPath;
	
	private int m_iCameraId;
	private Material m_Material;
	
	PostProcessEffectType GetType()
	{
		return m_Type;
	}

	void SetParam(string param, float value)
	{
		if (m_Material) m_Material.SetParam(param, value);
	}
	void ResetParam(string param)
	{
		if (m_Material) m_Material.ResetParam(param);
	}
	
	void SetMaterial()
	{
		SetMaterial(m_MaterialPath);
	}
	void SetMaterial(ResourceName material)
	{
		GetGame().GetWorld().SetCameraPostProcessEffect(m_iCameraId, m_iPriority, m_Type, material);
		
		m_Material = Material.GetMaterial(material);
		if (!m_Material)
		{
			Print(string.Format("Error when loading post-process material '%1'! Please check effect priority.", material), LogLevel.ERROR);
			DeleteEffect();
			return;
		}
	}
	void CreateEffect(int cameraId)
	{
		if (SCR_Global.IsEditMode())
			return;
		
		m_iCameraId = cameraId;
		SetMaterial();
	}
	void DeleteEffect()
	{
		if (SCR_Global.IsEditMode())
			return;
		
		GetGame().GetWorld().SetCameraPostProcessEffect(m_iCameraId, m_iPriority, PostProcessEffectType.None, "");
		
		m_iCameraId = 0;
		m_Material = null;
	}
	
	void ~SCR_CameraPostProcessEffect()
	{
		DeleteEffect();
	}
};
