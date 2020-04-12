
#include <emscripten.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <png.h>

struct fileEmulator {
  int size;
  int readPointer;
  unsigned char *data;
};

void fileEmulatorRead(png_structp png_ptr, png_bytep outBytes, png_size_t byteCount) {
  struct fileEmulator *emulator = (struct fileEmulator *) png_get_io_ptr(png_ptr);
  memcpy(outBytes, emulator->data + emulator->readPointer, byteCount);
  emulator->readPointer += byteCount;
}

EMSCRIPTEN_KEEPALIVE
struct fileEmulator *makeFileEmulator(unsigned char *data, int size) {
  struct fileEmulator *emulator = malloc(sizeof(struct fileEmulator));
  emulator->size = size;
  emulator->readPointer = 0;
  emulator->data = data;
  return(emulator);
}

EMSCRIPTEN_KEEPALIVE
void destroyFileEmulator(struct fileEmulator *emulator) {
  free(emulator);
}

EMSCRIPTEN_KEEPALIVE
void readHeader(png_structp png_ptr, png_infop info_ptr, struct fileEmulator *emulator) {
  if (setjmp(png_jmpbuf(png_ptr))) {
		return;
	}
  png_set_read_fn(png_ptr, emulator, fileEmulatorRead);
  png_read_info(png_ptr, info_ptr);
}

EMSCRIPTEN_KEEPALIVE
// pixelArray is of size returned by readHeader floats (not bytes or pixels)
void readToFloatArray(const png_structp png_ptr, const png_infop info_ptr, float *pixelArray)
{
	//
	// query the header info
	//
  png_uint_32 width, height;
  int bit_depth, color_type, interlace_type;
  int compression_type, filter_method;
  png_get_IHDR(png_ptr, info_ptr,
               &width, &height,
               &bit_depth, &color_type, &interlace_type,
               &compression_type, &filter_method);
  int channel_count;
  channel_count = png_get_channels(png_ptr, info_ptr);

	//
	// convert all data to one component grayscale, endian swapped
	//
  if (color_type == PNG_COLOR_TYPE_PALETTE) {
    png_set_palette_to_rgb(png_ptr);
  }
	int error_action = 1; // silent ignore warnings
	int color_weights = -1; // use defaults
	if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_RGB_ALPHA) {
		png_set_rgb_to_gray_fixed(png_ptr, error_action, color_weights, color_weights);
	}
	if (color_type & PNG_COLOR_MASK_ALPHA) {
		png_set_strip_alpha(png_ptr);
	}
	if (bit_depth == 16) {
		png_set_swap(png_ptr);
	}
	png_read_update_info(png_ptr, info_ptr);

	//
	// read by row and convert to float
	//
	png_uint_32 bytesPerRow = png_get_rowbytes(png_ptr, info_ptr);
	png_bytep byteRowData = (png_bytep) malloc(bytesPerRow);
	unsigned short *shortRowData = (unsigned short *)byteRowData;
	int bytesPerPixel = bit_depth <= 8 ? 1 : 2;
  for(png_uint_32 row = 0; row < height; ++row) {
		png_read_row(png_ptr, (png_bytep)byteRowData, NULL);
		png_uint_32 pixelRowOffset = row * width;
		png_uint_32 floatIndex = 0;
		for(png_uint_32 column = 0; column < width; ++column) {
			float pixel;
			if (bytesPerPixel == 1) {
				pixel = (float) byteRowData[column];
			} else {
				pixel = (float) shortRowData[column];
			}
			pixelArray[pixelRowOffset + column] = pixel;
		}
	}
	free(byteRowData);
}

void ignore(png_structp png_ptr, png_const_charp message) {
}

EMSCRIPTEN_KEEPALIVE
png_structp _png_create_read_struct() {

  return(png_create_read_struct(PNG_LIBPNG_VER_STRING, ignore, ignore, ignore));
}

EMSCRIPTEN_KEEPALIVE
png_infop _png_create_info_struct(png_structp png_ptr) {
  return(png_create_info_struct(png_ptr));
}

EMSCRIPTEN_KEEPALIVE
int _png_get_image_width(png_structp png_ptr, png_infop info_ptr) {
  return(png_get_image_width(png_ptr, info_ptr));
}

EMSCRIPTEN_KEEPALIVE
int _png_get_image_height(png_structp png_ptr, png_infop info_ptr) {
  return(png_get_image_height(png_ptr, info_ptr));
}

EMSCRIPTEN_KEEPALIVE
unsigned char *mallocBuffer(int size) {
  return(malloc(size));
}

EMSCRIPTEN_KEEPALIVE
void freeBuffer(unsigned char *buffer) {
  free(buffer);
}

EMSCRIPTEN_KEEPALIVE
float version() {
  return(0.888);
}
