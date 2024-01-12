//------------------------------------------------------------------------------------------------
// Ruleset for dosplay of all frindlies
[BaseContainerProps()]
class SCR_NameTagRulesetFriendlies : SCR_NameTagRulesetBase
{
	//------------------------------------------------------------------------------------------------
	override protected bool TestVisibilityFiltered(SCR_NameTagData data, float timeSlice)
	{
		if (data.m_Flags & ENameTagFlags.FADE_TIMER)
			data.m_fTimeSliceFade += timeSlice;
		
		if (data.m_Flags & ENameTagFlags.OBSTRUCTED) 	// obstructed, this is checked here in order for the LOS checks to have run for the smaller subset of tag data 
			return false;
		
		float distLerp = Math.InverseLerp(m_ZoneCfg.m_fFarthestZoneRangePow2, 0, data.m_fDistance); // reduce the angle required to show with distance -> the further is the entity, angle required to focus it gets smaller
		distLerp *= MAX_ANGLE/2;  // adjust for more standard ish FOV of 90 (45 radius) TODO: this should be taken from real FOV
		
		data.m_fAngleToScreenCenter = GetCameraToEntityAngle(data.m_vEntWorldPos, VERT_ANGLE_ADJUST);
		if (data.m_fAngleToScreenCenter < distLerp)		// pass if within visibility angle							
			return true;

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void DetermineVisibility(float timeSlice)
	{
		super.DetermineVisibility(timeSlice);

		int count = m_aCandidateTags.Count();
		for (int i = count - 1; i > -1; i--)
		{		
			SCR_NameTagData data = m_aCandidateTags.Get(i);

			UpdateVisibleTag(data, timeSlice);
		}
	}
};
