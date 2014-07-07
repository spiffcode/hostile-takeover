for (j = 0; j < 4; j++) {
	var str = "";
	for (i = 0; i < 16; i++) {
//		str += Math.floor((Math.cos(Math.PI / 4) * ((j * 16) + i) + .5)) + ", ";
		var wcMoveDist = ((j * 16) + i);
		str += Math.floor((256 + (wcMoveDist / 2)) / wcMoveDist) + ", ";
	}
	WScript.Echo(str);
}