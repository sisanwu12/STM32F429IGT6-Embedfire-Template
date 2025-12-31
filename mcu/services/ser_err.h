/**
 * @file		错误定义与处理头文件
 * @brief		用于错误类型的定义与处理函数的声明
 * @author	王广平
 * @date		2025/12/31
 */
#ifndef __ERR_H
#define __ERR_H

/* 头文件引用 */

/* 错误类型枚举定义 */
// clang-format off

/* 初始化结果类型 */
typedef enum RES_INIT
{
	INIT_FINISHED = 0x00U,
	INIT_ERROR		= 0x01U,
} RES_Init;

/* 运行错误结果类型 */
typedef enum RES_RUN
{
	RUN_FINISHED 		= 0x00U,  /* 成功执行 */
	RUN_ERROR_UNST	= 0x01U,	/* 对应模块未初始化 */
	RUN_BUSY     		= 0x02U,
  RUN_TIMEOUT			= 0x03U,
	RUN_ERROR_UDIP	= 0x04U,	/* 未定义的输入参数 */
	RUN_ERROR_CALL	= 0x05U,	/* 错误的函数调用 */
	RUN_ERROR_ERIP  = 0x06U,	/* 错误的输入参数 */
} RES_RUN;
// clang-format on

#endif