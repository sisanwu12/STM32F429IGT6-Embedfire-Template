#ifndef _LOG_H_
#define _LOG_H_

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef G_LOG
#define G_LOG extern
#endif
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

  // #include "cal.h"
  // #include "crm.h"
  // #include "mal.h"
  // #include "streammatrix.h"
  // #include "sysfs.h"

#define exit DBG_EXIT
#define CATY_TAG __FUNCTION__
#define CATY_MODULE _THIS_MODULE_NAME_

#define LOGNONE -1
#define LOGDEBUG 0
#define LOGINFO 1
#define LOGWARN 2
#define LOGERROR 3
#define LOGCRIT 4

#define LOG_LINE_NUM
#define _LOG_MSG_LEN_MAX 1096
#define _CATY_LEN_MAX 512

  typedef uint8_t u8;
  typedef char cx8;
  typedef char _x8_;
  typedef size_t nsize_t;

  typedef enum
  {
    bFALSE = 0,
    bTRUE = 1
  } boolean_e;

  static u8 GLogLevel = LOGDEBUG;
  static boolean_e GLogIfLineNum = bFALSE;
  static nsize_t GLogRowMax = 80;
  static nsize_t GLogLineMax = 24;

  /*
   字背景颜色范围:	40--49		字颜色: 30--39
   40: 黑		30: 黑
   41: 红		31: 红
   42: 绿		32: 绿
   43: 黄		33: 黄
   44: 蓝		34: 蓝
   45: 紫		35: 紫
   46: 深绿		36: 深绿
   47: 白色		37: 白色
   */

#define COLOR_NONE "\033[0m"
#define FONT_COLOR_WHITE "\033[0;37m"
#define FONT_COLOR_GREEN "\033[0;32m"
#define FONT_COLOR_RED "\033[0;31m"
#define FONT_COLOR_YELLOW "\033[0;33m"
#define FONT_COLOR_BLUE "\033[0;34m"

#define FONT_COLOR_GREEN_UDL "\033[0;32;4m"
#define FONT_COLOR_RED_UDL "\033[0;31;4m"

#define FONT_CP_YELLOW "\033[0;33m"

#define FONT_BLINK "\033[5m"
#define FONT_CUR_OFF "\033[?25l"
#define FONT_CUR_ON "\033[?25h"
#define FONT_CLEAR "\033[2j"
#define FONT_RESV "\033[7m"
#define FONT_CLEAR_SCREEN_NEW "\033[2j\033[0;0h"
#define FONT_CLEAR_CUR2LINE "\033[k"

#define BG_COLOR_RED "\033[41m"
#define BG_RED_FONT_YELLOW "\033[41;33m"
#define BG_WHITE_FONT_RED "\033[47;31m"
#define BG_YELLOW_FONT_BLACK "\033[43;30m"
#define BG_GREEN_FONT_BLACK "\033[42;30m"
#define BG_BLACK_FONT_WHITE "\033[40;37m"
#define BG_RED_FONT_BLACK "\033[41;30m"

#ifdef IF_IN_BIC
  G_LOG void _log_(u8 level, _x8_ *format, ...);
#define LOG(level, format, ...) _log_((level), (format), ##__VA_ARGS__)
#else
static cx8 *_LOG_STR[5][2] = {{COLOR_NONE, "D"},
                              {BG_GREEN_FONT_BLACK, "I"},
                              {BG_YELLOW_FONT_BLACK, "W"},
                              {BG_RED_FONT_BLACK, "E"},
                              {BG_WHITE_FONT_RED, "C"}};
#define LOG(level, format, ...)                                                \
  do                                                                           \
  {                                                                            \
    if (level >= GLogLevel)                                                    \
    {                                                                          \
      time_t __now__ = time(NULL);                                             \
      _x8_ __msg__[_LOG_MSG_LEN_MAX];                                          \
      _x8_ __buf__[32];                                                        \
      snprintf(__msg__, _LOG_MSG_LEN_MAX, format, ##__VA_ARGS__);              \
      strftime(__buf__, sizeof(__buf__), "%Y%m%d %H:%M:%S",                    \
               localtime(&__now__));                                           \
      (GLogIfLineNum                                                           \
           ? fprintf(stdout, "%s[%s] [%s] [%s:%d] %s" COLOR_NONE "\n",         \
                     _LOG_STR[level][0], __buf__, _LOG_STR[level][1],          \
                     __FILE__, __LINE__, __msg__)                              \
           : fprintf(stdout, "%s[%s] [%s] %s" COLOR_NONE "\n",                 \
                     _LOG_STR[level][0], __buf__, _LOG_STR[level][1],          \
                     __msg__));                                                \
      fflush(stdout);                                                          \
    }                                                                          \
    if (level >= LOGCRIT)                                                      \
    {                                                                          \
      exit(-1);                                                                \
    }                                                                          \
  } while (0)
#endif

/*
#define TINYPIG( dst, level, tag, format, ...) do{ \
if (level >= GLogLevel) {\
s8 catymsg[_TINYPIG_LEN_MAX];                        \
sprintf(catymsg, format, ##__VA_ARGS__);             \
if(dst){catyshm_put(dst,level,(u8*)tag,(u8*)catymsg);}\
}\
if (level >= LOGCRIT) {\
exit(-1); \
} \
} while(0)
*/
#if 0
#define CATY(tag, level, format, ...)                                          \
  do                                                                           \
  {                                                                            \
    s8 catymsg[_CATY_LEN_MAX];                                                 \
    snprintf(catymsg, _CATY_LEN_MAX, format, ##__VA_ARGS__);                   \
    {                                                                          \
      caty->write(0, (u8 *)catymsg, level, (osword_t *)(tag));                 \
    }                                                                          \
  } while (0)

#define TINYCATY(tag, level, catymsg)                                          \
  do                                                                           \
  {                                                                            \
    {                                                                          \
      caty->write(0, (u8 *)(catymsg), level, (osword_t *)(tag));               \
    }                                                                          \
  } while (0)
#else
#define CATY(tag, level, format, ...)                                          \
  do                                                                           \
  {                                                                            \
    printf(format, ##__VA_ARGS__);                                             \
    printf("\r\n");                                                            \
  } while (0)

#define TINYCATY(tag, level, catymsg)                                          \
  do                                                                           \
  {                                                                            \
    (void)(level);                                                             \
    printf("%s%s\r\n", tag, catymsg);                                          \
  } while (0)
#endif

#define LOGLine() LOG(LOGDEBUG, "%s(),%s:%d", __FUNCTION__, __FILE__, __LINE__)
#define CATYDEBUG(format, ...) CATY(CATY_TAG, LOGDEBUG, format, ##__VA_ARGS__)
#define CATYINFO(format, ...) CATY(CATY_TAG, LOGINFO, format, ##__VA_ARGS__)
#define CATYWARN(format, ...) CATY(CATY_TAG, LOGWARN, format, ##__VA_ARGS__)
#define CATYERROR(format, ...) CATY(CATY_TAG, LOGERROR, format, ##__VA_ARGS__)
#define CATYCRIT(format, ...) CATY(CATY_TAG, LOGCRIT, format, ##__VA_ARGS__)

#define TINYCATYDEBUG(catymsg) TINYCATY(CATY_TAG, LOGDEBUG, catymsg)
#define TINYCATYINFO(catymsg) TINYCATY(CATY_TAG, LOGINFO, catymsg)
#define TINYCATYWARN(catymsg) TINYCATY(CATY_TAG, LOGWARN, catymsg)
#define TINYCATYERROR(catymsg) TINYCATY(CATY_TAG, LOGERROR, catymsg)
#define TINYCATYCRIT(catymsg) TINYCATY(CATY_TAG, LOGCRIT, catymsg)

  G_LOG unsigned char LOG_IF_COLOR_XTERM;
  G_LOG void log_delayms_ani(volatile unsigned int ms);

  G_LOG unsigned int log_sleep(unsigned int s);
  G_LOG unsigned int log_usleep(unsigned int us);

#define vsleep(x) log_sleep(x)
#define vusleep(x) log_usleep(x)

#if 0
#include <stdio.h>

#define ENUM_DEF(ENUM)                                                         \
  ENUM(RED)                                                                    \
  ENUM(GREEN)                                                                  \
  ENUM(BLUE)

// 定义枚举
#define DEFINE_ENUM(name) name,
enum Color {
    ENUM_DEF(DEFINE_ENUM)
    MAX_COLOR
};

// 定义字符串数组
#define DEFINE_STRING(name) #name,
const char* color_names[] = {
    ENUM_DEF(DEFINE_STRING)
};

// 获取名称
const char* get_color_name(enum Color color) {
    if (color >= 0 && color < MAX_COLOR) {
        return color_names[color];
    }
    return "UNKNOWN";
}

int main() {
    enum Color color = BLUE;
    printf("Color: %s\n", get_color_name(color));
    return 0;
}

#endif

#ifdef __cplusplus
}
#endif
#endif
