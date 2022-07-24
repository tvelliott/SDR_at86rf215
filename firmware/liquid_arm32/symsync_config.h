//these values correspond to a symsync with 2 bits-per-symbol, 10 samples per symbol, 0.4f beta, 8 filter banks, and RRC filter type
#define SYMSYNC_HLEN 1281		// (2 * 2 * 10 * 8)+1
#define SYMSYNC_H_SUBLEN 40	// (HLEN-1) / N_FILTERS
#define SYMSYNC_NFILTERS 32 
