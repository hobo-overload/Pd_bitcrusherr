# Pd_bitcrusherr
A sampling-rate bitcrusher external for Pd

## Background information
General background information on bitcrush audio effects is available on [Wikipedia](https://en.wikipedia.org/wiki/Bitcrusher).
General background information and downloads for Pure Data is available at [puredata.info](https://puredata.info).

## Build information
An XCode project is provided to build the library. Alternatively a pre-built version is available in the "prebuilt" directory of the repo.
To install the prebuilt library, perform the following steps. An Apple Silicon Mac is assumed:
1) Install the vanilla version of [Pure Data](https://puredata.info/downloads/pure-data)
2) Create a ```~/Documents/Pd/externals/bitcrusherr~``` directory
3) Copy ```<repo>/prebuilt/bitcrusherr~.pd_darwin``` into the ```~/Documents/Pd/externals/bitcrusherr~``` directory.
4) In Pd Preferences, select Path. If not already added, add ```~/Documents/Pd/externals```.
5) Open <repo>/test patches/bitcrusherr~_test.pd in Pd. Activate DSP in Pd, and use the provided Pd number control to adjust the bitcrush sampling-rate reduction factor value on the provided 440 Hz oscillator. The graph included in the patch will illustrate the effect on the 440 Hz waveform.

## Algorithm information
Refer to the ```bitcrusherr_tilde_perform``` function in ```<repo>/bitcrusherr~.c``` for implementation and explanation of the algorithm.
