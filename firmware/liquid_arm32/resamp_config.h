#define RESAMP_CF_NFILTERS 13 
#define RESAMP_CF_M  13 
#define RESAMP_CF_HLEN ((2 * RESAMP_CF_M * RESAMP_CF_NFILTERS)+1)
#define RESAMP_CF_SUB_HLEN ((RESAMP_CF_HLEN-1)/RESAMP_CF_NFILTERS)

#define RESAMP_RF_NFILTERS 4 
#define RESAMP_RF_M 7 
#define RESAMP_RF_HLEN ((2 * RESAMP_RF_M * RESAMP_RF_NFILTERS)+1)
#define RESAMP_RF_SUB_HLEN ((RESAMP_RF_HLEN-1)/RESAMP_RF_NFILTERS)