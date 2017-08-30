/*
 *******************************************************************************
 *  [subdsp.h]
 *
 *  This program is under the terms of the GPLv3.
 *  https://www.gnu.org/licenses/gpl-3.0.html
 *
 *  Copyright(c) 2017 Keshikan (www.keshikan.net)
 *******************************************************************************
 */
#ifndef SUBDSP_H_
#define SUBDSP_H_


__attribute__( ( always_inline ) ) static inline int32_t __SMULWB (uint32_t op1, uint32_t op2)
{
  int32_t result;

  __asm volatile ("smulwb %0, %1, %2" : "=r" (result) : "r" (op1), "r" (op2) );
  return(result);
}

#endif /* SUBDSP_H_ */
