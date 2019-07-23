#include <cstring>
#include "Bitmap24Reader.h"
#include "Bitmap24Writer.h"

const size_t Map_Height = 224;
const size_t Map_Width = 224;
const size_t Fil_Height = 3;
const size_t Fil_Width = 3;

void Convolution(double __dst_map[3][Map_Height][Map_Width], double const __src_map[3][Map_Height + Fil_Height - 1][Map_Width + Fil_Width - 1],
                 double const __wgt_map[Fil_Height][Fil_Width]) {
	for(register size_t __P(0); __P < 3; ++__P) {
		for(register size_t __I(0); __I < Map_Height; ++__I) {
			for(register size_t __J(0); __J < Map_Width; ++__J) {
				for(register size_t __ht(0); __ht < Fil_Height; ++__ht) {
					for(register size_t __wd(0); __wd < Fil_Width; ++__wd) {
						__dst_map[__P][__I + __ht][__J + __wd] +=
							__src_map[__P][__I][__J] * __wgt_map[__ht][__wd];
					}
				}
			}
		}
	}
}

typedef unsigned char BYTE;

BYTE RGBs[3 * Map_Height * Map_Width];
double Dst_map[3][Map_Height][Map_Width] = {0};
double Src_map[3][Map_Height + Fil_Height - 1][Map_Width + Fil_Width - 1] = {0};

const double Wgt_map_edge[Fil_Height][Fil_Width] = {
	 0, -2,  0,
	-2,  8, -2,
	 0, -2,  0
};

const double Wgt_map_hori_up[Fil_Height][Fil_Width] = {
	 1,  2,  1,
	 0,  0,  0,
	-1, -2, -1
};

const double Wgt_map_hori_down[Fil_Height][Fil_Width] = {
	-1, -2, -1,
	 0,  0,  0,
	 1,  2,  1
};

const double Wgt_map_vert_left[Fil_Height][Fil_Width] = {
	 1,  0, -1,
	 2,  0, -2,
	 1,  0, -1
};

const double Wgt_map_vert_right[Fil_Height][Fil_Width] = {
	-1,  0,  1,
	-2,  0,  2,
	-1,  0,  1
};

const double Wgt_map_tmp[Fil_Height][Fil_Width] = {
	-6, -2,  0,
	-2,  4,  1,
	 0,  1,  2
};

#include <cassert>


int main() {
	Bitmap24_Reader bmr("1.bmp");
	assert(Map_Height == bmr.Height() && Map_Width == bmr.Width());
	BYTE r, g, b;
	
	for(register size_t i(0); i < Map_Height; ++i) {
		for(register size_t j(0); j < Map_Width; ++j) {
			bmr.GetDIBColor(i, j, &r, &g, &b);
			Src_map[0][i + 1][j + 1] = (r - 127.5) / 128.0;
			Src_map[1][i + 1][j + 1] = (g - 127.5) / 128.0;
			Src_map[2][i + 1][j + 1] = (b - 127.5) / 128.0;
		}
	}
	
	Convolution(Dst_map, Src_map, Wgt_map_tmp);
	
	double pixel_max = Dst_map[0][0][0], pixel_min = Dst_map[0][0][0];
	
	for(register size_t p(0); p < 3; ++p) {
		for(register size_t i(0); i < Map_Height; ++i) {
			for(register size_t j(0); j < Map_Width; ++j) {
				if(Dst_map[p][i][j] > pixel_max) {
					pixel_max = Dst_map[p][i][j];
				} else if(Dst_map[p][i][j] < pixel_min) {
					pixel_min = Dst_map[p][i][j];
				}
			}
		}
	}
	
	double ave = (pixel_max + pixel_min) / 2;
	double val = (pixel_max - pixel_min);
	
	for(register size_t i(0); i < Map_Height; ++i) {
		for(register size_t j(0); j < Map_Width; ++j) {
			long R, G, B;
			R = ((Dst_map[0][i][j] - ave) * 255.0 / val) + 127.5;
			assert((R & 255) == R);
			G = ((Dst_map[1][i][j] - ave) * 255.0 / val) + 127.5;
			assert((G & 255) == G);
			B = ((Dst_map[2][i][j] - ave) * 255.0 / val) + 127.5;
			assert((B & 255) == B);
			RGBs[0 + ((Map_Height - i - 1) * Map_Width + j) * 3] = B;
			RGBs[1 + ((Map_Height - i - 1) * Map_Width + j) * 3] = G;
			RGBs[2 + ((Map_Height - i - 1) * Map_Width + j) * 3] = R;
		}
	}
	
	Bitmap24_Writer("2.bmp", Map_Height, Map_Width, RGBs);
	
	return 0;
}
