var windriver = require(__dirname+'/build/Release/windriver');

windriver.FILE = {
	FILE_ANY_ACCESS: 0,
	FILE_SPECIAL_ACCESS: 0,
	FILE_READ_ACCESS: 0x0001,
	FILE_WRITE_ACCESS: 0x0002
};

windriver.METHOD = {
	BUFFERED: 0,
	IN_DIRECT: 1,
	OUT_DIRECT: 2,
	NEITHER: 3
};

windriver.TYPE = {
	FILEIO: 40001
};

module.exports = windriver;