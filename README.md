# BACnetVirtualDevicesBBMDExampleCPP

This example shows how to implement BACnet Virtual Devices with BBMD functionality using the CAS BACnet Stack.

## CAS BACnet Stack

This example was written using the CAS BACnet Stack version 5.1.6

Please contact Chipkin Automation Systems to purchase the CAS BACnet Stack

More information about the CAS BACnet Stack can be found here: [CAS BACnet Stack](https://store.chipkin.com/services/stacks/bacnet-stack)

## Implementation Notes

The following sections provided code-snippets from the example with instructions on how to implement each portion.

### 1. Loading the CAS BACnet Stack API, Connecting to UDP Resource, and Registering Callbacks

#### 1.1 Load the CAS BACnet Stack functions

Call the CASBACnetStackAdapter function to LoadBACnetFunctions. This will setup all the CAS BACnet Stack API functions to be linked and ready to use whether binding them from a static or dynamic library or linking directly to source code.  Printing out the version number of the CAS BACnet Stack is a good indicator that the library functions were properly linked.  Resolve any linking errors before proceeding.

```cpp
// Lines 80-88 in BACnetVirtualDevicesBBMDExcampleCPP
// 1. Load the CAS BACnet stack functions
// ---------------------------------------------------------------------------
std::cout << "FYI: Loading CAS BACnet Stack functions... ";
if (!LoadBACnetFunctions()) {
	std::cerr << "Failed to load the functions from the DLL" << std::endl;
	return 0;
}
std::cout << "OK" << std::endl;
std::cout << "FYI: CAS BACnet Stack version: " << fpGetAPIMajorVersion() << "." << fpGetAPIMinorVersion() << "." << fpGetAPIPatchVersion() << "." << fpGetAPIBuildVersion() << std::endl;
```
#### 1.2 Connect the UDP resource to the BACnet Port

This example uses a simple implementation of a UDP resource, SimpleUDP. Connect to the UDP resource which binds the port.  This example supports the NetworkPort object, so the port that is specified is stored in the NetworkPort object, which is set to 47808 by default.

``` cpp
// Lines 90-99 in BACnetVirtualDevicesBBMDExcampleCPP
// 2. Connect the UDP resource to the BACnet Port
// ---------------------------------------------------------------------------
std::cout << "FYI: Connecting UDP Resource to port=[" << g_database.networkPort.BACnetIPUDPPort << "]... ";
if (!g_udp.Connect(g_database.networkPort.BACnetIPUDPPort)) {
	std::cerr << "Failed to connect to UDP Resource" << std::endl;
	std::cerr << "Press any key to exit the application..." << std::endl;
	(void)getchar();
	return -1;
}
std::cout << "OK, Connected to port" << std::endl;
```

#### 1.3 Setup the Callbacks

Register any required callbacks with the CAS BACnet Stack.  Review the implemented callbacks in the example.  This example uses a minimal amount of objects and properties so only a few of the callbacks are implemented.

``` cpp
// Lines 101-117 in BACnetVirtualDevicesBBMDExcampleCPP
// 3. Setup the callbacks
// ---------------------------------------------------------------------------
std::cout << "FYI: Registering the callback Functions with the CAS BACnet Stack" << std::endl;

// Message Callback Functions
fpRegisterCallbackReceiveMessage(CallbackReceiveMessage);
fpRegisterCallbackSendMessage(CallbackSendMessage);

// System Time Callback Functions
fpRegisterCallbackGetSystemTime(CallbackGetSystemTime);

// Get Property Callback Functions
fpRegisterCallbackGetPropertyCharacterString(CallbackGetPropertyCharString);
fpRegisterCallbackGetPropertyEnumerated(CallbackGetPropertyEnum);
fpRegisterCallbackGetPropertyOctetString(CallbackGetPropertyOctetString);
fpRegisterCallbackGetPropertyReal(CallbackGetPropertyReal);
fpRegisterCallbackGetPropertyUnsignedInteger(CallbackGetPropertyUInt);
```

### 2. Setting up the Main Device

The Main Device represents the main interface to the BACnet IP network in this example.  It acts as the virtual router and will eventually be setup to be a BBMD later in the example.  In this example, the Main Device has only a NetworkPort object.

#### 2.1 Create the Main Device and enable any required BACnet Services

Create the Main Device using the fpAddDevice API function.  Then enable the BACnet Services that this device supports.  In this example, the Main Device will only support a minimal amount of services.

```cpp
// Lines 119-154 in BACnetVirtualDevicesBBMDExcampleCPP
// 4. Setup the BACnet device
// ---------------------------------------------------------------------------

std::cout << "Setting up main server device. device.instance=[" << g_database.mainDevice.instance << "]" << std::endl;

// Create the Main Device
if (!fpAddDevice(g_database.mainDevice.instance)) {
	std::cerr << "Failed to add Device." << std::endl;
	return false;
}
std::cout << "Created Device." << std::endl;

// Enable the services that this device supports
// Some services are mandatory for BACnet devices and are already enabled.
// These are: Read Property, Who Is, Who Has
//
// Any other services need to be enabled as below.
std::cout << "Enabling IAm... ";
if (!fpSetServiceEnabled(g_database.mainDevice.instance, ExampleConstants::SERVICE_I_AM, true)) {
	std::cerr << "Failed to enable the IAm" << std::endl;
	return -1;
}
std::cout << "OK" << std::endl;

std::cout << "Enabling ReadPropertyMultiple... ";
if (!fpSetServiceEnabled(g_database.mainDevice.instance, ExampleConstants::SERVICE_READ_PROPERTY_MULTIPLE, true)) {
	std::cerr << "Failed to enable the ReadPropertyMultiple" << std::endl;
	return -1;
}
std::cout << "OK" << std::endl;

// Enable Optional Device Properties
if (!fpSetPropertyEnabled(g_database.mainDevice.instance, ExampleConstants::OBJECT_TYPE_DEVICE, g_database.mainDevice.instance, ExampleConstants::PROPERTY_IDENTIFIER_DESCRIPTION, true)) {
	std::cerr << "Failed to enable the description property for the Main Device" << std::endl;
	return false;
}
```

#### 2.2 Add Main Device Objects

As mentioned above, in this example the Main Device only contains the NetworkPort object.  This NetworkPort object contains the information that is used by the Main Device to communicate on the BACnet IP network.  All properties pertaining to the NetworkPort object can be seen in the various CallbackGetProperty{{datatype}} functions.  For example: IP_Address property is in the CallbackGetPropertyOctetString.

```cpp
// Lines 156-165 in BACnetVirtualDevicesBBMDExcampleCPP
// Add Main Device Objects
// ---------------------------------------

// Add the Network Port Object
std::cout << "Adding NetworkPort. networkPort.instance=[" << g_database.networkPort.instance << "]... ";
if (!fpAddNetworkPortObject(g_database.mainDevice.instance, g_database.networkPort.instance, ExampleConstants::NETWORK_TYPE_IPV4, ExampleConstants::PROTOCOL_LEVEL_BACNET_APPLICATION, ExampleConstants::NETWORK_PORT_LOWEST_PROTOCOL_LAYER)) {
	std::cerr << "Failed to add NetworkPort" << std::endl;
	return -1;
}
std::cout << "OK" << std::endl;
```

### 3. Setting up Virtual Devices

Add the Virtual Devices and any objects and services supported by the Virtual Devices.  In this example, each Virtual Device contains just one AnalogInput Object.

```cpp
// Lines 167-213 in BACnetVirtualDevicesBBMDExcampleCPP
// Add Virtual Devices and Objects
std::cout << "Adding Virtual Devices and Objects..." << std::endl;
std::map<uint16_t, std::vector<ExampleDatabaseDevice> >::iterator it;
for (it = g_database.virtualDevices.begin(); it != g_database.virtualDevices.end(); ++it) {
	// Add the Virtual network
	if (!fpAddVirtualNetwork(g_database.mainDevice.instance, it->first, it->first)) {
		std::cerr << "Failed to add virtual network " << it->first << std::endl;
		return -1;
	}

	std::vector<ExampleDatabaseDevice>::iterator devIt;
	for (devIt = it->second.begin(); devIt != it->second.end(); ++devIt) {
		// Add the Virtual Device
		std::cout << "Adding Virtual Device. device.instance=[" << devIt->instance << "] to network=[" << it->first << "]...";
		if (!fpAddDeviceToVirtualNetwork(devIt->instance, it->first)) {
			std::cerr << "Failed to add Virtual Device" << std::endl;
			return -1;
		}
		std::cout << "OK" << std::endl;

		// Enable IAm
		std::cout << "Enabling IAm... ";
		if (!fpSetServiceEnabled(devIt->instance, ExampleConstants::SERVICE_I_AM, true)) {
			std::cerr << "Failed to enable IAm" << std::endl;
			return -1;
		}
		std::cout << "OK" << std::endl;

		// Enable Read Property Multiple
		if (!fpSetServiceEnabled(devIt->instance, ExampleConstants::SERVICE_READ_PROPERTY_MULTIPLE, true)) {
			std::cerr << "Failed to enable ReadPropertyMultiple" << std::endl;
			return -1;
		}
		std::cout << "OK" << std::endl;

		// Add the Analog Input to the Virtual Device
		std::cout << "Adding Analog Input to Virtual Device. device.instance=[" << devIt->instance << "], analogInput.instance=[" << g_database.analogInputs[devIt->instance].instance << "]...";
		if (!fpAddObject(devIt->instance, ExampleConstants::OBJECT_TYPE_ANALOG_INPUT, g_database.analogInputs[devIt->instance].instance)) {
			std::cerr << "Failed to add AnalogInput" << std::endl;
			return -1;
		}
		std::cout << "OK" << std::endl;

		// Enable Reliability property 
		fpSetPropertyByObjectTypeEnabled(devIt->instance, ExampleConstants::OBJECT_TYPE_ANALOG_INPUT, ExampleConstants::PROPERTY_IDENTIFIER_RELIABILITY, true);
	}
}
```

### 4. Setting up the BBMD

Finally, setup the BBMD functionality.

#### 4.1 Update the Main Device NetworkPort Object to enable the required BBMD properties

BBMD requires specific properties to be enabled on the Main Device's Network Port object.  These properties are BBMD_Accept_FD_Registrations, BBMD_Broadcast_Distribution_Table, and BBMD_Foreign_Device_Table

```cpp
// Lines 218-238 in BACnetVirtualDevicesBBMDExcampleCPP
// Add BBMD specific network port properties to the main device such as accept registrations, FDT, and BDT
std::cout << "Enabling bbmd_accept_fd_registrations property to the Main Network Port Object networkPort.instance=[" << g_database.networkPort.instance << "]... ";
if (!fpSetPropertyEnabled(g_database.mainDevice.instance, ExampleConstants::OBJECT_TYPE_NETWORK_PORT, g_database.networkPort.instance, ExampleConstants::PROPERTY_IDENTIFIER_BBMD_ACCEPT_FD_REGISTRATIONS, true)) {
	std::cerr << "Failed to enable the bbmd_accept_fd_registrations property for the Main Network Port Object" << std::endl;
	return -1;
}
std::cout << "OK" << std::endl;

std::cout << "Enabling bbmd_broadcast_distribution_table property to the Main Network Port Object networkPort.instance=[" << g_database.networkPort.instance << "]... ";
if (!fpSetPropertyEnabled(g_database.mainDevice.instance, ExampleConstants::OBJECT_TYPE_NETWORK_PORT, g_database.networkPort.instance, ExampleConstants::PROPERTY_IDENTIFIER_BBMD_BROADCAST_DISTRIBUTION_TABLE, true)) {
	std::cerr << "Failed to enable the bbmd_broadcast_distribution_table property for the Main Network Port Object" << std::endl;
	return -1;
}
std::cout << "OK" << std::endl;

std::cout << "Enabling bbmd_foreign_device_table property to the Main Network Port Object networkPort.instance=[" << g_database.networkPort.instance << "]... ";
if (!fpSetPropertyEnabled(g_database.mainDevice.instance, ExampleConstants::OBJECT_TYPE_NETWORK_PORT, g_database.networkPort.instance, ExampleConstants::PROPERTY_IDENTIFIER_BBMD_FOREIGN_DEVICE_TABLE, true)) {
	std::cerr << "Failed to enable the bbmd_foreign_device_table property for the Main Network Port Object" << std::endl;
	return -1;
}
std::cout << "OK" << std::endl;
```

#### 4.2 Add the Main Device to the BDT Table

The first entry in the BBMD's BDT (Broadcast Distribution Table) must be itself. 

```cpp
// Lines 240-251 in BACnetVirtualDevicesBBMDExcampleCPP
// Add Main Device to the BDT Table (the first entry must be this device)
std::cout << "Adding Main Device to BDT Table... ";
uint8_t bdtAddress[6];
memcpy(bdtAddress, g_database.networkPort.IPAddress, 4);
bdtAddress[4] = g_database.networkPort.BACnetIPUDPPort / 256;
bdtAddress[5] = g_database.networkPort.BACnetIPUDPPort % 256;
uint8_t bdtMask[4] = { 255, 255, 255, 255 };
if (!fpAddBDTEntry(bdtAddress, 6, bdtMask, 4)) {
	std::cerr << "Failed to add the Main Device to the BDT Table" << std::endl;
	return -1;
}
std::cout << "OK" << std::endl;
```

#### 4.3 Add a remote BBMD to the BDT Table

Then add additional BBMDs to the BDT Table.  This example uses a hardcoded BBMD IP Address, but production implementations should allow for this table to be configurable, either via BACnet or from a GUI.

```cpp
// Lines 253-261 in BACnetVirtualDevicesBBMDExcampleCPP
// Add BBMD to the BDT Table. In this example, this is a hard-coded value for testing.
// In an actual implementation, this should be configurable from a settings file / gui
// or users can use the BACnet BVLL function WriteBroadcastDistributionTable to update the BDT.
std::cout << "Adding BBMD to BDT Table... ";
if (!fpAddBDTEntry(g_bbmdAddress, 6, bdtMask, 4)) {
	std::cerr << "Failed to add the BBMD to the BDT Table" << std::endl;
	return -1;
}
std::cout << "OK" << std::endl;
```

#### 4.4 Enable BBMD

Finally, call fpSetBBMD to turn on the BBMD Functionality.  This function will return with a false value if there was an error with any of the BBMD setup. Review the callstack and resolve any errors.

```cpp
// Lines 263-269 in BACnetVirtualDevicesBBMDExcampleCPP
// Enable BBMD
std::cout << "Enabling BBMD... ";
if (!fpSetBBMD(g_database.mainDevice.instance, g_database.networkPort.instance)) {
	std::cerr << "Failed to enable the BBMD" << std::endl;
	return -1;
}
std::cout << "OK" << std::endl;
```

### 5. Send I-Ams for the Main Device and the Virtual Devices

As part of the boot-up process, once all the devices have been setup, send an I-Am for the Main device and for each Virtual Device by callin ghte fpSendIam API function.

```cpp
// Lines 272-296 in BACnetVirtualDevicesBBMDExcampleCPP
// 5. Send I-Am of this device
// ---------------------------------------------------------------------------
// To be a good citizen on a BACnet network. We should announce  ourselves when we start up. 
std::cout << "FYI: Sending I-AM broadcast" << std::endl;
uint8_t connectionString[6]; //= { 0xC0, 0xA8, 0x01, 0xFF, 0xBA, 0xC0 };
memcpy(connectionString, g_database.networkPort.BroadcastIPAddress, 4);
connectionString[4] = g_database.networkPort.BACnetIPUDPPort / 256;
connectionString[5] = g_database.networkPort.BACnetIPUDPPort % 256;

// Send IAm for the Main Device
if (!fpSendIAm(g_database.mainDevice.instance, connectionString, 6, ExampleConstants::NETWORK_TYPE_IP, true, 65535, NULL, 0)) {
	std::cerr << "Unable to send IAm broadcast for mainDevice.instance=[" << g_database.mainDevice.instance << "]" << std::endl;
	return false;
}

// Send IAm for each virtual device
for (it = g_database.virtualDevices.begin(); it != g_database.virtualDevices.end(); ++it) {
	std::vector<ExampleDatabaseDevice>::iterator devIt;
	for (devIt = it->second.begin(); devIt != it->second.end(); ++devIt) {
		if (!fpSendIAm(devIt->instance, connectionString, 6, ExampleConstants::NETWORK_TYPE_IP, true, 65535, NULL, 0)) {
			std::cerr << "Unable to send IAm broadcast for virtualDevice.instance=[" << devIt->instance << "]" << std::endl;
			return false;
		}
	}
}
```

### 6. Send I-Am-Router-To-Network from the Main Device as a Virtual Router

Next announce to the network that the Main Device is also a Virtual Router to each of the Virtual Networks that have been setup.

```cpp
// Lines 298-302 in BACnetVirtualDevicesBBMDExcampleCPP
// Send IAmRouterToNetwork
if (!fpSendIAmRouterToNetwork(connectionString, 6, ExampleConstants::NETWORK_TYPE_IP, true, 65535, NULL, 0)) {
	std::cerr << "Unable to send IAmRouterToNetwork broadcast" << std::endl;
	return false;
}
```

### 7. Enter the Main Loop

Start the Main Loop calling the fpTick API function to process and handle incoming messages.

```cpp
// Lines 304-325 in BACnetVirtualDevicesBBMDExcampleCPP
// 6. Start the main loop
// ---------------------------------------------------------------------------
std::cout << "FYI: Entering main loop..." << std::endl;
for (;;) {
	// Call the DLLs loop function which checks for messages and processes them.
	fpTick();

	// Handle any user input.
	// Note: User input in this example is used for the following:
	//		h - Display options
	//		q - Quit
	if (!DoUserInput()) {
		// User press 'q' to quit the example application.
		break;
	}

	// Update values in the example database
	g_database.Loop();

	// Call Sleep to give some time back to the system
	Sleep(0); // Windows 
}
```
