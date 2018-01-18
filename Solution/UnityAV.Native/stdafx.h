// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include <string>
#include <queue>
#include <memory>
#include <atomic>
#include <mutex>
#include <fstream>
#include <unordered_map>
#include <ctime>

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <libavcodec/avcodec.h>
#include <libavutil/frame.h>
#include <libavutil/time.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#ifdef __cplusplus
}
#endif

#include <IUnityInterface.h>
#include <IUnityGraphics.h>

#include "Logging/Debug.h"