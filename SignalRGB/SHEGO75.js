export function Name() { return "SHEGO75"; }
export function Version() { return "1.1.9"; }
export function VendorId() { return 0xDEAD; }
export function ProductId() { return 0xDEAD; }
export function Publisher() { return "WhirlwindFX"; }
export function Documentation(){ return "qmk/srgbmods-qmk-firmware"; }
export function DeviceType() { return "keyboard"; }
export function Size() { return [1, 1]; }
export function DefaultPosition(){return [10, 100]; }
export function DefaultScale(){return 8.0;}
/* global
shutdownMode:readonly
shutdownColor:readonly
LightingMode:readonly
forcedColor:readonly
*/
export function ControllableParameters() {
	return [
		{"property":"shutdownMode", "group":"lighting", "label":"Shutdown Mode", "type":"combobox", "values":["SignalRGB", "Hardware"], "default":"SignalRGB"},
		{"property":"shutdownColor", "group":"lighting", "label":"Shutdown Color", "min":"0", "max":"360", "type":"color", "default":"#0011ffff"},
		{"property":"LightingMode", "group":"lighting", "label":"Lighting Mode", "type":"combobox", "values":["Canvas", "Forced"], "default":"Canvas"},
		{"property":"forcedColor", "group":"lighting", "label":"Forced Color", "min":"0", "max":"360", "type":"color", "default":"#009bde"},
	];
}

//Plugin Version: Built for Protocol V1.0.6

// LED mapping based on serpentine wiring pattern from keyboard.json
const vKeys = [
	// Row 0: LEDs 0-13 (Esc → F12) - Forward direction
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
	// Row 1: LEDs 14-28 (Delete → `) - Reverse direction  
	28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14,
	// Row 2: LEDs 29-43 (Tab → PgUp) - Forward direction
	29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43,
	// Row 3: LEDs 44-57 (PgDown → Caps) - Reverse direction  
	57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44,
	// Row 4: LEDs 58-70 (Shift → ↑) - Forward direction
	58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70,
	// Row 5: LEDs 71-80 (Ctrl → →) - Reverse direction  
	80, 79, 78, 77, 76, 75, 74, 73, 72, 71
	
	//81 keys (LEDs 0-80)
];

// Ambient strip (LEDs 81-90) - Reversed direction
const vAmbientKeys = [
	90, 89, 88, 87, 86, 85, 84, 83, 82, 81
];

// Status strip (LEDs 91-93) - Reversed direction
const vStatusKeys = [
	93, 92, 91
];


const vKeyNames = [
	//Main Board keys
	"Esc", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "Delete",
	"¬`", "!1", "@2", "#3", "$4", "%5", "^6", "&7", "*8", "(9", ")0", "_-", "+=", "Backspace", "Home",
	"Tab", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "[", "]", "#", "PgUp",
	"Caps Lock", "A", "S", "D", "F", "G", "H", "J", "K", "L", ";", "'", "Enter", "PgDown",
	"Left Shift", "Z", "X", "C", "V", "B", "N", "M", "<,", ">.", "?", "Right Shift", "↑",
	"Left Ctrl", "Win", "Left Alt", "Space", "Right Alt", "Fn", "Right Ctrl", "←", "↓", "→"
];

// Names for ambient strip
const vAmbientKeyNames = [
	"Ambient 1", "Ambient 2", "Ambient 3", "Ambient 4", "Ambient 5",
	"Ambient 6", "Ambient 7", "Ambient 8", "Ambient 9", "Ambient 10",
];

//Status Strip keys (3 LEDs)
const vStatusKeyNames = [
	"Status 1", "Status 2", "Status 3"
];

const vKeyPositions = [
	[0,0], [1,0], [2,0], [3,0], [4,0], [5,0], [6,0], [7,0], [8,0], [9,0],  [10,0], [11,0], [12,0], [13,0],
	[0,1], [1,1], [2,1], [3,1], [4,1], [5,1], [6,1], [7,1], [8,1], [9,1],  [10,1], [11,1], [12,1], [13,1], [14,1],
	[0,2], [1,2], [2,2], [3,2], [4,2], [5,2], [6,2], [7,2], [8,2], [9,2],  [10,2], [11,2], [12,2], [13,2], [14,2],
	[0,3], [1,3], [2,3], [3,3], [4,3], [5,3], [6,3], [7,3], [8,3], [9,3],  [10,3], [11,3], [12,3], 		   [14,3],
	[0,4], [1,4], [2,4], [3,4], [4,4], [5,4], [6,4], [7,4], [8,4], [9,4],  [10,4], [11,4], 		   [13,4],
	[0,5], [1,5], [2,5], 					  [6,5], 			   [9,5],  [10,5], [11,5], [12,5], [13,5], [14,5],
];

// Positions for ambient strip (horizontal strip)
const vAmbientKeyPositions = [
	[0, 0], [1, 0], [2, 0], [3, 0], [4, 0], [5, 0], [6, 0], [7, 0], [8, 0], [9, 0]
];

const vStatusKeyPositions = [
	[0, 0], [1, 0], [2, 0]
];

let LEDCount = 94; //94 LEDs on this keyboard
let IsViaKeyboard = false;
const MainlineQMKFirmware = 1;
const VIAFirmware = 2;
const PluginProtocolVersion = "1.0.6";

export function LedNames() {
	return [];
};

export function LedPositions() {
	return [];
}

export function LedGroups() {
	return [
		{ name: "Keys",    leds: vKeys },
		{ name: "Ambient", leds: vAmbientKeys },
		{ name: "Status", leds: vStatusKeys }
	];
}

export function vKeysArrayCount() {
	const totalKeys = vKeys.length + vAmbientKeys.length + vStatusKeys.length;
	const totalNames = vKeyNames.length + vAmbientKeyNames.length + vStatusKeyNames.length;
	const totalPositions = vKeyPositions.length + vAmbientKeyPositions.length + vStatusKeyPositions.length;

	device.log('Main Keys: ' + vKeys.length);
	device.log('Ambient Keys: ' + vAmbientKeys.length);
	device.log('Status Keys: ' + vStatusKeys.length);
	device.log('Total Keys: ' + totalKeys);
	device.log('Total Names: ' + totalNames);
	device.log('Total Positions: ' + totalPositions);
}


export function Initialize() {
	requestFirmwareType();
	requestQMKVersion();
	requestSignalRGBProtocolVersion();
	requestUniqueIdentifier();
	requestTotalLeds();

	// Create subdevices for independent zones
	device.createSubdevice("MainKeys");
	device.setSubdeviceName("MainKeys", "SHEGO Keys");
	device.setSubdeviceSize("MainKeys", 15, 6);
	device.setSubdeviceLeds("MainKeys", vKeyNames, vKeyPositions);

	device.createSubdevice("AmbientStrip");
	device.setSubdeviceName("AmbientStrip", "SHEGO Ambient");
	device.setSubdeviceSize("AmbientStrip", 10, 1);
	device.setSubdeviceLeds("AmbientStrip", vAmbientKeyNames, vAmbientKeyPositions);

	device.createSubdevice("StatusStrip");
	device.setSubdeviceName("StatusStrip", "SHEGO Status");
	device.setSubdeviceSize("StatusStrip", 3, 1);
	device.setSubdeviceLeds("StatusStrip", vStatusKeyNames, vStatusKeyPositions);

	effectEnable();

}

export function Render() {
	sendColors();
}

export function Shutdown(SystemSuspending) {

	if(SystemSuspending) {
		sendColors("#91000aff"); // Go Dark on System Sleep/Shutdown
	} else {
		if (shutdownMode === "SignalRGB") {
			sendColors(shutdownColor);
		} else {
			effectDisable();
		}
	}

	vKeysArrayCount(); // For debugging array counts

}

function commandHandler() {
	const readCounts = [];

	do {
		const returnpacket = device.read([0x00], 32, 10);
		processCommands(returnpacket);

		readCounts.push(device.getLastReadSize());

		// Extra Read to throw away empty packets from Via
		// Via always sends a second packet with the same Command Id.
		if(IsViaKeyboard) {
			device.read([0x00], 32, 10);
		}
	}
	while(device.getLastReadSize() > 0);

}

function processCommands(data) {
	switch(data[1]) {
	case 0x21:
		returnQMKVersion(data);
		break;
	case 0x22:
		returnSignalRGBProtocolVersion(data);
		break;
	case 0x23:
		returnUniqueIdentifier(data);
		break;
	case 0x24:
		sendColors();
		break;
	case 0x27:
		returnTotalLeds(data);
		break;
	case 0x28:
		returnFirmwareType(data);
		break;
	}
}

function requestQMKVersion() //Check the version of QMK Firmware that the keyboard is running
{
	device.write([0x00, 0x21], 32);
	device.pause(30);
	commandHandler();
}

function returnQMKVersion(data) {
	const QMKVersionByte1 = data[2];
	const QMKVersionByte2 = data[3];
	const QMKVersionByte3 = data[4];
	device.log("QMK Version: " + QMKVersionByte1 + "." + QMKVersionByte2 + "." + QMKVersionByte3);
	device.log("QMK SRGB Plugin Version: "+ Version());
	device.pause(30);
}

function requestSignalRGBProtocolVersion() //Grab the version of the SignalRGB Protocol the keyboard is running
{
	device.write([0x00, 0x22], 32);
	device.pause(30);
	commandHandler();
}

function returnSignalRGBProtocolVersion(data) {
	const ProtocolVersionByte1 = data[2];
	const ProtocolVersionByte2 = data[3];
	const ProtocolVersionByte3 = data[4];

	const SignalRGBProtocolVersion = ProtocolVersionByte1 + "." + ProtocolVersionByte2 + "." + ProtocolVersionByte3;
	device.log(`SignalRGB Protocol Version: ${SignalRGBProtocolVersion}`);


	if(PluginProtocolVersion !== SignalRGBProtocolVersion) {
		device.notify("Unsupported Protocol Version", `This plugin is intended for SignalRGB Protocol version ${PluginProtocolVersion}. This device is version: ${SignalRGBProtocolVersion}`, 2, "Documentation");
	}

	device.pause(30);
}

function requestUniqueIdentifier() //Grab the unique identifier for this keyboard model
{
	if(device.write([0x00, 0x23], 32) === -1) {
		device.notify("Unsupported Firmware", "This device is not running SignalRGB-compatible firmware. Click the Documentation button to learn more.", 3, "Documentation");
	}

	device.pause(30);
	commandHandler();
}


function returnUniqueIdentifier(data) {
	const UniqueIdentifierByte1 = data[2];
	const UniqueIdentifierByte2 = data[3];
	const UniqueIdentifierByte3 = data[4];

	if(!(UniqueIdentifierByte1 === 0 && UniqueIdentifierByte2 === 0 && UniqueIdentifierByte3 === 0)) {
		device.log("Unique Device Identifier: " + UniqueIdentifierByte1 + UniqueIdentifierByte2 + UniqueIdentifierByte3);
	}

	device.pause(30);
}

function requestTotalLeds() //Calculate total number of LEDs
{
	device.write([0x00, 0x27], 32);
	device.pause(30);
	commandHandler();
}

function returnTotalLeds(data) {
	LEDCount = data[2];
	device.log("Device Total LED Count: " + LEDCount);
	device.pause(30);
}

function requestFirmwareType() {
	device.write([0x00, 0x28], 32);
	device.pause(30);
	commandHandler();
}

function returnFirmwareType(data) {
	const FirmwareTypeByte = data[2];

	if(!(FirmwareTypeByte === MainlineQMKFirmware || FirmwareTypeByte === VIAFirmware)) {
		device.notify("Unsupported Firmware", "Click the Documentation button to learn more.", 3, "Documentation");
	}

	if(FirmwareTypeByte === MainlineQMKFirmware) {
		IsViaKeyboard = false;
		device.log("Firmware Type: Mainline");
	}

	if(FirmwareTypeByte === VIAFirmware) {
		IsViaKeyboard = true;
		device.log("Firmware Type: VIA");
	}

	device.pause(30);
}

function effectEnable() //Enable the SignalRGB Effect Mode
{
	device.write([0x00, 0x25], 32);
	device.pause(30);
}

function effectDisable() //Revert to Hardware Mode
{
	device.write([0x00, 0x26], 32);
	device.pause(30);
}

function createSolidColorArray(color) {
	const rgbdata = new Array(vKeys.length * 3).fill(0);

	for(let iIdx = 0; iIdx < vKeys.length; iIdx++) {
		const iLedIdx = vKeys[iIdx] * 3;
		rgbdata[iLedIdx] = color[0];
		rgbdata[iLedIdx+1] = color[1];
		rgbdata[iLedIdx+2] = color[2];
	}

	return rgbdata;
}

function grabColors(overrideColor) {
	if(overrideColor) {
		return createSolidColorArray(hexToRgb(overrideColor));
	} else if (LightingMode === "Forced") {
		return createSolidColorArray(hexToRgb(forcedColor));
	}

	const rgbdata = new Array(94 * 3).fill(0);

	// Handle main keys (using MainKeys subdevice)
	for(let iIdx = 0; iIdx < vKeys.length; iIdx++) {
		const iPxX = vKeyPositions[iIdx][0];
		const iPxY = vKeyPositions[iIdx][1];
		const color = device.subdeviceColor("MainKeys", iPxX, iPxY);

		const iLedIdx = vKeys[iIdx] * 3;
		rgbdata[iLedIdx] = color[0];
		rgbdata[iLedIdx+1] = color[1];
		rgbdata[iLedIdx+2] = color[2];
	}

	// Handle ambient strip (using AmbientStrip subdevice)
	for(let iIdx = 0; iIdx < vAmbientKeys.length; iIdx++) {
		const iPxX = vAmbientKeyPositions[iIdx][0];
		const iPxY = vAmbientKeyPositions[iIdx][1];
		const color = device.subdeviceColor("AmbientStrip", iPxX, iPxY);

		const iLedIdx = vAmbientKeys[iIdx] * 3;
		rgbdata[iLedIdx] = color[0];
		rgbdata[iLedIdx+1] = color[1];
		rgbdata[iLedIdx+2] = color[2];
	}

	// Handle status strip (using StatusStrip subdevice)
	for(let iIdx = 0; iIdx < vStatusKeys.length; iIdx++) {
		const iPxX = vStatusKeyPositions[iIdx][0];
		const iPxY = vStatusKeyPositions[iIdx][1];
		const color = device.subdeviceColor("StatusStrip", iPxX, iPxY);

		const iLedIdx = vStatusKeys[iIdx] * 3;
		rgbdata[iLedIdx] = color[0];
		rgbdata[iLedIdx+1] = color[1];
		rgbdata[iLedIdx+2] = color[2];
	}

	return rgbdata;
}

function sendColors(overrideColor) {
	const rgbdata = grabColors(overrideColor);

	const LedsPerPacket = 9;
	let BytesSent = 0;
	let BytesLeft = rgbdata.length;

	while(BytesLeft > 0) {
		const BytesToSend = Math.min(LedsPerPacket * 3, BytesLeft);
		StreamLightingData(Math.floor(BytesSent / 3), rgbdata.splice(0, BytesToSend));

		BytesLeft -= BytesToSend;
		BytesSent += BytesToSend;
	}
}

function StreamLightingData(StartLedIdx, RGBData) {
	const packet = [0x00, 0x24, StartLedIdx, Math.floor(RGBData.length / 3)].concat(RGBData);
	device.write(packet, 33);
}

function hexToRgb(hex) {
	const result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex);
	const colors = [];
	colors[0] = parseInt(result[1], 16);
	colors[1] = parseInt(result[2], 16);
	colors[2] = parseInt(result[3], 16);

	return colors;
}

export function Validate(endpoint) {
	return endpoint.interface === 1;
}

export function Image() {
	return "";
}