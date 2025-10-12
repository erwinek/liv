// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Copyright (C) 2015 Henner Zeller <h.zeller@acm.org>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation version 2.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://gnu.org/licenses/gpl-2.0.txt>

// To use this image viewer, first get image-magick development files
// $ sudo apt-get install libgraphicsmagick++-dev libwebp-dev
//
// Then compile with
// $ make led-image-viewer

#include "LedImgViewer.h"
#include "led-matrix.h"
#include "pixel-mapper.h"
#include "content-streamer.h"

#include <fcntl.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include <Magick++.h>
#include <magick/image.h>

using rgb_matrix::Canvas;
using rgb_matrix::FrameCanvas;
using rgb_matrix::RGBMatrix;
using rgb_matrix::StreamReader;

typedef int64_t tmillis_t;
static const tmillis_t distant_future = (1LL<<40); // that is a while.

volatile bool interrupt_received = false;



static tmillis_t GetTimeInMillis() {
  struct timeval tp;
  gettimeofday(&tp, NULL);
  return tp.tv_sec * 1000 + tp.tv_usec / 1000;
}

static void SleepMillis(tmillis_t milli_seconds) {
  if (milli_seconds <= 0) return;
  struct timespec ts;
  ts.tv_sec = milli_seconds / 1000;
  ts.tv_nsec = (milli_seconds % 1000) * 1000000;
  nanosleep(&ts, NULL);
}

void StoreInStream(const Magick::Image &img, int delay_time_us,
                          bool do_center,
                          rgb_matrix::FrameCanvas *scratch,
                          rgb_matrix::StreamWriter *output) {
  scratch->Clear();
  const int x_offset = do_center ? (scratch->width() - img.columns()) / 2 : 0;
  const int y_offset = do_center ? (scratch->height() - img.rows()) / 2 : 0;
  for (size_t y = 0; y < img.rows(); ++y) {
    for (size_t x = 0; x < img.columns(); ++x) {
      const Magick::Color &c = img.pixelColor(x, y);
      if (c.alphaQuantum() < 255) {
        scratch->SetPixel(x + x_offset, y + y_offset,
                          ScaleQuantumToChar(c.redQuantum()),
                          ScaleQuantumToChar(c.greenQuantum()),
                          ScaleQuantumToChar(c.blueQuantum()));
      }
    }
  }
  output->Stream(*scratch, delay_time_us);
}

bool LoadImageAndScale(const char *filename,
                              int target_width, int target_height,
                              bool fill_width, bool fill_height,
                              std::vector<Magick::Image> *result,
                              std::string *err_msg) {
  std::vector<Magick::Image> frames;
  try {
    readImages(&frames, filename);
  } catch (std::exception& e) {
    if (e.what()) *err_msg = e.what();
    return false;
  }
  if (frames.size() == 0) {
    fprintf(stderr, "No image found.");
    return false;
  }

  // Put together the animation from single frames. GIFs can have nasty
  // disposal modes, but they are handled nicely by coalesceImages()
  if (frames.size() > 1) {
    Magick::coalesceImages(result, frames.begin(), frames.end());
  } else {
    result->push_back(frames[0]);   // just a single still image.
  }

  const int img_width = (*result)[0].columns();
  const int img_height = (*result)[0].rows();
  const float width_fraction = (float)target_width / img_width;
  const float height_fraction = (float)target_height / img_height;
  if (fill_width && fill_height) {
    // Scrolling diagonally. Fill as much as we can get in available space.
    // Largest scale fraction determines that.
    const float larger_fraction = (width_fraction > height_fraction)
      ? width_fraction
      : height_fraction;
    target_width = (int) roundf(larger_fraction * img_width);
    target_height = (int) roundf(larger_fraction * img_height);
  }
  else if (fill_height) {
    // Horizontal scrolling: Make things fit in vertical space.
    // While the height constraint stays the same, we can expand to full
    // width as we scroll along that axis.
    target_width = (int) roundf(height_fraction * img_width);
  }
  else if (fill_width) {
    // dito, vertical. Make things fit in horizontal space.
    target_height = (int) roundf(width_fraction * img_height);
  }

  for (size_t i = 0; i < result->size(); ++i) {
    (*result)[i].scale(Magick::Geometry(target_width, target_height));
  }

  return true;
}

void DisplayAnimation(const FileInfo *file,
                      RGBMatrix *matrix, FrameCanvas *offscreen_canvas) {
  const tmillis_t duration_ms = (file->is_multi_frame
                                 ? file->params.anim_duration_ms
                                 : file->params.wait_ms);
  rgb_matrix::StreamReader reader(file->content_stream);
  int loops = file->params.loops;
  const tmillis_t end_time_ms = GetTimeInMillis() + duration_ms;
  const tmillis_t override_anim_delay = file->params.anim_delay_ms;
  for (int k = 0;
       (loops < 0 || k < loops)
         && !interrupt_received
         && GetTimeInMillis() < end_time_ms;
       ++k) {
    uint32_t delay_us = 0;
    while (!interrupt_received && GetTimeInMillis() <= end_time_ms
           && reader.GetNext(offscreen_canvas, &delay_us)) {
      const tmillis_t anim_delay_ms =
        override_anim_delay >= 0 ? override_anim_delay : delay_us / 1000;
      const tmillis_t start_wait_ms = GetTimeInMillis();
      offscreen_canvas = matrix->SwapOnVSync(offscreen_canvas,
                                             file->params.vsync_multiple);
      const tmillis_t time_already_spent = GetTimeInMillis() - start_wait_ms;
      SleepMillis(anim_delay_ms - time_already_spent);
    }
    reader.Rewind();
  }
}


