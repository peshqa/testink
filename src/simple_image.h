/*
ppm_image_loader.h - this header file combines different file format loaders
2023/12/28, peshqa
*/
#pragma once

// define some common stuff
typedef struct
{
	int width;
	int height;
	// assuming its 4 bytes per pixel
	int *pixels;
	// TODO: remove index, have some way of tracking loaded images
	int index;
} SimpleImage;

#pragma pack(push, 1)
typedef struct
{
	unsigned short magic_number;
	unsigned int file_size;
	unsigned short reserved1;
	unsigned short reserved2;
	unsigned int offset;
	// start of BITMAPINFOHEADER
	unsigned int header_size;
	int width;
	int height;
	unsigned short color_planes; // always 1
	unsigned short bits_per_pixel;
	unsigned int compression_method; // BI_RGB is 0 (most common)
	unsigned int raw_bmp_size;
	int horiz_resolution; // pixel per metre (?)
	int vertical_resolution; // pixel per metre (?)
	unsigned int num_colors;
	unsigned int num_important_colors; // ?
} BitmapFileHeader;
#pragma pack(pop)
static char *FindNextToken(u32 str_size, char *str, u32 *leftover_size)
{
	for (u32 i = 0; i < str_size; i++)
	{
		if (str[i] == ' ' || str[i] == '\n' || str[i] == '\t')
		{
			continue;
		}
		*leftover_size = str_size - i;
		return &str[i];
	}
	*leftover_size = 0;
	return 0;
}
static char *FindNextLine(u32 str_size, char *str, u32 *leftover_size)
{
	for (u32 i = 0; i < str_size; i++)
	{
		if (str[i] == '\n' && i < str_size-1)
		{
			*leftover_size = str_size - i - 1;
			return &str[i]+1;
		}
	}
	*leftover_size = 0;
	return 0;
}
static char *ParseInteger(u32 str_size, char *str, u32 *leftover_size, u32 *out)
{
	u32 result = 0;
	u32 i = 0;
	for (; i < str_size; i++)
	{
		if (str[i] < '0' || str[i] > '9')
		{
			break;
		}
		
		result = result*10 + str[i] - '0';
	}
	*out = result;
	*leftover_size = str_size - i;
	return &str[i];
}
static int CountNTString(char* str)
{
	int count = 0;
	while (*str && count < 128)
	{
		str++;
		count++;
	}
	return count;
}
static int LoadPPMImage(SharedState *s, char* file_path, SimpleImage *image)
{
	void *memory;
	u32 memory_size = PlatformReadWholeFile(s, file_path, memory);
	u32 leftover_size = memory_size;
	if (memory_size == 0)
	{
		ASSERT(!"LoadPPMImage - Couldn't open the file");
		return 1;
	}
	
	char *line = (char*)memory;
	
	//if (line != "P6")
	while (line[0] == '#')
	{
		line = FindNextLine(leftover_size, line, &leftover_size);
	}
	if (line[0] != 'P' && line[1] != '6')
	{
		ASSERT(!"LoadPPMImage - Magic number not matching");
		return 2;
	}
	
	unsigned int width;
	unsigned int height;
	int max_value;

	do { line = FindNextLine(leftover_size, line, &leftover_size); } while (line[0] == '#');
	line = ParseInteger(leftover_size, line, &leftover_size, &width);
	line = FindNextToken(leftover_size, line, &leftover_size);
	line = ParseInteger(leftover_size, line, &leftover_size, &height);
	line = FindNextToken(leftover_size, line, &leftover_size);
	line = ParseInteger(leftover_size, line, &leftover_size, (u32*)&max_value);
	line = FindNextToken(leftover_size, line, &leftover_size);
	
	if (max_value > 255)
	{
		return 3;
	}
	
	//file.ignore();
	//char ignored;
	//ReadAssetBytesA(s, &ignored, 1);
	image->width = width;
	image->height = height;
	//image->pixels = (i32*)line;
	image->pixels = new int[width*height];
	
	//char *buffer = new char[width*height*3]{};
	//file.read(buffer, width*height*3);
	//ReadAssetBytesA(s, buffer, width*height*3);
	char *buffer = line;
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{

			image->pixels[y*width+x] = MakeColor(255,
				buffer[y*width*3+x*3],
				buffer[y*width*3+x*3+1],
				buffer[y*width*3+x*3+2]
				);
		}
	}
	
	//delete [] buffer;
	//file.close();
	//CloseAssetFile(s);
	return 0;
}

/*
static int oldLoadBMPImage(SharedState *s, std::string file_path, SimpleImage *image)
{
	if (OpenAssetFileA(s, file_path) != 0)
	{
		ASSERT(!"LoadBMPImage - Couldn't open the file");
		return 1;
	}
	
	//char *buffer = new char[sizeof(BitmapFileHeader)]{};
	BitmapFileHeader header{};
	ReadAssetBytesA(s, (char*)&header, sizeof(BitmapFileHeader));
	
	if (header.magic_number != 'MB')
	{
		ASSERT(!"LoadBMPImage - Magic number not matching");
		return 2;
	}
	
	if (header.offset < 54 || header.bits_per_pixel < 32 || header.compression_method != 0)
	{
		ASSERT(!"LoadBMPImage - Unrecognized bmp file format");
		return 3;
	}

	if (image->pixels != 0)
	{
		delete [] image->pixels;
	}
	image->pixels = new int[header.width*header.height]{};
	image->width = header.width;
	image->height = header.height;
	
	char *buffer = new char[header.width*header.height*4]{};

	ReadAssetBytesA(s, buffer, header.width*header.height*4);
	for (int y = 0; y < header.height; y++)
	{
		for (int x = 0; x < header.width; x++)
		{
			image->pixels[(header.height-y-1)*header.width+x] = MakeColor(255,
				buffer[y*header.width*4+x*4],
				buffer[y*header.width*4+x*4+1],
				buffer[y*header.width*4+x*4+2]
				);
		}
	}
	
	delete [] buffer;
	
	CloseAssetFile(s);
	return 0;
}*/
static int LoadBMPImage(SharedState *s, char *file_path, SimpleImage *image)
{
	void *memory;
	if (PlatformReadWholeFile(s, file_path, memory) == 0)
	{
		ASSERT(!"LoadBMPImage - Couldn't open the file");
		return 1;
	}

	BitmapFileHeader *header = (BitmapFileHeader*)memory;
	
	if (header->magic_number != 0x4d42) // 'MB'
	{
		ASSERT(!"LoadBMPImage - Magic number not matching");
		return 2;
	}
	
	if (header->offset < 54 || header->bits_per_pixel < 32 || header->compression_method != 0)
	{
		ASSERT(!"LoadBMPImage - Unrecognized bmp file format");
		return 3;
	}

	image->pixels = (int*)((u8*)memory + header->offset);
	//image->pixels = (int*)&memory;
	image->width = header->width;
	image->height = header->height;
	
	ASSERT(image->height > 0); // if image->height < 0, then the image is top-to-bottom

	// NOTE: probably need to enable this to work on other platforms
	/*for (int y = 0; y < header->height; y++)
	{
		for (int x = 0; x < header->width; x++)
		{
			image->pixels[(header->height)*header->width+x] = MakeColor(255,
				(int)(((i8*)(image->pixels))[y*header->width*4+x*4]  ),
				(int)(((i8*)(image->pixels))[y*header->width*4+x*4+1]),
				(int)(((i8*)(image->pixels))[y*header->width*4+x*4+2])
				);
		}
	}*/

	return 0;
}
// function determines image format based on file extension and calls the actual image loader
static int LoadImage(SharedState *s, char* file_path, SimpleImage *image)
{
	int str_count = CountNTString(file_path);
	//ASSERT(0);
	if (str_count < 4)
	{
		return 0;
	}
	
	if (file_path[str_count-4] != '.')
	{
		return 0;
	}
	int res;
	if (file_path[str_count-3] == 'b')
	{
		res = LoadBMPImage(s, file_path, image);
	}
	if (file_path[str_count-3] == 'p')
	{
		res = LoadPPMImage(s, file_path, image);
	}
	if (res == 0)
	{
		//image->index = s->texture_count++;
		image->index = 0;
		return 1;
	}
	return 0;
}
static int SampleTexture(SimpleImage *img, float u, float v)
{
	if (u >= 0.0f && u <= 1.0f && v >= 0.0f && v <= 1.0f)
	{
		return img->pixels[img->width*(int)((img->height)*(1.0f-v)) + (int)(img->width*u)];
		//return img->pixels[img->width*(int)((img->height-1)*(1.0f-v)) + (int)(img->width*u)]; // TODO: which is correct?
	}
	return MakeColor(255, 241, 87, 236);
}

static Vec4 ColorIntToVec4(int color)
{
	u8 *b = (u8*)&color;
	Vec4 res = {(float)b[2] / 255,(float)b[1] / 255, (float)b[0] / 255, (float)b[3] / 255};
	return res;
}

static int ColorVec4ToInt(Vec4 v)
{
	int res = MakeColor(v.a*255, v.r*255, v.g*255, v.b*255);
	return res;
}

// TODO: move to simple_math.h?
static Vec4 Lerp(Vec4 a, Vec4 b, float t)
{
	return a*(1.0-t) + b*t;
}

static int BilinearSampleTexture(SimpleImage *img, float u, float v)
{
	if (u >= 0.0f && u <= 1.0f && v >= 0.0f && v <= 1.0f)
	{
		v = (1.0f-v);
		int sample0 = img->pixels[img->width*(int)((img->height)*(v)) + (int)(img->width*u)];
		int sample1 = img->pixels[img->width*(int)((img->height)*(v)) + (int)(img->width*u) + 1];
		int sample2 = img->pixels[img->width*(int)((img->height)*(v)+1) + (int)(img->width*u)];
		int sample3 = img->pixels[img->width*(int)((img->height)*(v)+1) + (int)(img->width*u) + 1];
		
		Vec4 v0 = ColorIntToVec4(sample0);
		Vec4 v1 = ColorIntToVec4(sample1);
		Vec4 v2 = ColorIntToVec4(sample2);
		Vec4 v3 = ColorIntToVec4(sample3);
		
		float t_x = (img->width*u - (int)(img->width*u));
		float t_y = (img->height*v - (int)(img->height*v));
		ASSERT(t_x >= 0.0f && t_x <= 1.0f && t_y >= 0.0f && t_y <= 1.0f)
		Vec4 result = Lerp(Lerp(v0, v1, t_x), Lerp(v2, v3, t_x), t_y);
		
		return ColorVec4ToInt(result);
	}
	return MakeColor(255, 241, 87, 236);
}
