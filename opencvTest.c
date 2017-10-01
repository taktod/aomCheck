#include <stdio.h>
#include <ttLibC/container/mp4.h>
#include <ttLibC/decoder/avcodecDecoder.h>
#include <ttLibC/frame/video/bgr.h>
#include <ttLibC/frame/video/h264.h>
#include <ttLibC/util/opencvUtil.h>
#include <ttLibC/resampler/imageResampler.h>
#include <ttLibC/util/ioUtil.h>

typedef struct {
  ttLibC_CvWindow *window;
  ttLibC_AvcodecDecoder *decoder;
  ttLibC_Bgr *dbgr;
} data_t;

bool decodeCallback(void *ptr, ttLibC_Frame *frame) {
  data_t *data = (data_t *)ptr;
  if(frame->type != frameType_yuv420) {
    puts("only yuv420 works.");
    return false;
  }
  ttLibC_Bgr *b = ttLibC_ImageResampler_makeBgrFromYuv420(data->dbgr, BgrType_bgr, (ttLibC_Yuv420 *)frame);
  if(b == NULL) {
    puts("failed to make bgr image from yuv.");
    return false;
  }
  data->dbgr = b;
  ttLibC_CvWindow_showBgr(data->window, data->dbgr);
  uint8_t key = ttLibC_CvWindow_waitForKeyInput(1);
  if(key == Keychar_Esc) {
    puts("escape is pushed.");
    return false;;
  }
  return true;
}

bool frameCallback(void *ptr, ttLibC_Frame *frame) {
  if(frame->type == frameType_h264) {
    data_t *data = (data_t *)ptr;
    ttLibC_H264 *h264 = (ttLibC_H264 *)frame;
    if(h264->type == H264Type_unknown) {
      return true;
    }
    if(data->decoder == NULL) {
      data->decoder = ttLibC_AvcodecVideoDecoder_make(frameType_h264, h264->inherit_super.width, h264->inherit_super.height);
    }
    return ttLibC_AvcodecDecoder_decode(data->decoder, frame, decodeCallback, ptr);
  }
  return true;
}

bool mp4ReadCallback(void *ptr, ttLibC_Mp4 *mp4) {
  return ttLibC_Mp4_getFrame(mp4, frameCallback, ptr);
}

int main() {
  puts("opencvTest");
  FILE *fp = fopen("../test.h264.aac.mp4", "rb");
  ttLibC_Mp4Reader *reader = ttLibC_Mp4Reader_make();

  data_t data;
  data.window = ttLibC_CvWindow_make("imageCheck");
  data.decoder = NULL;
  data.dbgr = NULL;

  do {
    uint8_t buffer[65536];
    if(fp == NULL) {
      puts("fp is null");
      break;
    }
    size_t read_size = fread(buffer, 1, 65536, fp);
    if(!ttLibC_Mp4Reader_read(reader, buffer, read_size, mp4ReadCallback, &data)) {
      puts("error on mp4Reader.");
      break;
    }
  } while(!feof(fp));
  if(fp) {
    fclose(fp);
  }
  ttLibC_Bgr_close(&data.dbgr);
  ttLibC_AvcodecDecoder_close(&data.decoder);
#ifdef __HOGEHOGE__
  ttLibC_CvWindow_close(&data.window);
#endif
  ttLibC_Mp4Reader_close(&reader);
  return 0;
}