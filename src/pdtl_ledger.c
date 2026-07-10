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

static int pdtl_string_eq(const char *left, const char *right)
{
    return strcmp(left, right) == 0;
}

const char *pdtl_error_name(PdtlErrorCode code)
{
    switch (code) {
    case PDTL_OK:
        return "PDTL_OK";
    case PDTL_ERR_CAPACITY:
        return "PDTL_ERR_CAPACITY";
    case PDTL_ERR_NOT_FOUND:
        return "PDTL_ERR_NOT_FOUND";
    case PDTL_ERR_DUPLICATE:
        return "PDTL_ERR_DUPLICATE";
    case PDTL_ERR_POLICY:
        return "PDTL_ERR_POLICY";
    case PDTL_ERR_ROUTE:
        return "PDTL_ERR_ROUTE";
    case PDTL_ERR_CELL_CLOSED:
        return "PDTL_ERR_CELL_CLOSED";
    case PDTL_ERR_ROUTE_CLOSED:
        return "PDTL_ERR_ROUTE_CLOSED";
    case PDTL_ERR_ASSET:
        return "PDTL_ERR_ASSET";
    case PDTL_ERR_BALANCE:
        return "PDTL_ERR_BALANCE";
    case PDTL_ERR_RECEIPT_STATE:
        return "PDTL_ERR_RECEIPT_STATE";
    case PDTL_ERR_BINDING:
        return "PDTL_ERR_BINDING";
    case PDTL_ERR_RECONCILIATION:
        return "PDTL_ERR_RECONCILIATION";
    case PDTL_ERR_ARGUMENT:
        return "PDTL_ERR_ARGUMENT";
    default:
        return "PDTL_ERR_UNKNOWN";
    }
}

const char *pdtl_error_message(PdtlErrorCode code)
{
    switch (code) {
    case PDTL_OK:
        return "ok";
    case PDTL_ERR_CAPACITY:
        return "configured capacity reached";
    case PDTL_ERR_NOT_FOUND:
        return "requested object was not found";
    case PDTL_ERR_DUPLICATE:
        return "object already exists";
    case PDTL_ERR_POLICY:
        return "policy validation failed";
    case PDTL_ERR_ROUTE:
        return "route validation failed";
    case PDTL_ERR_CELL_CLOSED:
        return "cell is closed";
    case PDTL_ERR_ROUTE_CLOSED:
        return "route is closed";
    case PDTL_ERR_ASSET:
        return "asset mismatch";
    case PDTL_ERR_BALANCE:
        return "insufficient available balance";
    case PDTL_ERR_RECEIPT_STATE:
        return "receipt is not in the required state";
    case PDTL_ERR_BINDING:
        return "receipt origin and paying cell are not bound";
    case PDTL_ERR_RECONCILIATION:
        return "global reconciliation failed";
    case PDTL_ERR_ARGUMENT:
        return "invalid argument";
    default:
        return "unknown error";
    }
}

const char *pdtl_receipt_status_name(PdtlReceiptStatus status)
{
    switch (status) {
    case PDTL_RECEIPT_PENDING:
        return "pending";
    case PDTL_RECEIPT_SETTLED:
        return "settled";
    case PDTL_RECEIPT_CANCELLED:
        return "cancelled";
    default:
        return "unknown";
    }
}

const char *pdtl_cell_state_name(PdtlCellState state)
{
    switch (state) {
    case PDTL_CELL_OPEN:
        return "open";
    case PDTL_CELL_CLOSED:
        return "closed";
    default:
        return "unknown";
    }
}

void pdtl_clear_error(PdtlLedger *ledger)
{
    ledger->last_error = PDTL_OK;
    ledger->last_error_detail[0] = '\0';
}

PdtlErrorCode pdtl_set_error(PdtlLedger *ledger, PdtlErrorCode code, const char *detail)
{
    if (ledger != NULL) {
        ledger->last_error = code;
        pdtl_copy_string(ledger->last_error_detail, sizeof(ledger->last_error_detail), detail);
    }
    return code;
}

void pdtl_init(PdtlLedger *ledger, const char *network_id)
{
    memset(ledger, 0, sizeof(*ledger));
    pdtl_copy_string(ledger->network_id, sizeof(ledger->network_id), network_id);
    pdtl_clear_error(ledger);
}

PdtlPolicy *pdtl_find_policy(PdtlLedger *ledger, const char *name)
{
    size_t i;

    for (i = 0; i < ledger->policy_count; i++) {
        if (pdtl_string_eq(ledger->policies[i].name, name)) {
            return &ledger->policies[i];
        }
    }
    return NULL;
}

const PdtlPolicy *pdtl_find_policy_const(const PdtlLedger *ledger, const char *name)
{
    size_t i;

    for (i = 0; i < ledger->policy_count; i++) {
        if (pdtl_string_eq(ledger->policies[i].name, name)) {
            return &ledger->policies[i];
        }
    }
    return NULL;
}

PdtlRoute *pdtl_find_route(PdtlLedger *ledger, const char *name)
{
    size_t i;

    for (i = 0; i < ledger->route_count; i++) {
        if (pdtl_string_eq(ledger->routes[i].name, name)) {
            return &ledger->routes[i];
        }
    }
    return NULL;
}

const PdtlRoute *pdtl_find_route_const(const PdtlLedger *ledger, const char *name)
{
    size_t i;

    for (i = 0; i < ledger->route_count; i++) {
        if (pdtl_string_eq(ledger->routes[i].name, name)) {
            return &ledger->routes[i];
        }
    }
    return NULL;
}

PdtlCell *pdtl_get_cell(PdtlLedger *ledger, uint32_t index)
{
    if (index >= ledger->cell_count) {
        return NULL;
    }
    return &ledger->cells[index];
}

const PdtlCell *pdtl_get_cell_const(const PdtlLedger *ledger, uint32_t index)
{
    if (index >= ledger->cell_count) {
        return NULL;
    }
    return &ledger->cells[index];
}

PdtlReceipt *pdtl_get_receipt(PdtlLedger *ledger, uint32_t index)
{
    if (index >= ledger->receipt_count) {
        return NULL;
    }
    return &ledger->receipts[index];
}

const PdtlReceipt *pdtl_get_receipt_const(const PdtlLedger *ledger, uint32_t index)
{
    if (index >= ledger->receipt_count) {
        return NULL;
    }
    return &ledger->receipts[index];
}

PdtlErrorCode pdtl_add_policy(
    PdtlLedger *ledger,
    const char *name,
    uint32_t flags,
    uint32_t withdrawal_delay_slots,
    PdtlAmount min_reserve,
    uint32_t max_exit_bps)
{
    PdtlPolicy *policy;

    if (name == NULL || name[0] == '\0') {
        return pdtl_set_error(ledger, PDTL_ERR_ARGUMENT, "policy name is required");
    }
    if (pdtl_find_policy(ledger, name) != NULL) {
        return pdtl_set_error(ledger, PDTL_ERR_DUPLICATE, "policy already exists");
    }
    if (ledger->policy_count >= PDTL_MAX_POLICIES) {
        return pdtl_set_error(ledger, PDTL_ERR_CAPACITY, "policy capacity reached");
    }
    if (max_exit_bps > 10000u) {
        return pdtl_set_error(ledger, PDTL_ERR_POLICY, "max exit bps cannot exceed 10000");
    }

    policy = &ledger->policies[ledger->policy_count++];
    memset(policy, 0, sizeof(*policy));
    pdtl_copy_string(policy->name, sizeof(policy->name), name);
    policy->flags = flags;
    policy->withdrawal_delay_slots = withdrawal_delay_slots;
    policy->min_reserve = min_reserve;
    policy->max_exit_bps = max_exit_bps;
    return PDTL_OK;
}

PdtlErrorCode pdtl_add_route(
    PdtlLedger *ledger,
    const char *name,
    const char *asset,
    const char *policy,
    uint32_t fee_bps,
    uint32_t fee_cell)
{
    PdtlRoute *route;

    if (name == NULL || name[0] == '\0' || asset == NULL || asset[0] == '\0') {
        return pdtl_set_error(ledger, PDTL_ERR_ARGUMENT, "route name and asset are required");
    }
    if (pdtl_find_route(ledger, name) != NULL) {
        return pdtl_set_error(ledger, PDTL_ERR_DUPLICATE, "route already exists");
    }
    if (pdtl_find_policy(ledger, policy) == NULL) {
        return pdtl_set_error(ledger, PDTL_ERR_POLICY, "route policy is unknown");
    }
    if (fee_bps > 10000u) {
        return pdtl_set_error(ledger, PDTL_ERR_ROUTE, "fee bps cannot exceed 10000");
    }
    if (fee_cell != PDTL_NO_INDEX && fee_cell >= ledger->cell_count) {
        return pdtl_set_error(ledger, PDTL_ERR_NOT_FOUND, "route fee cell is unknown");
    }
    if (ledger->route_count >= PDTL_MAX_ROUTES) {
        return pdtl_set_error(ledger, PDTL_ERR_CAPACITY, "route capacity reached");
    }

    route = &ledger->routes[ledger->route_count++];
    memset(route, 0, sizeof(*route));
    pdtl_copy_string(route->name, sizeof(route->name), name);
    pdtl_copy_string(route->asset, sizeof(route->asset), asset);
    pdtl_copy_string(route->policy, sizeof(route->policy), policy);
    route->fee_bps = fee_bps;
    route->fee_cell = fee_cell;
    route->closed = 0;
    return PDTL_OK;
}

PdtlErrorCode pdtl_create_cell(
    PdtlLedger *ledger,
    const char *owner,
    const char *asset,
    const char *route_name,
    const char *policy_name,
    uint32_t *out_index)
{
    PdtlRoute *route;
    PdtlPolicy *policy;
    PdtlCell *cell;
    uint32_t index;

    if (owner == NULL || owner[0] == '\0') {
        return pdtl_set_error(ledger, PDTL_ERR_ARGUMENT, "cell owner is required");
    }
    route = pdtl_find_route(ledger, route_name);
    if (route == NULL) {
        return pdtl_set_error(ledger, PDTL_ERR_ROUTE, "cell route is unknown");
    }
    policy = pdtl_find_policy(ledger, policy_name);
    if (policy == NULL) {
        return pdtl_set_error(ledger, PDTL_ERR_POLICY, "cell policy is unknown");
    }
    if (!pdtl_string_eq(route->asset, asset)) {
        return pdtl_set_error(ledger, PDTL_ERR_ASSET, "cell asset does not match route asset");
    }
    if (!pdtl_string_eq(route->policy, policy->name)) {
        return pdtl_set_error(ledger, PDTL_ERR_POLICY, "cell policy does not match route policy");
    }
    if (ledger->cell_count >= PDTL_MAX_CELLS) {
        return pdtl_set_error(ledger, PDTL_ERR_CAPACITY, "cell capacity reached");
    }

    index = (uint32_t)ledger->cell_count;
    cell = &ledger->cells[ledger->cell_count++];
    memset(cell, 0, sizeof(*cell));
    cell->index = index;
    pdtl_make_id(cell->id, "cell", index, ledger->sequence + 1u, ledger->cell_count);
    pdtl_copy_string(cell->owner, sizeof(cell->owner), owner);
    pdtl_copy_string(cell->asset, sizeof(cell->asset), asset);
    pdtl_copy_string(cell->route, sizeof(cell->route), route_name);
    pdtl_copy_string(cell->policy, sizeof(cell->policy), policy_name);
    cell->state = PDTL_CELL_OPEN;
    cell->sequence = ledger->sequence + 1u;

    if (out_index != NULL) {
        *out_index = index;
    }
    return pdtl_append_journal(
        ledger,
        PDTL_EVENT_CELL_CREATED,
        index,
        PDTL_NO_INDEX,
        PDTL_NO_INDEX,
        0,
        0,
        0,
        "cell registered");
}

PdtlErrorCode pdtl_deposit_genesis(PdtlLedger *ledger, uint32_t cell_index, PdtlAmount amount)
{
    PdtlCell *cell = pdtl_get_cell(ledger, cell_index);
    PdtlAmount before;

    if (cell == NULL) {
        return pdtl_set_error(ledger, PDTL_ERR_NOT_FOUND, "deposit cell is unknown");
    }
    if (cell->state != PDTL_CELL_OPEN) {
        return pdtl_set_error(ledger, PDTL_ERR_CELL_CLOSED, "deposit cell is closed");
    }

    before = cell->reserve;
    cell->reserve += amount;
    ledger->initial_supply += amount;
    cell->sequence = ledger->sequence + 1u;
    return pdtl_append_journal(
        ledger,
        PDTL_EVENT_GENESIS_DEPOSIT,
        cell_index,
        PDTL_NO_INDEX,
        PDTL_NO_INDEX,
        amount,
        before,
        cell->reserve,
        "genesis reserve");
}

PdtlAmount pdtl_cell_available(const PdtlLedger *ledger, uint32_t cell_index)
{
    const PdtlCell *cell = pdtl_get_cell_const(ledger, cell_index);
    const PdtlPolicy *policy;
    PdtlAmount locked;

    if (cell == NULL || cell->state != PDTL_CELL_OPEN) {
        return 0;
    }
    policy = pdtl_find_policy_const(ledger, cell->policy);
    if (policy == NULL) {
        return 0;
    }
    locked = cell->reserved_out + policy->min_reserve;
    if (cell->reserve <= locked) {
        return 0;
    }
    return cell->reserve - locked;
}

PdtlErrorCode pdtl_transfer_between_cells(
    PdtlLedger *ledger,
    uint32_t from_cell,
    uint32_t to_cell,
    PdtlAmount amount,
    const char *label)
{
    PdtlCell *from = pdtl_get_cell(ledger, from_cell);
    PdtlCell *to = pdtl_get_cell(ledger, to_cell);
    PdtlAmount before_from;
    PdtlAmount before_to;

    if (from == NULL || to == NULL) {
        return pdtl_set_error(ledger, PDTL_ERR_NOT_FOUND, "transfer cell is unknown");
    }
    if (from->state != PDTL_CELL_OPEN || to->state != PDTL_CELL_OPEN) {
        return pdtl_set_error(ledger, PDTL_ERR_CELL_CLOSED, "transfer touches a closed cell");
    }
    if (!pdtl_string_eq(from->asset, to->asset)) {
        return pdtl_set_error(ledger, PDTL_ERR_ASSET, "transfer asset mismatch");
    }
    if (pdtl_cell_available(ledger, from_cell) < amount) {
        return pdtl_set_error(ledger, PDTL_ERR_BALANCE, "transfer exceeds available balance");
    }

    before_from = from->reserve;
    before_to = to->reserve;
    from->reserve -= amount;
    from->settled_out += amount;
    to->reserve += amount;
    to->settled_in += amount;
    from->sequence = ledger->sequence + 1u;
    to->sequence = ledger->sequence + 1u;

    if (pdtl_append_journal(ledger, PDTL_EVENT_CELL_TRANSFER, from_cell, to_cell, PDTL_NO_INDEX, amount, before_from, from->reserve, label) != PDTL_OK) {
        return ledger->last_error;
    }
    return pdtl_append_journal(ledger, PDTL_EVENT_CELL_TRANSFER, to_cell, from_cell, PDTL_NO_INDEX, amount, before_to, to->reserve, label);
}

PdtlErrorCode pdtl_consolidate_cell(
    PdtlLedger *ledger,
    uint32_t target_cell,
    uint32_t source_cell,
    const char *label)
{
    PdtlCell *target = pdtl_get_cell(ledger, target_cell);
    PdtlCell *source = pdtl_get_cell(ledger, source_cell);
    const PdtlPolicy *policy;
    PdtlAmount before_target;
    PdtlAmount moved;

    if (target == NULL || source == NULL) {
        return pdtl_set_error(ledger, PDTL_ERR_NOT_FOUND, "consolidation cell is unknown");
    }
    if (target_cell == source_cell) {
        return pdtl_set_error(ledger, PDTL_ERR_ARGUMENT, "cannot consolidate a cell into itself");
    }
    if (target->state != PDTL_CELL_OPEN || source->state != PDTL_CELL_OPEN) {
        return pdtl_set_error(ledger, PDTL_ERR_CELL_CLOSED, "consolidation touches a closed cell");
    }
    policy = pdtl_find_policy_const(ledger, target->policy);
    if (policy == NULL || (policy->flags & PDTL_POLICY_CONSOLIDATE) == 0u) {
        return pdtl_set_error(ledger, PDTL_ERR_POLICY, "target policy does not allow consolidation");
    }
    if (!pdtl_string_eq(target->owner, source->owner) ||
        !pdtl_string_eq(target->asset, source->asset) ||
        !pdtl_string_eq(target->route, source->route) ||
        !pdtl_string_eq(target->policy, source->policy)) {
        return pdtl_set_error(ledger, PDTL_ERR_POLICY, "cells are not consolidation-compatible");
    }
    if (source->reserved_out != 0) {
        return pdtl_set_error(ledger, PDTL_ERR_BALANCE, "source cell has pending reserved obligations");
    }

    before_target = target->reserve;
    moved = source->reserve;
    target->reserve += moved;
    target->settled_in += moved;
    source->reserve = 0;
    source->state = PDTL_CELL_CLOSED;
    source->sequence = ledger->sequence + 1u;
    target->sequence = ledger->sequence + 1u;

    if (pdtl_append_journal(ledger, PDTL_EVENT_CELL_CONSOLIDATED, source_cell, target_cell, PDTL_NO_INDEX, moved, moved, 0, label) != PDTL_OK) {
        return ledger->last_error;
    }
    return pdtl_append_journal(ledger, PDTL_EVENT_CELL_CONSOLIDATED, target_cell, source_cell, PDTL_NO_INDEX, moved, before_target, target->reserve, label);
}

PdtlErrorCode pdtl_issue_receipt(
    PdtlLedger *ledger,
    uint32_t origin_cell,
    uint32_t beneficiary_cell,
    PdtlAmount gross,
    uint64_t nonce,
    uint32_t *out_receipt)
{
    PdtlCell *origin = pdtl_get_cell(ledger, origin_cell);
    PdtlCell *beneficiary = pdtl_get_cell(ledger, beneficiary_cell);
    PdtlRoute *route;
    PdtlReceipt *receipt;
    uint32_t receipt_index;
    PdtlAmount before_reserved;

    if (origin == NULL || beneficiary == NULL) {
        return pdtl_set_error(ledger, PDTL_ERR_NOT_FOUND, "receipt cell is unknown");
    }
    if (origin->state != PDTL_CELL_OPEN || beneficiary->state != PDTL_CELL_OPEN) {
        return pdtl_set_error(ledger, PDTL_ERR_CELL_CLOSED, "receipt touches a closed cell");
    }
    if (!pdtl_string_eq(origin->asset, beneficiary->asset)) {
        return pdtl_set_error(ledger, PDTL_ERR_ASSET, "receipt asset mismatch");
    }
    route = pdtl_find_route(ledger, origin->route);
    if (route == NULL) {
        return pdtl_set_error(ledger, PDTL_ERR_ROUTE, "origin route is unknown");
    }
    if (route->closed) {
        return pdtl_set_error(ledger, PDTL_ERR_ROUTE_CLOSED, "origin route is closed");
    }
    if (pdtl_cell_available(ledger, origin_cell) < gross) {
        return pdtl_set_error(ledger, PDTL_ERR_BALANCE, "receipt exceeds origin available balance");
    }
    if (ledger->receipt_count >= PDTL_MAX_RECEIPTS) {
        return pdtl_set_error(ledger, PDTL_ERR_CAPACITY, "receipt capacity reached");
    }

    receipt_index = (uint32_t)ledger->receipt_count;
    receipt = &ledger->receipts[ledger->receipt_count++];
    memset(receipt, 0, sizeof(*receipt));
    receipt->index = receipt_index;
    pdtl_make_id(receipt->id, "rcpt", origin_cell, beneficiary_cell, nonce);
    receipt->origin_cell = origin_cell;
    receipt->beneficiary_cell = beneficiary_cell;
    receipt->paid_by_cell = PDTL_NO_INDEX;
    pdtl_copy_string(receipt->asset, sizeof(receipt->asset), origin->asset);
    pdtl_copy_string(receipt->route, sizeof(receipt->route), origin->route);
    pdtl_copy_string(receipt->policy, sizeof(receipt->policy), origin->policy);
    receipt->gross = gross;
    receipt->fee = (gross * route->fee_bps) / 10000u;
    receipt->net = gross - receipt->fee;
    receipt->nonce = nonce;
    pdtl_route_digest(route, receipt->route_digest);
    receipt->status = PDTL_RECEIPT_PENDING;

    before_reserved = origin->reserved_out;
    origin->reserved_out += gross;
    origin->sequence = ledger->sequence + 1u;

    if (out_receipt != NULL) {
        *out_receipt = receipt_index;
    }
    return pdtl_append_journal(
        ledger,
        PDTL_EVENT_RECEIPT_ISSUED,
        origin_cell,
        beneficiary_cell,
        receipt_index,
        gross,
        before_reserved,
        origin->reserved_out,
        "receipt generated by origin cell");
}

static PdtlErrorCode pdtl_validate_settlement_shape(
    PdtlLedger *ledger,
    PdtlReceipt *receipt,
    PdtlCell *payer,
    uint32_t paying_cell)
{
    PdtlRoute *route;

    if (receipt == NULL || payer == NULL) {
        return pdtl_set_error(ledger, PDTL_ERR_NOT_FOUND, "settlement object is unknown");
    }
    if (receipt->status != PDTL_RECEIPT_PENDING) {
        return pdtl_set_error(ledger, PDTL_ERR_RECEIPT_STATE, "receipt is not pending");
    }
    if (payer->state != PDTL_CELL_OPEN) {
        return pdtl_set_error(ledger, PDTL_ERR_CELL_CLOSED, "paying cell is closed");
    }
    route = pdtl_find_route(ledger, receipt->route);
    if (route == NULL) {
        return pdtl_set_error(ledger, PDTL_ERR_ROUTE, "receipt route is unknown");
    }
    if (route->closed) {
        return pdtl_set_error(ledger, PDTL_ERR_ROUTE_CLOSED, "receipt route is closed");
    }
    if (!pdtl_string_eq(payer->asset, receipt->asset)) {
        return pdtl_set_error(ledger, PDTL_ERR_ASSET, "paying cell asset does not match receipt");
    }
    if (!pdtl_string_eq(payer->route, receipt->route) || !pdtl_string_eq(payer->policy, receipt->policy)) {
        return pdtl_set_error(ledger, PDTL_ERR_POLICY, "paying cell has incompatible route surface");
    }
    if (paying_cell == receipt->origin_cell) {
        if (payer->reserve < receipt->gross || payer->reserved_out < receipt->gross) {
            return pdtl_set_error(ledger, PDTL_ERR_BALANCE, "origin cell cannot cover its reserved receipt");
        }
    } else if (pdtl_cell_available(ledger, paying_cell) < receipt->gross) {
        return pdtl_set_error(ledger, PDTL_ERR_BALANCE, "foreign paying cell cannot cover receipt");
    }
    return PDTL_OK;
}

static PdtlErrorCode pdtl_apply_settlement(
    PdtlLedger *ledger,
    PdtlReceipt *receipt,
    uint32_t paying_cell)
{
    PdtlCell *payer = pdtl_get_cell(ledger, paying_cell);
    PdtlCell *origin = pdtl_get_cell(ledger, receipt->origin_cell);
    PdtlCell *beneficiary = pdtl_get_cell(ledger, receipt->beneficiary_cell);
    PdtlRoute *route = pdtl_find_route(ledger, receipt->route);
    PdtlCell *fee_cell = NULL;
    PdtlAmount before_payer;
    PdtlAmount before_beneficiary;
    PdtlAmount before_fee = 0;

    if (payer == NULL || origin == NULL || beneficiary == NULL || route == NULL) {
        return pdtl_set_error(ledger, PDTL_ERR_NOT_FOUND, "settlement state is incomplete");
    }
    if (route->fee_cell != PDTL_NO_INDEX) {
        fee_cell = pdtl_get_cell(ledger, route->fee_cell);
        if (fee_cell == NULL) {
            return pdtl_set_error(ledger, PDTL_ERR_NOT_FOUND, "fee cell is unknown");
        }
    }
    if (origin->reserved_out < receipt->gross) {
        return pdtl_set_error(ledger, PDTL_ERR_RECONCILIATION, "origin reserved amount underflow");
    }

    before_payer = payer->reserve;
    before_beneficiary = beneficiary->reserve;
    if (fee_cell != NULL) {
        before_fee = fee_cell->reserve;
    }

    payer->reserve -= receipt->gross;
    payer->settled_out += receipt->gross;
    beneficiary->reserve += receipt->net;
    beneficiary->settled_in += receipt->net;
    if (fee_cell != NULL && receipt->fee > 0) {
        fee_cell->reserve += receipt->fee;
        fee_cell->settled_in += receipt->fee;
    }
    origin->reserved_out -= receipt->gross;
    receipt->paid_by_cell = paying_cell;
    receipt->status = PDTL_RECEIPT_SETTLED;
    payer->sequence = ledger->sequence + 1u;
    beneficiary->sequence = ledger->sequence + 1u;
    origin->sequence = ledger->sequence + 1u;
    if (fee_cell != NULL) {
        fee_cell->sequence = ledger->sequence + 1u;
    }

    if (pdtl_append_journal(ledger, PDTL_EVENT_RECEIPT_SETTLED, paying_cell, receipt->beneficiary_cell, receipt->index, receipt->gross, before_payer, payer->reserve, "receipt paid by selected cell") != PDTL_OK) {
        return ledger->last_error;
    }
    if (pdtl_append_journal(ledger, PDTL_EVENT_RECEIPT_SETTLED, receipt->beneficiary_cell, paying_cell, receipt->index, receipt->net, before_beneficiary, beneficiary->reserve, "receipt beneficiary credited") != PDTL_OK) {
        return ledger->last_error;
    }
    if (fee_cell != NULL && receipt->fee > 0) {
        return pdtl_append_journal(ledger, PDTL_EVENT_RECEIPT_SETTLED, route->fee_cell, paying_cell, receipt->index, receipt->fee, before_fee, fee_cell->reserve, "route fee credited");
    }
    return PDTL_OK;
}

PdtlErrorCode pdtl_settle_receipt_vulnerable(
    PdtlLedger *ledger,
    uint32_t receipt_index,
    uint32_t paying_cell)
{
    PdtlReceipt *receipt = pdtl_get_receipt(ledger, receipt_index);
    PdtlCell *payer = pdtl_get_cell(ledger, paying_cell);
    PdtlErrorCode shape;

    /*
     * CTF bug: the final reconciliation validates that some compatible cell
     * pays the receipt and that global reserves are conserved. It does not
     * require the paying cell to be the same cell that generated the receipt.
     */
    shape = pdtl_validate_settlement_shape(ledger, receipt, payer, paying_cell);
    if (shape != PDTL_OK) {
        return shape;
    }
    return pdtl_apply_settlement(ledger, receipt, paying_cell);
}

PdtlErrorCode pdtl_settle_receipt_strict(
    PdtlLedger *ledger,
    uint32_t receipt_index,
    uint32_t paying_cell)
{
    PdtlReceipt *receipt = pdtl_get_receipt(ledger, receipt_index);
    PdtlCell *payer = pdtl_get_cell(ledger, paying_cell);
    PdtlErrorCode shape;

    shape = pdtl_validate_settlement_shape(ledger, receipt, payer, paying_cell);
    if (shape != PDTL_OK) {
        return shape;
    }
    if (receipt->origin_cell != paying_cell) {
        (void)pdtl_append_journal(
            ledger,
            PDTL_EVENT_SETTLEMENT_REJECTED,
            paying_cell,
            receipt->origin_cell,
            receipt_index,
            receipt->gross,
            payer->reserve,
            payer->reserve,
            "strict binding rejected foreign payer");
        return pdtl_set_error(ledger, PDTL_ERR_BINDING, "paying cell does not match receipt origin");
    }
    return pdtl_apply_settlement(ledger, receipt, paying_cell);
}

PdtlErrorCode pdtl_cancel_receipt(
    PdtlLedger *ledger,
    uint32_t receipt_index,
    const char *label)
{
    PdtlReceipt *receipt = pdtl_get_receipt(ledger, receipt_index);
    PdtlCell *origin;
    PdtlAmount before_reserved;

    if (receipt == NULL) {
        return pdtl_set_error(ledger, PDTL_ERR_NOT_FOUND, "receipt is unknown");
    }
    if (receipt->status != PDTL_RECEIPT_PENDING) {
        return pdtl_set_error(ledger, PDTL_ERR_RECEIPT_STATE, "only pending receipts can be cancelled");
    }
    origin = pdtl_get_cell(ledger, receipt->origin_cell);
    if (origin == NULL) {
        return pdtl_set_error(ledger, PDTL_ERR_NOT_FOUND, "receipt origin is unknown");
    }
    if (origin->reserved_out < receipt->gross) {
        return pdtl_set_error(ledger, PDTL_ERR_RECONCILIATION, "origin reserved amount underflow during cancel");
    }

    before_reserved = origin->reserved_out;
    origin->reserved_out -= receipt->gross;
    origin->sequence = ledger->sequence + 1u;
    receipt->status = PDTL_RECEIPT_CANCELLED;
    receipt->paid_by_cell = PDTL_NO_INDEX;

    return pdtl_append_journal(
        ledger,
        PDTL_EVENT_RECEIPT_CANCELLED,
        receipt->origin_cell,
        receipt->beneficiary_cell,
        receipt_index,
        receipt->gross,
        before_reserved,
        origin->reserved_out,
        label == NULL ? "receipt cancelled and reserve released" : label);
}

PdtlErrorCode pdtl_withdraw(
    PdtlLedger *ledger,
    uint32_t cell_index,
    PdtlAmount amount,
    const char *label)
{
    PdtlCell *cell = pdtl_get_cell(ledger, cell_index);
    const PdtlPolicy *policy;
    PdtlAmount before;

    if (cell == NULL) {
        return pdtl_set_error(ledger, PDTL_ERR_NOT_FOUND, "withdrawal cell is unknown");
    }
    if (cell->state != PDTL_CELL_OPEN) {
        return pdtl_set_error(ledger, PDTL_ERR_CELL_CLOSED, "withdrawal cell is closed");
    }
    policy = pdtl_find_policy_const(ledger, cell->policy);
    if (policy == NULL || (policy->flags & PDTL_POLICY_WITHDRAW) == 0u) {
        return pdtl_set_error(ledger, PDTL_ERR_POLICY, "cell policy does not allow withdrawal");
    }
    if (pdtl_cell_available(ledger, cell_index) < amount) {
        return pdtl_set_error(ledger, PDTL_ERR_BALANCE, "withdrawal exceeds available balance");
    }

    before = cell->reserve;
    cell->reserve -= amount;
    cell->withdrawn += amount;
    ledger->external_withdrawals += amount;
    cell->sequence = ledger->sequence + 1u;
    return pdtl_append_journal(ledger, PDTL_EVENT_WITHDRAWAL, cell_index, PDTL_NO_INDEX, PDTL_NO_INDEX, amount, before, cell->reserve, label);
}

PdtlErrorCode pdtl_update_route_fee(
    PdtlLedger *ledger,
    const char *route_name,
    uint32_t new_fee_bps,
    uint32_t new_fee_cell)
{
    PdtlRoute *route = pdtl_find_route(ledger, route_name);
    PdtlCell *fee_cell;
    uint32_t old_fee_bps;

    if (route == NULL) {
        return pdtl_set_error(ledger, PDTL_ERR_ROUTE, "route is unknown");
    }
    if (route->closed) {
        return pdtl_set_error(ledger, PDTL_ERR_ROUTE_CLOSED, "closed route fee cannot be updated");
    }
    if (new_fee_bps > 10000u) {
        return pdtl_set_error(ledger, PDTL_ERR_ROUTE, "fee bps cannot exceed 10000");
    }
    if (pdtl_route_has_pending_receipts(ledger, route_name)) {
        return pdtl_set_error(ledger, PDTL_ERR_ROUTE, "route has pending receipts");
    }
    if (new_fee_cell != PDTL_NO_INDEX) {
        fee_cell = pdtl_get_cell(ledger, new_fee_cell);
        if (fee_cell == NULL) {
            return pdtl_set_error(ledger, PDTL_ERR_NOT_FOUND, "new fee cell is unknown");
        }
        if (fee_cell->state != PDTL_CELL_OPEN) {
            return pdtl_set_error(ledger, PDTL_ERR_CELL_CLOSED, "new fee cell is closed");
        }
        if (!pdtl_string_eq(fee_cell->asset, route->asset)) {
            return pdtl_set_error(ledger, PDTL_ERR_ASSET, "new fee cell asset mismatch");
        }
    }

    old_fee_bps = route->fee_bps;
    route->fee_bps = new_fee_bps;
    route->fee_cell = new_fee_cell;
    return pdtl_append_journal(
        ledger,
        PDTL_EVENT_ROUTE_FEE_UPDATED,
        new_fee_cell,
        PDTL_NO_INDEX,
        PDTL_NO_INDEX,
        new_fee_bps,
        old_fee_bps,
        new_fee_bps,
        route_name);
}

int pdtl_route_has_pending_receipts(const PdtlLedger *ledger, const char *route_name)
{
    size_t i;

    for (i = 0; i < ledger->receipt_count; i++) {
        if (ledger->receipts[i].status == PDTL_RECEIPT_PENDING &&
            pdtl_string_eq(ledger->receipts[i].route, route_name)) {
            return 1;
        }
    }
    return 0;
}

PdtlErrorCode pdtl_close_route(PdtlLedger *ledger, const char *route_name)
{
    PdtlRoute *route = pdtl_find_route(ledger, route_name);
    size_t i;

    if (route == NULL) {
        return pdtl_set_error(ledger, PDTL_ERR_ROUTE, "route is unknown");
    }
    if (route->closed) {
        return PDTL_OK;
    }
    if (pdtl_route_has_pending_receipts(ledger, route_name)) {
        return pdtl_set_error(ledger, PDTL_ERR_ROUTE, "route has pending receipts");
    }
    for (i = 0; i < ledger->cell_count; i++) {
        if (pdtl_string_eq(ledger->cells[i].route, route_name) && ledger->cells[i].reserved_out != 0) {
            return pdtl_set_error(ledger, PDTL_ERR_ROUTE, "route has cells with reserved obligations");
        }
    }
    route->closed = 1;
    return pdtl_append_journal(ledger, PDTL_EVENT_ROUTE_CLOSED, PDTL_NO_INDEX, PDTL_NO_INDEX, PDTL_NO_INDEX, 0, 0, 0, route_name);
}

PdtlAmount pdtl_total_cell_reserves(const PdtlLedger *ledger)
{
    PdtlAmount total = 0;
    size_t i;

    for (i = 0; i < ledger->cell_count; i++) {
        total += ledger->cells[i].reserve;
    }
    return total;
}

PdtlAmount pdtl_total_reserved_out(const PdtlLedger *ledger)
{
    PdtlAmount total = 0;
    size_t i;

    for (i = 0; i < ledger->cell_count; i++) {
        total += ledger->cells[i].reserved_out;
    }
    return total;
}

PdtlAmount pdtl_total_settled_in(const PdtlLedger *ledger)
{
    PdtlAmount total = 0;
    size_t i;

    for (i = 0; i < ledger->cell_count; i++) {
        total += ledger->cells[i].settled_in;
    }
    return total;
}

PdtlAmount pdtl_total_settled_out(const PdtlLedger *ledger)
{
    PdtlAmount total = 0;
    size_t i;

    for (i = 0; i < ledger->cell_count; i++) {
        total += ledger->cells[i].settled_out;
    }
    return total;
}

PdtlAmount pdtl_route_reserve_total(const PdtlLedger *ledger, const char *route_name)
{
    PdtlAmount total = 0;
    size_t i;

    for (i = 0; i < ledger->cell_count; i++) {
        if (pdtl_string_eq(ledger->cells[i].route, route_name)) {
            total += ledger->cells[i].reserve;
        }
    }
    return total;
}

PdtlAmount pdtl_route_available_total(const PdtlLedger *ledger, const char *route_name)
{
    PdtlAmount total = 0;
    size_t i;

    for (i = 0; i < ledger->cell_count; i++) {
        if (pdtl_string_eq(ledger->cells[i].route, route_name)) {
            total += pdtl_cell_available(ledger, (uint32_t)i);
        }
    }
    return total;
}

PdtlAmount pdtl_route_reserved_total(const PdtlLedger *ledger, const char *route_name)
{
    PdtlAmount total = 0;
    size_t i;

    for (i = 0; i < ledger->cell_count; i++) {
        if (pdtl_string_eq(ledger->cells[i].route, route_name)) {
            total += ledger->cells[i].reserved_out;
        }
    }
    return total;
}

PdtlAmount pdtl_route_pending_gross(const PdtlLedger *ledger, const char *route_name)
{
    PdtlAmount total = 0;
    size_t i;

    for (i = 0; i < ledger->receipt_count; i++) {
        if (ledger->receipts[i].status == PDTL_RECEIPT_PENDING &&
            pdtl_string_eq(ledger->receipts[i].route, route_name)) {
            total += ledger->receipts[i].gross;
        }
    }
    return total;
}

PdtlErrorCode pdtl_route_exposure(
    const PdtlLedger *ledger,
    const char *route_name,
    PdtlRouteExposure *out)
{
    const PdtlRoute *route = pdtl_find_route_const(ledger, route_name);
    const PdtlPolicy *policy;
    size_t i;

    if (route == NULL) {
        return PDTL_ERR_ROUTE;
    }
    memset(out, 0, sizeof(*out));
    pdtl_copy_string(out->route, sizeof(out->route), route->name);
    pdtl_copy_string(out->asset, sizeof(out->asset), route->asset);
    pdtl_copy_string(out->policy, sizeof(out->policy), route->policy);
    out->fee_bps = route->fee_bps;
    out->fee_cell = route->fee_cell;
    out->closed = route->closed;

    for (i = 0; i < ledger->cell_count; i++) {
        const PdtlCell *cell = &ledger->cells[i];
        if (!pdtl_string_eq(cell->route, route_name)) {
            continue;
        }
        if (cell->state == PDTL_CELL_OPEN) {
            out->open_cells++;
        } else {
            out->closed_cells++;
        }
        out->reserves += cell->reserve;
        out->available += pdtl_cell_available(ledger, (uint32_t)i);
        out->reserved_out += cell->reserved_out;
    }

    for (i = 0; i < ledger->receipt_count; i++) {
        const PdtlReceipt *receipt = &ledger->receipts[i];
        if (!pdtl_string_eq(receipt->route, route_name)) {
            continue;
        }
        if (receipt->status == PDTL_RECEIPT_PENDING) {
            out->pending_receipts++;
            out->pending_gross += receipt->gross;
        } else if (receipt->status == PDTL_RECEIPT_SETTLED) {
            out->settled_receipts++;
            out->settled_gross += receipt->gross;
        } else if (receipt->status == PDTL_RECEIPT_CANCELLED) {
            out->cancelled_receipts++;
            out->cancelled_gross += receipt->gross;
        }
    }

    policy = pdtl_find_policy_const(ledger, route->policy);
    if (policy != NULL) {
        out->max_exit_budget = (out->reserves * policy->max_exit_bps) / 10000u;
    }
    if (out->reserves > 0) {
        out->utilization_bps = (uint32_t)((out->reserved_out * 10000u) / out->reserves);
    }
    if (out->utilization_bps >= 10000u) {
        out->health_score = 0;
    } else {
        out->health_score = 100u - (out->utilization_bps / 100u);
    }
    return PDTL_OK;
}

int pdtl_global_reconciliation_ok(const PdtlLedger *ledger)
{
    PdtlAmount left = pdtl_total_cell_reserves(ledger) + ledger->external_withdrawals + ledger->burned_after_genesis;
    PdtlAmount right = ledger->initial_supply + ledger->minted_after_genesis;
    return left == right;
}

int pdtl_receipt_bindings_ok(const PdtlLedger *ledger)
{
    size_t i;

    for (i = 0; i < ledger->receipt_count; i++) {
        const PdtlReceipt *receipt = &ledger->receipts[i];
        if (receipt->status == PDTL_RECEIPT_SETTLED && receipt->paid_by_cell != receipt->origin_cell) {
            return 0;
        }
    }
    return 1;
}
