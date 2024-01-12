#include "scripts/Game/config.c"
class SCR_RespawnTimer
{
	//! Time at which respawn timer was engaged
	#ifndef AR_RESPAWN_TIMER_TIMESTAMP
	protected float m_fStartTime;
	#else
	protected WorldTimestamp m_fStartTime;
	#endif
	//! The duration of this timer in seconds
	protected float m_fDuration;
	
	/*!
		Start the timer by providing new start time.
		\param timeNow Current (synchronized) time to ask at.
	*/
	#ifndef AR_RESPAWN_TIMER_TIMESTAMP
	void Start(float timeNow)
	#else
	void Start(WorldTimestamp timeNow)
	#endif
	{
		m_fStartTime = timeNow;
	}
	
	/*!
		Returns current start time of this timer as seconds of synchronized time.
	*/
	#ifndef AR_RESPAWN_TIMER_TIMESTAMP
	float GetStartTime()
	#else
	WorldTimestamp GetStartTime()
	#endif
	{
		return m_fStartTime;
	}
	
	/*!
		\param timeNow Current (synchronized) time to ask at.
		\return Returns true in case timer is finished, false otherwise.
	*/
	#ifndef AR_RESPAWN_TIMER_TIMESTAMP
	bool IsFinished(float timeNow)
	{	
		return timeNow >= m_fStartTime + m_fDuration;
	}
	#else
	bool IsFinished(WorldTimestamp timeNow)
	{
		return timeNow.GreaterEqual(m_fStartTime.PlusSeconds(m_fDuration));
	}
	#endif
	
	/*!
		\param timeNow Current (synchronized) time to ask at.
		\return Returns remaining time in seconds or 0 if finished.
	*/
	#ifndef AR_RESPAWN_TIMER_TIMESTAMP
	float GetRemainingTime(float timeNow)
	{
		float rem = (m_fStartTime + m_fDuration) - timeNow;
		if (rem <= 0)
			return 0.0;
	
		return rem;
	}
	#else
	float GetRemainingTime(WorldTimestamp timeNow)
	{
		WorldTimestamp endTime = m_fStartTime.PlusSeconds(m_fDuration);
		float rem = endTime.DiffMilliseconds(timeNow);
		if (rem <= 0)
			return 0.0;

		return rem / 1000.0;
	}
	#endif
	
	/*!
		Returns duration in seconds of this timer.
	*/
	float GetDuration()
	{
		return m_fDuration;
	}
	
	/*!
		\param duration New duration in seconds to set.
	*/
	void SetDuration(float duration)
	{
		m_fDuration = duration;
	}
	
	/*!
		Serialize this class using provided ScriptBitWriter.
	*/
	bool RplSave(ScriptBitWriter writer)
	{
		#ifndef AR_RESPAWN_TIMER_TIMESTAMP
		writer.WriteFloat(m_fStartTime);
		#else
		writer.Write(m_fStartTime, 64);
		#endif
		writer.WriteFloat(m_fDuration);
		
		return true;
	}
	
	/*!
		Deserialize this class using provided ScriptBitWriter.
	*/
    bool RplLoad(ScriptBitReader reader)
	{
		#ifndef AR_RESPAWN_TIMER_TIMESTAMP
		reader.ReadFloat(m_fStartTime);
		#else
		reader.Read(m_fStartTime, 64);
		#endif
		reader.ReadFloat(m_fDuration);
		
		return true;
	}	
	
	//################################################################################################
	//! Codec methods
	//------------------------------------------------------------------------------------------------
	static void Encode(SSnapSerializerBase snapshot, ScriptCtx ctx, ScriptBitSerializer packet) 
	{
		#ifndef AR_RESPAWN_TIMER_TIMESTAMP
		snapshot.Serialize(packet, 8);
		#else
		snapshot.Serialize(packet, 12);
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Decode(ScriptBitSerializer packet, ScriptCtx ctx, SSnapSerializerBase snapshot) 
	{
		#ifndef AR_RESPAWN_TIMER_TIMESTAMP
		return snapshot.Serialize(packet, 8);
		#else
		return snapshot.Serialize(packet, 12);
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	static bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs, ScriptCtx ctx) 
	{		
		#ifndef AR_RESPAWN_TIMER_TIMESTAMP
		return lhs.CompareSnapshots(rhs, 8);
		#else
		return lhs.CompareSnapshots(rhs, 12);
		#endif
	}

	//------------------------------------------------------------------------------------------------
	static bool PropCompare(SCR_RespawnTimer prop, SSnapSerializerBase snapshot, ScriptCtx ctx) 
	{
		#ifndef AR_RESPAWN_TIMER_TIMESTAMP
		return snapshot.Compare(prop.m_fStartTime, 4) 
		#else
		return snapshot.Compare(prop.m_fStartTime, 8)
		#endif
			&& snapshot.Compare(prop.m_fDuration, 4);
			}
			
	//------------------------------------------------------------------------------------------------
	static bool Extract(SCR_RespawnTimer prop, ScriptCtx ctx, SSnapSerializerBase snapshot) 
	{
		#ifndef AR_RESPAWN_TIMER_TIMESTAMP
		snapshot.SerializeBytes(prop.m_fStartTime, 4);
		#else
		snapshot.SerializeBytes(prop.m_fStartTime, 8);
		#endif
		snapshot.SerializeBytes(prop.m_fDuration, 4);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, SCR_RespawnTimer prop) 
	{
		#ifndef AR_RESPAWN_TIMER_TIMESTAMP
		snapshot.SerializeBytes(prop.m_fStartTime, 4);
		#else
		snapshot.SerializeBytes(prop.m_fStartTime, 8);
		#endif
		snapshot.SerializeBytes(prop.m_fDuration, 4);	
		return true;
	}
};
