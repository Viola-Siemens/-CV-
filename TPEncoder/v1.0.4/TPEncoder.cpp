#include "BitmapReader.h"
#include <io.h>
#include <ctime>

void print_help() {
	printf("********************************Text In Picture Encoder********************************\n");
	printf("********************************             v 1.0.4   ********************************\n");
	printf("Commands: \n");
	printf("    -e/-encode {-f %%INPUT_FILE_NAME%% | -t %%CONTENT%%} %%OUTPUT_FILE_NAME%% {-h %%PICTURE_HEIGHT%% -w %%PICTURE_WIDTH%% [-d %%DEFAULT_RGB%%] | -c %%SOURCE_FILE_NAME%%} [-s %%SCRAMBLE_RATE%%]\n");
	printf("        encode a text file or a text into a picture file [with default RGB value] [with scramble rate].\n");
	printf("    -d/-decode -f %%INPUT_FILE_NAME%% [%%OUTPUT_FILE_NAME%%] {[-d %%DEFAULT_RGB%%] | -c %%SOURCE_FILE_NAME%%} [-s %%SCRAMBLE_RATE%%]\n");
	printf("        decode a picture file [to a text file (or print to screen)] [with default RGB value] [with scramble rate].\n");
	printf("    -h/-help\n");
	printf("        print help information.\n");
	printf("Parameters: \n");
	printf("    %%INPUT_FILE_NAME%%: the path and name of an input text/picture file.\n");
	printf("    %%CONTENT%%: a sentence to be encoded.\n");
	printf("    %%OUTPUT_FILE_NAME%%: the path and name of an output text/picture file.\n");
	printf("    %%PICTURE_HEIGHT%%: the height of the output picture file (max value 1920).\n");
	printf("    %%PICTURE_WIDTH%%: the width of the output picture file (max value 1080).\n");
	printf("    %%DEFAULT_RGB%%: the default rgb value of the picture (3D hexadecimal). Default 0xff0000.\n");
	printf("    %%SOURCE_FILE_NAME%%: the source picture for background.\n");
	printf("    %%SCRAMBLE_RATE%%: the scramble rate of the picture (3D hexadecimal, the value of each dimension must be power of 2, especially 0 stands for 0x100 in any dimension). Default 0x010000.\n");
	printf("        %%DEFAULT_RGB%% + %%SCRAMBLE_RATE%% must be less than or equal to 256.\n");
	printf("        %%PICTURE_HEIGHT%% * %%PICTURE_WIDTH%% * log2(%%SCRAMBLE_RATE%%.x * %%SCRAMBLE_RATE%%.y * %%SCRAMBLE_RATE%%.z) must be more than bitcount(%%CONTENT%%).\n");
	printf("Examples: \n");
	printf("    TPEncoder -encode -t \"���祸�硢���g�äƤΤ��������޽礬����ʡ������̤�������ѧ������Ȥϡ����g�ϲߤ�Ū�����Ū����ۤɡ����ڤ����B�ǲߤ�����ȥ��äƤ��Ȥ������g�򳬤����Τˤʤ�ͤФʡ����ϡ����g����뤾�����祸�磡\" test.bmp -h 28 -w 28 -d 0xff7f7f -s 0x010808\n");
	printf("    TPEncoder -decode -f test.bmp -d 0xff7f7f -s 0x010808\n");
}
//-encode -t "���祸�硢���g�äƤΤ��������޽礬����ʡ������̤�������ѧ������Ȥϡ����g�ϲߤ�Ū�����Ū����ۤɡ����ڤ����B�ǲߤ�����ȥ��äƤ��Ȥ������g�򳬤����Τˤʤ�ͤФʡ����ϡ����g����뤾�����祸�磡" test.bmp -h 28 -w 28 -d 0xff7f7f -s 0x010808
//-decode -f test.bmp -d 0xff7f7f -s 0x010808

inline bool is2Pow(int x) { 
	return (x & (-x)) == x;
}

inline int LogInt(int x) {
	int cnt = -1;
	while(x) {
		++cnt;
		x >>= 1;
	}
	
	return cnt;
}

struct TPEncoderConfig {
	const char* inputFilename;
	const char* outputFilename;
	const char* inputContent;
	const char* sourceFilename;
	unsigned char* sourceRGBs;
	int picHeight;
	int picWidth;
	int dR, dG, dB;
	int sR, sG, sB;
	bool hasGotDefault, hasGotScramble;
	
	TPEncoderConfig() : inputFilename(nullptr), outputFilename(nullptr),
		inputContent(nullptr), sourceFilename(nullptr),
		sourceRGBs(nullptr), picHeight(0), picWidth(0),
		dR(0xff), dG(0x00), dB(0x00), sR(0x01), sG(0x100), sB(0x100),
		hasGotDefault(false), hasGotScramble(false) {}
	
	void getInputFilename(const char* arg) {
		if(inputFilename == nullptr && inputContent == nullptr) {
			inputFilename = arg;
		} else {
			throw "Command Error: Multiple input commands.";
		}
	}
	
	void getText(const char* arg) {
		if(inputFilename == nullptr && inputContent == nullptr) {
			inputContent = arg;
		} else {
			throw "Command Error: Multiple input commands.";
		}
	}
	
	void getHeight(const char* arg) {
		if(picHeight == 0) {
			sscanf(arg, "%d", &picHeight);
			if(picHeight <= 0 || picHeight > 1920) {
				throw "Value Error: Invalid height value.";
			}
		} else {
			throw "Command Error: More than one height value assigned.";
		}
	}
	
	void getWidth(const char* arg) {
		if(picWidth == 0) {
			sscanf(arg, "%d", &picWidth);
			if(picWidth <= 0 || picWidth > 1080) {
				throw "Value Error: Invalid width value.";
			}
		} else {
			throw "Command Error: More than one width value assigned.";
		}
	}
	
	void getDefaultRGB(const char* arg) {
		if(hasGotDefault) {
			throw "Command Error: More than one default value assigned.";
		} else {
			int defaultRGB = -1;
			sscanf(arg, "%x", &defaultRGB);
			if(defaultRGB < 0 || defaultRGB > 0xffffff) {
				throw "Value Error: Invalid default RGB value.";
			}
			dB = defaultRGB & 255; defaultRGB >>= 8;
			dG = defaultRGB & 255; defaultRGB >>= 8;
			dR = defaultRGB & 255; defaultRGB >>= 8;
			
			hasGotDefault = true;
		}
	}
	
	void getSourceFilename(const char* arg) {
		if(sourceFilename == nullptr && picHeight == 0 && picWidth == 0 && !hasGotDefault) {
			sourceFilename = arg;
			hasGotDefault = true;
		} else {
			throw "Command Error: More than one source filename assigned.";
		}
	}
	
	void getScrambleRGB(const char* arg) {
		if(hasGotScramble) {
			throw "Command Error: More than one scramble value assigned.";
		} else {
			int scrambleRGB = -1;
			sscanf(arg, "%x", &scrambleRGB);
			if(scrambleRGB < 0 || scrambleRGB > 0xffffff) {
				throw "Value Error: Invalid scramble RGB value.";
			}
			sB = scrambleRGB & 255; scrambleRGB >>= 8; if(sB == 0) sB = 0x100;
			sG = scrambleRGB & 255; scrambleRGB >>= 8; if(sG == 0) sG = 0x100;
			sR = scrambleRGB & 255; scrambleRGB >>= 8; if(sR == 0) sR = 0x100;
			if(!is2Pow(sR) || !is2Pow(sG) || !is2Pow(sB)) {
				throw "Value Error: Scramble RGB value is not pow of 2.";
			}
			
			hasGotScramble = true;
		}
	}
	
	void getOutputFilename(const char* arg) {
		if(outputFilename == nullptr) {
			outputFilename = arg;
		} else {
			throw "Command Error: Multiple output filenames.";
		}
	}
	
	void processSource() {
		if(sourceFilename != nullptr) {
			if(access(sourceFilename, 0) != 0) {
				throw "File Error: Cannot open source file.";
			}
			Bitmap24_Reader bmr(sourceFilename);
			if(picHeight == 0 && picWidth == 0) {
				picHeight = bmr.Height();
				picWidth  = bmr.Width();
			} else if(picHeight != bmr.Height() || picWidth != bmr.Width()) {
				throw "Data Error: Input image has different shape with source image.";
			}
			
			sourceRGBs = new unsigned char[picHeight * picWidth * 3];
			for(int i = 0; i < picHeight; ++i) {
				for(int j = 0; j < picWidth; ++j) {
					int rank_posi = (j + (picHeight - i - 1) * picWidth) * 3;
					bmr.GetDIBColor(i, j,
						&sourceRGBs[rank_posi + 2],
						&sourceRGBs[rank_posi + 1],
						&sourceRGBs[rank_posi + 0]);
				}
			}
		}
	}
	
	void selfCheck() const {
		if(inputFilename == nullptr && inputContent == nullptr) {
			throw "Parameter Error: No input command.";
		}
	}
	
	int getDR(int rank_posi) const {
		if(sourceRGBs) {
			return sourceRGBs[rank_posi + 2];
		}
		return dR;
	}
	
	int getDG(int rank_posi) const {
		if(sourceRGBs) {
			return sourceRGBs[rank_posi + 1];
		}
		return dG;
	}
	
	int getDB(int rank_posi) const {
		if(sourceRGBs) {
			return sourceRGBs[rank_posi + 0];
		}
		return dB;
	}
};

void __mergeRGB(unsigned char RGBs[], TPEncoderConfig const& conf,
				const int R_bit, const int G_bit, const int B_bit,
				int rank_posi, int& elem_bin) {
	if(conf.getDB(rank_posi) + (elem_bin & (conf.sB - 1)) <= 0xff) {
		RGBs[rank_posi + 0] = conf.getDB(rank_posi) + (elem_bin & (conf.sB - 1));
	} else {
		RGBs[rank_posi + 0] = 0xff - (elem_bin & (conf.sB - 1));
	}
	elem_bin >>= B_bit;
	
	if(conf.getDG(rank_posi) + (elem_bin & (conf.sG - 1)) <= 0xff) {
		RGBs[rank_posi + 1] = conf.getDG(rank_posi) + (elem_bin & (conf.sG - 1));
	} else {
		RGBs[rank_posi + 1] = 0xff - (elem_bin & (conf.sG - 1));
	}
	elem_bin >>= G_bit;
	
	if(conf.getDR(rank_posi) + (elem_bin & (conf.sR - 1)) <= 0xff) {
		RGBs[rank_posi + 2] = conf.getDR(rank_posi) + (elem_bin & (conf.sR - 1));
	} else {
		RGBs[rank_posi + 2] = 0xff - (elem_bin & (conf.sR - 1));
	}
	elem_bin >>= R_bit;
}

void __encode(TPEncoderConfig const& conf) {
	unsigned char* RGBs = new unsigned char[conf.picHeight * conf.picWidth * 3];
	int R_bit = LogInt(conf.sR);
	int G_bit = LogInt(conf.sG);
	int B_bit = LogInt(conf.sB);
	int RGB_bits = R_bit + G_bit + B_bit;
	int now_bit = 0;
	int elem_bin = 0;
	int row = 0;
	int col = 0;
	char ch;
	if(conf.inputFilename != nullptr) {
		if(access(conf.inputFilename, 0) != 0) {
			throw "File Error: Cannot open input file.";
		}
		FILE* fin = fopen(conf.inputFilename, "r");
		ch = getc(fin);
		while(!feof(fin)) {
			while(now_bit < RGB_bits) {
				elem_bin |= ((int)ch & 0xff) << now_bit;
				now_bit += 8;
				ch = getc(fin);
				if(feof(fin)) {
					now_bit += 16;
					break;
				} 
			}
			while(now_bit >= RGB_bits || (feof(fin) && now_bit > 0)) {
				__mergeRGB(RGBs, conf, R_bit, G_bit, B_bit,
					(col + (conf.picHeight - row - 1) * conf.picWidth) * 3, elem_bin);
				
				now_bit -= RGB_bits;
				++col;
				if(col == conf.picWidth) {
					++row;
					col = 0;
					if(row == conf.picHeight) {
						throw "Data Error: Picture size is too small to contain all the data.";
					}
				}
			}
		}
		fclose(fin);
	} else {
		int size = 0;
		ch = conf.inputContent[size++];
		while(ch != '\0') {
			while(now_bit < RGB_bits) {
				elem_bin |= ((int)ch & 0xff) << now_bit;
				now_bit += 8;
				ch = conf.inputContent[size++];
				if(ch == '\0') {
					now_bit += 16;
					break;
				}
			}
			while(now_bit >= RGB_bits || (ch == '\0' && now_bit > 0)) {
				__mergeRGB(RGBs, conf, R_bit, G_bit, B_bit,
					(col + (conf.picHeight - row - 1) * conf.picWidth) * 3, elem_bin);
				
				now_bit -= RGB_bits;
				++col;
				if(col == conf.picWidth) {
					++row;
					col = 0;
					if(row == conf.picHeight) {
						throw "Data Error: Picture size is too small to contain all the data.";
					}
				}
			}
		}
	}
	while(row < conf.picHeight) {
		unsigned char randomB, randomG, randomR;
		int rank_posi = (col + (conf.picHeight - row - 1) * conf.picWidth) * 3;
		
		randomB = (rand() & (conf.sB - 1));
		if(conf.getDB(rank_posi) + randomB <= 0xff) {
			RGBs[rank_posi + 0] = conf.getDB(rank_posi) + randomB;
		} else {
			RGBs[rank_posi + 0] = 0xff - randomB;
		}
		
		randomG = (rand() & (conf.sG - 1));
		if(conf.getDG(rank_posi) + randomG <= 0xff) {
			RGBs[rank_posi + 1] = conf.getDG(rank_posi) + randomG;
		} else {
			RGBs[rank_posi + 1] = 0xff - randomG;
		}
		
		randomR = (rand() & (conf.sR - 1));
		if(conf.getDR(rank_posi) + randomR <= 0xff) {
			RGBs[rank_posi + 2] = conf.getDR(rank_posi) + randomR;
		} else {
			RGBs[rank_posi + 2] = 0xff - randomR;
		}
		
		++col;
		if(col == conf.picWidth) {
			++row;
			col = 0;
		}
	}
	
	Bitmap24_Reader(conf.outputFilename, conf.picWidth, conf.picHeight, RGBs);
}

void encode(int argc, const char* argv[]) {
	TPEncoderConfig conf;
	
	for(int i = 0; i < argc; ++i) {
		if(argv[i][0] == '-') {
			if(strcmp(argv[i], "-f") == 0) {
				conf.getInputFilename(argv[++i]);
			} else if(strcmp(argv[i], "-t") == 0) {
				conf.getText(argv[++i]);
			} else if(strcmp(argv[i], "-h") == 0) {
				conf.getHeight(argv[++i]);
			} else if(strcmp(argv[i], "-w") == 0) {
				conf.getWidth(argv[++i]); 
			} else if(strcmp(argv[i], "-d") == 0) {
				conf.getDefaultRGB(argv[++i]);
			} else if(strcmp(argv[i], "-c") == 0) {
				conf.getSourceFilename(argv[++i]);
			} else if(strcmp(argv[i], "-s") == 0) {
				conf.getScrambleRGB(argv[++i]);
			}
		} else {
			conf.getOutputFilename(argv[i]);
		}
	}
	
	conf.selfCheck();
	conf.processSource();
	__encode(conf); 
}

void __decode(TPEncoderConfig const& conf) {
	if(access(conf.inputFilename, 0) != 0) {
		throw "File Error: Cannot open input file.";
	}
	Bitmap24_Reader bmr(conf.inputFilename);
	
	int R_bit = LogInt(conf.sR);
	int G_bit = LogInt(conf.sG);
	int B_bit = LogInt(conf.sB);
	int now_bit = 0;
	int elem_bin = 0;
	int row = 0;
	int col = 0;
	char ch;
	
	FILE* fout;
	if(conf.outputFilename == nullptr) {
		fout = stdout;
	} else {
		fout = fopen(conf.outputFilename, "w");
	}
	
	while(row < bmr.Height()) {
		while(now_bit < 8) {
			unsigned char r, g, b;
			bmr.GetDIBColor(row, col, &r, &g, &b);
			
			int rank_posi = (col + (conf.picHeight - row - 1) * conf.picWidth) * 3;
			
			int csR = (r - conf.getDR(rank_posi)); if(csR < 0) csR = 0xff - r; 
			int csG = (g - conf.getDG(rank_posi)); if(csG < 0) csG = 0xff - g;
			int csB = (b - conf.getDB(rank_posi)); if(csB < 0) csB = 0xff - b;
			if(csR < 0 || csG < 0 || csB < 0 || csR >= conf.sR || csG >= conf.sG || csB >= conf.sB) {
				throw "Data Error: -d and/or -s options required/incorrect.";
			}
			
			elem_bin |= csB << now_bit;
			now_bit += B_bit;
			elem_bin |= csG << now_bit;
			now_bit += G_bit;
			elem_bin |= csR << now_bit;
			now_bit += R_bit;
			
			++col;
			if(col == bmr.Width()) {
				++row;
				col = 0;
				if(row == bmr.Height()) {
					throw "Data Error: Reached end of the picture.";
				}
			}
		}
		while(now_bit >= 8) {
			ch = (char)(elem_bin & 0xff);
			now_bit -= 8;
			elem_bin >>= 8;
			if(ch == '\0') {
				goto FinalReturn;
			}
			putc(ch, fout);
		}
	}
	
FinalReturn:
	putc('\n', fout);
	fclose(fout);
}

void decode(int argc, const char* argv[]) {
	TPEncoderConfig conf;
	
	for(int i = 0; i < argc; ++i) {
		if(argv[i][0] == '-') {
			if(strcmp(argv[i], "-f") == 0) {
				conf.getInputFilename(argv[++i]);
			} else if(strcmp(argv[i], "-d") == 0) {
				conf.getDefaultRGB(argv[++i]);
			} else if(strcmp(argv[i], "-c") == 0) {
				conf.getSourceFilename(argv[++i]);
			} else if(strcmp(argv[i], "-s") == 0) {
				conf.getScrambleRGB(argv[++i]);
			}
		} else {
			conf.getOutputFilename(argv[i]);
		}
	}
	
	conf.selfCheck();
	conf.processSource();
	__decode(conf);
}

void launch_parameters(int argc, const char* argv[]) {
	if(argc == 0) {
		print_help();
	} else {
		if(strcmp(argv[0], "-e") == 0 || strcmp(argv[0], "-encode") == 0) {
			encode(argc - 1, argv + 1);
		} else if(strcmp(argv[0], "-d") == 0 || strcmp(argv[0], "-decode") == 0) {
			decode(argc - 1, argv + 1);
		} else if(strcmp(argv[0], "-h") == 0 || strcmp(argv[0], "-help") == 0) {
			print_help();
		} else if(argv[0][0] == '-') {
			throw "Command Error: Unknown command.";
		} else {
			throw "Command Error: Command not found.";
		}
	}
}

int main(int argc, const char* argv[]) {
	srand(time(nullptr));
	try {
		launch_parameters(argc - 1, argv + 1);
	} catch(const char* err) {
		puts(err);
	}
	
	return 0;
}

