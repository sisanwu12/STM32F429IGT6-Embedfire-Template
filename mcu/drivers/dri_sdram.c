#include "dri_sdram.h"

/*
 * 说明：
 * - SDRAM 基地址通常为：
 *   - FMC SDRAM Bank1: 0xC0000000
 *   - FMC SDRAM Bank2: 0xD0000000
 * - 本工程将 LTDC 帧缓冲放在 0xD0000000（Bank2），因此按 Bank2 初始化。
 */

static SDRAM_HandleTypeDef hsdram;

static void SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef *hsdram_handle)
{
  FMC_SDRAM_CommandTypeDef command;

  /* 1) 使能时钟配置命令 */
  command.CommandMode = FMC_SDRAM_CMD_CLK_ENABLE;
  command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK2;
  command.AutoRefreshNumber = 1;
  command.ModeRegisterDefinition = 0;
  HAL_SDRAM_SendCommand(hsdram_handle, &command, HAL_MAX_DELAY);

  HAL_Delay(1);

  /* 2) 预充电所有 Bank */
  command.CommandMode = FMC_SDRAM_CMD_PALL;
  command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK2;
  command.AutoRefreshNumber = 1;
  command.ModeRegisterDefinition = 0;
  HAL_SDRAM_SendCommand(hsdram_handle, &command, HAL_MAX_DELAY);

  /* 3) 自动刷新 */
  command.CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
  command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK2;
  command.AutoRefreshNumber = 8;
  command.ModeRegisterDefinition = 0;
  HAL_SDRAM_SendCommand(hsdram_handle, &command, HAL_MAX_DELAY);

  /*
   * 4) 加载模式寄存器
   *
   * W9825G6KH 等常见 SDRAM：
   * - Burst Length = 1 (0x0000)
   * - Burst Type = Sequential (0x0000)
   * - CAS Latency = 3 (0x0030)
   * - Operating Mode = Standard (0x0000)
   * - Write Burst Mode = Single (0x0200)
   *
   * 如果你的 SDRAM CAS/突发配置不同，需要按芯片手册修改该值。
   */
  uint32_t mode_register = 0x0230;

  command.CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
  command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK2;
  command.AutoRefreshNumber = 1;
  command.ModeRegisterDefinition = mode_register;
  HAL_SDRAM_SendCommand(hsdram_handle, &command, HAL_MAX_DELAY);

  /*
   * 5) 设置刷新计数器
   *
   * 刷新计数器的计算依赖 SDRAM 时钟频率（FMC 时钟）与刷新周期。
   * 这里给出一组常见参考值（90MHz 时钟、7.81us 刷新周期）：
   *   RefreshRate = 683
   *
   * 如果你修改了系统时钟 / FMC 时钟，请重新计算。
   */
  HAL_SDRAM_ProgramRefreshRate(hsdram_handle, 683);
}

HAL_StatusTypeDef dri_sdram_init(void)
{
  FMC_SDRAM_TimingTypeDef sdram_timing;

  hsdram.Instance = FMC_SDRAM_DEVICE;

  /*
   * hsdram.Init：
   * - SDBank：选择 Bank2，对应 0xD0000000（常见于 LTDC+SDRAM 共存的板卡）
   * - ColumnBitsNumber / RowBitsNumber：与 SDRAM 芯片一致
   * - MemoryDataWidth：核心板常见 16bit
   */
  hsdram.Init.SDBank = FMC_SDRAM_BANK2;
  hsdram.Init.ColumnBitsNumber = FMC_SDRAM_COLUMN_BITS_NUM_8;
  hsdram.Init.RowBitsNumber = FMC_SDRAM_ROW_BITS_NUM_12;
  hsdram.Init.MemoryDataWidth = FMC_SDRAM_MEM_BUS_WIDTH_16;
  hsdram.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
  hsdram.Init.CASLatency = FMC_SDRAM_CAS_LATENCY_3;
  hsdram.Init.WriteProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
  hsdram.Init.SDClockPeriod = FMC_SDRAM_CLOCK_PERIOD_2;
  hsdram.Init.ReadBurst = FMC_SDRAM_RBURST_ENABLE;
  hsdram.Init.ReadPipeDelay = FMC_SDRAM_RPIPE_DELAY_1;

  /*
   * SDRAM 时序（以 90MHz SDRAM 时钟为例的常见值）
   *
   * 这些参数强依赖：
   * - FMC 时钟频率
   * - SDRAM 芯片数据手册（tMRD/tXSR/tRAS/tRC/tWR/tRP/tRCD）
   *
   * 如果 SDRAM 访问异常（HardFault/数据错乱/LTDC 花屏），请优先校准这里。
   */
  sdram_timing.LoadToActiveDelay = 2;
  sdram_timing.ExitSelfRefreshDelay = 7;
  sdram_timing.SelfRefreshTime = 4;
  sdram_timing.RowCycleDelay = 7;
  sdram_timing.WriteRecoveryTime = 2;
  sdram_timing.RPDelay = 2;
  sdram_timing.RCDDelay = 2;

  HAL_StatusTypeDef status = HAL_SDRAM_Init(&hsdram, &sdram_timing);
  if (status != HAL_OK)
  {
    return status;
  }

  SDRAM_Initialization_Sequence(&hsdram);
  return HAL_OK;
}

SDRAM_HandleTypeDef *dri_sdram_handle(void)
{
  return &hsdram;
}
