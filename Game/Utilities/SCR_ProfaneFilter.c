//------------------------------------------------------------------------------------------------
//! SCR_ProfaneFilter
//!
//! Handles filtering profanities in texts
//------------------------------------------------------------------------------------------------
class SCR_ProfaneFilter
{
	//todo@mour:speedrun solution, look into performance!!!
	//todo@mour:solve lowercasing
	
	//------------------------------------------------------------------------------------------------
	bool ReplaceProfanities(string inputText, out string output, string replaceText = "*")
	{
		bool profanityFound = false;
		
		if (inputText.IsEmpty())
			return false;
		
		inputText.ToLower();
		foreach (string profane : m_aProfanityBlacklist)
		{
			if (inputText.Contains(profane))
			{
				inputText.Replace(profane, SCR_StringHelper.PadLeft("", profane.Length(), replaceText));			
				profanityFound = true;
			}
		}
		
		output = inputText;
		return profanityFound;
	}
	
	protected ref array<string> m_aProfanityBlacklist = {	"figlio di puttana",
												"shokubutsuningen",
												"madonna puttana",
												"seishinhakujaku",
												"seisinhakuzyaku",
												"syokubutuningen",
												"chingada madre",
												"testa di cazzo",
												"klu klux klan",
												"masturbazione",
												"porca madonna",
												"succhia cazzi",
												"goldenshowers",
												"koroshiteyaru",
												"motherfuckers",
												"tokushugakkyu",
												"tokusyugakkyu",
												"jungle bunny",
												"masturbacion",
												"moon cricket",
												"porch monkey",
												"fukashokumin",
												"fuuuuuuuuuck",
												"hakuzyakusya",
												"hukasyokumin",
												"ku klux klan",
												"motherfucker",
												"semushiotoko",
												"cameljockey",
												"ching chong",
												"durka durka",
												"beefwhistle",
												"enzyokousai",
												"fujinoyamai",
												"fuuuuuuuuck",
												"fuuuuuuuuuu",
												"hakujakusha",
												"huzinoyamai",
												"intercourse",
												"junglebunny",
												"m07th3rfukr",
												"m0th3rfvk3r",
												"m0th3rfvker",
												"moth3rfucer",
												"moth3rfvk3r",
												"moth3rfvker",
												"semusiotoko",
												"ushigoroshi",
												"vvhitepower",
												"white power",
												"cocksucker",
												"dio bestia",
												"masturbare",
												"masturbate",
												"nhiggerman",
												"paedophile",
												"puttaniere",
												"schwuchtel",
												"spanishick",
												"spanishook",
												"spanishunk",
												"vaffanculo",
												"xblrewards",
												"zipperhead",
												"arseholing",
												"arselicker",
												"bestiality",
												"butagorosi",
												"chikusatsu",
												"circlejerk",
												"cotton pic",
												"cotton pik",
												"enjokousai",
												"fuuuuuuuck",
												"hakuroubyo",
												"inugoroshi",
												"manberries",
												"masterbate",
												"masturbait",
												"mitsukuchi",
												"sangokujin",
												"sangokuzin",
												"seishinijo",
												"seisinizyo",
												"shinheimin",
												"torukoburo",
												"whitepower",
												"yabunirami",
												"arschloch",
												"auschwitz",
												"chlamydia",
												"crackhead",
												"deez nuts",
												"ejaculate",
												"hurensohn",
												"kilurself",
												"masterbat",
												"miststück",
												"ricchione",
												"testicles",
												"vaccagare",
												"vatefaire",
												"akimekura",
												"arseholed",
												"arseholes",
												"beeeyotch",
												"bin laden",
												"chieokure",
												"chourimbo",
												"chourinbo",
												"chourippo",
												"chuurembo",
												"chuurenbo",
												"coon hunt",
												"coon kill",
												"cottonpic",
												"cottonpik",
												"fuuuuuuck",
												"golliwogg",
												"hantoujin",
												"hantouzin",
												"holocaust",
												"inugorosi",
												"koon hunt",
												"koon kill",
												"koumoujin",
												"koumouzin",
												"n1ggerman",
												"n4ggerman",
												"naggerman",
												"niggerman",
												"nyggerman",
												"pedophile",
												"sinheimin",
												"sofa king",
												"torukozyo",
												"towelhead",
												"tyourinbo",
												"tyourippo",
												"usigorosi",
												"anushead",
												"ballsack",
												"bitching",
												"bullshit",
												"butthole",
												"buttplug",
												"chingado",
												"chingate",
												"coglione",
												"coglioni",
												"dickhead",
												"fickdich",
												"inculato",
												"jack off",
												"red tube",
												"schlampe",
												"tarlouse",
												"abortion",
												"anuslick",
												"arsehole",
												"bakachon",
												"bakatyon",
												"beeyotch",
												"binladen",
												"butthead",
												"castrate",
												"chankoro",
												"chonkoro",
												"clitoris",
												"coonhunt",
												"coonkill",
												"deeznuts",
												"etahinin",
												"foreskin",
												"fuuuuuck",
												"god damn",
												"gollywog",
												"jiggaboo",
												"kichigai",
												"klu klux",
												"knobhead",
												"koonhunt",
												"koonkill",
												"manshaft",
												"mitukuti",
												"molester",
												"molestor",
												"n1german",
												"negerman",
												"nggerman",
												"nigerman",
												"nimpinin",
												"ninpinin",
												"santorum",
												"shinajin",
												"shirakko",
												"sofaking",
												"tieokure",
												"torukojo",
												"tyankoro",
												"tyonkoro",
												"tyurenbo",
												"a55hole",
												"bastard",
												"beeatch",
												"bellend",
												"bukakke",
												"caralho",
												"cocaine",
												"dumbass",
												"fanculo",
												"fucking",
												"jerkoff",
												"jigaboo",
												"maricon",
												"mulatto",
												"nipples",
												"nutsack",
												"panties",
												"pendejo",
												"pussies",
												"puttana",
												"puttane",
												"scheiße",
												"shemale",
												"tapatte",
												"tapette",
												"xl3lpet",
												"ainujin",
												"ainuzin",
												"asshole",
												"beeotch",
												"beyitch",
												"beyotch",
												"blowjob",
												"coituss",
												"cojelon",
												"cojones",
												"deeznut",
												"facking",
												"fuuuuck",
												"fxuxcxk",
												"gestapo",
												"hairpie",
												"jackass",
												"kikeiji",
												"kikeizi",
												"kitigai",
												"kluklux",
												"kurombo",
												"n166ers",
												"n1gg3rs",
												"n1ggers",
												"niggers",
												"nigglet",
												"phaggot",
												"preteen",
												"raghead",
												"rizzape",
												"rompari",
												"schlong",
												"sinazin",
												"suck my",
												"tea bag",
												"teensex",
												"tosatsu",
												"w3tb4ck",
												"w3tback",
												"wetb4ck",
												"wetback",
												"xbl pet",
												"beaner",
												"beetch",
												"cabron",
												"cadela",
												"cagada",
												"chinga",
												"cooter",
												"culear",
												"culero",
												"darkie",
												"downie",
												"encule",
												"fetish",
												"foutre",
												"herpes",
												"hooker",
												"mierda",
												"molest",
												"nignog",
												"nutted",
												"pinche",
												"putain",
												"rapage",
												"raping",
												"reggin",
												"salaud",
												"salope",
												"scrote",
												"shitty",
												"tampon",
												"tranny",
												"xblpet",
												"anuses",
												"asshat",
												"biotch",
												"blowme",
												"blyadt",
												"ceemen",
												"chinpo",
												"chonga",
												"chonko",
												"cocain",
												"coitus",
												"condom",
												"douche",
												"etambo",
												"etanbo",
												"faggot",
												"fatass",
												"fucker",
												"fuuuck",
												"hitl3r",
												"hitler",
												"inc3st",
												"incest",
												"j1g4b0",
												"j1g4bo",
												"j1gab0",
												"j1gabo",
												"jig4b0",
												"jig4bo",
												"jigabo",
												"lezzie",
												"n1663r",
												"n166er",
												"n1gg3r",
												"n1gger",
												"n4gg3r",
												"nagg3r",
												"ni66er",
												"niggaz",
												"nigger",
												"nigguh",
												"niggur",
												"niglet",
												"niqqer",
												"pedoph",
												"penile",
												"phagot",
												"pu555y",
												"pun4ni",
												"pun4nl",
												"punan1",
												"punani",
												"punanl",
												"pusss1",
												"pussse",
												"pusssi",
												"pusssl",
												"pusssy",
												"r3c7um",
												"r4p15t",
												"r4p1st",
												"r4pi5t",
												"r4pist",
												"raibyo",
												"rap15t",
												"rap1st",
												"rapi5t",
												"rapist",
												"rectum",
												"retard",
												"rimjob",
												"suckmy",
												"teabag",
												"teebag",
												"tosatu",
												"tyonga",
												"tyonko",
												"v461n4",
												"v461na",
												"v46in4",
												"v46ina",
												"v4g1n4",
												"v4g1na",
												"v4gin4",
												"v4gina",
												"va61n4",
												"va61na",
												"va6in4",
												"va6ina",
												"vag1n4",
												"vag1na",
												"vagin4",
												"vagina",
												"wanker",
												"x8lp3t",
												"かわらこじき",
												"ころしてやる",
												"とるこじょう",
												"はんとうじん",
												"カワラコジキ",
												"コロシテヤル",
												"arsch",
												"baise",
												"biach",
												"cacca",
												"cazzo",
												"downy",
												"fanny",
												"fotze",
												"gooch",
												"injun",
												"joder",
												"kurva",
												"kurwa",
												"merda",
												"merde",
												"minge",
												"nonce",
												"penis",
												"perra",
												"pikey",
												"pizda",
												"polla",
												"porra",
												"punal",
												"pussy",
												"raped",
												"semen",
												"skank",
												"sodom",
												"taint",
												"ahole",
												"anuss",
												"aokan",
												"arsed",
												"arses",
												"b00bz",
												"bitch",
												"bladt",
												"blyad",
												"bon3r",
												"boner",
												"boobs",
												"ch1nk",
												"chink",
												"choad",
												"chode",
												"dildo",
												"dongs",
												"f3lch",
												"fcuuk",
												"felch",
												"fuuck",
												"g000k",
												"goook",
												"h1tl3",
												"h1tle",
												"hitlr",
												"honky",
												"hymen",
												"k k k",
												"kxkxk",
												"l3sb0",
												"lezbo",
												"manko",
												"n1664",
												"n166a",
												"n1g3r",
												"n3gro",
												"n4g3r",
												"nag3r",
												"natzi",
												"ni666",
												"ni66a",
												"ni66g",
												"ni6g6",
												"ni6gg",
												"nig66",
												"nig6g",
												"nigar",
												"nigg3",
												"nigg6",
												"nigga",
												"niggr",
												"niggy",
												"niqqa",
												"nugga",
												"omeko",
												"p3n15",
												"p3n1s",
												"p3ni5",
												"p3nis",
												"p3nl5",
												"p3nls",
												"pen15",
												"pen1s",
												"peni5",
												"penis",
												"penl5",
												"penls",
												"penus",
												"phuck",
												"pr1ck",
												"prick",
												"pu55y",
												"pub1c",
												"pubic",
												"puss1",
												"puss3",
												"puss5",
												"pusse",
												"pussi",
												"puzzy",
												"pvssy",
												"queef",
												"raape",
												"reipu",
												"secks",
												"teino",
												"tinpo",
												"tunbo",
												"wh0r3",
												"wh0re",
												"whor3",
												"whore",
												"Блядь",
												"いぬごろし",
												"かわらもの",
												"ちょんこう",
												"ちょんころ",
												"とるこぶろ",
												"にんぴにん",
												"イヌゴロシ",
												"カワラモノ",
												"チョンコウ",
												"チョンコロ",
												"トルコ風呂",
												"1488",
												"aids",
												"anal",
												"bamf",
												"btch",
												"csam",
												"culo",
												"dyke",
												"fckn",
												"fick",
												"fupa",
												"hure",
												"joto",
												"kike",
												"meth",
												"mong",
												"orgy",
												"paki",
												"pedo",
												"porn",
												"pube",
												"puta",
												"pute",
												"puto",
												"raip",
												"stds",
												"shat",
												"tard",
												"thot",
												"tits",
												"vank",
												"anus",
												"arse",
												"b1tc",
												"blad",
												"bung",
												"c0ck",
												"cl1t",
												"cli7",
												"clit",
												"cock",
												"coon",
												"cun7",
												"cunt",
												"cvn7",
												"cvnt",
												"cyka",
												"d1kc",
												"d4go",
												"dago",
												"dikc",
												"dong",
												"f0ck",
												"f0kc",
												"fcuk",
												"fock",
												"fokc",
												"fucc",
												"fuck",
												"fuct",
												"fvck",
												"fxck",
												"g00k",
												"g0ok",
												"go0k",
												"gook",
												"h0m0",
												"h0mo",
												"hom0",
												"homo",
												"hor3",
												"jizz",
												"k1k3",
												"kik3",
												"kun7",
												"milf",
												"n4z1",
												"naz1",
												"nazi",
												"nazl",
												"nggr",
												"ni6g",
												"r4p3",
												"rape",
												"sh17",
												"sh1t",
												"shi7",
												"shit",
												"shl7",
												"shlt",
												"slut",
												"tw47",
												"tw4t",
												"twat",
												"сука",
												"あおかん",
												"いんばい",
												"おなにー",
												"きけいじ",
												"きちがい",
												"きんたま",
												"くろんぼ",
												"しなじん",
												"たちんぼ",
												"ちょん公",
												"アオカン",
												"インバイ",
												"オナニー",
												"キケイジ",
												"キチガイ",
												"キンタマ",
												"クロンボ",
												"シナジン",
												"タチンボ",
												"トルコ嬢",
												"8=d",
												"ass",
												"cum",
												"fgt",
												"gyp",
												"nig",
												"smd",
												"std",
												"vag",
												"vog",
												"vop",
												"fag",
												"fck",
												"fku",
												"jap",
												"kkk",
												"sex",
												"ちんぽ",
												"つんぼ",
												"まんこ",
												"れいぷ",
												"オメコ",
												"チンポ",
												"ツンボ",
												"ニガー",
												"ニグロ",
												"マンコ",
												"レイプ",
												"支那人",
												"精薄者",
												"低能",
												"屠殺",
												"強姦",
												"援交",
												"精薄",
												"輪姦"};
};