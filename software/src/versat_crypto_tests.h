#pragma once

#include <stdint.h>

// Functions needed by crypto side but implemented elsewhere
// Somewhere someone must implement such functions so that we can time the algorithms relative performance and receive the testcases by ethernet
int GetTime();
uint32_t uart_recvfile_ethernet(const char *file_name);

void InitializeCryptoSide(int versatAddress);

// Returns zero if pass, nonzero otherwise
int VersatSHATests();
int VersatAESTests();
int VersatMcElieceTests();
