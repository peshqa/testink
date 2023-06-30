/*
simple_qr.h - header file made for work with Quick Response (QR) codes xD
2023/06/22, peshqa
*/

int abs(int i) { return (i < 0) ? -i : i; }

enum QRVersion : int
{
	QR_VER_M4 = -4,
	QR_VER_M3,
	QR_VER_M2,
	QR_VER_M1
};

enum QRModule : unsigned char
{
	QR_RESERVED = 10,
	QR_UNASSIGNED,
	QR_WHITE = 0,
	QR_DARK
};

// contains information necessary to display the image
typedef struct
{
	int version; // determines size
	unsigned char *code_bytes;
} QRCode;

typedef struct
{
	unsigned char error_correction;
	unsigned char mode;
	int num_of_chars;
	unsigned char *encoded_data;
} QRData;

int VersionToSize(int version)
{
	if (version > 40)
	{
		return -1;
	}
	
	if (version < 0)
	{
		return -1; // TODO mini versions
	}
	
	return 4*version + 17;
}

int InitQRCode(QRCode *qr, int version)
{
	if (qr == 0)
	{
		return -1;
	}
	
	int size = VersionToSize(version);
	qr->version = version;
	qr->code_bytes = new unsigned char[size*size]{};
	for (int i = 0; i < size*size; i++)
	{
		qr->code_bytes[i] = QR_UNASSIGNED;
	}
	
	return 0;
}

int TerminateQRCode(QRCode *qr)
{
	delete [] qr->code_bytes;
	return 0;
}

int ApplyAlignmentPatterns(QRCode *qr)
{
	int size = VersionToSize(qr->version);
	
	int count{};
	int values[7]{};
	switch (qr->version)
	{
		default:
		{
			return -1;
		}
		
		case 1:
		{
			return 0;
		}
		break;
		
		case 2:
		{
			count = 2; values[0] = 6; values[1] = 18;
		}
		break;
		
		case 8:
		{
			count = 3; values[0] = 6; values[1] = 24; values[2] = 42;
		}
		break;
	}
	
	for (int i = 0; i < count; i++)
	{
		for (int j = 0; j < count; j++)
		{
			if (i == 0 && j == 0 || i == count-1 && j == 0 || i == 0 && j == count-1)
			{
				continue;
			}
			int center_x = values[i];
			int center_y = values[j];
			
			for (int radius = 0; radius <= 2; radius++)
			{
				for (int y = -radius; y <= radius; y++)
				{
					for (int x = -radius; x <= radius; x++)
					{
						
						if (abs(y) == radius || abs(x) == radius)
						{
							if (radius == 1)
							{
								qr->code_bytes[(center_y+y)*size+center_x+x] = QR_WHITE;
							}
							else
							{
								qr->code_bytes[(center_y+y)*size+center_x+x] = QR_DARK;
							}
						}
						
					}
				}
			}
			
		}
	}
	
	return 0;
}

int ApplyCornerFinderPattern(QRCode *qr, int center_x, int center_y)
{
	int size = VersionToSize(qr->version);
	
	for (int radius = 0; radius <= 3; radius++)
	{
		for (int y = -radius; y <= radius; y++)
		{
			for (int x = -radius; x <= radius; x++)
			{
				if (abs(y) == radius || abs(x) == radius)
				{
					if (radius == 2)
					{
						qr->code_bytes[(center_y+y)*size+center_x+x] = QR_WHITE;
					}
					else
					{
						qr->code_bytes[(center_y+y)*size+center_x+x] = QR_DARK;
					}
				}
			}
		}
	}

	return 0;
}

int ApplyCornerFinderPatterns(QRCode *qr)
{
	int size = VersionToSize(qr->version);
	ApplyCornerFinderPattern(qr, 3, 3);
	ApplyCornerFinderPattern(qr, size-3-1, 3);
	ApplyCornerFinderPattern(qr, 3, size-3-1);
	return 0;
}

int ApplySeparators(QRCode *qr)
{
	int size = VersionToSize(qr->version);
	for (int i = 0; i < 8; i++)
	{
		qr->code_bytes[i*size+7] = QR_WHITE;
		qr->code_bytes[7*size+i] = QR_WHITE;
		qr->code_bytes[(size-8)*size+i] = QR_WHITE;
		qr->code_bytes[i*size+(size-8)] = QR_WHITE;
		qr->code_bytes[7*size+(size-i-1)] = QR_WHITE;
		qr->code_bytes[(size-i-1)*size+7] = QR_WHITE;
	}
	
	return 0;
}

int ApplyTimingPatterns(QRCode *qr)
{
	int size = VersionToSize(qr->version);
	for (int i = 8; i < size-8; i++)
	{
		qr->code_bytes[i*size+6] = (i&1)!=1 ? QR_DARK : QR_WHITE;
		qr->code_bytes[6*size+i] = (i&1)!=1 ? QR_DARK : QR_WHITE;
	}
	return 0;
}

int ApplyDarkModule(QRCode *qr)
{
	int size = VersionToSize(qr->version);
	qr->code_bytes[(size-8)*size+8] = QR_DARK;
	return 0;
}

int ReserveFormatInformation(QRCode *qr)
{
	int size = VersionToSize(qr->version);
	for (int i = 0; i < 9; i++)
	{
		if (i != 6)
			qr->code_bytes[i*size+8] = QR_RESERVED;
		qr->code_bytes[8*size+i] = QR_RESERVED;

		if (i < 8)
		{
			qr->code_bytes[8*size+(size-i-1)] = QR_RESERVED;
		}
		if (i < 7)
		{
			qr->code_bytes[(size-i-1)*size+8] = QR_RESERVED;
		}
	}
	return 0;
}

int ReserveVersionInformation(QRCode *qr)
{
	if (qr->version >= 7)
	{
		int size = VersionToSize(qr->version);
		for (int i = 0; i < 6; i++)
		{
			qr->code_bytes[(size-9)*size+i] = QR_RESERVED;
			qr->code_bytes[(size-10)*size+i] = QR_RESERVED;
			qr->code_bytes[(size-11)*size+i] = QR_RESERVED;
			
			qr->code_bytes[i*size+size-9] = QR_RESERVED;
			qr->code_bytes[i*size+size-10] = QR_RESERVED;
			qr->code_bytes[i*size+size-11] = QR_RESERVED;
		}
	}
	return 0;
}

int AlphaExpToInt(int e)
{
	int i = 1;
	for (int j = 0; j < e; j++)
	{
		i = i * 2 % 256;
	}
	return i;
}

int xD()
{
	int alpha_exponents[256]{};
	int integer = 1;
	for (int i = 0; i < 256; i++)
	{
		alpha_exponents[integer] = i;
		integer = integer * 2 % 256;
	}
	
	return 0;
}

int ApplyDataAndMask(QRCode *qr)
{
	unsigned char *data_modules = new unsigned char[19*8]{}; // 26 total codewords, 19 data, 7 error correction
	char data[] = "Hello everynyan!";
	int data_count = 16;
	int i = 0;
	int size = VersionToSize(qr->version);
	
	// Mode - Byte Mode (0100)
	data_modules[i++] = QR_WHITE;
	data_modules[i++] = QR_DARK;
	data_modules[i++] = QR_WHITE;
	data_modules[i++] = QR_WHITE;
	
	// Number of characters - 16 (0001 0000)
	data_modules[i++] = QR_WHITE;
	data_modules[i++] = QR_WHITE;
	data_modules[i++] = QR_WHITE;
	data_modules[i++] = QR_DARK;
	
	data_modules[i++] = QR_WHITE;
	data_modules[i++] = QR_WHITE;
	data_modules[i++] = QR_WHITE;
	data_modules[i++] = QR_WHITE;
	
	// Data string - "Hello everynyan!"
	for (int j = 0; j < data_count; j++)
	{
		for (int bit = 0; bit < 8; bit++)
		{
			data_modules[i++] = (data[j]>>(7-bit)) & 1 ? QR_DARK : QR_WHITE;
		}
	}
	// Null terminator
	data_modules[i++] = QR_WHITE;
	data_modules[i++] = QR_WHITE;
	data_modules[i++] = QR_WHITE;
	data_modules[i++] = QR_WHITE;
	
	// Pad Codewords 1110 1100 and 0001 0001 to fill all remaining data codewords
	data_modules[i++] = QR_DARK;
	data_modules[i++] = QR_DARK;
	data_modules[i++] = QR_DARK;
	data_modules[i++] = QR_WHITE;
	
	data_modules[i++] = QR_DARK;
	data_modules[i++] = QR_DARK;
	data_modules[i++] = QR_WHITE;
	data_modules[i++] = QR_WHITE;
	
	// Error correction modules
	unsigned char message_polynomial[26]{};
	for (int i = 18; i >= 0; i--)
	{
		for (int bit = 7; bit >= 0; bit--)
		{
			message_polynomial[i+7] += data_modules[i*8+bit] << bit;
		}
	}
	
	unsigned char generator_polynomial[26]{};
	generator_polynomial[25] = AlphaExpToInt(0);
	generator_polynomial[24] = AlphaExpToInt(87);
	generator_polynomial[23] = AlphaExpToInt(229);
	generator_polynomial[22] = AlphaExpToInt(146);
	generator_polynomial[21] = AlphaExpToInt(149);
	generator_polynomial[20] = AlphaExpToInt(238);
	generator_polynomial[19] = AlphaExpToInt(102);
	generator_polynomial[18] = AlphaExpToInt(21);
	// TODO: finish this
	
	// Applying modules
	int iterator = 0;
	
	for (int x1 = size-1; x1 >= 0; x1-=2)
	{
		for (int y1 = size-1; y1 >= 0; y1--)
		{
			for (int x_offset = 0; x_offset >= -1; x_offset--)
			{
				if (qr->code_bytes[y1*size+x1+x_offset] != QR_UNASSIGNED)
				{
					continue;
				}
				if (iterator < 19*8)
				{
					qr->code_bytes[y1*size+x1+x_offset] = ((x1+x_offset+y1) % 2 == 0) ^ data_modules[iterator++];
				}
			}
		}
		
		x1-=2;
		
		for (int y1 = 0; y1 < size; y1++) // switch the direction
		{
			for (int x_offset = 0; x_offset >= -1; x_offset--)
			{
				if (qr->code_bytes[y1*size+x1+x_offset] != QR_UNASSIGNED)
				{
					continue;
				}
				if (iterator < 19*8)
				{
					qr->code_bytes[y1*size+x1+x_offset] = ((x1+x_offset+y1) % 2 == 0) ^ data_modules[iterator++];
				}
			}
		}
	}
	
	delete [] data_modules;
	return 0;
}