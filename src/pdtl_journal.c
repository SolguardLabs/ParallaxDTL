#include "pdtl.h"

#include <stdio.h>
#include <string.h>

static void pdtl_copy_string(char *dst, size_t cap, const char *src)
{
    if (cap == 0) {
        return;
    }
    if (src == NULL) {
        src = "";
    }
    (void)snprintf(dst, cap, "%s", src);
}

const char *pdtl_event_kind_name(PdtlEventKind kind)
{
    switch (kind) {
    case PDTL_EVENT_CELL_CREATED:
        return "cell_created";
    case PDTL_EVENT_GENESIS_DEPOSIT:
        return "genesis_deposit";
    case PDTL_EVENT_CELL_TRANSFER:
        return "cell_transfer";
    case PDTL_EVENT_CELL_CONSOLIDATED:
        return "cell_consolidated";
    case PDTL_EVENT_RECEIPT_ISSUED:
        return "receipt_issued";
    case PDTL_EVENT_RECEIPT_SETTLED:
        return "receipt_settled";
    case PDTL_EVENT_WITHDRAWAL:
        return "withdrawal";
    case PDTL_EVENT_ROUTE_CLOSED:
        return "route_closed";
    case PDTL_EVENT_SETTLEMENT_REJECTED:
        return "settlement_rejected";
    case PDTL_EVENT_RECEIPT_CANCELLED:
        return "receipt_cancelled";
    case PDTL_EVENT_ROUTE_FEE_UPDATED:
        return "route_fee_updated";
    default:
        return "unknown";
    }
}

PdtlErrorCode pdtl_append_journal(
    PdtlLedger *ledger,
    PdtlEventKind kind,
    uint32_t cell,
    uint32_t counterparty,
    uint32_t receipt,
    PdtlAmount amount,
    PdtlAmount before_balance,
    PdtlAmount after_balance,
    const char *label)
{
    PdtlJournalEntry *entry;
    PdtlHash hash;

    if (ledger->journal_count >= PDTL_MAX_JOURNAL) {
        return pdtl_set_error(ledger, PDTL_ERR_CAPACITY, "journal capacity reached");
    }

    ledger->sequence++;
    entry = &ledger->journal[ledger->journal_count++];
    entry->sequence = ledger->sequence;
    entry->kind = kind;
    entry->cell = cell;
    entry->counterparty = counterparty;
    entry->receipt = receipt;
    entry->amount = amount;
    entry->before_balance = before_balance;
    entry->after_balance = after_balance;
    pdtl_copy_string(entry->label, sizeof(entry->label), label);

    hash = pdtl_hash_start();
    pdtl_hash_update_str(&hash, "journal");
    pdtl_hash_update_u64(&hash, entry->sequence);
    pdtl_hash_update_u64(&hash, (uint64_t)entry->kind);
    pdtl_hash_update_u64(&hash, entry->cell);
    pdtl_hash_update_u64(&hash, entry->counterparty);
    pdtl_hash_update_u64(&hash, entry->receipt);
    pdtl_hash_update_u64(&hash, entry->amount);
    pdtl_hash_update_u64(&hash, entry->before_balance);
    pdtl_hash_update_u64(&hash, entry->after_balance);
    pdtl_hash_update_str(&hash, entry->label);
    pdtl_hash_to_hex(&hash, entry->digest);

    return PDTL_OK;
}
