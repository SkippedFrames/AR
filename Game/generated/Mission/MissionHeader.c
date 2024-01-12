/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Mission
\{
*/

/*!
Base class for interaction with mission headers.
It is done this way so c++ can add its own parameters along with scripted ones as easily as possible

class TestingMissionHeader : MissionHeader
{
 	[Attribute(0, UIWidgets.EditBox)]
		int m_someValue;
};


Example missionHeader.conf :

	TestingMissionHeader {
	}


After creating this you should be able to read your mission header with all data by calling
	TestingMissionHeader myData = TestingMissionHeader.Cast(MissionHeader.ReadMissionHeader("path/to/missionHeader.conf"))\n
*/
class MissionHeader: ScriptAndConfig
{
	//! Returns the path to the world file
	proto external string GetWorldPath();
	//! Returns the path to this mission header
	proto external string GetHeaderResourcePath();
	proto ResourceName GetHeaderResourceName();
	//! Reads mission header object from given file
	static proto ref MissionHeader ReadMissionHeader(string path);
}

/*!
\}
*/
