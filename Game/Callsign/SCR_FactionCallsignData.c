/*!
For each faction, holds availible companies, which in turn holds availible platoons, which in turn holds availible squads
*/
class SCR_FactionCallsignData
{
	protected ref map<int, ref SCR_CallsignCompanyData> m_mCompanyCallsigns = new ref map<int, ref SCR_CallsignCompanyData>;
	protected ref map<int, ref SCR_CallsignCompanyData> m_mCompanyOverflowCallsigns = new ref map<int, ref SCR_CallsignCompanyData>;
	
	protected int m_iOverflowIndex;
	protected bool m_bRandomizedCallsigns;
	
	//---------------------------------------- On Init ----------------------------------------\\
	void SCR_FactionCallsignData(SCR_FactionCallsignInfo factionCallsignInfo)
	{
		m_iOverflowIndex = factionCallsignInfo.GetCompanyOverflowIndex();
		m_bRandomizedCallsigns = factionCallsignInfo.GetIsAssignedRandomly();
		
		array<ref SCR_CallsignInfo> companyArray = new array<ref SCR_CallsignInfo>;
		factionCallsignInfo.GetCompanyArray(companyArray);
		int count = companyArray.Count();
		
		for(int i = 0; i < count; i++)
		{
			SCR_CallsignCompanyData companyData = SCR_CallsignCompanyData();
			companyData.Init(factionCallsignInfo);
			
			//Check if overflow company and add it to correct map
			if (i < m_iOverflowIndex)
				m_mCompanyCallsigns.Insert(i, companyData);
			//Overflow means it only grabs these if non of the default companies are availible
			else 
				m_mCompanyOverflowCallsigns.Insert(i, companyData);
		}
	}
	
	//---------------------------------------- Get Random Callsign ----------------------------------------\\
	/*!
	Gets company, platoon and squad index randomly from availible callsigns
	Will first go through default companies, if these are all taken then go though overflow companies
	\param[out] company index
	\param[out] platoon index
	\param[out] squad index
	\return bool true if succesfully found a callsign
	*/ 
	bool GetRandomCallsign(out int company, out int platoon, out int squad)
	{
		SCR_CallsignCompanyData randomCompany;
		int random;
		
		//Default companies still availible
		if (!m_mCompanyCallsigns.IsEmpty())
		{
			Math.Randomize(-1);
			random = Math.RandomInt(0, m_mCompanyCallsigns.Count());
			company = m_mCompanyCallsigns.GetKey(random);
			randomCompany = m_mCompanyCallsigns.GetElement(random);
		}
			
		//Use overflow companies
		else if (!m_mCompanyOverflowCallsigns.IsEmpty())
		{
			Math.Randomize(-1);
			random = Math.RandomInt(0, m_mCompanyOverflowCallsigns.Count());
			company = m_mCompanyOverflowCallsigns.GetKey(random);
			randomCompany = m_mCompanyOverflowCallsigns.GetElement(random);
		}
		//No companies availible
		else 
			return false;
		
		randomCompany.GetRandomCallsign(platoon, squad);
		
		return true;
	}
	
	
	//---------------------------------------- Get first availible Callsign ----------------------------------------\\
	/*!
	Goes through all availible companies, platoons and squads and gets the first availible. Meaning if platoon 1 is taken it will get 2 next not 3.
	Will first go through default companies, if these are all taken then go though overflow companies
	\param[out] company index
	\param[out] platoon index
	\param[out] squad index
	\return bool true if succesfully found a callsign
	*/ 
	bool GetFirstAvailibleCallsign(out int firstAvailibleCompany, out int firstAvailiblePlatoon, out int firstAvailibleSquad)
	{
		firstAvailibleCompany = int.MAX;
		SCR_CallsignCompanyData firstAvailibleCompanyData;
		
		//Default companies still availible
		if (!m_mCompanyCallsigns.IsEmpty())
		{
			foreach (int index, SCR_CallsignCompanyData company: m_mCompanyCallsigns)
			{
				if (index < firstAvailibleCompany)
				{
					firstAvailibleCompany = index;
					firstAvailibleCompanyData = company;
				}
			}
		}
		//Use overflow companies
		else if (!m_mCompanyOverflowCallsigns.IsEmpty())
		{
			foreach (int index, SCR_CallsignCompanyData company: m_mCompanyOverflowCallsigns)
			{
				if (index < firstAvailibleCompany)
				{
					firstAvailibleCompany = index;
					firstAvailibleCompanyData = company;
				}
			}
		}
		//No companies availible
		else 
		{
			return false;
		}
	
		firstAvailibleCompanyData.GetFirstAvailibleCallsign(firstAvailiblePlatoon, firstAvailibleSquad);
		return true;
	}
	
	
	//---------------------------------------- Get Company specific Callsign ----------------------------------------\\
	/*!
	Gets a callsign for a specific company given.
	Will however use either GetRandomCallsign or GetFirstAvailibleCallsign (depending on callsign settings) if specific company is not availible
	\param[out] company index
	\param[out] platoon index
	\param[out] squad index
	\return bool true if succesfully found a callsign (either the first availible or GetRandomCallsign/GetFirstAvailibleCallsign)
	*/ 
	bool GetSpecificCompanyCallsign(out int company, out int platoon, out int squad)
	{
		 SCR_CallsignCompanyData companyData;
		
		//Get specific company
		if (!m_mCompanyCallsigns.Find(company, companyData))
			m_mCompanyOverflowCallsigns.Find(company, companyData);
		
		//Specific company found
		if (companyData)
		{
			if (!m_bRandomizedCallsigns)
				companyData.GetFirstAvailibleCallsign(platoon, squad);
			else
				companyData.GetRandomCallsign(platoon, squad);
		}
		//Company not availible, use default logics
		else 
		{
			if (!m_bRandomizedCallsigns)
				return GetFirstAvailibleCallsign(company, platoon, squad);
			else
				return GetRandomCallsign(company, platoon, squad);
		}
		
		return true;
	}
	
	
	//---------------------------------------- Add availible Callsign ----------------------------------------\\
	/*!
	Adds callsign back to availible callsign pool
	\param company index
	\param platoon index
	\param squad index
	*/ 
	void AddCallsign(int companyIndex, int plattonIndex, int squadIndex)
	{
		SCR_CallsignCompanyData companyData;
		
		if (companyIndex < m_iOverflowIndex)
		{
			if (m_mCompanyCallsigns.Find(companyIndex, companyData))
			{
				companyData.AddCallsign(plattonIndex, squadIndex);
			}
			else
			{
				companyData = new SCR_CallsignCompanyData();
				companyData.AddCallsign(plattonIndex, squadIndex);
				m_mCompanyCallsigns.Insert(companyIndex, companyData);
			}
		}
		else 
		{
			if (m_mCompanyOverflowCallsigns.Find(companyIndex, companyData))
			{
				companyData.AddCallsign(plattonIndex, squadIndex);
			}
			else
			{
				companyData = new SCR_CallsignCompanyData();
				companyData.AddCallsign(plattonIndex, squadIndex);
				m_mCompanyOverflowCallsigns.Insert(companyIndex, companyData);
			}
		}
	}
	
	//---------------------------------------- Remove availible Callsign ----------------------------------------\\
	/*!
	Removes callsign from availible callsign pool
	\param company index
	\param platoon index
	\param squad index
	*/ 
	void RemoveCallsign(int companyIndex, int plattonIndex, int squadIndex)
	{
		SCR_CallsignCompanyData companyData;
		
		if (companyIndex < m_iOverflowIndex)
		{
			if (m_mCompanyCallsigns.Find(companyIndex, companyData))
			{
				//Check if all platoons are used then remove company
				if (companyData.RemoveCallsign(plattonIndex, squadIndex))
				{
					m_mCompanyCallsigns.Remove(companyIndex);
				}
			}
		}
		else 
		{
			if (m_mCompanyOverflowCallsigns.Find(companyIndex, companyData))
			{
				//Check if all platoons are used then remove company
				if (companyData.RemoveCallsign(plattonIndex, squadIndex))
				{
					m_mCompanyOverflowCallsigns.Remove(companyIndex);
				}
			}
		}
	}
};
