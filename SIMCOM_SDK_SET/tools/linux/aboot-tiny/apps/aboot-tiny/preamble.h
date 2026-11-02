/**
 * @section LICENSE
 * Copyright (C) 2020, ASR microelectronics, All rights reserved.
 *
 * @section DESCRIPTION
 *
 * The time class represents a moment of time.
 */
#ifndef PREAMBLE_H
#define PREAMBLE_H

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * Aboot preamble type definition
 */
#define ABOOT_PREAMBLE_SIZE     4       /* preamble char numbers */
#define ABOOT_PREAMBLE_UUUU     "UUUU"  /* aboot standard */
#define ABOOT_PREAMBLE_UABT     "UABT"  /* aboot tiny */
#define ABOOT_FRAME_DELIMITER   (0x7E)  /* frame delimiter */

int preamble_start(const char *dev_path, int baud_rate);
void preamble_stop(void);

#ifdef  __cplusplus
}
#endif

#endif /* PREAMBLE_H */
