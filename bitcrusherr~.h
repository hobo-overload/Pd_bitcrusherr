//
//  bitcrusherr~.h
//  Pd_bitcrusherr~
//
//  Copyright © 2024 Clint Woker. All rights reserved.
//

#pragma once
#include "m_pd.h"

typedef struct _bitcrusherr_tilde {
    t_object  x_obj;
    t_sample f_reduction_factor;
    t_sample f;
    
    t_inlet  *x_in2;
    t_outlet *x_out;
    
    // In order to "bit crush" the sampling rate, we need
    // somewhere to stash the last sample value
    // of the block of samples. We also need to stash the remaining
    // span of samples where the value should then be applied, since
    // the last value of the block may need to span
    // multiple multiple sample blocks if the inlet (x_in2)
    // value isn't equally divisible by Pd's sample block size
    // (typically 64 samples unless otherwise re-configured).
    t_sample buffer_carry_sample_value;
    t_int buffer_carry_sample_count;
} t_bitcrusherr_tilde;

void bitcrusherr_tilde_setup(void);
void *bitcrusherr_tilde_new(t_floatarg reduction_factor);
void bitcrusherr_tilde_free(t_bitcrusherr_tilde *x);
void bitcrusherr_tilde_dsp(t_bitcrusherr_tilde *x, t_signal **sp);
t_int *bitcrusherr_tilde_perform(t_int *w);
