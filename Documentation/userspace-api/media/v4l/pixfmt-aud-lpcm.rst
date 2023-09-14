.. SPDX-License-Identifier: GFDL-1.1-no-invariants-or-later

.. _v4l2-aud-fmt-lpcm:

*************************
V4L2_AUDIO_FMT_LPCM ('LPCM')
*************************

Linear Pulse-Code Modulation (LPCM)


Description
===========

This describes audio format used by the audio memory to memory driver.

It contains the following fields:

.. flat-table::
    :widths: 1 4
    :header-rows:  1
    :stub-columns: 0

    * - Field
      - Description
    * - u32 samplerate;
      - which is the number of times per second that samples are taken.
    * - u32 sampleformat;
      - which determines the number of possible digital values that can be used to represent each sample
    * - u32 channels;
      - channel number for each sample.
