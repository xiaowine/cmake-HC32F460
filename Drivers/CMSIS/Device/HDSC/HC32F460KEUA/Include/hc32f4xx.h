/**
 *******************************************************************************
 * @file  hc32f4xx.h
 * @brief HC32F460KEUA adapter header for the DDL driver library.
 *******************************************************************************
 */
#ifndef __HC32F4XX_H__
#define __HC32F4XX_H__

#ifdef __cplusplus
extern "C" {
#endif

#if defined(HC32F460KEUA) || defined(HC32F460)
#include "hc32f460keua.h"
#include "system_hc32f460keua.h"
#else
#error "Please define HC32F460KEUA for this template project."
#endif

#ifdef __cplusplus
}
#endif

#endif /* __HC32F4XX_H__ */

