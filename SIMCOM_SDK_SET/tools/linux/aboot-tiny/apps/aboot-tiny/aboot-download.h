/**
 * @section LICENSE
 * Copyright (C) 2020, ASR microelectronics, All rights reserved.
 *
 * @section DESCRIPTION
 *
 * The time class represents a moment of time.
 */
#ifndef ABOOT_DOWNLOAD_H
#define ABOOT_DOWNLOAD_H

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * Aboot command/response size definition
 */
#define ABOOT_COMMAND_SZ        128
#define ABOOT_RESPONSE_SZ       128

/*
 * process event
 */
#define ABOOT_RESPONSE_EVENT    0

/* Aboot download process */
PROCESS_NAME(aboot_download_process);

/* Aboot download external api */
int aboot_download_init(void);
int aboot_download_start_file(FILE *file, size_t size, bool reboot);
int aboot_download_start_data(const void *data, size_t size, bool reboot);
int aboot_download_exit(void);

#ifdef  __cplusplus
}
#endif

#endif /* ABOOT_DOWNLOAD_H */
