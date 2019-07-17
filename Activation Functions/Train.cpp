#include <cstdio>

double rand_double() {
	static size_t seed = 19260817uLL;
	seed = seed * 20001121uLL + 4321uLL;
	return (double((seed >> 8uLL) & 0xffff) / double(0x10000)) - 0.5;
}

class RBM {
	public:
		static const int hidsz = 10;
		static double ReLU(double x) {
			return x < 0.0 ? 0.0 : x;
		}
		static double df_ReLU(double x) {
			return x < 0.0 ? 0.0 : 1.0;
		}
		
		double l_rate = 0.0002;
		double m_rate = 0.9;
		double l2_reg = 0.001;
	private:
		double inp;
		double hid[hidsz];
		double outp;
		
		double wt1[hidsz];
		double wt2[hidsz] = {1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, -1.0, -1.0, 1.0};
		double bias1[hidsz];
		double bias2 = 1.0;
		
		double grad_wt1[hidsz];
		double grad_wt2[hidsz];
		double grad_bias1[hidsz];
		double grad_bias2;
		
		void fwd_prop(double in) {
			inp = in;
			for(register int i(0); i < hidsz; ++i) {
				hid[i] = wt1[i] * inp + bias1[i];
			}
			outp = bias2;
			for(register int i(0); i < hidsz; ++i) {
				outp += wt2[i] * ReLU(hid[i]);
			}
		}
		
		double bwd_prop(double ans) {
			double los = outp - ans;
			double Loss = 0.5 * los * los;
			double err[hidsz];
			grad_bias2 += los;
			
			for(register int i(0); i < hidsz; ++i) {
				grad_wt2[i] += los * ReLU(hid[i]);
				err[i] = los * wt2[i] * df_ReLU(hid[i]);
				grad_bias1[i] += err[i];
				grad_wt1[i] += err[i] * inp;
			}
			
			return Loss;
		}
		
		void tune() {
			for(register int i(0); i < hidsz; ++i) {
				wt1[i] -= l_rate * (grad_wt1[i] + l2_reg * wt1[i]);
				bias1[i] -= l_rate * (grad_bias1[i] + l2_reg * bias1[i]);
			//	wt2[i] -= l_rate * (grad_wt2[i] + l2_reg * wt2[i]);
				
				grad_wt1[i] *= m_rate;
				grad_bias1[i] *= m_rate;
				grad_wt2[i] *= m_rate;
			}
		//	bias2 -= l_rate * (grad_bias2 + l2_reg * bias2);
			grad_bias2 *= m_rate;
		}
	public:
		RBM() {
			for(register int i(0); i < hidsz; ++i) {
				wt1[i] = rand_double();
			//	wt2[i] = rand_double();
				bias1[i] = rand_double();
				
				grad_wt1[i] = grad_wt2[i] = grad_bias1[i] = 0;
			}
		//	bias2 = rand_double();
			grad_bias2 = 0;
		}
		
		double train(double input_array[], double answer_array[], int batch) {
			double ret = 0;
			for(register int i(0); i < batch; ++i) {
				fwd_prop(input_array[i]);
				ret += bwd_prop(answer_array[i]);
			}
			tune();
			
			return ret / batch;
		}
		
		void print() {
			printf("wt1:\n\t%.3lf", wt1[0]);
			for(register int i(1); i < hidsz; ++i) {
				printf(", %.3lf", wt1[i]);
			}	putchar('\n');
			
			printf("bias1:\n\t%.3lf", bias1[0]);
			for(register int i(1); i < hidsz; ++i) {
				printf(", %.3lf", bias1[i]);
			}	putchar('\n');
			
			printf("wt2:\n\t%.3lf", wt2[0]);
			for(register int i(1); i < hidsz; ++i) {
				printf(", %.3lf", wt2[i]);
			}	putchar('\n');
			printf("bias2:\n\t%.3lf\n", bias2);
		}
};

double input_array[1024];
double answer_array[1024];


double func(double x) {
	return ((x - 2.0) * x - 1.0) * x + 1.0;
}

void getData() {
	for(register int i(0); i < 1024; ++i) {
		input_array[i] = (i - 512.0) / 512.0;
		answer_array[i] = func(input_array[i]);
	}
}

int main() {
	getData();
	RBM rbm;
	for(register int i(0); i < 32768; ++i) {
		double Loss = rbm.train(input_array, answer_array, 1024);
		if((i & 0x3ff) == 0) {
			printf("Epoch = %d, Loss = %.14lf\n", i, Loss);
			rbm.l_rate *= 0.98;
			rbm.m_rate *= 0.99;
		}
	}
	rbm.print();
	
	return 0;
}

