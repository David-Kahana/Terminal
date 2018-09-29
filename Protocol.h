#pragma once
#include <stdint.h>

#pragma region Defines
#define START_MSG 'S'
#define END_MSG 'E'
#pragma endregion //Defines

#pragma region Enums
enum MessageTypes : uint8_t
{
	REPLAY = 0,
	CONFIG = 1,
	DATA = 2
};

enum ConfigTypes : uint8_t
{
	PIXEL = 0,
};
#pragma endregion //Enums

#pragma region Structs
struct Message
{
	const uint8_t start = START_MSG; //1 byte
	uint32_t size;                   //4 bytes  //add 7 to payload size
	MessageTypes type;               //1 byte
	uint8_t* payload;                //payload size
	const uint8_t end = START_MSG;   //1 byte
	                                 // total bytes: 7
};

struct ConfigPixlePayload
{
	const ConfigTypes type = PIXEL; //1 byte
	const uint32_t size = 16; //4 bytes
	uint8_t bpp;              //1 byte
	uint16_t width;           //2 bytes
	uint16_t height;          //2 bytes
	uint16_t pitch;           //2 bytes
	uint32_t frameSize;       //4 bytes
	                          // total bytes: 16
};
#pragma endregion //Structs