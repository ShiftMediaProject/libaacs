#ifndef MMC_H_
#define MMC_H_

#include <stdint.h>

typedef struct mmc MMC;
struct mmc {
    int fd;
    uint8_t sk, asc, ascq;
    uint8_t host_priv_key[20], host_cert[92], host_nonce[20];
    uint8_t host_key_point[40];
};

MMC *mmc_open(const char *path, const uint8_t *host_priv_key,
              const uint8_t *host_cert, const uint8_t *host_nonce,
              const uint8_t *host_key_point);
void mmc_close(MMC *drive);
int mmc_read_vid(MMC *mmc, uint8_t *vid);

#endif /* MMC_H_ */
