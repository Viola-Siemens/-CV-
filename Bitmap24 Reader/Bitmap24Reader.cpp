#include <cstdio>
#include <cstdlib>

class Bitmap24_Reader {
	private:
		typedef unsigned long       DWORD;
		typedef unsigned char       BYTE;
		typedef unsigned short      WORD;
		
		struct BITMAPINFOHEADER {
			DWORD	biSize;
			long	biWidth;
			long	biHeight;
			WORD	biPlanes;
			WORD	biBitCount;
			DWORD	biCompression;
			DWORD	biSizeImage;
			long	biXPelsPerMeter;
			long	biYPelsPerMeter;
			DWORD	biClrUsed;
			DWORD	biClrImportant;
		};
		
		BITMAPINFOHEADER bih;
		BYTE *Buffer;
		long LineByteWidth;
		
		bool ReadBmp(const char* szFileName) {
			FILE *file;
			WORD bfh[7];
			long dpixeladd;
			
			if(NULL == (file = fopen(szFileName, "rb"))) {
				return false;
			}
			
			fread(&bfh, sizeof(WORD), 7, file);
			if(bfh[0] != (WORD)(((WORD)'B')|('M'<<8))) {
				fclose(file);
				return false;
			}
			
			fread(&bih, sizeof(BITMAPINFOHEADER), 1, file);
			
			if(bih.biBitCount != 24) {
				fclose(file);
				return false;
			}
			
			dpixeladd = bih.biBitCount / 8;
			LineByteWidth = bih.biWidth * (dpixeladd);
			if((LineByteWidth % 4) != 0) {
				LineByteWidth += 4 - (LineByteWidth % 4);
			}
			
			if((Buffer = (BYTE*)malloc(sizeof(BYTE) * LineByteWidth * bih.biHeight)) != NULL) {
				fread(Buffer, LineByteWidth * bih.biHeight, 1, file);
				
				fclose(file);
				return true;
			}
		
			fclose(file);
			return false;
		}
		
	public:
		bool GetDIBColor(int X, int Y, BYTE* r, BYTE* g, BYTE* b) const {
			int dpixeladd;
			BYTE *ptr;
			if(X < 0 || X >= bih.biHeight || Y < 0 || Y >= bih.biWidth) {
				return false;
			}
		
			dpixeladd = bih.biBitCount / 8;
			ptr = Buffer + (bih.biHeight - 1 - X) * LineByteWidth + Y * dpixeladd;
		
			*b = *ptr;
			*g = *(ptr + 1);
			*r = *(ptr + 2);
		
			return true;
		}
		
		long Height() const {
			return bih.biHeight;
		}
		
		long Width() const {
			return bih.biWidth;
		}
		
		WORD BitCount() const {
			return bih.biBitCount;
		}
		
		Bitmap24_Reader(const char* szfilename) : Buffer(NULL) {
			if(!ReadBmp(szfilename)) {
				fprintf(stderr, "failure to read file %s", szfilename);
				return;
			}
		}
		
		~Bitmap24_Reader() {
			free(Buffer);
		}
};

int main() {
	unsigned char R, G, B;
	double Luminance = 0;
//	Method1
	Bitmap24_Reader bmr("1.bmp");
	for(register int x(0); x < bmr.Height(); ++x) {
		for(register int y(0); y < bmr.Width(); ++y) {
			bmr.GetDIBColor(x, y, &R, &G, &B);
			Luminance += 0.299 * R + 0.587 * G + 0.114 * B;
		}
	}
	Luminance /= bmr.Height() * bmr.Width();
	printf("Luminance of 1.bmp: %.0lf\n", Luminance);
	
	Luminance = 0;
//	Method2
	Bitmap24_Reader* ptr_bmr;
	ptr_bmr = new Bitmap24_Reader("2.bmp");
	for(register int x(0); x < ptr_bmr->Height(); ++x) {
		for(register int y(0); y < ptr_bmr->Width(); ++y) {
			ptr_bmr->GetDIBColor(x, y, &R, &G, &B);
			Luminance += 0.299 * R + 0.587 * G + 0.114 * B;
		}
	}
	Luminance /= ptr_bmr->Height() * ptr_bmr->Width();
	printf("Luminance of 2.bmp: %.0lf\n", Luminance);
	
	return 0;
}
