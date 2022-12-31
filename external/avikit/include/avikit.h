// AVIKit
// ---------------------------------------------------------------------------
//
// Copyright (c) 2003, Ruari O'Sullivan
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
// 
// Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution. 
// 
// Neither the name of AVIKit's copyright owner nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission. 
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// ---------------------------------------------------------------------------
//
// Using AVIKit:
//
// - you must link the following libraries: vfw32.lib, msacm32.lib, avikit.lib.
// - if you initialise VFW yourself, define AVIKIT_NOINIT_VFW.

#ifndef AVIKIT_HEADER
#define AVIKIT_HEADER

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vfw.h> // video for windows

#include <list>



enum
{
  AVIKIT_NOERROR = 0,

  // errors opening file
  AVIKIT_ERROR_BADFILE, // "No handler found, or file does not exist."
  AVIKIT_ERROR_BADFORMAT, // "Corrupt AVI or unknown format."
  AVIKIT_ERROR_MEMERROR, // "Not enough memory to open AVI."
  AVIKIT_ERROR_DISKERROR, // "Disk error attempting to read AVI."

  // errors opening video
  AVIKIT_ERROR_NOVIDEO, // "No video stream found in specified file."
  AVIKIT_ERROR_VIDEOERROR, // "VFW failed to read video stream."

  // errors opening audio
  AVIKIT_ERROR_ACM, // "ACM failed to open conversion stream."
  AVIKIT_ERROR_ACMCODEC // "ACM does not support this audio codec." (WMA).
};

#ifndef AVIKIT_NOINIT_VFW
static long avikits = 0; // reference counter for VFW use
#endif

class AVIKit
{
private:
  bool active; // "true" if loaded correctly, "false" otherwise

  PAVIFILE pfile; // avi file pointer

  PAVISTREAM video_stream; // video stream pointer
  PGETFRAME	video_getframe; // pointer to getframe object for video stream
  long video_frames; // total frames
  long video_xres, video_yres; // video stream resolution
  float video_fps; // video stream fps

  PAVISTREAM audio_stream; // audio stream pointer
  WAVEFORMAT *audio_header; // audio stream header
  long audio_header_size; // WAVEFORMAT is returned for PCM data; WAVEFORMATEX for others
  long audio_codec; // WAVE_FORMAT_PCM is oldstyle: anything else needs conversion
  long audio_length; // in converted samples

  long audio_bytes_per_sample; // guess.

  // compressed audio specific data
  unsigned long cpa_blockalign; // block size to read
  HACMSTREAM cpa_conversion_stream;
  ACMSTREAMHEADER cpa_conversion_header;
  unsigned char *cpa_srcbuffer, *cpa_dstbuffer; // maintained buffer for raw data

  unsigned long cpa_blocknum; // current block
  unsigned long cpa_blockpos; // read position in current block
  unsigned long cpa_blockoffset; // corresponding offset in bytes in the output stream

  std::list<int> errorList;
  void addError(int error);

  // Converts a compressed audio stream into uncompressed PCM.
  bool ACMConvertAudio(PAVISTREAM audio_stream, WAVEFORMAT *audio_header, short *bits);
  bool seekPosition(unsigned long offset); // sync the current audio read to a specific offset

public:
  AVIKit(const char *filename, bool load_audio = true);
  ~AVIKit(void); // clean up and leave.

  int getError(char **emsg); // get errors in order of occurrance.

  bool getVideoInfo(long *xres, long *yres, float *duration); // gets basic information about video
  long getVideoFrameNumber(float time); // returns a unique frame identifier
  void getVideoFrame(char *framedata, long frame); // gets the raw frame data

  bool getAudioInfo(long *frequency, long *bytes_per_sample, long *channels, bool *is16bit); // gets audio info
  void getAudioChunk(char *audiodata, long offset, long length); // get a chunk of audio from the stream
};

class AVIKitBuffer
{
private:
  AVIKit *avi;

  long s_buffer_size;
  long s_buffer_data_start, s_buffer_data_end; // markers for data positions in buffer
  long s_stream_data_start, s_stream_data_end; // relative positions in the driving stream

  long bytes_per_sample;
  long channels;
  long frequency;

  char *bytedump;
  long bytedump_length;

public:
  AVIKitBuffer(AVIKit *avi, long s_buffer_size); // init from the avi itself
  ~AVIKitBuffer(void); // clean up data

  // passthrough to the AVIKit info thing.
  bool getAudioInfo(long *frequency, long *bytes_per_sample, long *channels, bool *is16bit); // gets audio info

  // updates internal buffer data, and tells you what you need to fetch.
  void makeUpdate(long playpos, float time, long *s_update_pos, long *s_update_length);

  // functions for fetching raw data from the internal buffer.
  void getUpdate(char *rawbuffer, long s_update_pos, long s_update_length);
  void getUpdate(char *leftbuffer, char *rightbuffer, long s_update_pos, long s_update_length);
};

#endif
