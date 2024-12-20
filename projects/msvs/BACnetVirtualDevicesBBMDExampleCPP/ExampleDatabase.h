/*
 * BACnet Virtual Devices and BBMD Example C++
 * ----------------------------------------------------------------------------
 * ExampleDatabase.h
 *
 * Data storage that contains the example data used in the BACnet Virtual 
 * Devices and BBMD Example. This data is represented by BACnet objects for this
 * example. The database will contain multiple virtual devices that have one 
 * object each.
 *
 * Created by: Alex Fontaine
 */

#ifndef __ExampleDatabase_h__
#define __ExampleDatabase_h__

#include <string>
#include <map>
#include <vector>

// Constants
#define NUMBER_OF_VIRTUAL_NETWORKS		3
#define STARTING_VIRTUAL_NETWORK		1000
#define VIRTUAL_NETWORK_OFFSET			1000
#define NUMBER_OF_DEVICES_PER_NETWORK	1
#define STARTING_DEVICE_INSTANCE		100000

// Base class for all object types. 
class ExampleDatabaseBaseObject
{
public:
	// Const
	static const uint8_t PRIORITY_ARRAY_LENGTH = 16;

	// All objects will have the following properties 
	std::string objectName;
	uint32_t instance;
};

class ExampleDatabaseAnalogInput : public ExampleDatabaseBaseObject
{
public:
	float presentValue;
	uint32_t reliability;
};

class ExampleDatabaseDevice : public ExampleDatabaseBaseObject
{
public:
	std::string description;
	uint32_t systemStatus;
};

class ExampleDatabaseNetworkPort : public ExampleDatabaseBaseObject
{
public:
	// Network Port Properties
	uint16_t BACnetIPUDPPort;
	uint8_t IPAddress[4];
	uint8_t IPAddressLength;
	uint8_t IPDefaultGateway[4];
	uint8_t IPDefaultGatewayLength;
	uint8_t IPSubnetMask[4];
	uint8_t IPSubnetMaskLength;
	std::vector<uint8_t*> IPDNSServers;
	uint8_t IPDNSServerLength;

	uint8_t BroadcastIPAddress[4];
};


class ExampleDatabase {
public:

	ExampleDatabaseDevice mainDevice;
	ExampleDatabaseNetworkPort networkPort;

	std::map<uint16_t, std::vector<ExampleDatabaseDevice> > virtualDevices;
	std::map<uint32_t, ExampleDatabaseAnalogInput> analogInputs;

	// Constructor/Deconstructor
	ExampleDatabase();
	~ExampleDatabase();

	// Set all the objects to have a default value
	void Setup();

	// Update the values as needed
	void Loop();

	// Helper functions
	void LoadNetworkPortProperties();

private:
	const std::string GetColorName();



};

#endif // __ExampleDatabase_h__