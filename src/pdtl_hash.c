#include "pdtl.h"

#include <stdio.h>
#include <string.h>

static uint64_t pdtl_rotl64(uint64_t value, unsigned int bits)
{
    return (value << bits) | (value >> (64u - bits));
}

PdtlHash pdtl_hash_start(void)
{
    PdtlHash hash;
    hash.left = 1469598103934665603ull;
    hash.right = 1099511628211ull ^ 0x9e3779b97f4a7c15ull;
    return hash;
}

void pdtl_hash_update_bytes(PdtlHash *hash, const void *data, size_t len)
{
    const unsigned char *bytes = (const unsigned char *)data;
    size_t i;

    for (i = 0; i < len; i++) {
        hash->left ^= (uint64_t)bytes[i];
        hash->left *= 1099511628211ull;
        hash->right ^= pdtl_rotl64((uint64_t)bytes[i] + hash->left, (unsigned int)((i % 31u) + 1u));
        hash->right *= 14029467366897019727ull;
        hash->right ^= hash->right >> 29;
    }
}

void pdtl_hash_update_str(PdtlHash *hash, const char *value)
{
    if (value == NULL) {
        pdtl_hash_update_bytes(hash, "<null>", 6);
        return;
    }
    pdtl_hash_update_bytes(hash, value, strlen(value));
    pdtl_hash_update_bytes(hash, "|", 1);
}

void pdtl_hash_update_u64(PdtlHash *hash, uint64_t value)
{
    unsigned char bytes[8];
    size_t i;

    for (i = 0; i < 8; i++) {
        bytes[i] = (unsigned char)((value >> (i * 8u)) & 0xffu);
    }
    pdtl_hash_update_bytes(hash, bytes, sizeof(bytes));
    pdtl_hash_update_bytes(hash, "#", 1);
}

void pdtl_hash_to_hex(const PdtlHash *hash, char out[PDTL_DIGEST_LEN])
{
    uint64_t third = hash->left ^ pdtl_rotl64(hash->right, 17);
    uint64_t fourth = hash->right ^ pdtl_rotl64(hash->left, 41);

    (void)snprintf(
        out,
        PDTL_DIGEST_LEN,
        "%016llx%016llx%016llx%016llx",
        (unsigned long long)hash->left,
        (unsigned long long)hash->right,
        (unsigned long long)third,
        (unsigned long long)fourth);
}

void pdtl_make_id(char out[PDTL_ID_LEN], const char *prefix, uint64_t a, uint64_t b, uint64_t c)
{
    PdtlHash hash = pdtl_hash_start();
    char digest[PDTL_DIGEST_LEN];

    pdtl_hash_update_str(&hash, prefix);
    pdtl_hash_update_u64(&hash, a);
    pdtl_hash_update_u64(&hash, b);
    pdtl_hash_update_u64(&hash, c);
    pdtl_hash_to_hex(&hash, digest);
    (void)snprintf(out, PDTL_ID_LEN, "%s_%.24s", prefix, digest);
}

void pdtl_route_digest(const PdtlRoute *route, char out[PDTL_DIGEST_LEN])
{
    PdtlHash hash = pdtl_hash_start();

    pdtl_hash_update_str(&hash, "route");
    pdtl_hash_update_str(&hash, route->name);
    pdtl_hash_update_str(&hash, route->asset);
    pdtl_hash_update_str(&hash, route->policy);
    pdtl_hash_update_u64(&hash, route->fee_bps);
    pdtl_hash_update_u64(&hash, route->fee_cell);
    pdtl_hash_update_u64(&hash, (uint64_t)route->closed);
    pdtl_hash_to_hex(&hash, out);
}

void pdtl_cell_digest(const PdtlCell *cell, char out[PDTL_DIGEST_LEN])
{
    PdtlHash hash = pdtl_hash_start();

    pdtl_hash_update_str(&hash, "cell");
    pdtl_hash_update_str(&hash, cell->id);
    pdtl_hash_update_str(&hash, cell->owner);
    pdtl_hash_update_str(&hash, cell->asset);
    pdtl_hash_update_str(&hash, cell->route);
    pdtl_hash_update_str(&hash, cell->policy);
    pdtl_hash_update_u64(&hash, cell->reserve);
    pdtl_hash_update_u64(&hash, cell->reserved_out);
    pdtl_hash_update_u64(&hash, cell->settled_in);
    pdtl_hash_update_u64(&hash, cell->settled_out);
    pdtl_hash_update_u64(&hash, cell->withdrawn);
    pdtl_hash_update_u64(&hash, cell->sequence);
    pdtl_hash_update_u64(&hash, (uint64_t)cell->state);
    pdtl_hash_to_hex(&hash, out);
}

void pdtl_receipt_digest(const PdtlReceipt *receipt, char out[PDTL_DIGEST_LEN])
{
    PdtlHash hash = pdtl_hash_start();

    pdtl_hash_update_str(&hash, "receipt");
    pdtl_hash_update_str(&hash, receipt->id);
    pdtl_hash_update_u64(&hash, receipt->origin_cell);
    pdtl_hash_update_u64(&hash, receipt->beneficiary_cell);
    pdtl_hash_update_u64(&hash, receipt->paid_by_cell);
    pdtl_hash_update_str(&hash, receipt->asset);
    pdtl_hash_update_str(&hash, receipt->route);
    pdtl_hash_update_str(&hash, receipt->policy);
    pdtl_hash_update_u64(&hash, receipt->gross);
    pdtl_hash_update_u64(&hash, receipt->fee);
    pdtl_hash_update_u64(&hash, receipt->net);
    pdtl_hash_update_u64(&hash, receipt->nonce);
    pdtl_hash_update_str(&hash, receipt->route_digest);
    pdtl_hash_update_u64(&hash, (uint64_t)receipt->status);
    pdtl_hash_to_hex(&hash, out);
}

void pdtl_state_digest(const PdtlLedger *ledger, char out[PDTL_DIGEST_LEN])
{
    PdtlHash hash = pdtl_hash_start();
    char digest[PDTL_DIGEST_LEN];
    size_t i;

    pdtl_hash_update_str(&hash, "parallax-state");
    pdtl_hash_update_str(&hash, ledger->network_id);
    pdtl_hash_update_u64(&hash, ledger->sequence);
    pdtl_hash_update_u64(&hash, ledger->initial_supply);
    pdtl_hash_update_u64(&hash, ledger->external_withdrawals);
    pdtl_hash_update_u64(&hash, ledger->cell_count);
    pdtl_hash_update_u64(&hash, ledger->receipt_count);
    pdtl_hash_update_u64(&hash, ledger->journal_count);

    for (i = 0; i < ledger->route_count; i++) {
        pdtl_route_digest(&ledger->routes[i], digest);
        pdtl_hash_update_str(&hash, digest);
    }
    for (i = 0; i < ledger->cell_count; i++) {
        pdtl_cell_digest(&ledger->cells[i], digest);
        pdtl_hash_update_str(&hash, digest);
    }
    for (i = 0; i < ledger->receipt_count; i++) {
        pdtl_receipt_digest(&ledger->receipts[i], digest);
        pdtl_hash_update_str(&hash, digest);
    }
    pdtl_hash_to_hex(&hash, out);
}
