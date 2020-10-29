function riot_testmain() {
	var coName = "riot_test_root";

	Riot_Append("riot_body", @@_CreateElement(coName + "_Main"));
}

riot_main = riot_testmain;
