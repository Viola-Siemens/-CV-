#include "LossFunctions.h"

double DiffSquare(double* grads, double const* out, double const* ans, size_t sz) {
	double E_tot = 0;
	for(register size_t i(0u); i < sz; ++i) {
		grads[i] = out[i] - ans[i];
		E_tot += 0.5 * grads[i] * grads[i];
	}
	return E_tot;
}
/*
double CrossEntropy(double* grads, double const* out, double const* ans, size_t sz) {
	double E_tot = 0;
	for(register size_t i(0u); i < sz; ++i) {
		grads[i] = -ans[i] / out[i];
		E_tot -= ans[i] * log(out[i]);
	}
	return E_tot;
}*/

double CrossEntropy(double* grads, double const* out, double const* ans, size_t sz) {
	double E_tot = 0;
	for(register size_t i(0u); i < sz; ++i) {
		grads[i] =
			(ans[i] - 1.0) / (out[i] - 1.0) - 
			ans[i] / out[i];
		E_tot -=
			ans[i] * log(out[i]) + 
			(1.0 - ans[i]) * log(1.0 - out[i]);
	}
	return E_tot;
}
