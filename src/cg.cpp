// .CG loader
//
// .CG contains information about sprite mappings from the ENC and PVR tiles.

#include "cg.h"
#include "misc.h"

#include <cstdlib>
#include <cstring>

#include <iostream>

const CG_Image* CG::get_image(unsigned int n) {
	if (n >= m_nimages) {
		return 0;
	}

	unsigned int index = m_indices[n];
	if (index < 0) {
		return 0;
	}

	if (index + sizeof(CG_Image) > m_data_size) {
		return 0;
	}

	return (const CG_Image*)(m_data + index);
}

const char* CG::get_filename(unsigned int n) {
	if (!m_loaded) {
		return 0;
	}

	const CG_Image* image = get_image(n);
	if (!image) {
		return 0;
	}

	return image->filename;
}

int CG::get_width(int n)
{
	if (!m_loaded) {
		return 0;
	}

	const CG_Image* image = get_image(n);
	if (!image) {
		return 0;
	}

	return image->width;
}

int CG::get_height(int n)
{
	if (!m_loaded) {
		return 0;
	}

	const CG_Image* image = get_image(n);
	if (!image) {
		return 0;
	}

	return image->height;
}

int CG::get_image_count() {
	return m_nimages;
}

void CG::copy_cells(const CG_Image* image,
	const CG_Alignment* align,
	unsigned char* pixels,
	unsigned int x1,
	unsigned int y1,
	unsigned int width,
	unsigned int height,
	unsigned int* palette,
	bool is_8bpp) {
	int w = align->width / 0x10;
	int h = align->height / 0x10;
	int x = align->source_x / 0x10;
	int y = align->source_y / 0x10;
	int cell_n = (y * 0x10) + x;
	Page* im = &pages[align->source_image];

	for (int a = 0; a < h; ++a) {
		for (int b = 0; b < w; ++b) {
			ImageCell* cell = &im->cell[cell_n + b];

			if (cell->start == 0) {
				continue;
			}

			unsigned char* dest = pixels;
			unsigned int offset;

			offset = (align->y + (a * 0x10) - y1) * width;
			offset += align->x + (b * 0x10) - x1;

			if (is_8bpp) {
				// 8bpp -> 8bpp
				unsigned char* src = ((unsigned char*)m_data) + cell->start + cell->offset;
				int cellw = cell->width;

				dest += offset;

				for (int c = 0; c < 0x10; ++c) {
					for (int d = 0; d < 0x10; ++d) {
						dest[d] = src[d];
					}

					src += cellw;
					dest += width;
				}
			}
			else if (image->type_id == 4) {
				// two pass: first 8bit palettized, second 8bit alpha
				unsigned int* ldest = (unsigned int*)dest;
				unsigned char* src = ((unsigned char*)m_data) + cell->start + cell->offset;
				int cellw = cell->width;

				ldest += offset;

				for (int c = 0; c < 0x10; ++c) {
					for (int d = 0; d < 0x10; ++d) {
						ldest[d] = palette[src[d]] & 0xffffff;
					}

					src += cellw;
					ldest += width;
				}

				ldest = (unsigned int*)dest;
				ldest += offset;

				src = ((unsigned char*)m_data) + cell->start + cell->offset;
				src += align->width * align->height;

				for (int c = 0; c < 0x10; ++c) {
					for (int d = 0; d < 0x10; ++d) {
						ldest[d] |= src[d] << 24;
					}

					src += cellw;
					ldest += width;
				}
			}
			else if (image->type_id == 1) {
				// 32bpp bgr -> rgb
				unsigned int* ldest = (unsigned int*)dest;
				unsigned int* src = (unsigned int*)(m_data + cell->start + cell->offset);
				int cellw = cell->width;

				ldest += offset;

				for (int c = 0; c < 0x10; ++c) {
					for (int d = 0; d < 0x10; ++d) {
						unsigned int v = src[d];
						v = (v & 0xff00ff00) | ((v & 0xff) << 16) | ((v & 0xff0000) >> 16);
						ldest[d] = v;
					}

					src += cellw;
					ldest += width;
				}
			}
			else {
				// palettized 8bpp -> 32bpp
				unsigned int* ldest = (unsigned int*)dest;
				unsigned char* src = ((unsigned char*)m_data) + cell->start + cell->offset;
				int cellw = cell->width;

				ldest += offset;


				for (int c = 0; c < 0x10; ++c) {
					for (int d = 0; d < 0x10; ++d) {
						ldest[d] = palette[src[d]];
					}

					src += cellw;
					ldest += width;
				}
			}
		}

		cell_n += 0x10;
	}
}


ImageData* CG::draw_texture(unsigned int n, bool to_pow2_flg, bool draw_8bpp) {
	const CG_Image* image = get_image(n);
	if (!image) {
		return 0;
	}

	if (image->type_id == -1) {
		return 0; //The game doesn't draw them either.
	}

	if ((image->align_start + image->align_len) > m_nalign) {
		return 0;
	}

	// initialize texture and boundaries
	int x1 = 0;
	int y1 = 0;

	if (!draw_8bpp) {
		x1 = image->bounds_x1;
		y1 = image->bounds_y1;
	}

	int width = image->bounds_x2 - x1 + 1;
	int height = image->bounds_y2 - y1 + 1;

	if (width == 0 || height == 0) {
		return 0;
	}

	if (to_pow2_flg) {
		width = to_pow2(width);
		height = to_pow2(height);
	}

	// check to see if we need a custom palette
	static unsigned int custom_palette[256];
	bool needsCustom = false;
	if (image->bpp == 32) {
		if (image->type_id == 3) {
			unsigned int color = *(unsigned int*)image->data;

			color &= 0xffffff;

			custom_palette[0] = 0;
			for (int i = 1; i < 256; ++i) {
				custom_palette[i] = (i << 24) | color;
			}

			needsCustom = true;
		}
		else if (image->type_id == 2 || image->type_id == 4) {
			memcpy(custom_palette, image->data, 1024);

			for (int i = 0; i < 256; ++i) {
				custom_palette[i] = (0xff << 24) | custom_palette[i];
			}
			needsCustom = true;
		}
	}

	unsigned char* pixels = new unsigned char[width * height * 4];
	memset(pixels, 0, width * height * 4);

	// run through all tile region data
	const CG_Alignment* align;

	bool is_8bpp;

	if (draw_8bpp && image->bpp <= 8) {
		is_8bpp = 1;
	}
	else {
		is_8bpp = 0;
	}

	align = &m_align[image->align_start];
	for (unsigned int i = 0; i < image->align_len; ++i, ++align) {
		copy_cells(image, align, pixels, x1, y1, width, height, needsCustom ? custom_palette : palette, is_8bpp);
	}

	// finalize in texture
	ImageData* texture;

	if (!(texture = new ImageData{ pixels, width, height, is_8bpp, false, image->bounds_x1, image->bounds_y1 }))
	{
		delete texture;
		delete[] pixels;
		texture = nullptr;
	}

	return texture;
}

void CG::build_image_table() {

	// Create new image table and initialize it.
	pages = new Page[page_count];
	memset(pages, 0, sizeof(Page) * page_count);

	// Go through and initialize all the cells.
	int maxCelln = 0;
	for (unsigned int i = 0; i < 0x3000; ++i) {
		const CG_Image* image = get_image(i);

		if (!image) {
			continue;
		}

		if (image->type_id == -1)
			continue;

		if ((image->align_start + image->align_len) > m_nalign) {
			continue;
		}

		const CG_Alignment* align = &m_align[image->align_start];
		unsigned int address = ((char*)image->data) - m_data;

		if (image->bpp == 32) {
			if (image->type_id == 3) {
				address += 4; //Color key I think?
			}
			else if (image->type_id == 2 || image->type_id == 4) {
				address += 1024; //Indexed and alpha indexed?
			}
		}

		for (unsigned int j = 0; j < image->align_len; ++j, ++align) {
			if (align->copy_flag != 0) {
				continue;
			}


			int w = align->width / 0x10;
			int h = align->height / 0x10;
			int x = align->source_x / 0x10;
			int y = align->source_y / 0x10;
			int cell_n = (y * 0x10) + x;
			Page* im = &pages[align->source_image];

			if (cell_n > maxCelln)
				maxCelln = cell_n;

			if (x + w >= 0x10) {
				w = 0x10 - x;
			}
			if (y + h >= 0x10) {
				h = 0x10 - y;
			}

			int mult = 1;
			if (image->type_id == 1) {
				mult = 4;
			}

			for (int a = 0; a < h; ++a) {
				ImageCell* cell = &im->cell[cell_n];
				for (int b = 0; b < w; ++b, ++cell) {
					cell->start = address;
					cell->width = align->width;
					cell->height = align->height;
					cell->offset = (b * 0x10) + (a * align->width * 0x10) * mult;
					cell->type_id = image->type_id;
					cell->bpp = image->bpp;
				}
				cell_n += 0x10;
			}

			if (image->type_id == 4) {
				mult = 2;
			}

			address += align->width * align->height * mult;
		}
	}

}

int CG::getColorFromPal(unsigned char location)
{
	if (paletteData) {
		return palette[location];
	}
	return 0;
}

int CG::getPalNumber()
{
	return palMax;
}

bool CG::loadPalette(const char* name) {
	if (paletteData) {
		palette = origPalette;
		delete[] paletteData;
		palMax = 0;
	}

	unsigned int size;
	char* data;
	if (!ReadInMem(name, paletteData, size)) {
		return false;
	}

	unsigned int* d = (unsigned int*)paletteData;
	palMax = d[0];

	//Quick filesize check to make sure it's valid.
	if (palMax * 0x400 + 4 > size)
	{
		palMax = d[3];
		if (palMax * 0x400 + 4 * 4 > size)
		{
			delete[] paletteData;
			paletteData = nullptr;
			palMax = 0;
			return false;
		}
		paletteOffset = 4;
	}
	else
		paletteOffset = 1;

	palette = d + paletteOffset;

	unsigned int* paletteIterator = palette;
	for (int i = 0; i < palMax; i++)
	{
		unsigned int* p = paletteIterator;
		for (int j = 0; j < 256; ++j) {
			unsigned int v = *p;
			unsigned int alpha = v >> 24;

			alpha = (alpha != 0) ? 255 : 0;

			*p = (v & 0xffffff) | (alpha << 24);
			++p;
		}
		paletteIterator[0] = 0;
		paletteIterator += 0x100;
	}
	return true;
}

bool CG::changePaletteNumber(int number)
{
	if (paletteData && number < palMax && number >= 0)
	{
		unsigned int* d = (unsigned int*)paletteData;
		palette = d + paletteOffset + number * 0x100;
		return true;
	}
	return false;
}

bool CG::load(const char* name) {
	if (m_loaded) {
		free();
	}

	if (paletteData) {
		delete[] paletteData;
		paletteData = nullptr;
		palMax = 0;
	}

	char* data;
	unsigned int size;

	if (!ReadInMem(name, data, size)) {
		return 0;
	}

	// verify size and header
	if (size < 0x4f30 || memcmp(data, "BMP Cutter3", 11)) {
		delete[] data;

		return 0;
	}

	// palette data.
	unsigned int* d = (unsigned int*)(data + 0x10);
	d += 1; // has palette data?
	palette = d;
	origPalette = d;
	palMax = 1;
	d += 0x800;	// There are 8 dupe palettes. The game doesn't use them. - always included.

	unsigned int* p = palette;
	for (int j = 0; j < 256; ++j) {
		unsigned int v = *p;
		unsigned int alpha = v >> 24;

		alpha = (alpha != 0) ? 255 : 0;

		*p = (v & 0xffffff) | (alpha << 24);
		++p;
	}
	palette[0] = 0;

	// parse header
	page_count = (*d) + 1;
	m_nalign = *(d + 2);

	unsigned int* indices = d + 12;
	unsigned int image_count = d[3];

	//was 2999
	if (image_count >= 3000) {
		delete[] data;

		return 0;
	}

	// alignment data
	// store everything for lookup later
	m_align = (CG_Alignment*)(data + indices[3000]);


	m_data = data;
	m_data_size = size;

	m_indices = indices;

	m_nimages = image_count;

	// but wait, there's more!
	// because of the compression added to AACC, we need to go create
	// an image table for this crap.
	build_image_table();

	// we're done, so finish up

	m_loaded = 1;

	return 1;
}

void CG::free() {
	if (paletteData) {
		delete[] paletteData;
	}
	if (m_data) {
		delete[] m_data;
	}
	palMax = 0;
	paletteData = nullptr;
	m_data = nullptr;
	m_data_size = 0;

	if (pages) {
		delete[] pages;
	}
	pages = nullptr;
	page_count = 0;

	m_indices = nullptr;

	m_nimages = 0;

	m_align = nullptr;
	m_nalign = 0;

	m_loaded = 0;
}

CG::CG() {
	m_data = 0;
	m_data_size = 0;

	m_indices = 0;

	m_nimages = 0;

	pages = 0;
	page_count = 0;

	m_align = 0;
	m_nalign = 0;

	m_loaded = 0;
}

CG::~CG() {
	free();
}

