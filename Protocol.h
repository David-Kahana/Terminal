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

enum Replays : uint8_t
{
	READY = 0,
	ACK = 1,
	ERROR = 2,
	NO_REPLAY = 255
};
enum ConfigTypes : uint8_t
{
	PIXEL = 0,
};
#pragma endregion //Enums

#pragma region Structs
#pragma pack(push) 
#pragma pack(1) 
struct MessageReplay
{
	uint8_t start;        //1 byte
	uint32_t size;        //4 bytes  
	MessageTypes msgType; //1 byte   MessageTypes::REPLAY
	Replays replay;       //1 byte
	uint8_t end;	      //1 byte
	                      // total bytes: 8
};

#pragma pack(1)   
struct MessageConfigPixle 
{
	uint8_t start;         //1 byte
	uint32_t size;         //4 bytes  
	MessageTypes msgType;  //1 byte   MessageTypes::CONFIG
	ConfigTypes cfgType;   //1 byte   ConfigTypes::PIXEL
	uint8_t bpp;           //1 byte
	uint16_t width;        //2 bytes
	uint16_t height;       //2 bytes
	uint16_t pitch;        //2 bytes
	uint32_t frameSize;    //4 bytes
	uint8_t end;           //1 byte
						   // total bytes: 19
};

#pragma pack(1) 
struct MessageFrame
{
	uint8_t start;         //1 byte
	uint32_t size;         //4 bytes  
	MessageTypes msgType;  //1 byte   MessageTypes::DATA
	uint8_t* data;         //x bytes 
	uint8_t end;           //1 byte
						   // total bytes: x + 7
};
#pragma pack(pop) 
#pragma endregion //Structs