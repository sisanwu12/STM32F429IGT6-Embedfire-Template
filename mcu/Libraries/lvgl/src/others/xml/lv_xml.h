/**
 * @file lv_xml.h
 *
 * 占位头文件：用于通过 `lv_init.c` 的无条件 include。
 * 当前工程在 `lv_conf.h` 中关闭了 `LV_USE_XML`。
 */

#ifndef LV_XML_H
#define LV_XML_H

#ifdef __cplusplus
extern "C" {
#endif

void lv_xml_init(void);
void lv_xml_deinit(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_XML_H*/

