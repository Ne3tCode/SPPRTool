/**
 * MIT License
 *
 * Copyright (c) 2017 Nephrite
 */

#include <string.h> // memmove

unsigned int ReadProtoNumber(const unsigned char *pubInput, unsigned int cubAvail, void *pubDest, unsigned int cubSize, bool bFixed = false)
{
	if (bFixed)
	{
		switch (cubSize)
		{
			case 1:
				*(unsigned char *)pubDest = *pubInput;
				break;
			case 2:
				*(unsigned short *)pubDest = *(unsigned short *)pubInput;
				break;
			case 4:
				*(unsigned int *)pubDest = *(unsigned int *)pubInput;
				break;
			case 8:
				*(unsigned long long *)pubDest = *(unsigned long long *)pubInput;
				break;
			default:
				return 0;
		}

		return cubSize;
	}

	union
	{
		char result8;
		short result16;
		int result32;
		long long result64 = 0;
	};

	unsigned int bytes_read = 0;

	while (bytes_read < cubAvail)
	{
		switch (cubSize)
		{
			case 1:
			case 2:
			case 3:
			case 4:
				result32 |= (pubInput[bytes_read] & 0x7F) << (bytes_read * 7);
				break;
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
				result64 |= (long long)(pubInput[bytes_read] & 0x7F) << (bytes_read * 7);
				break;
			default:
				break;
		}
		if ((pubInput[bytes_read++] & 0x80) == 0) {
			break;
		}
	}

	if (pubDest)
	{
		switch (cubSize)
		{
			case 1:
				*(char *)pubDest = result8;
				break;
			case 2:
				*(short *)pubDest = result16;
				break;
			case 4:
				*(int *)pubDest = result32;
				break;
			case 8:
				*(long long *)pubDest = result64;
				break;
			default:
				if (result64 >> 32)
					*(long long *)pubDest = result64;
				else
					*(int *)pubDest = result32;
		}
	}

	return bytes_read;
}

unsigned int ReadProtoLengthDelimited(const unsigned char *pubInput, unsigned int cubAvail, void *pubDest = NULL, unsigned int *cubSize = NULL)
{
	unsigned int bytes_read = 0;
	unsigned int length = 0;

	bytes_read = ReadProtoNumber(pubInput, cubAvail, &length, 4);
	cubAvail -= bytes_read;

	unsigned int max_size_to_read = (length <= cubAvail) ? length : cubAvail;
	unsigned int size_to_copy = max_size_to_read;

	if (cubSize)
	{
		size_to_copy = (*cubSize == 0 || max_size_to_read <= *cubSize) ? max_size_to_read : *cubSize;
		*cubSize = length;
	}
	if (pubDest && size_to_copy)
	{
		memmove(pubDest, pubInput + bytes_read, size_to_copy);
	}

	return bytes_read + max_size_to_read;
}

unsigned int ReadProtoField(const unsigned char *pubInput, unsigned int cubAvail, unsigned int *punTag, unsigned int *punWireType, void *pubDest = NULL, unsigned int *cubSize = NULL)
{
	unsigned int bytes_read = 0;
	unsigned int data_size = 0;
	unsigned int tag = 0;
	unsigned int wire_type = 0;

	bytes_read = ReadProtoNumber(pubInput, cubAvail, &tag, 4);
	cubAvail -= bytes_read;
	wire_type = tag & 0x07;
	tag = tag >> 3;

	switch (wire_type)
	{
		case 0: // Varint - int32, int64, uint32, uint64, sint32, sint64, bool, enum
			data_size = ReadProtoNumber(pubInput + bytes_read, cubAvail, pubDest, cubSize ? *cubSize : 0, false);
			bytes_read += data_size;
			break;
		case 1: // 64-bit - fixed64, sfixed64, double
			if (pubDest)
				ReadProtoNumber(pubInput + bytes_read, cubAvail, pubDest, 8, true);
			data_size = 8;
			bytes_read += 8;
			break;
		case 5: // 32-bit - fixed32, sfixed32, float
			if (pubDest)
				ReadProtoNumber(pubInput + bytes_read, cubAvail, pubDest, 4, true);
			data_size = 4;
			bytes_read += 4;
			break;
		case 2: // Length-delimited - string, bytes, embedded messages, packed repeated fields
			if (cubSize)
				data_size = *cubSize;
			bytes_read += ReadProtoLengthDelimited(pubInput + bytes_read, cubAvail, pubDest, &data_size);
			break;
		default:
			break;
	}

	if (punTag)
		*punTag = tag;
	if (punWireType)
		*punWireType = wire_type;
	if (cubSize)
		*cubSize = data_size;

	return bytes_read;
}

bool GetFieldDataByTag(unsigned int nTag, const unsigned char *pubInput, unsigned int cubAvail, void *pubDest, unsigned int *cubSize, unsigned int *cubTotalRead = NULL)
{
	unsigned int bytes_read = 0;
	unsigned int bytes_read_field = 0;
	unsigned int data_size;
	unsigned int tag = 0;
	unsigned int wire_type = 0;
	bool result = false;

	while (cubAvail > 0)
	{
		data_size = *cubSize;
		bytes_read_field = ReadProtoField(pubInput + bytes_read, cubAvail, &tag, &wire_type, NULL, &data_size);

		if ((wire_type != 0 && wire_type != 1 && wire_type != 2 && wire_type != 5) || (data_size > cubAvail))
		{
			break;
		}

		if (tag == nTag)
		{
			bytes_read += ReadProtoField(pubInput + bytes_read, cubAvail, NULL, NULL, pubDest, cubSize);
			if (cubTotalRead)
				*cubTotalRead = bytes_read;
			result = true;
			break;
		}
		bytes_read += bytes_read_field;
		cubAvail -= bytes_read_field;
	}

	return result;
}
