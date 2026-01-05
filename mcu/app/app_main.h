#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

  /*
   * app 层入口
   *
   * 设计目的：
   * - core/main.c 只负责芯片启动、时钟与 RTOS 启动
   * - 业务/界面初始化由 app 层统一编排
   */

  void app_init(void);
  void app_start(void);

#ifdef __cplusplus
}
#endif
