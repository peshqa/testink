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
		{
			qr->code_bytes[i*size+8] = QR_RESERVED;
			qr->code_bytes[8*size+i] = QR_RESERVED;
		}

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
	e = e % 255;
	int i = 1;
	for (int j = 0; j < e; j++)
	{
		i = i * 2;
		if (i > 255)
		{
			i ^= 285;
		}
	}
	return i;
}

int ApplyDataAndMask(QRCode *qr)
{
	unsigned char *data_modules = new unsigned char[26*8]{}; // 26 total codewords, 19 data, 7 error correction
	char data[] = "This is working!";
	int data_count = 16;
	int counter = 0;
	int size = VersionToSize(qr->version);
	
	// Mode - Byte Mode (0100)
	data_modules[counter++] = QR_WHITE;
	data_modules[counter++] = QR_DARK;
	data_modules[counter++] = QR_WHITE;
	data_modules[counter++] = QR_WHITE;
	
	// Number of characters - 16 (0001 0000)
	data_modules[counter++] = QR_WHITE;
	data_modules[counter++] = QR_WHITE;
	data_modules[counter++] = QR_WHITE;
	data_modules[counter++] = QR_DARK;
	
	data_modules[counter++] = QR_WHITE;
	data_modules[counter++] = QR_WHITE;
	data_modules[counter++] = QR_WHITE;
	data_modules[counter++] = QR_WHITE;
	
	// Data string - "Hello everynyan!"
	for (int j = 0; j < data_count; j++)
	{
		for (int bit = 0; bit < 8; bit++)
		{
			data_modules[counter++] = (data[j]>>(7-bit)) & 1 ? QR_DARK : QR_WHITE;
		}
	}
	// Null terminator
	data_modules[counter++] = QR_WHITE;
	data_modules[counter++] = QR_WHITE;
	data_modules[counter++] = QR_WHITE;
	data_modules[counter++] = QR_WHITE;
	
	// Pad Codewords 1110 1100 and 0001 0001 to fill all remaining data codewords
	data_modules[counter++] = QR_DARK;
	data_modules[counter++] = QR_DARK;
	data_modules[counter++] = QR_DARK;
	data_modules[counter++] = QR_WHITE;
	
	data_modules[counter++] = QR_DARK;
	data_modules[counter++] = QR_DARK;
	data_modules[counter++] = QR_WHITE;
	data_modules[counter++] = QR_WHITE;
	
	// Error correction modules
	int alpha_exponents[256]{};
	int integer = 1;
	for (int i = 0; i < 255; i++)
	{
		alpha_exponents[integer] = i;
		integer = integer * 2;
		if (integer > 255)
		{
			integer ^= 285;
		}
	}
	
	unsigned char message_polynomial[26]{};
	for (int i = 18; i >= 0; i--)
	{
		for (int bit = 7; bit >= 0; bit--)
		{
			message_polynomial[i+7] += data_modules[(18-i)*8+bit] << (7-bit);
		}
	}
	
	for (int i = 0; i < 19; i++)
	{
		int lead_term = message_polynomial[25-i];
		message_polynomial[25-i] = AlphaExpToInt(0		+alpha_exponents[lead_term]) ^ message_polynomial[25-i];
		message_polynomial[24-i] = AlphaExpToInt(87		+alpha_exponents[lead_term]) ^ message_polynomial[24-i];
		message_polynomial[23-i] = AlphaExpToInt(229	+alpha_exponents[lead_term]) ^ message_polynomial[23-i];
		message_polynomial[22-i] = AlphaExpToInt(146	+alpha_exponents[lead_term]) ^ message_polynomial[22-i];
		message_polynomial[21-i] = AlphaExpToInt(149	+alpha_exponents[lead_term]) ^ message_polynomial[21-i];
		message_polynomial[20-i] = AlphaExpToInt(238	+alpha_exponents[lead_term]) ^ message_polynomial[20-i];
		message_polynomial[19-i] = AlphaExpToInt(102	+alpha_exponents[lead_term]) ^ message_polynomial[19-i];
		message_polynomial[18-i] = AlphaExpToInt(21		+alpha_exponents[lead_term]) ^ message_polynomial[18-i];
	}
	
	for (int i = 0; i < 7; i++)
	{
		for (int bit = 7; bit >= 0; bit--)
		{
			data_modules[counter++] = (message_polynomial[6-i] & (1 << bit)) == (1 << bit) ? 1 : 0;
		}
	}
	
	// Applying modules
	int iterator = 0;
	char direction = 'u';
	for (int x1 = size-1; x1 >= 0; x1-=2)
	{
		if (x1 == 6)
		{
			x1--; // skip timing pattern
		}

		int y1_start{};
		int y1_end{};
		int y1_inc{};
		if (direction == 'u')
		{
			y1_start = size-1;
			y1_end = -1;
			y1_inc = -1;
			direction = 'd';
		} else {
			y1_start = 0;
			y1_end = size;
			y1_inc = 1;
			direction = 'u';
		}
		
		for (int y1 = y1_start; y1 != y1_end; y1+=y1_inc)
		{
			for (int x_offset = 0; x_offset >= -1; x_offset--)
			{
				if (qr->code_bytes[y1*size+x1+x_offset] != QR_UNASSIGNED)
				{
					continue;
				}
				if (iterator < 26*8)
				{
					qr->code_bytes[y1*size+x1+x_offset] = ((x1+x_offset+y1) % 2 == 0) ^ data_modules[iterator++];
				}
			}
		}
		
	}
	
	/*unsigned char message_polynomial2[26]{};
	message_polynomial2[25] = 32;
	message_polynomial2[24] = 91;
	message_polynomial2[23] = 11;
	message_polynomial2[22] = 120;
	message_polynomial2[21] = 209;
	message_polynomial2[20] = 114;
	message_polynomial2[19] = 220;
	message_polynomial2[18] = 77;
	message_polynomial2[17] = 67;
	message_polynomial2[16] = 64;
	message_polynomial2[15] = 236;
	message_polynomial2[14] = 17;
	message_polynomial2[13] = 236;
	message_polynomial2[12] = 17;
	message_polynomial2[11] = 236;
	message_polynomial2[10] = 17;
	
	for (int i = 0; i < 16; i++)
	{
		int lead_term = message_polynomial2[25-i];
		message_polynomial2[25-i] = AlphaExpToInt(0		+alpha_exponents[lead_term]) ^ message_polynomial2[25-i];
		message_polynomial2[24-i] = AlphaExpToInt(251	+alpha_exponents[lead_term]) ^ message_polynomial2[24-i];
		message_polynomial2[23-i] = AlphaExpToInt(67	+alpha_exponents[lead_term]) ^ message_polynomial2[23-i];
		message_polynomial2[22-i] = AlphaExpToInt(46	+alpha_exponents[lead_term]) ^ message_polynomial2[22-i];
		message_polynomial2[21-i] = AlphaExpToInt(61	+alpha_exponents[lead_term]) ^ message_polynomial2[21-i];
		message_polynomial2[20-i] = AlphaExpToInt(118	+alpha_exponents[lead_term]) ^ message_polynomial2[20-i];
		message_polynomial2[19-i] = AlphaExpToInt(70	+alpha_exponents[lead_term]) ^ message_polynomial2[19-i];
		message_polynomial2[18-i] = AlphaExpToInt(64	+alpha_exponents[lead_term]) ^ message_polynomial2[18-i];
		message_polynomial2[17-i] = AlphaExpToInt(94	+alpha_exponents[lead_term]) ^ message_polynomial2[17-i];
		message_polynomial2[16-i] = AlphaExpToInt(32	+alpha_exponents[lead_term]) ^ message_polynomial2[16-i];
		message_polynomial2[15-i] = AlphaExpToInt(45	+alpha_exponents[lead_term]) ^ message_polynomial2[15-i];
	}
	*/
	
	// Calculate & apply format information
	
	unsigned char xor_mask[15] = {1,0,1,0,1,0,0,0,0,0,1,0,0,1,0};
	unsigned char format_string[15]{};
	unsigned char polynomial1[15]{};
	unsigned char polynomial2[15]{};
	counter = 0;
	// low level error correction
	polynomial1[counter] = format_string[counter] = 0; counter++;
	polynomial1[counter] = format_string[counter] = 1; counter++;
	// mask pattern bits
	polynomial1[counter] = format_string[counter] = 0; counter++;
	polynomial1[counter] = format_string[counter] = 0; counter++;
	polynomial1[counter] = format_string[counter] = 0; counter++;
	
	counter = 0;
	polynomial2[counter++] = 1;
	polynomial2[counter++] = 0;
	polynomial2[counter++] = 1;
	polynomial2[counter++] = 0;
	polynomial2[counter++] = 0;
	polynomial2[counter++] = 1;
	polynomial2[counter++] = 1;
	polynomial2[counter++] = 0;
	polynomial2[counter++] = 1;
	polynomial2[counter++] = 1;
	polynomial2[counter++] = 1;
	
	for (int i = 0; i < 5; i++)
	{
		if (polynomial1[i] == 0)
		{
			continue;
		}
		for (int j = 0; j < 11; j++)
		{
			polynomial1[0+i+j] ^= polynomial2[j];
		}
	}
	
	for (int i = 0; i < 5; i++)
	{
		polynomial1[i] = format_string[i];
	}
	
	for (int i = 0; i < 15; i++)
	{
		polynomial1[i] ^= xor_mask[i];
		
		if (i <= 5)
		{
			qr->code_bytes[8*size+i] = polynomial1[i];
			qr->code_bytes[(size-1-i)*size+8] = polynomial1[i];
		} else if (i == 6) {
			qr->code_bytes[8*size+i+1] = polynomial1[i];
			qr->code_bytes[(size-1-i)*size+8] = polynomial1[i];
		} else if (i <= 8) {
			qr->code_bytes[(15-i)*size+8] = polynomial1[i];
			qr->code_bytes[8*size+(size-15+i)] = polynomial1[i];
		} else {
			qr->code_bytes[(14-i)*size+8] = polynomial1[i];
			qr->code_bytes[8*size+(size-15+i)] = polynomial1[i];
		}
	}
	
	delete [] data_modules;
	return 0;
}