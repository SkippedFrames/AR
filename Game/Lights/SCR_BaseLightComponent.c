[EntityEditorProps(category: "GameScripted/BaseLightComponent", description: "Basic light component to spawn a light")]
class SCR_BaseLightComponentClass : ScriptComponentClass
{
};

class SCR_BaseLightComponent : ScriptComponent
{		
	//DEBUG
	/*Shape shape;
	Shape shape1;*/
	
	//------------------------------------------------------------------------------------------------
	//LightEntity CreateLight(LightType lightType, LightFlags flags, float effectRadius, vector color, float emmisivity, vector position, vector direction = vector.Zero, LightLensFlareType flareType, float lensFlareScale, vector lensFlareOffset)
	LightEntity CreateLight(SCR_BaseLightData lightData, vector position, vector direction, float emmisivity)
	{	
		if (!lightData)
			return null;
		
		LightEntity light = LightEntity.CreateLight(lightData.GetLightType(), lightData.GetLightFlag(), lightData.GetEffectRadius(), Color.FromVector(lightData.GetLightColor()), emmisivity, position, direction);
		if (!light)
			return null;
			
		GetOwner().AddChild(light, -1);
		light.SetOrigin(position);
		light.SetLensFlareType(lightData.GetFlareType());
		light.SetLensFlareScale(lightData.GetLensFlareScale());
		light.SetLensFlareOffset(lightData.GetLensFlareOffset());
		light.SetConeAngle(lightData.GetLightConeAngle());
		light.SetIntensityEVClip(lightData.GetIntensityClipEV());
		
		// DEBUG POSITION OF THE LIGHT
		//shape = Shape.CreateCylinder(ARGB(255, 255, 0, 0), ShapeFlags.VISIBLE, position, 0.008, 0.008);
		//shape1 = Shape.CreateArrow(lightOffset.Multiply4(mat), componentData.m_aLightData[i].GetLightConeDirection().Multiply4(mat), 1.5, ARGB(255, 0, 255, 0), ShapeFlags.VISIBLE);*/
		
		return light;
	}
};
