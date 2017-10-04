var windriver = new require("./windriver.js");
var nonpnp = windriver.Driver("NONPNP", "\\\\.\\NONPNP\\nonpnp.log", "C:\\nonpnp.sys", "C:\\nonpnp.inf");

try{
	nonpnp.load();
	nonpnp.open();
	var retBuf = nonpnp.ioctl(windriver.TYPE.FILEIO, 0x905, windriver.METHOD.BUFFERED, windriver.FILE.ANY_ACCESS, new Buffer("This is some data sent from NodeJS"), 1000);//
	console.log("Driver JSON:", retBuf.toString());
	nonpnp.close();
	nonpnp.unload();
}
catch(e){console.log(e);}

console.log('Press any key to exit');
process.stdin.setRawMode(true);
process.stdin.resume();
process.stdin.on('data', process.exit.bind(process, 0));
