#include <math.h>
#include "audio_leveler.h"


static float AdjLimit[LEVELER_FACTORS];
static float AddOnValue[LEVELER_FACTORS];
static float Limit[LEVELER_FACTORS] = { 0.0001, 0.0002, 0.1, 0.3, 0.5, 1.0 };
//static float AdjFactor[LEVELER_FACTORS] = { 0.80, 1.00, 1.20, 1.20, 1.00, 0.80 };
static float AdjFactor[LEVELER_FACTORS] = { 0.70, 0.90, 1.50, 1.50, 0.90, 0.70 };

static int leveler_init;

int highpass_boost_filter_on=1;
int leveler_passes=2;
float hf_boost_level=0.18f;	//good values are <= 0.25f;	// should use fft to make auto-hf frequency boost from 0.0 to 0.25f

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
void init_leveler(void) {

   int    prev          = 0;
   float addOnValue    = 0.0;
   float prevLimit     = 0.0;
   float limit         = Limit[0];
   AddOnValue[0]       = addOnValue;
   float adjFactor     = AdjFactor[0];
   float upperAdjLimit = Limit[0] * adjFactor;
   float prevAdjLimit  = upperAdjLimit;
   AdjLimit[0]         = upperAdjLimit;

   for (int f = 1; f < LEVELER_FACTORS; ++f) {
      prev          = f - 1;
      adjFactor     = AdjFactor[f];
      prevLimit     = Limit[prev];
      limit         = Limit[f];
      prevAdjLimit  = AdjLimit[prev];
      addOnValue    = prevAdjLimit - (adjFactor * prevLimit);

      AddOnValue[f] = addOnValue;
      AdjLimit[f]   = (adjFactor * limit) + addOnValue;
   }
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
void audio_leveler_execute(float *buffer, int len, float hf) {

int i;
int j;
	
	hf_boost_level = hf;

	 if(!leveler_init) {
		 leveler_init=1;
		 init_leveler();
	 }

	 float in;

	//n passes
	for(i=0;i<len;i++) {
		in = buffer[i];
		for(j=0;j<leveler_passes;j++) {
			in = leveler_adj(in);
		}
		buffer[i] = in;
	}


	//high freq boost
	if(highpass_boost_filter_on) {
		float last_input=buffer[0];
		for(i=1;i<len;i++) {
			float output = buffer[i] - 0.95f * last_input;
			last_input = buffer[i];
			buffer[i] += output*hf_boost_level*1.2;
		}
	}

	//n passes
	for(i=0;i<len;i++) {
		in = buffer[i];
		for(j=0;j<leveler_passes;j++) {
			in = leveler_adj(in);
		}
		buffer[i] = in;
	}


}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
float leveler_adj(float val) {
float  val_abs;
float  val_sign;

	 if (val < 0.0) {
			val_sign = -1.0;
	 }
	 else {
			val_sign = 1.0;
	 }
	 val_abs = (float)fabs(val);

	 for (int f = 0; f < LEVELER_FACTORS; ++f) {
		 if (val_abs <= Limit[f]) {
				val *= (float)AdjFactor[f];
				val += (float)(AddOnValue[f] * val_sign);
				return val;
		 }
	 }
	 return 0.99f;
}
