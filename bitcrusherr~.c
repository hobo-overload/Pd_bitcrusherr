//
//  bitcrusherr~.c
//  Pd_bitcrusherr~
//
//  Copyright © 2024 Clint Woker. All rights reserved.
//
//  This file implements a Pd external to perform a "bit crush"
//  audio effect by reducing the effective sampling rate of the incoming
//  audio signal.
//  The first (left) inlet is the audio signal
//  (thus the tilde naming, per typical Pd naming convention).
//  The second (right) inlet is the number of reduction factor, which indicates how
//  how many samples to repeat in a row.

//  One can think of this Pd external as performing a "horizontal" reduction of possible values
//  on an audio waveform, where the waveform has time on the x-axis and amplitude on the y-axis.

#include <stdlib.h>
#include <math.h>
#include "bitcrusherr~.h"

static t_class *bitcrusherr_tilde_class;

// boilerplate setup of the external symbol in Pd.
// The external will be available in Pd as "bitcrusherr~".
void bitcrusherr_tilde_setup(void) {
    bitcrusherr_tilde_class = class_new(gensym("bitcrusherr~"),
                                       (t_newmethod)bitcrusherr_tilde_new,
                                       (t_method)bitcrusherr_tilde_free,
                                       sizeof(t_bitcrusherr_tilde),
                                       CLASS_DEFAULT,
                                       A_DEFFLOAT, 0);
    class_addmethod(bitcrusherr_tilde_class,
                    (t_method)bitcrusherr_tilde_dsp, gensym("dsp"), A_CANT, 0);
    CLASS_MAINSIGNALIN(bitcrusherr_tilde_class, t_bitcrusherr_tilde, f);
}

// Establish a floating point right inlet for sample reduction factor,
// and a signal as output. Also starts with the initial sample carryover count and
// value as 0 (no current carryover).
void *bitcrusherr_tilde_new(t_floatarg reduction_factor)
{
    t_bitcrusherr_tilde *x = (t_bitcrusherr_tilde *)pd_new(bitcrusherr_tilde_class);
    x->f_reduction_factor = reduction_factor;
    x->x_in2 = floatinlet_new(&x->x_obj, &x->f_reduction_factor);
    x->x_out = outlet_new(&x->x_obj, &s_signal);
    x->buffer_carry_sample_count = 0;
    x->buffer_carry_sample_value = 0;
    return (void *)x;
}

// Cleanup operations for inlets and outlets which
// were created in the ..._new function.
void bitcrusherr_tilde_free(t_bitcrusherr_tilde *x)
{
    inlet_free(x->x_in2);
    outlet_free(x->x_out);
}

// Boilerplate operation to add a signal input to the left inlet.
void bitcrusherr_tilde_dsp(t_bitcrusherr_tilde *x, t_signal **sp)
{
    dsp_add(bitcrusherr_tilde_perform, 4, x,
            sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

// Rate bitcrusher algorithm.
// Pd issues a callback on this function for every batch of samples received on the left (DSP) inlet.
// Note that inputs and output signals are floating point values!
t_int *bitcrusherr_tilde_perform(t_int *w)
{
    t_bitcrusherr_tilde *x = (t_bitcrusherr_tilde *)(w[1]);
    t_sample *in1 = (t_sample *)(w[2]);
    t_sample *out = (t_sample *)(w[3]);
    t_int n = w[4];
    int batch_size = abs((int)(roundf(x->f_reduction_factor)));
    
    // Avoid any possibility of the input value resulting in a
    // divide-by-0 error; "incorrect" audio is preferable to crashing the
    // calling Pd application:
    if (batch_size == 0)
    {
        batch_size = 1;
    }
    t_int remaining_samples = n;
    
    // Compute the number of recurring samples to apply *in this batch*.
    // Obviously this value can be no larger than the size of the batch itself:
    t_int current_buffer_carry_sample_count = (x->buffer_carry_sample_count <= n ? x->buffer_carry_sample_count : n);
    
    // If we have carryover samples from the last batch, fill those
    // out first with the stashed value from the final sample of the
    // last batch:
    for (t_int i = 0; i < current_buffer_carry_sample_count; ++i)
    {
        *out = x->buffer_carry_sample_value;
        (void) *in1++;
        (void) *out++;
        remaining_samples--;
    }
    
    // If there are still remaining samples in this batch:
    if (remaining_samples > 0)
    {
        // batch_count indicates the unique number of fully-filled sample values
        // remaining for this batch of samples:
        t_int batch_count = remaining_samples / batch_size;
        // partial_buffer_samples indicates the remaining samples in this batch
        // which will need to also be carried over into the next batch:
        t_int partial_buffer_samples = remaining_samples % batch_size;
        for (int i = 0; i < batch_count; ++i)
        {
            t_sample current_sample = *in1;
            for (int j = 0; j < batch_size; ++j)
            {
                // In theory, I could take an average of all the samples
                // in the batch for a "smoother" bitcrush.
                // In practice there's no way to do it when spanning batches,
                // since Pd does not provide a lookahead into the values
                // forthcoming in the next batch.
                // The result is a bit "crunchier" sound output.
                // So, I just use the first sample in the set of samples to be
                // reduced, and repeat that, since this strategy works both
                // within and across sample batches:
                *out = current_sample;
                (void) *in1++;
                (void) *out++;
            }
        }
        // If we need to carryover repeated sample values in the next batch,
        // we do that by filling out the remaining samples in this batch,
        // then setting the carry count and value for application
        // on the beginning of the next batch:
        if (partial_buffer_samples > 0)
        {
            t_sample current_sample = *in1;
            for (int i = 0; i < partial_buffer_samples; ++i)
            {
                *out = current_sample;
                (void) *in1++;
                (void) *out++;
            }
            x->buffer_carry_sample_count = batch_size - partial_buffer_samples;
            x->buffer_carry_sample_value = current_sample;
        }
        // else our reduction factor is equally divisible by the Pd
        // sample block size (typically 64):
        else
        {
            x->buffer_carry_sample_count = 0;
            x->buffer_carry_sample_value = 0;
        }
    }
    // else we have no remaining samples in this batch:
    else
    {
        // If the carryover sample count is greater than the number
        // of samples in this batch, subtract out the batch size and
        // move on to the next batch of samples:
        if (x->buffer_carry_sample_count > n)
        {
            x->buffer_carry_sample_count -= n;
        }
        // else we have no samples to carry over into the next batch,
        // so zero out the carryover sample count and value:
        else
        {
            x->buffer_carry_sample_count = 0;
            x->buffer_carry_sample_value = 0;
        }
    }
    // return the pointer to the next set of block information, per Pd convention:
    return (w+5);
}
