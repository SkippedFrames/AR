class SCR_BIKIStringBuilder
{
	protected string m_sContent;

	//------------------------------------------------------------------------------------------------
	void AddLine(string line)
	{
		m_sContent += line + "\n";
	}

	//------------------------------------------------------------------------------------------------
	void AddTitle(int titleLevel, string text)
	{
		if (titleLevel < 1)
			titleLevel = 1;

		if (titleLevel > 6)
			titleLevel = 6;

		if (m_sContent != "")
		{
			if (m_sContent.Length() < 2 || m_sContent.Substring(m_sContent.Length() - 2, 2) != "\n\n")
			{
				if (titleLevel < 3)
					m_sContent += "\n\n";
				else if (titleLevel == 3)
					m_sContent += "\n";
			}
		}

		switch (titleLevel)
		{
			case 1: m_sContent += "= " + text + " =\n\n"; break;
			case 2: m_sContent += "== " + text + " ==\n\n"; break;
			case 3: m_sContent += "=== " + text + " ===\n\n"; break;
			case 4: m_sContent += "==== " + text + " ====\n"; break;
			case 5: m_sContent += "===== " + text + " =====\n"; break;
			case 6: m_sContent += "====== " + text + " ======\n"; break;
		}
	}

	//------------------------------------------------------------------------------------------------
	void BeginTable(string style = string.Empty)
	{
		if (style == string.Empty)
			m_sContent += "{| class=\"wikitable\"\n";
		else
			m_sContent += "{| class=\"wikitable\" style=\"" + style + "\"\n";
	}

	//------------------------------------------------------------------------------------------------
	//! This method allows empty headers
	void AddTableHeadersLine(notnull array<string> titles)
	{
		foreach (string title : titles)
		{
			if (title == string.Empty)
				m_sContent += "!\n";
			else
				m_sContent += "! " + title + "\n";
		}
	}

	//------------------------------------------------------------------------------------------------
	//! This method forbids empty headers and stops on the first encountered one
	void AddTableHeadersLine(string title1, string title2 = string.Empty, string title3 = string.Empty, string title4 = string.Empty, string title5 = string.Empty)
	{
		array<string> titles = { title1, title2, title3, title4, title5 };
		foreach (string title : titles)
		{
			if (title == string.Empty)
				return;

			m_sContent += "! " + title + "\n";
		}
	}

	//------------------------------------------------------------------------------------------------
	//! This method allows an empty header
	void AddTableHeader(string title, string style = string.Empty)
	{
		if (style == string.Empty)
			m_sContent += "! " + title + "\n";
		else
			m_sContent += "! style=\"" + style + "\" | " + title + "\n";
	}

	//------------------------------------------------------------------------------------------------
	//! This method allows empty data cells
	void AddTableDataCellsLine(notnull array<string> data)
	{
		m_sContent += "|-\n";
		foreach (string datum : data)
		{
			m_sContent += "| " + datum + "\n";
		}
	}

	//------------------------------------------------------------------------------------------------
	//! This method forbids empty data cells and stops on the first encountered one
	void AddTableDataCellsLine(string data1, string data2 = string.Empty, string data3 = string.Empty, string data4 = string.Empty, string data5 = string.Empty)
	{
		m_sContent += "|-\n";
		array<string> data = { data1, data2, data3, data4, data5 };
		foreach (string datum : data)
		{
			if (datum == string.Empty)
				return;

			m_sContent += "| " + datum + "\n";
		}
	}

	//------------------------------------------------------------------------------------------------
	//! This method allows an empty data cell
	void AddTableDataCell(string datum, string style = string.Empty)
	{
		if (style == string.Empty)
			m_sContent += "| " + datum + "\n";
		else
			m_sContent += "| style=\"" + style + "\" | " + datum + "\n";
	}

	//------------------------------------------------------------------------------------------------
	void AddHeaderAndDataCellsLine(string title, notnull array<string> data)
	{
		m_sContent += "|-\n" +
			"! " + title + "\n";

		foreach (string datum : data)
		{
			m_sContent += "| " + datum + "\n";
		}
	}

	//------------------------------------------------------------------------------------------------
	void EndTable()
	{
		m_sContent += "|}\n";
	}

	//------------------------------------------------------------------------------------------------
	void AddCategory(string catLv1 = string.Empty, string catLv2 = string.Empty, string catLv3 = string.Empty, string catLv4 = string.Empty, string catLv5 = string.Empty)
	{
		if (catLv1 == string.Empty)
			m_sContent += "{{GameCategory|armaR}}\n";
		if (catLv2 == string.Empty)
			m_sContent += "{{GameCategory|armaR|" + catLv1 + "}}\n";
		else if (catLv3 == string.Empty)
			m_sContent += "{{GameCategory|armaR|" + catLv1 + "|" + catLv2 + "}}\n";
		else if (catLv4 == string.Empty)
			m_sContent += "{{GameCategory|armaR|" + catLv1 + "|" + catLv2 + "|" + catLv3 + "}}\n";
		else if (catLv5 == string.Empty)
			m_sContent += "{{GameCategory|armaR|" + catLv1 + "|" + catLv2 + "|" + catLv3 + "|" + catLv4 + "}}\n";
		else
			m_sContent += "{{GameCategory|armaR|" + catLv1 + "|" + catLv2 + "|" + catLv3 + "|" + catLv4 + "|" + catLv5 + "}}\n";
	}

	//------------------------------------------------------------------------------------------------
	string GetContent()
	{
		return m_sContent;
	}

	//------------------------------------------------------------------------------------------------
	void Clear()
	{
		m_sContent = string.Empty;
	}
}
