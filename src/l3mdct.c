/* L3mdct */

#include "types.h"
#include "l3mdct.h"
#include "l3subband.h"

/* This is table B.9: coefficients for aliasing reduction */
#define MDCT_CA(coef)	(int32_t)(coef / sqrt(1.0 + (coef * coef)) * 0x7fffffff)
#define MDCT_CS(coef)	(int32_t)(1.0  / sqrt(1.0 + (coef * coef)) * 0x7fffffff)
#define MDCT_PAIR(coef) {MDCT_CA(coef), MDCT_CS(coef)}
static const int32_t MDCT_CA_CS[8][2] = {
  MDCT_PAIR(-0.6),
  MDCT_PAIR(-0.535),
  MDCT_PAIR(-0.33),
  MDCT_PAIR(-0.185),
  MDCT_PAIR(-0.095),
  MDCT_PAIR(-0.041),
  MDCT_PAIR(-0.0142),
  MDCT_PAIR(-0.0037)
};

/*
 * shine_mdct_initialise:
 * -------------------
 */
void shine_mdct_initialise(shine_global_config *config)
{
  int m,k;

  /* prepare the mdct coefficients */
  for(m=18; m--; )
    for(k=36; k--; )
      /* combine window and mdct coefficients into a single table */
      /* scale and convert to fixed point before storing */
      config->mdct.cos_l[m][k] = (int32_t)(sin(PI36*(k+0.5))
                                      * cos((PI/72)*(2*k+19)*(2*m+1)) * 0x7fffffff);
}

/*
 * shine_mdct_sub:
 * ------------
 */
void SHINE_IRAM shine_mdct_sub(shine_global_config *config, int stride)
{
  /* note. we wish to access the array 'config->mdct_freq[2][2][576]' as
   * [2][2][32][18]. (32*18=576),
   */
  int32_t (*mdct_enc)[18];

  int  ch,gr,band,j,k;
  int32_t mdct_in[36];

  for(ch=config->wave.channels; ch--; )
  {
    for(gr=0; gr<config->mpeg.granules_per_frame; gr++)
    {
      /* set up pointer to the part of config->mdct_freq we're using */
      mdct_enc = (int32_t (*)[18]) config->mdct_freq[ch][gr];

      /* polyphase filtering */
      for(k=0; k<18; k+=2)
      {
      	shine_window_filter_subband(&config->buffer[ch], &config->l3_sb_sample[ch][gr+1][k  ][0], ch, config, stride);
      	shine_window_filter_subband(&config->buffer[ch], &config->l3_sb_sample[ch][gr+1][k+1][0], ch, config, stride);
        /* Compensate for inversion in the analysis filter
         * (every odd index of band AND k)
         */
        for(band=1; band<32; band+=2)
          config->l3_sb_sample[ch][gr+1][k+1][band] *= -1;
      }

      /* Perform imdct of 18 previous subband samples + 18 current subband samples */
      for(band=0; band<32; band++)
      {
        for(k=18; k--; )
        {
          mdct_in[k   ] = config->l3_sb_sample[ch][gr  ][k][band];
          mdct_in[k+18] = config->l3_sb_sample[ch][gr+1][k][band];
        }

        /* Calculation of the MDCT
         * In the case of long blocks ( block_type 0,1,3 ) there are
         * 36 coefficients in the time domain and 18 in the frequency
         * domain.
         */

        for(k=18; k--; )
        {
          int32_t vm;
          uint32_t vm_lo;

          mul0(vm, vm_lo, mdct_in[35], config->mdct.cos_l[k][35]);
          for(j=35; j; j-=7) {
            muladd(vm, vm_lo, mdct_in[j-1], config->mdct.cos_l[k][j-1]);
            muladd(vm, vm_lo, mdct_in[j-2], config->mdct.cos_l[k][j-2]);
            muladd(vm, vm_lo, mdct_in[j-3], config->mdct.cos_l[k][j-3]);
            muladd(vm, vm_lo, mdct_in[j-4], config->mdct.cos_l[k][j-4]);
            muladd(vm, vm_lo, mdct_in[j-5], config->mdct.cos_l[k][j-5]);
            muladd(vm, vm_lo, mdct_in[j-6], config->mdct.cos_l[k][j-6]);
            muladd(vm, vm_lo, mdct_in[j-7], config->mdct.cos_l[k][j-7]);
          }
          mulz(vm, vm_lo);
          mdct_enc[band][k] = vm;
        }

        if (band == 0) continue;
        /* Perform aliasing reduction butterfly */
        asm ("#cmuls:");
        for(k=0; k<8; k++)
          cmuls(mdct_enc[band][k], mdct_enc[band-1][17-k], mdct_enc[band][k], mdct_enc[band-1][17-k], MDCT_CA_CS[k][1], MDCT_CA_CS[k][0]);
      }
    }

    /* Save latest granule's subband samples to be used in the next mdct call */
    memcpy(config->l3_sb_sample[ch][0], config->l3_sb_sample[ch][config->mpeg.granules_per_frame], sizeof(config->l3_sb_sample[0][0]));
  }
}
