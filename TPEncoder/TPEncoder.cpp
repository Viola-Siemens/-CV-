#include "C:\人工智能\戴米娜·普莉格丽\BitmapReader.h"
#include <io.h>

void print_help() {
	printf("********************************Text In Picture Decoder********************************\n");
	printf("********************************             v 1.0.1   ********************************\n");
	printf("Commands: \n");
	printf("    -e/-encode {-f %%INPUT_FILE_NAME%% | -t %%CONTENT%%} %%OUTPUT_FILE_NAME%% -h %%PICTURE_HEIGHT%% -w %%PICTURE_WIDTH%% [-d %%DEFAULT_RGB%%] [-s %%SCRAMBLE_RATE%%]\n");
	printf("        encode a text file or a text into a picture file [with default RGB value] [with scramble rate].\n");
	printf("    -d/-decode -f %%INPUT_FILE_NAME%% [%%OUTPUT_FILE_NAME%%] [-d %%DEFAULT_RGB%%] [-s %%SCRAMBLE_RATE%%]\n");
	printf("        decode a picture file [to a text file (or print to screen)] [with default RGB value] [with scramble rate].\n");
	printf("    -h/-help\n");
	printf("        print help information.\n");
	printf("Parameters: \n");
	printf("    %%INPUT_FILE_NAME%%: the path and name of an input text/picture file.\n");
	printf("    %%CONTENT%%: a sentence to be encoded.\n");
	printf("    %%OUTPUT_FILE_NAME%%: the path and name of an output text/picture file.\n");
	printf("    %%PICTURE_HEIGHT%%: the height of the output picture file (max value 1920).\n");
	printf("    %%PICTURE_WIDTH%%: the width of the output picture file (max value 1080).\n");
	printf("    %%DEFAULT_RGB%%: the default rgb value of the picture (3D hexadecimal). Default ff0000\n");
	printf("    %%SCRAMBLE_RATE%%: the scramble rate of the picture (3D hexadecimal, the value of each dimension must be power of 2, especially 0 stands for 0x100). Default 010000\n");
	printf("        %%DEFAULT_RGB%% + %%SCRAMBLE_RATE%% must be less than or equal to 256\n");
	printf("        %%PICTURE_HEIGHT%% * %%PICTURE_WIDTH%% * log2(%%SCRAMBLE_RATE%%.x * %%SCRAMBLE_RATE%%.y * %%SCRAMBLE_RATE%%.z) must be more than bitcount(%%CONTENT%%)\n");
	printf("Examples: \n");
	printf("    TPEncoder -encode -t \"ジョジョ、人間ってのは能力に限界があるな。俺が短い人生で学んだことは、人間は策を弄すれば弄するほど、予期せぬ事態で策が崩れ去るってことだ。人間を超えるものにならねばな。俺は、人間をやめるぞ、ジョジョ！\" test.bmp -h 28 -w 28 -d 0xff7f7f -s 0x010808\n");
	printf("    TPEncoder -decode -f test.bmp -d 0xff7f7f -s 0x010808\n");
}
//-encode -t "ジョジョ、人間ってのは能力に限界があるな。俺が短い人生で学んだことは、人間は策を弄すれば弄するほど、予期せぬ事態で策が崩れ去るってことだ。人間を超えるものにならねばな。俺は、人間をやめるぞ、ジョジョ！" test.bmp -h 28 -w 28 -d 0xff7f7f -s 0x010808
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
	int picHeight;
	int picWidth;
	int dR, dG, dB;
	int sR, sG, sB;
	bool hasGotDefault, hasGotScramble;
	
	TPEncoderConfig() : inputFilename(nullptr), outputFilename(nullptr),
		inputContent(nullptr), picHeight(0), picWidth(0),
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
				throw "Value Error: Invalid height value.";
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
			if(defaultRGB <= 0 || defaultRGB > 0xffffff) {
				throw "Value Error: Invalid default RGB value.";
			}
			dB = defaultRGB & 255; defaultRGB >>= 8;
			dG = defaultRGB & 255; defaultRGB >>= 8;
			dR = defaultRGB & 255; defaultRGB >>= 8;
		}
	}
	
	void getScrambleRGB(const char* arg) {
		if(hasGotScramble) {
			throw "Command Error: More than one scramble value assigned.";
		} else {
			int scrambleRGB = -1;
			sscanf(arg, "%x", &scrambleRGB);
			if(scrambleRGB <= 0 || scrambleRGB > 0xffffff) {
				throw "Value Error: Invalid scramble RGB value.";
			}
			sB = scrambleRGB & 255; scrambleRGB >>= 8; if(sB == 0) sB = 0x100;
			sG = scrambleRGB & 255; scrambleRGB >>= 8; if(sG == 0) sG = 0x100;
			sR = scrambleRGB & 255; scrambleRGB >>= 8; if(sR == 0) sR = 0x100;
			if(!is2Pow(sR) || !is2Pow(sG) || !is2Pow(sB)) {
				throw "Value Error: Scramble RGB value is not pow of 2.";
			}
		}
	}
	
	void getOutputFilename(const char* arg) {
		if(outputFilename == nullptr) {
			outputFilename = arg;
		} else {
			throw "Command Error: Multiple output filenames.";
		}
	}
	
	void selfCheck() const {
		if(inputFilename == nullptr && inputContent == nullptr) {
			throw "Parameter Error: No input command.";
		}
		if(dR + sR > 0x100 || dG + sG > 0x100 || dB + sB > 0x100) {
			throw "Parameter Error: Default RGB + Scramble RGB > 256.";
		}
	}
};

void __encode(TPEncoderConfig const& conf) {
	unsigned char* RGBs = new unsigned char[conf.picHeight * conf.picWidth * 3];
	int R_bit = LogInt(conf.sR);
	int B_bit = LogInt(conf.sB);
	int G_bit = LogInt(conf.sG);
	int RGB_bits = R_bit + B_bit + G_bit;
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
				if(feof(fin)) break;
			}
			while(now_bit >= RGB_bits || (feof(fin) && now_bit > 0)) {
				RGBs[(col + (conf.picHeight - row - 1) * conf.picWidth) * 3 + 0] = conf.dB + (elem_bin & (conf.sB - 1));
				elem_bin >>= B_bit;
				RGBs[(col + (conf.picHeight - row - 1) * conf.picWidth) * 3 + 1] = conf.dG + (elem_bin & (conf.sG - 1));
				elem_bin >>= G_bit;
				RGBs[(col + (conf.picHeight - row - 1) * conf.picWidth) * 3 + 2] = conf.dR + (elem_bin & (conf.sR - 1));
				elem_bin >>= R_bit;
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
				if(ch == '\0') break;
			}
			while(now_bit >= RGB_bits || (ch == '\0' && now_bit > 0)) {
				RGBs[(col + (conf.picHeight - row - 1) * conf.picWidth) * 3 + 0] = conf.dB + (elem_bin & (conf.sB - 1));
				elem_bin >>= B_bit;
				RGBs[(col + (conf.picHeight - row - 1) * conf.picWidth) * 3 + 1] = conf.dG + (elem_bin & (conf.sG - 1));
				elem_bin >>= G_bit;
				RGBs[(col + (conf.picHeight - row - 1) * conf.picWidth) * 3 + 2] = conf.dR + (elem_bin & (conf.sR - 1));
				elem_bin >>= R_bit;
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
		RGBs[(col + (conf.picHeight - row - 1) * conf.picWidth) * 3 + 0] = conf.dB;
		RGBs[(col + (conf.picHeight - row - 1) * conf.picWidth) * 3 + 1] = conf.dG;
		RGBs[(col + (conf.picHeight - row - 1) * conf.picWidth) * 3 + 2] = conf.dR;
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
			} else if(strcmp(argv[i], "-s") == 0) {
				conf.getScrambleRGB(argv[++i]);
			}
		} else {
			conf.getOutputFilename(argv[i]);
		}
	}
	
	conf.selfCheck();
	__encode(conf); 
}

void __decode(TPEncoderConfig const& conf) {
	if(access(conf.inputFilename, 0) != 0) {
		throw "File Error: Cannot open input file.";
	}
	Bitmap24_Reader bmr(conf.inputFilename);
	
	int R_bit = LogInt(conf.sR);
	int B_bit = LogInt(conf.sB);
	int G_bit = LogInt(conf.sG);
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
			++col;
			if(col == bmr.Width()) {
				++row;
				col = 0;
				if(row == bmr.Height()) {
					throw "Data Error: Reached end of the picture.";
				}
			}
			
			int csR = (r - conf.dR);
			int csG = (g - conf.dG);
			int csB = (b - conf.dB);
			if(csR < 0 || csG < 0 || csB < 0 || csR >= conf.sR || csG >= conf.sG || csB >= conf.sB) {
				throw "Data Error: -d and/or -s options required/incorrect.";
			}
			
			elem_bin |= csB << now_bit;
			now_bit += B_bit;
			elem_bin |= csG << now_bit;
			now_bit += G_bit;
			elem_bin |= csR << now_bit;
			now_bit += R_bit;
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
			} else if(strcmp(argv[i], "-s") == 0) {
				conf.getScrambleRGB(argv[++i]);
			}
		} else {
			conf.getOutputFilename(argv[i]);
		}
	}
	
	conf.selfCheck();
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
	try {
		launch_parameters(argc - 1, argv + 1);
	} catch(const char* err) {
		puts(err);
	}
	
	return 0;
}

