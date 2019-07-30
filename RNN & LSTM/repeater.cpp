#include <cstdio>
#include <cstring>
using namespace std;

#define Input_Size  3
#define Hidden_Size 8
#define Recur_Size  5
#define Sample_Size 4096

static const double alpha  = 0.1;
double l_rate = 3e-5;
double m_rate = 0.92;

double rand_double() {
	static size_t seed = 19260817uLL;
	seed = seed * 20001121uLL + 4321uLL;
	return (double((seed >> 8uLL) & 0xffff) / double(0x10000)) - 0.5;
}

class RNN {
	private:
		static double LReLU(double x) {
			return (x < 0 ? alpha * x : x);
		}
		static double df_LReLU(double x) {
			return (x < 0 ? alpha : 1.0);
		}
		
		double inp[Input_Size];
		double hid_net[Recur_Size][Hidden_Size];
		double hid_out[Recur_Size][Hidden_Size];
		double out[Recur_Size][Input_Size];
		
		double wt_e[Hidden_Size][Input_Size];
		double wt_h[Hidden_Size][Hidden_Size];
		double wt_o[Input_Size][Hidden_Size];
		double bias_hid[Hidden_Size];
		double bias_out[Input_Size];
		
		double grad_wt_e[Hidden_Size][Input_Size];
		double grad_wt_h[Hidden_Size][Hidden_Size];
		double grad_wt_o[Input_Size][Hidden_Size];
		double grad_bias_hid[Hidden_Size];
		double grad_bias_out[Input_Size];
		
		double err_out[Recur_Size][Input_Size];
		double err_hid[Recur_Size][Hidden_Size];
		
		void fwd_prop(double vct[Input_Size]) {
			//Input Layer
			memcpy(inp, vct, sizeof(inp));
			
			//Hidden Layer
			for(register size_t i(0); i < Hidden_Size; ++i) {
				double ret = 0;
				for(register size_t j(0); j < Input_Size; ++j) {
					ret += wt_e[i][j] * inp[j];
				}
				ret += bias_hid[i];
				hid_net[0][i] = ret;
				hid_out[0][i] = LReLU(ret);
			}
			
			for(register size_t t(1); t < Recur_Size; ++t) {
				for(register size_t i(0); i < Hidden_Size; ++i) {
					double ret = 0;
					for(register size_t j(0); j < Input_Size; ++j) {
						ret += wt_e[i][j] * inp[j];
					}
					for(register size_t j(0); j < Hidden_Size; ++j) {
						ret += wt_h[i][j] * hid_out[t - 1][j];
					}
					ret += bias_hid[i];
					hid_net[t][i] = ret;
					hid_out[t][i] = LReLU(ret);
				}
			}
			
			//Output Layer
			for(register size_t t(0); t < Recur_Size; ++t) {
				for(register size_t i(0); i < Input_Size; ++i) {
					double ret = 0;
					for(register size_t j(0); j < Hidden_Size; ++j) {
						ret += wt_o[i][j] * hid_out[t][j];
					}
					ret += bias_out[i];
					out[t][i] = ret;
				}
			}
		}
		
		double Diff_Error(double ans[Input_Size]) {
			double Loss = 0;
			for(register size_t t(0); t < Recur_Size; ++t) {
				for(register size_t i(0); i < Input_Size; ++i) {
					double et = out[t][i] - ans[i];
					Loss += 0.5 * et * et;
					err_out[t][i] = et;
				}
			}
			
			return Loss;
		}
		
		void bwd_prop() {
			//Hidden Layer
			for(register size_t i(0); i < Hidden_Size; ++i) {
				double ret = 0;
				for(register size_t j(0); j < Input_Size; ++j) {
					ret += wt_o[j][i] * err_out[Recur_Size - 1][j];
				}
				err_hid[Recur_Size - 1][i] = ret * df_LReLU(hid_net[Recur_Size - 1][i]);
			}
			
			for(register size_t t(Recur_Size - 1); t > 0; --t) {
				for(register size_t i(0); i < Hidden_Size; ++i) {
					double ret = 0;
					for(register size_t j(0); j < Input_Size; ++j) {
						ret += wt_o[j][i] * err_out[t - 1][j];
					}
					for(register size_t j(0); j < Hidden_Size; ++j) {
						ret += wt_h[j][i] * err_hid[t][j];
					}
					err_hid[t - 1][i] = ret * df_LReLU(hid_net[t - 1][i]);
				}
			}
			
			for(register size_t t(0); t < Recur_Size; ++t) {
				for(register size_t i(0); i < Input_Size; ++i) {
					for(register size_t j(0); j < Hidden_Size; ++j) {
						grad_wt_o[i][j] += err_out[t][i] * hid_out[t][j];
					}
					grad_bias_out[i] += err_out[t][i];
				}
			}
			for(register size_t t(1); t < Recur_Size; ++t) {
				for(register size_t i(0); i < Hidden_Size; ++i) {
					for(register size_t j(0); j < Hidden_Size; ++j) {
						grad_wt_h[i][j] += err_hid[t][i] * hid_out[t - 1][j];
					}
				}
			}
			
			//Input Layer
			for(register size_t t(0); t < Recur_Size; ++t) {
				for(register size_t i(0); i < Hidden_Size; ++i) {
					for(register size_t j(0); j < Input_Size; ++j) {
						grad_wt_e[i][j] += err_hid[t][i] * inp[j];
					}
					grad_bias_hid[i] += err_hid[t][i];
				}
			}
		}
		
		void tune() {
			//wt_e
			for(register size_t i(0); i < Hidden_Size; ++i) {
				for(register size_t j(0); j < Input_Size; ++j) {
					wt_e[i][j] -= l_rate * grad_wt_e[i][j];
					grad_wt_e[i][j] *= m_rate;
				}
			}
			//wt_h
			for(register size_t i(0); i < Hidden_Size; ++i) {
				for(register size_t j(0); j < Hidden_Size; ++j) {
					wt_h[i][j] -= l_rate * grad_wt_h[i][j];
					grad_wt_h[i][j] *= m_rate;
				}
			}
			//wt_o
			for(register size_t i(0); i < Input_Size; ++i) {
				for(register size_t j(0); j < Hidden_Size; ++j) {
					wt_o[i][j] -= l_rate * grad_wt_o[i][j];
					grad_wt_o[i][j] *= m_rate;
				}
			}
			//bias_h
			for(register size_t i(0); i < Hidden_Size; ++i) {
				bias_hid[i] -= l_rate * grad_bias_hid[i];
				grad_bias_hid[i] *= m_rate;
			}
			//bias_o
			for(register size_t i(0); i < Input_Size; ++i) {
				bias_out[i] -= l_rate * grad_bias_out[i];
				grad_bias_out[i] *= m_rate;
			}
		}
	public:
		RNN() {
			//wt_e
			for(register size_t i(0); i < Hidden_Size; ++i) {
				for(register size_t j(0); j < Input_Size; ++j) {
					wt_e[i][j] = rand_double() * 0.2;
				}
			}
			//wt_h
			for(register size_t i(0); i < Hidden_Size; ++i) {
				for(register size_t j(0); j < Hidden_Size; ++j) {
					wt_h[i][j] = rand_double() * 0.2;
				}
			}
			//wt_o
			for(register size_t i(0); i < Input_Size; ++i) {
				for(register size_t j(0); j < Hidden_Size; ++j) {
					wt_o[i][j] = rand_double() * 0.2;
				}
			}
			//bias_h
			for(register size_t i(0); i < Hidden_Size; ++i) {
				bias_hid[i] = rand_double() * 0.1;
			}
			//bias_o
			for(register size_t i(0); i < Input_Size; ++i) {
				bias_out[i] = rand_double() * 0.1;
			}
		}
		
		double train(double (*vct)[Input_Size], size_t Sz) {
			double ret = 0;
			for(register size_t i(0); i < Sz; ++i) {
				fwd_prop(vct[i]);
				ret += Diff_Error(vct[i]);
				bwd_prop();
			}
			tune();
			return ret / Sz;
		}
		
		void load(FILE* fin) {
			fread(wt_e, sizeof(double), Hidden_Size * Input_Size, fin);
			fread(wt_h, sizeof(double), Hidden_Size * Hidden_Size, fin);
			fread(wt_o, sizeof(double), Input_Size * Hidden_Size, fin);
			fread(bias_hid, sizeof(double), Hidden_Size, fin);
			fread(bias_out, sizeof(double), Input_Size, fin);
		}
		
		void save(FILE* fout) const {
			fwrite(wt_e, sizeof(double), Hidden_Size * Input_Size, fout);
			fwrite(wt_h, sizeof(double), Hidden_Size * Hidden_Size, fout);
			fwrite(wt_o, sizeof(double), Input_Size * Hidden_Size, fout);
			fwrite(bias_hid, sizeof(double), Hidden_Size, fout);
			fwrite(bias_out, sizeof(double), Input_Size, fout);
		}
		
		void test(double vct[Input_Size]) {
			fwd_prop(vct);
			for(register size_t t(0); t < Recur_Size; ++t) {
				for(register size_t i(0); i < Input_Size; ++i) {
					printf("%lf ", out[t][i]);
				}
				putchar('\n');
			}
		}
};

double arr[Sample_Size][Input_Size];

void build() {
	for(register size_t i(0); i < Sample_Size; ++i) {
		for(register size_t j(0); j < Input_Size; ++j) {
			arr[i][j] = rand_double() * 2.0;	//[-1.0, 1.0)
		}
	}
}

//#define TRAINING

int main() {
	RNN My_RNN;
	build();
#ifdef TRAINING					//Training
	double tot_err;
	size_t cnt(0);
	do {
		++cnt;
		tot_err = My_RNN.train(arr, Sample_Size);
		if((cnt & 0x1ff) == 0) {
			printf("Epoch = %zu, Loss = %.9lf\n", cnt, tot_err);
			l_rate *= 0.99;
		}
	} while(tot_err > 2e-7);
	FILE* fout = fopen("RNN_save.bin", "wb");
	My_RNN.save(fout);
	fclose(fout);
#else							//Testing
	FILE* fin = fopen("RNN_save.bin", "rb");
	My_RNN.load(fin);
	fclose(fin);
	
	double inp[Input_Size];
	
	printf("Input %d double in range (-1.0, 1.0) >>> ", Input_Size);
	while(~scanf("%lf%lf%lf", inp, inp + 1, inp + 2)) {
		My_RNN.test(inp);
		printf("Input %d double in range (-1.0, 1.0) >>> ", Input_Size);
	}
#endif
	
	return 0;
}
