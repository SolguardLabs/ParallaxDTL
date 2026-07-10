#include "pdtl.h"

#include <stdio.h>
#include <string.h>

typedef struct PdtlScenarioCells {
    uint32_t treasury;
    uint32_t origin;
    uint32_t aux;
    uint32_t sponsor;
    uint32_t beneficiary;
} PdtlScenarioCells;

static void pdtl_result_init(PdtlScenarioResult *result, const char *scenario)
{
    memset(result, 0, sizeof(*result));
    (void)snprintf(result->scenario, sizeof(result->scenario), "%s", scenario);
    result->origin_cell = PDTL_NO_INDEX;
    result->aux_cell = PDTL_NO_INDEX;
    result->sponsor_cell = PDTL_NO_INDEX;
    result->beneficiary_cell = PDTL_NO_INDEX;
    result->treasury_cell = PDTL_NO_INDEX;
    result->receipt = PDTL_NO_INDEX;
    result->secondary_receipt = PDTL_NO_INDEX;
}

static PdtlErrorCode pdtl_setup_surface(PdtlLedger *ledger, PdtlScenarioCells *cells)
{
    PdtlErrorCode err;

    pdtl_init(ledger, "parallax-localnet-ctf");
    memset(cells, 0, sizeof(*cells));
    cells->treasury = PDTL_NO_INDEX;
    cells->origin = PDTL_NO_INDEX;
    cells->aux = PDTL_NO_INDEX;
    cells->sponsor = PDTL_NO_INDEX;
    cells->beneficiary = PDTL_NO_INDEX;

    err = pdtl_add_policy(
        ledger,
        "treasury-policy",
        PDTL_POLICY_TREASURY | PDTL_POLICY_WITHDRAW,
        0,
        0,
        10000);
    if (err != PDTL_OK) {
        return err;
    }
    err = pdtl_add_policy(
        ledger,
        "retail-exit",
        PDTL_POLICY_WITHDRAW | PDTL_POLICY_CONSOLIDATE | PDTL_POLICY_ALLOW_ROUTE_CLOSE,
        4,
        20,
        9000);
    if (err != PDTL_OK) {
        return err;
    }
    err = pdtl_add_route(ledger, "treasury", "USDC", "treasury-policy", 0, PDTL_NO_INDEX);
    if (err != PDTL_OK) {
        return err;
    }
    err = pdtl_create_cell(ledger, "protocol-fees", "USDC", "treasury", "treasury-policy", &cells->treasury);
    if (err != PDTL_OK) {
        return err;
    }
    err = pdtl_add_route(ledger, "blue-route", "USDC", "retail-exit", 200, cells->treasury);
    if (err != PDTL_OK) {
        return err;
    }
    err = pdtl_create_cell(ledger, "alice", "USDC", "blue-route", "retail-exit", &cells->origin);
    if (err != PDTL_OK) {
        return err;
    }
    err = pdtl_create_cell(ledger, "alice", "USDC", "blue-route", "retail-exit", &cells->aux);
    if (err != PDTL_OK) {
        return err;
    }
    err = pdtl_create_cell(ledger, "sponsor-pool", "USDC", "blue-route", "retail-exit", &cells->sponsor);
    if (err != PDTL_OK) {
        return err;
    }
    err = pdtl_create_cell(ledger, "merchant", "USDC", "blue-route", "retail-exit", &cells->beneficiary);
    if (err != PDTL_OK) {
        return err;
    }

    err = pdtl_deposit_genesis(ledger, cells->origin, 1000);
    if (err != PDTL_OK) {
        return err;
    }
    err = pdtl_deposit_genesis(ledger, cells->aux, 200);
    if (err != PDTL_OK) {
        return err;
    }
    err = pdtl_deposit_genesis(ledger, cells->sponsor, 2000);
    if (err != PDTL_OK) {
        return err;
    }
    return PDTL_OK;
}

static void pdtl_capture_cells(PdtlScenarioResult *result, const PdtlScenarioCells *cells)
{
    result->origin_cell = cells->origin;
    result->aux_cell = cells->aux;
    result->sponsor_cell = cells->sponsor;
    result->beneficiary_cell = cells->beneficiary;
    result->treasury_cell = cells->treasury;
}

static PdtlErrorCode pdtl_run_snapshot(PdtlLedger *ledger, PdtlScenarioResult *result)
{
    PdtlScenarioCells cells;
    PdtlErrorCode err = pdtl_setup_surface(ledger, &cells);

    if (err != PDTL_OK) {
        return err;
    }
    pdtl_capture_cells(result, &cells);
    result->settlement_accepted = 0;
    return PDTL_OK;
}

static PdtlErrorCode pdtl_run_flow(PdtlLedger *ledger, PdtlScenarioResult *result)
{
    PdtlScenarioCells cells;
    PdtlErrorCode err = pdtl_setup_surface(ledger, &cells);
    uint32_t receipt;

    if (err != PDTL_OK) {
        return err;
    }
    pdtl_capture_cells(result, &cells);

    err = pdtl_transfer_between_cells(ledger, cells.aux, cells.origin, 100, "rebalance aux into origin");
    if (err != PDTL_OK) {
        return err;
    }
    err = pdtl_consolidate_cell(ledger, cells.origin, cells.aux, "merge aux liquidity after route rebalance");
    if (err != PDTL_OK) {
        return err;
    }
    err = pdtl_issue_receipt(ledger, cells.origin, cells.beneficiary, 700, 42, &receipt);
    if (err != PDTL_OK) {
        return err;
    }
    result->receipt = receipt;
    result->legitimate_withdrawable_before = pdtl_cell_available(ledger, cells.origin);

    err = pdtl_settle_receipt_vulnerable(ledger, receipt, cells.origin);
    if (err != PDTL_OK) {
        return err;
    }
    result->settlement_accepted = 1;
    result->third_party_paid = 0;
    result->available_after_foreign_payment = pdtl_cell_available(ledger, cells.origin);

    err = pdtl_withdraw(ledger, cells.origin, 280, "operator withdrawal after normal settlement");
    if (err != PDTL_OK) {
        return err;
    }
    result->attacker_withdrawn = 280;

    err = pdtl_close_route(ledger, "blue-route");
    if (err != PDTL_OK) {
        return err;
    }
    result->route_closed = 1;
    return PDTL_OK;
}

static PdtlErrorCode pdtl_run_exploit(PdtlLedger *ledger, PdtlScenarioResult *result)
{
    PdtlScenarioCells cells;
    PdtlErrorCode err = pdtl_setup_surface(ledger, &cells);
    uint32_t receipt;
    PdtlAmount available_after;

    if (err != PDTL_OK) {
        return err;
    }
    pdtl_capture_cells(result, &cells);

    err = pdtl_issue_receipt(ledger, cells.origin, cells.beneficiary, 700, 777, &receipt);
    if (err != PDTL_OK) {
        return err;
    }
    result->receipt = receipt;
    result->legitimate_withdrawable_before = pdtl_cell_available(ledger, cells.origin);

    err = pdtl_settle_receipt_vulnerable(ledger, receipt, cells.sponsor);
    if (err != PDTL_OK) {
        return err;
    }
    result->settlement_accepted = 1;
    result->third_party_paid = 700;
    available_after = pdtl_cell_available(ledger, cells.origin);
    result->available_after_foreign_payment = available_after;
    if (available_after > result->legitimate_withdrawable_before) {
        result->excess_withdrawal = available_after - result->legitimate_withdrawable_before;
    }

    err = pdtl_withdraw(ledger, cells.origin, available_after, "withdraw released surplus after foreign payment");
    if (err != PDTL_OK) {
        return err;
    }
    result->attacker_withdrawn = available_after;

    err = pdtl_close_route(ledger, "blue-route");
    if (err != PDTL_OK) {
        return err;
    }
    result->route_closed = 1;
    return PDTL_OK;
}

static PdtlErrorCode pdtl_run_strict_exploit(PdtlLedger *ledger, PdtlScenarioResult *result)
{
    PdtlScenarioCells cells;
    PdtlErrorCode err = pdtl_setup_surface(ledger, &cells);
    uint32_t receipt;

    if (err != PDTL_OK) {
        return err;
    }
    pdtl_capture_cells(result, &cells);

    err = pdtl_issue_receipt(ledger, cells.origin, cells.beneficiary, 700, 778, &receipt);
    if (err != PDTL_OK) {
        return err;
    }
    result->receipt = receipt;
    result->legitimate_withdrawable_before = pdtl_cell_available(ledger, cells.origin);

    err = pdtl_settle_receipt_strict(ledger, receipt, cells.sponsor);
    if (err == PDTL_ERR_BINDING) {
        result->settlement_accepted = 0;
        result->strict_rejection = 1;
        result->third_party_paid = 0;
        result->available_after_foreign_payment = pdtl_cell_available(ledger, cells.origin);
        pdtl_clear_error(ledger);
        return PDTL_OK;
    }
    if (err != PDTL_OK) {
        return err;
    }
    result->settlement_accepted = 1;
    return PDTL_OK;
}

static PdtlErrorCode pdtl_run_cancel(PdtlLedger *ledger, PdtlScenarioResult *result)
{
    PdtlScenarioCells cells;
    PdtlErrorCode err = pdtl_setup_surface(ledger, &cells);
    uint32_t receipt;
    PdtlAmount available_after_cancel;

    if (err != PDTL_OK) {
        return err;
    }
    pdtl_capture_cells(result, &cells);

    err = pdtl_issue_receipt(ledger, cells.origin, cells.beneficiary, 300, 601, &receipt);
    if (err != PDTL_OK) {
        return err;
    }
    result->receipt = receipt;
    result->legitimate_withdrawable_before = pdtl_cell_available(ledger, cells.origin);

    err = pdtl_cancel_receipt(ledger, receipt, "merchant timeout released origin reserve");
    if (err != PDTL_OK) {
        return err;
    }
    result->cancelled_gross = 300;
    available_after_cancel = pdtl_cell_available(ledger, cells.origin);
    result->available_after_foreign_payment = available_after_cancel;

    err = pdtl_withdraw(ledger, cells.origin, 180, "post-cancel operator withdrawal");
    if (err != PDTL_OK) {
        return err;
    }
    result->attacker_withdrawn = 180;

    err = pdtl_close_route(ledger, "blue-route");
    if (err != PDTL_OK) {
        return err;
    }
    result->route_closed = 1;
    return PDTL_OK;
}

static PdtlErrorCode pdtl_run_fee_rotation(PdtlLedger *ledger, PdtlScenarioResult *result)
{
    PdtlScenarioCells cells;
    PdtlErrorCode err = pdtl_setup_surface(ledger, &cells);
    uint32_t receipt;

    if (err != PDTL_OK) {
        return err;
    }
    pdtl_capture_cells(result, &cells);
    result->fee_before_bps = 200;

    err = pdtl_update_route_fee(ledger, "blue-route", 350, cells.treasury);
    if (err != PDTL_OK) {
        return err;
    }
    result->fee_after_bps = 350;
    result->fee_update_applied = 1;

    err = pdtl_issue_receipt(ledger, cells.sponsor, cells.beneficiary, 800, 902, &receipt);
    if (err != PDTL_OK) {
        return err;
    }
    result->receipt = receipt;
    result->legitimate_withdrawable_before = pdtl_cell_available(ledger, cells.sponsor);

    err = pdtl_settle_receipt_strict(ledger, receipt, cells.sponsor);
    if (err != PDTL_OK) {
        return err;
    }
    result->settlement_accepted = 1;
    result->available_after_foreign_payment = pdtl_cell_available(ledger, cells.sponsor);
    return PDTL_OK;
}

PdtlErrorCode pdtl_run_scenario(
    const char *scenario,
    PdtlLedger *ledger,
    PdtlScenarioResult *result)
{
    const char *name = scenario;

    if (name == NULL || name[0] == '\0') {
        name = "flow";
    }
    pdtl_result_init(result, name);

    if (strcmp(name, "snapshot") == 0) {
        return pdtl_run_snapshot(ledger, result);
    }
    if (strcmp(name, "flow") == 0 || strcmp(name, "routed") == 0) {
        (void)snprintf(result->scenario, sizeof(result->scenario), "flow");
        return pdtl_run_flow(ledger, result);
    }
    if (strcmp(name, "exploit") == 0) {
        return pdtl_run_exploit(ledger, result);
    }
    if (strcmp(name, "strict-exploit") == 0) {
        return pdtl_run_strict_exploit(ledger, result);
    }
    if (strcmp(name, "cancel") == 0) {
        return pdtl_run_cancel(ledger, result);
    }
    if (strcmp(name, "fee-rotation") == 0) {
        return pdtl_run_fee_rotation(ledger, result);
    }

    pdtl_init(ledger, "parallax-localnet-ctf");
    return pdtl_set_error(ledger, PDTL_ERR_ARGUMENT, "unknown scenario");
}

static void pdtl_print_cell(PdtlJson *json, const PdtlLedger *ledger, const char *role, uint32_t index)
{
    const PdtlCell *cell = pdtl_get_cell_const(ledger, index);
    char digest[PDTL_DIGEST_LEN];

    if (cell == NULL) {
        pdtl_json_prop_null(json, role);
        return;
    }
    pdtl_cell_digest(cell, digest);
    pdtl_json_prop_object_begin(json, role);
    pdtl_json_prop_u64(json, "index", cell->index);
    pdtl_json_prop_string(json, "id", cell->id);
    pdtl_json_prop_string(json, "owner", cell->owner);
    pdtl_json_prop_string(json, "asset", cell->asset);
    pdtl_json_prop_string(json, "route", cell->route);
    pdtl_json_prop_string(json, "policy", cell->policy);
    pdtl_json_prop_string(json, "state", pdtl_cell_state_name(cell->state));
    pdtl_json_prop_u64(json, "reserve", cell->reserve);
    pdtl_json_prop_u64(json, "reserved_out", cell->reserved_out);
    pdtl_json_prop_u64(json, "available", pdtl_cell_available(ledger, index));
    pdtl_json_prop_u64(json, "settled_in", cell->settled_in);
    pdtl_json_prop_u64(json, "settled_out", cell->settled_out);
    pdtl_json_prop_u64(json, "withdrawn", cell->withdrawn);
    pdtl_json_prop_string(json, "digest", digest);
    pdtl_json_end_object(json);
}

static void pdtl_print_receipts(PdtlJson *json, const PdtlLedger *ledger)
{
    size_t i;

    pdtl_json_prop_array_begin(json, "receipts");
    for (i = 0; i < ledger->receipt_count; i++) {
        const PdtlReceipt *receipt = &ledger->receipts[i];
        char digest[PDTL_DIGEST_LEN];
        pdtl_receipt_digest(receipt, digest);

        pdtl_json_array_object_begin(json);
        pdtl_json_prop_u64(json, "index", receipt->index);
        pdtl_json_prop_string(json, "id", receipt->id);
        pdtl_json_prop_u64(json, "origin_cell", receipt->origin_cell);
        pdtl_json_prop_u64(json, "beneficiary_cell", receipt->beneficiary_cell);
        if (receipt->paid_by_cell == PDTL_NO_INDEX) {
            pdtl_json_prop_null(json, "paid_by_cell");
        } else {
            pdtl_json_prop_u64(json, "paid_by_cell", receipt->paid_by_cell);
        }
        pdtl_json_prop_string(json, "asset", receipt->asset);
        pdtl_json_prop_string(json, "route", receipt->route);
        pdtl_json_prop_string(json, "policy", receipt->policy);
        pdtl_json_prop_u64(json, "gross", receipt->gross);
        pdtl_json_prop_u64(json, "fee", receipt->fee);
        pdtl_json_prop_u64(json, "net", receipt->net);
        pdtl_json_prop_u64(json, "nonce", receipt->nonce);
        pdtl_json_prop_string(json, "status", pdtl_receipt_status_name(receipt->status));
        pdtl_json_prop_string(json, "route_digest", receipt->route_digest);
        pdtl_json_prop_string(json, "digest", digest);
        pdtl_json_prop_bool(json, "payer_matches_origin", receipt->paid_by_cell == receipt->origin_cell);
        pdtl_json_end_object(json);
    }
    pdtl_json_end_array(json);
}

static void pdtl_print_routes(PdtlJson *json, const PdtlLedger *ledger)
{
    size_t i;

    pdtl_json_prop_array_begin(json, "routes");
    for (i = 0; i < ledger->route_count; i++) {
        const PdtlRoute *route = &ledger->routes[i];
        PdtlRouteExposure exposure;
        char digest[PDTL_DIGEST_LEN];

        if (pdtl_route_exposure(ledger, route->name, &exposure) != PDTL_OK) {
            continue;
        }
        pdtl_route_digest(route, digest);
        pdtl_json_array_object_begin(json);
        pdtl_json_prop_string(json, "name", exposure.route);
        pdtl_json_prop_string(json, "asset", exposure.asset);
        pdtl_json_prop_string(json, "policy", exposure.policy);
        pdtl_json_prop_u64(json, "fee_bps", exposure.fee_bps);
        if (exposure.fee_cell == PDTL_NO_INDEX) {
            pdtl_json_prop_null(json, "fee_cell");
        } else {
            pdtl_json_prop_u64(json, "fee_cell", exposure.fee_cell);
        }
        pdtl_json_prop_bool(json, "closed", exposure.closed);
        pdtl_json_prop_u64(json, "open_cells", exposure.open_cells);
        pdtl_json_prop_u64(json, "closed_cells", exposure.closed_cells);
        pdtl_json_prop_u64(json, "reserves", exposure.reserves);
        pdtl_json_prop_u64(json, "available", exposure.available);
        pdtl_json_prop_u64(json, "reserved_out", exposure.reserved_out);
        pdtl_json_prop_u64(json, "pending_gross", exposure.pending_gross);
        pdtl_json_prop_u64(json, "settled_gross", exposure.settled_gross);
        pdtl_json_prop_u64(json, "cancelled_gross", exposure.cancelled_gross);
        pdtl_json_prop_u64(json, "pending_receipts", exposure.pending_receipts);
        pdtl_json_prop_u64(json, "settled_receipts", exposure.settled_receipts);
        pdtl_json_prop_u64(json, "cancelled_receipts", exposure.cancelled_receipts);
        pdtl_json_prop_u64(json, "max_exit_budget", exposure.max_exit_budget);
        pdtl_json_prop_u64(json, "utilization_bps", exposure.utilization_bps);
        pdtl_json_prop_u64(json, "health_score", exposure.health_score);
        pdtl_json_prop_string(json, "digest", digest);
        pdtl_json_end_object(json);
    }
    pdtl_json_end_array(json);
}

static void pdtl_print_journal_tail(PdtlJson *json, const PdtlLedger *ledger)
{
    size_t start = ledger->journal_count > 8 ? ledger->journal_count - 8 : 0;
    size_t i;

    pdtl_json_prop_array_begin(json, "journal_tail");
    for (i = start; i < ledger->journal_count; i++) {
        const PdtlJournalEntry *entry = &ledger->journal[i];

        pdtl_json_array_object_begin(json);
        pdtl_json_prop_u64(json, "sequence", entry->sequence);
        pdtl_json_prop_string(json, "kind", pdtl_event_kind_name(entry->kind));
        if (entry->cell == PDTL_NO_INDEX) {
            pdtl_json_prop_null(json, "cell");
        } else {
            pdtl_json_prop_u64(json, "cell", entry->cell);
        }
        if (entry->counterparty == PDTL_NO_INDEX) {
            pdtl_json_prop_null(json, "counterparty");
        } else {
            pdtl_json_prop_u64(json, "counterparty", entry->counterparty);
        }
        if (entry->receipt == PDTL_NO_INDEX) {
            pdtl_json_prop_null(json, "receipt");
        } else {
            pdtl_json_prop_u64(json, "receipt", entry->receipt);
        }
        pdtl_json_prop_u64(json, "amount", entry->amount);
        pdtl_json_prop_u64(json, "before_balance", entry->before_balance);
        pdtl_json_prop_u64(json, "after_balance", entry->after_balance);
        pdtl_json_prop_string(json, "label", entry->label);
        pdtl_json_prop_string(json, "digest", entry->digest);
        pdtl_json_end_object(json);
    }
    pdtl_json_end_array(json);
}

void pdtl_print_report(FILE *out, const PdtlLedger *ledger, const PdtlScenarioResult *result)
{
    PdtlJson json;
    char state_digest[PDTL_DIGEST_LEN];
    int global_ok = pdtl_global_reconciliation_ok(ledger);
    int binding_ok = pdtl_receipt_bindings_ok(ledger);

    pdtl_state_digest(ledger, state_digest);
    pdtl_json_init(&json, out);
    pdtl_json_begin_object(&json);
    pdtl_json_prop_string(&json, "scenario", result->scenario);
    pdtl_json_prop_string(&json, "network_id", ledger->network_id);
    pdtl_json_prop_string(&json, "state_digest", state_digest);
    pdtl_json_prop_bool(&json, "conservation_ok", global_ok);
    pdtl_json_prop_bool(&json, "binding_ok", binding_ok);
    pdtl_json_prop_bool(&json, "ctf_vulnerability_triggered", global_ok && !binding_ok);

    pdtl_json_prop_object_begin(&json, "surface");
    pdtl_json_prop_u64(&json, "policies", ledger->policy_count);
    pdtl_json_prop_u64(&json, "routes", ledger->route_count);
    pdtl_json_prop_u64(&json, "cells", ledger->cell_count);
    pdtl_json_prop_u64(&json, "receipts", ledger->receipt_count);
    pdtl_json_prop_u64(&json, "journal_entries", ledger->journal_count);
    pdtl_json_prop_bool(&json, "route_closed", result->route_closed);
    pdtl_json_end_object(&json);

    pdtl_json_prop_object_begin(&json, "totals");
    pdtl_json_prop_u64(&json, "initial_supply", ledger->initial_supply);
    pdtl_json_prop_u64(&json, "cell_reserves", pdtl_total_cell_reserves(ledger));
    pdtl_json_prop_u64(&json, "external_withdrawals", ledger->external_withdrawals);
    pdtl_json_prop_u64(&json, "reserved_out", pdtl_total_reserved_out(ledger));
    pdtl_json_prop_u64(&json, "settled_in", pdtl_total_settled_in(ledger));
    pdtl_json_prop_u64(&json, "settled_out", pdtl_total_settled_out(ledger));
    pdtl_json_prop_bool(&json, "global_ok", global_ok);
    pdtl_json_prop_bool(&json, "receipt_cell_binding_ok", binding_ok);
    pdtl_json_end_object(&json);

    pdtl_json_prop_object_begin(&json, "cells");
    pdtl_print_cell(&json, ledger, "origin", result->origin_cell);
    pdtl_print_cell(&json, ledger, "aux", result->aux_cell);
    pdtl_print_cell(&json, ledger, "sponsor", result->sponsor_cell);
    pdtl_print_cell(&json, ledger, "beneficiary", result->beneficiary_cell);
    pdtl_print_cell(&json, ledger, "treasury", result->treasury_cell);
    pdtl_json_end_object(&json);

    pdtl_json_prop_object_begin(&json, "exploit");
    pdtl_json_prop_bool(&json, "settlement_accepted", result->settlement_accepted);
    pdtl_json_prop_bool(&json, "strict_rejection", result->strict_rejection);
    pdtl_json_prop_u64(&json, "legitimate_withdrawable_before", result->legitimate_withdrawable_before);
    pdtl_json_prop_u64(&json, "available_after_foreign_payment", result->available_after_foreign_payment);
    pdtl_json_prop_u64(&json, "attacker_withdrawn", result->attacker_withdrawn);
    pdtl_json_prop_u64(&json, "third_party_paid", result->third_party_paid);
    pdtl_json_prop_u64(&json, "excess_withdrawal", result->excess_withdrawal);
    pdtl_json_prop_bool(&json, "payer_matches_receipt_origin", binding_ok);
    pdtl_json_end_object(&json);

    pdtl_json_prop_object_begin(&json, "operations");
    if (result->secondary_receipt == PDTL_NO_INDEX) {
        pdtl_json_prop_null(&json, "secondary_receipt");
    } else {
        pdtl_json_prop_u64(&json, "secondary_receipt", result->secondary_receipt);
    }
    pdtl_json_prop_u64(&json, "cancelled_gross", result->cancelled_gross);
    pdtl_json_prop_u64(&json, "fee_before_bps", result->fee_before_bps);
    pdtl_json_prop_u64(&json, "fee_after_bps", result->fee_after_bps);
    pdtl_json_prop_bool(&json, "fee_update_applied", result->fee_update_applied);
    pdtl_json_end_object(&json);

    pdtl_print_routes(&json, ledger);
    pdtl_print_receipts(&json, ledger);
    pdtl_print_journal_tail(&json, ledger);
    pdtl_json_end_object(&json);
    (void)fputc('\n', out);
}

void pdtl_print_usage(FILE *out, const char *program)
{
    (void)fprintf(out, "Usage: %s [scenario]\n", program);
    (void)fputs("\nScenarios:\n", out);
    (void)fputs("  flow            normal movement, consolidation, settlement, withdrawal and route close\n", out);
    (void)fputs("  snapshot        initialized liquidity surface without receipts\n", out);
    (void)fputs("  exploit         vulnerable settlement paid by a foreign cell\n", out);
    (void)fputs("  strict-exploit  same exploit against strict receipt/cell binding\n", out);
    (void)fputs("  cancel          pending receipt cancellation, withdrawal and route close\n", out);
    (void)fputs("  fee-rotation    route fee update followed by strict settlement\n", out);
}
