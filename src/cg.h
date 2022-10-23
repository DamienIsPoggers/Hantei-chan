#ifndef CG_H_GUARD
#define CG_H_GUARD

#include "image.h"

struct CG_Alignment {
	int	x;
	int	y;
	int	width;
	int	height;
	short	source_x;
	short	source_y;
	short	source_image;
	short	copy_flag;
};

struct CG_Image {
	char		filename[32];
	int		type_id;	// i think this is render mode
	unsigned int	width;
	unsigned int	height;
	unsigned int	bpp;
	int		bounds_x1;
	int		bounds_y1;
	int		bounds_x2;
	int		bounds_y2;
	unsigned int	align_start;
	unsigned int	align_len;
	unsigned char	data[1]; 	// for indexing.
};

class CG {
protected:
	unsigned int* origPalette;
	unsigned int* palette;
	char* paletteData = nullptr;
	int palMax = 0;
	int paletteOffset = 0;

	char* m_data;
	unsigned int			m_data_size;

	const unsigned int* m_indices;

	unsigned int			m_nimages;

	const CG_Alignment* m_align;
	unsigned int			m_nalign;

	struct ImageCell {
		unsigned int		start;
		unsigned int		width;
		unsigned int		height;
		unsigned int		offset;
		unsigned short		type_id;
		unsigned short		bpp;
	};

	//Hantei4 calls these "pages".
	struct Page {
		ImageCell		cell[256];
	};

	Page* pages;
	unsigned int			page_count;

	void			copy_cells(
		const CG_Image* image,
		const CG_Alignment* align,
		unsigned char* pixels,
		unsigned int x1,
		unsigned int y1,
		unsigned int width,
		unsigned int height,
		unsigned int* palette,
		bool is_8bpp);

	void			build_image_table();

	const CG_Image* get_image(unsigned int n);
public:
	bool m_loaded;
	bool load(const char* name);
	bool loadPalette(const char* name);
	bool changePaletteNumber(int number);
	int getPalNumber();
	int getColorFromPal(unsigned char location);

	int get_width(int n);
	int get_height(int n);

	void			free();

	const char* get_filename(unsigned int n);

	ImageData* draw_texture(unsigned int n, bool to_pow2, bool draw_8bpp = 0);

	int			get_image_count();

	CG();
	~CG();
};

#endif /* CG_H_GUARD */
