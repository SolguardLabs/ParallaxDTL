import assert from "node:assert/strict";
import test from "node:test";

import { expectCommon, runScenario } from "../helpers/parallax-cli.js";

test("strict-exploit rechaza el pago desde una celda distinta", () => {
  const report = runScenario("strict-exploit");

  expectCommon(report, "strict-exploit");
  assert.equal(report.binding_ok, true);
  assert.equal(report.ctf_vulnerability_triggered, false);
  assert.equal(report.exploit.settlement_accepted, false);
  assert.equal(report.exploit.strict_rejection, true);
  assert.equal(report.exploit.third_party_paid, 0);
  assert.equal(report.exploit.legitimate_withdrawable_before, 280);
  assert.equal(report.exploit.available_after_foreign_payment, 280);
  assert.equal(report.receipts[0].status, "pending");
  assert.equal(report.receipts[0].paid_by_cell, null);
  assert.equal(report.cells.origin.reserved_out, 700);
  assert.equal(report.cells.sponsor.reserve, 2_000);
  assert.equal(report.cells.beneficiary.reserve, 0);
});

test("el journal deja visible el rechazo de binding estricto", () => {
  const report = runScenario("strict-exploit");
  const rejection = report.journal_tail.find((entry) => entry.kind === "settlement_rejected");

  assert.ok(rejection);
  assert.equal(rejection.cell, report.cells.sponsor.index);
  assert.equal(rejection.counterparty, report.cells.origin.index);
  assert.equal(rejection.receipt, report.receipts[0].index);
  assert.equal(rejection.amount, 700);
});
