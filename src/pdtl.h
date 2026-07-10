#ifndef PDTL_H
#define PDTL_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define PDTL_ID_LEN 40
#define PDTL_NAME_LEN 48
#define PDTL_ASSET_LEN 16
#define PDTL_DIGEST_LEN 65
#define PDTL_DETAIL_LEN 160

#define PDTL_MAX_POLICIES 12
#define PDTL_MAX_ROUTES 16
#define PDTL_MAX_CELLS 40
#define PDTL_MAX_RECEIPTS 80
#define PDTL_MAX_JOURNAL 320
#define PDTL_MAX_JSON_DEPTH 32

#define PDTL_NO_INDEX UINT32_MAX

typedef uint64_t PdtlAmount;

typedef enum PdtlErrorCode {
    PDTL_OK = 0,
    PDTL_ERR_CAPACITY,
    PDTL_ERR_NOT_FOUND,
    PDTL_ERR_DUPLICATE,
    PDTL_ERR_POLICY,
    PDTL_ERR_ROUTE,
    PDTL_ERR_CELL_CLOSED,
    PDTL_ERR_ROUTE_CLOSED,
    PDTL_ERR_ASSET,
    PDTL_ERR_BALANCE,
    PDTL_ERR_RECEIPT_STATE,
    PDTL_ERR_BINDING,
    PDTL_ERR_RECONCILIATION,
    PDTL_ERR_ARGUMENT
} PdtlErrorCode;

typedef enum PdtlCellState {
    PDTL_CELL_OPEN = 1,
    PDTL_CELL_CLOSED = 2
} PdtlCellState;

typedef enum PdtlReceiptStatus {
    PDTL_RECEIPT_PENDING = 1,
    PDTL_RECEIPT_SETTLED = 2,
    PDTL_RECEIPT_CANCELLED = 3
} PdtlReceiptStatus;

typedef enum PdtlPolicyFlags {
    PDTL_POLICY_WITHDRAW = 1u << 0,
    PDTL_POLICY_CONSOLIDATE = 1u << 1,
    PDTL_POLICY_ALLOW_ROUTE_CLOSE = 1u << 2,
    PDTL_POLICY_TREASURY = 1u << 3
} PdtlPolicyFlags;

typedef enum PdtlEventKind {
    PDTL_EVENT_CELL_CREATED = 1,
    PDTL_EVENT_GENESIS_DEPOSIT = 2,
    PDTL_EVENT_CELL_TRANSFER = 3,
    PDTL_EVENT_CELL_CONSOLIDATED = 4,
    PDTL_EVENT_RECEIPT_ISSUED = 5,
    PDTL_EVENT_RECEIPT_SETTLED = 6,
    PDTL_EVENT_WITHDRAWAL = 7,
    PDTL_EVENT_ROUTE_CLOSED = 8,
    PDTL_EVENT_SETTLEMENT_REJECTED = 9,
    PDTL_EVENT_RECEIPT_CANCELLED = 10,
    PDTL_EVENT_ROUTE_FEE_UPDATED = 11
} PdtlEventKind;

typedef struct PdtlPolicy {
    char name[PDTL_NAME_LEN];
    uint32_t flags;
    uint32_t withdrawal_delay_slots;
    PdtlAmount min_reserve;
    uint32_t max_exit_bps;
} PdtlPolicy;

typedef struct PdtlRoute {
    char name[PDTL_NAME_LEN];
    char asset[PDTL_ASSET_LEN];
    char policy[PDTL_NAME_LEN];
    uint32_t fee_bps;
    uint32_t fee_cell;
    int closed;
} PdtlRoute;

typedef struct PdtlCell {
    uint32_t index;
    char id[PDTL_ID_LEN];
    char owner[PDTL_NAME_LEN];
    char asset[PDTL_ASSET_LEN];
    char route[PDTL_NAME_LEN];
    char policy[PDTL_NAME_LEN];
    PdtlAmount reserve;
    PdtlAmount reserved_out;
    PdtlAmount settled_in;
    PdtlAmount settled_out;
    PdtlAmount withdrawn;
    uint64_t sequence;
    PdtlCellState state;
} PdtlCell;

typedef struct PdtlReceipt {
    uint32_t index;
    char id[PDTL_ID_LEN];
    uint32_t origin_cell;
    uint32_t beneficiary_cell;
    uint32_t paid_by_cell;
    char asset[PDTL_ASSET_LEN];
    char route[PDTL_NAME_LEN];
    char policy[PDTL_NAME_LEN];
    PdtlAmount gross;
    PdtlAmount fee;
    PdtlAmount net;
    uint64_t nonce;
    char route_digest[PDTL_DIGEST_LEN];
    PdtlReceiptStatus status;
} PdtlReceipt;

typedef struct PdtlJournalEntry {
    uint64_t sequence;
    PdtlEventKind kind;
    uint32_t cell;
    uint32_t counterparty;
    uint32_t receipt;
    PdtlAmount amount;
    PdtlAmount before_balance;
    PdtlAmount after_balance;
    char label[PDTL_DETAIL_LEN];
    char digest[PDTL_DIGEST_LEN];
} PdtlJournalEntry;

typedef struct PdtlLedger {
    char network_id[PDTL_NAME_LEN];
    uint64_t sequence;
    PdtlPolicy policies[PDTL_MAX_POLICIES];
    size_t policy_count;
    PdtlRoute routes[PDTL_MAX_ROUTES];
    size_t route_count;
    PdtlCell cells[PDTL_MAX_CELLS];
    size_t cell_count;
    PdtlReceipt receipts[PDTL_MAX_RECEIPTS];
    size_t receipt_count;
    PdtlJournalEntry journal[PDTL_MAX_JOURNAL];
    size_t journal_count;
    PdtlAmount initial_supply;
    PdtlAmount external_withdrawals;
    PdtlAmount minted_after_genesis;
    PdtlAmount burned_after_genesis;
    PdtlErrorCode last_error;
    char last_error_detail[PDTL_DETAIL_LEN];
} PdtlLedger;

typedef struct PdtlHash {
    uint64_t left;
    uint64_t right;
} PdtlHash;

typedef struct PdtlJson {
    FILE *out;
    int depth;
    int first[PDTL_MAX_JSON_DEPTH];
    char kind[PDTL_MAX_JSON_DEPTH];
} PdtlJson;

typedef struct PdtlRouteExposure {
    char route[PDTL_NAME_LEN];
    char asset[PDTL_ASSET_LEN];
    char policy[PDTL_NAME_LEN];
    uint32_t fee_bps;
    uint32_t fee_cell;
    int closed;
    uint32_t open_cells;
    uint32_t closed_cells;
    PdtlAmount reserves;
    PdtlAmount available;
    PdtlAmount reserved_out;
    PdtlAmount pending_gross;
    PdtlAmount settled_gross;
    PdtlAmount cancelled_gross;
    uint32_t pending_receipts;
    uint32_t settled_receipts;
    uint32_t cancelled_receipts;
    PdtlAmount max_exit_budget;
    uint32_t utilization_bps;
    uint32_t health_score;
} PdtlRouteExposure;

typedef struct PdtlScenarioResult {
    char scenario[PDTL_NAME_LEN];
    uint32_t origin_cell;
    uint32_t aux_cell;
    uint32_t sponsor_cell;
    uint32_t beneficiary_cell;
    uint32_t treasury_cell;
    uint32_t receipt;
    uint32_t secondary_receipt;
    PdtlAmount legitimate_withdrawable_before;
    PdtlAmount available_after_foreign_payment;
    PdtlAmount attacker_withdrawn;
    PdtlAmount third_party_paid;
    PdtlAmount excess_withdrawal;
    PdtlAmount cancelled_gross;
    uint32_t fee_before_bps;
    uint32_t fee_after_bps;
    int fee_update_applied;
    int settlement_accepted;
    int strict_rejection;
    int route_closed;
} PdtlScenarioResult;

const char *pdtl_error_name(PdtlErrorCode code);
const char *pdtl_error_message(PdtlErrorCode code);
const char *pdtl_receipt_status_name(PdtlReceiptStatus status);
const char *pdtl_cell_state_name(PdtlCellState state);
const char *pdtl_event_kind_name(PdtlEventKind kind);

void pdtl_init(PdtlLedger *ledger, const char *network_id);
void pdtl_clear_error(PdtlLedger *ledger);
PdtlErrorCode pdtl_set_error(PdtlLedger *ledger, PdtlErrorCode code, const char *detail);

PdtlErrorCode pdtl_add_policy(
    PdtlLedger *ledger,
    const char *name,
    uint32_t flags,
    uint32_t withdrawal_delay_slots,
    PdtlAmount min_reserve,
    uint32_t max_exit_bps);

PdtlErrorCode pdtl_add_route(
    PdtlLedger *ledger,
    const char *name,
    const char *asset,
    const char *policy,
    uint32_t fee_bps,
    uint32_t fee_cell);

PdtlErrorCode pdtl_create_cell(
    PdtlLedger *ledger,
    const char *owner,
    const char *asset,
    const char *route,
    const char *policy,
    uint32_t *out_index);

PdtlErrorCode pdtl_deposit_genesis(PdtlLedger *ledger, uint32_t cell_index, PdtlAmount amount);
PdtlErrorCode pdtl_transfer_between_cells(
    PdtlLedger *ledger,
    uint32_t from_cell,
    uint32_t to_cell,
    PdtlAmount amount,
    const char *label);
PdtlErrorCode pdtl_consolidate_cell(
    PdtlLedger *ledger,
    uint32_t target_cell,
    uint32_t source_cell,
    const char *label);
PdtlErrorCode pdtl_issue_receipt(
    PdtlLedger *ledger,
    uint32_t origin_cell,
    uint32_t beneficiary_cell,
    PdtlAmount gross,
    uint64_t nonce,
    uint32_t *out_receipt);
PdtlErrorCode pdtl_settle_receipt_vulnerable(
    PdtlLedger *ledger,
    uint32_t receipt_index,
    uint32_t paying_cell);
PdtlErrorCode pdtl_settle_receipt_strict(
    PdtlLedger *ledger,
    uint32_t receipt_index,
    uint32_t paying_cell);
PdtlErrorCode pdtl_cancel_receipt(
    PdtlLedger *ledger,
    uint32_t receipt_index,
    const char *label);
PdtlErrorCode pdtl_withdraw(
    PdtlLedger *ledger,
    uint32_t cell_index,
    PdtlAmount amount,
    const char *label);
PdtlErrorCode pdtl_update_route_fee(
    PdtlLedger *ledger,
    const char *route_name,
    uint32_t new_fee_bps,
    uint32_t new_fee_cell);
PdtlErrorCode pdtl_close_route(PdtlLedger *ledger, const char *route_name);

PdtlPolicy *pdtl_find_policy(PdtlLedger *ledger, const char *name);
const PdtlPolicy *pdtl_find_policy_const(const PdtlLedger *ledger, const char *name);
PdtlRoute *pdtl_find_route(PdtlLedger *ledger, const char *name);
const PdtlRoute *pdtl_find_route_const(const PdtlLedger *ledger, const char *name);
PdtlCell *pdtl_get_cell(PdtlLedger *ledger, uint32_t index);
const PdtlCell *pdtl_get_cell_const(const PdtlLedger *ledger, uint32_t index);
PdtlReceipt *pdtl_get_receipt(PdtlLedger *ledger, uint32_t index);
const PdtlReceipt *pdtl_get_receipt_const(const PdtlLedger *ledger, uint32_t index);

PdtlAmount pdtl_cell_available(const PdtlLedger *ledger, uint32_t cell_index);
PdtlAmount pdtl_total_cell_reserves(const PdtlLedger *ledger);
PdtlAmount pdtl_total_reserved_out(const PdtlLedger *ledger);
PdtlAmount pdtl_total_settled_in(const PdtlLedger *ledger);
PdtlAmount pdtl_total_settled_out(const PdtlLedger *ledger);
PdtlAmount pdtl_route_reserve_total(const PdtlLedger *ledger, const char *route_name);
PdtlAmount pdtl_route_available_total(const PdtlLedger *ledger, const char *route_name);
PdtlAmount pdtl_route_reserved_total(const PdtlLedger *ledger, const char *route_name);
PdtlAmount pdtl_route_pending_gross(const PdtlLedger *ledger, const char *route_name);
PdtlErrorCode pdtl_route_exposure(
    const PdtlLedger *ledger,
    const char *route_name,
    PdtlRouteExposure *out);
int pdtl_global_reconciliation_ok(const PdtlLedger *ledger);
int pdtl_receipt_bindings_ok(const PdtlLedger *ledger);
int pdtl_route_has_pending_receipts(const PdtlLedger *ledger, const char *route_name);

PdtlErrorCode pdtl_append_journal(
    PdtlLedger *ledger,
    PdtlEventKind kind,
    uint32_t cell,
    uint32_t counterparty,
    uint32_t receipt,
    PdtlAmount amount,
    PdtlAmount before_balance,
    PdtlAmount after_balance,
    const char *label);

void pdtl_make_id(char out[PDTL_ID_LEN], const char *prefix, uint64_t a, uint64_t b, uint64_t c);
void pdtl_route_digest(const PdtlRoute *route, char out[PDTL_DIGEST_LEN]);
void pdtl_cell_digest(const PdtlCell *cell, char out[PDTL_DIGEST_LEN]);
void pdtl_receipt_digest(const PdtlReceipt *receipt, char out[PDTL_DIGEST_LEN]);
void pdtl_state_digest(const PdtlLedger *ledger, char out[PDTL_DIGEST_LEN]);
PdtlHash pdtl_hash_start(void);
void pdtl_hash_update_bytes(PdtlHash *hash, const void *data, size_t len);
void pdtl_hash_update_str(PdtlHash *hash, const char *value);
void pdtl_hash_update_u64(PdtlHash *hash, uint64_t value);
void pdtl_hash_to_hex(const PdtlHash *hash, char out[PDTL_DIGEST_LEN]);

void pdtl_json_init(PdtlJson *json, FILE *out);
void pdtl_json_begin_object(PdtlJson *json);
void pdtl_json_end_object(PdtlJson *json);
void pdtl_json_prop_object_begin(PdtlJson *json, const char *name);
void pdtl_json_prop_array_begin(PdtlJson *json, const char *name);
void pdtl_json_array_object_begin(PdtlJson *json);
void pdtl_json_end_array(PdtlJson *json);
void pdtl_json_prop_string(PdtlJson *json, const char *name, const char *value);
void pdtl_json_prop_u64(PdtlJson *json, const char *name, uint64_t value);
void pdtl_json_prop_i32(PdtlJson *json, const char *name, int value);
void pdtl_json_prop_bool(PdtlJson *json, const char *name, int value);
void pdtl_json_prop_null(PdtlJson *json, const char *name);

PdtlErrorCode pdtl_run_scenario(
    const char *scenario,
    PdtlLedger *ledger,
    PdtlScenarioResult *result);
void pdtl_print_report(FILE *out, const PdtlLedger *ledger, const PdtlScenarioResult *result);
void pdtl_print_usage(FILE *out, const char *program);

#endif
