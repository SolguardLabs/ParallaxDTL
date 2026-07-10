import assert from "node:assert/strict";
import test from "node:test";

import { expectCommon, expectDigest, routeByName, runScenario } from "../helpers/parallax-cli.js";

test("cancel libera reserva pendiente y permite cerrar ruta sin settlement", () => {
  const report = runScenario("cancel");
  const blue = routeByName(report, "blue-route");

  expectCommon(report, "cancel");
  assert.equal(report.binding_ok, true);
  assert.equal(report.ctf_vulnerability_triggered, false);
  assert.equal(report.surface.route_closed, true);
  assert.equal(report.operations.cancelled_gross, 300);
  assert.equal(report.exploit.legitimate_withdrawable_before, 680);
  assert.equal(report.exploit.available_after_foreign_payment, 980);
  assert.equal(report.exploit.attacker_withdrawn, 180);
  assert.equal(report.receipts[0].status, "cancelled");
  assert.equal(report.receipts[0].paid_by_cell, null);
  assert.equal(report.cells.origin.reserve, 820);
  assert.equal(report.cells.origin.reserved_out, 0);
  assert.equal(report.totals.cell_reserves, 3_020);
  assert.equal(report.totals.external_withdrawals, 180);
  assert.equal(blue.cancelled_receipts, 1);
  assert.equal(blue.cancelled_gross, 300);
  assert.equal(blue.pending_receipts, 0);
  assert.equal(blue.pending_gross, 0);
  assert.equal(blue.closed, true);
});

test("fee-rotation actualiza fees y el receipt nuevo usa la tarifa vigente", () => {
  const report = runScenario("fee-rotation");
  const blue = routeByName(report, "blue-route");

  expectCommon(report, "fee-rotation");
  assert.equal(report.operations.fee_update_applied, true);
  assert.equal(report.operations.fee_before_bps, 200);
  assert.equal(report.operations.fee_after_bps, 350);
  assert.equal(report.receipts[0].origin_cell, report.cells.sponsor.index);
  assert.equal(report.receipts[0].paid_by_cell, report.cells.sponsor.index);
  assert.equal(report.receipts[0].gross, 800);
  assert.equal(report.receipts[0].fee, 28);
  assert.equal(report.receipts[0].net, 772);
  assert.equal(report.cells.sponsor.reserve, 1_200);
  assert.equal(report.cells.beneficiary.reserve, 772);
  assert.equal(report.cells.treasury.reserve, 28);
  assert.equal(blue.fee_bps, 350);
  assert.equal(blue.settled_receipts, 1);
  assert.equal(blue.settled_gross, 800);
  assert.equal(blue.pending_receipts, 0);
  assert.equal(blue.reserved_out, 0);
  assert.equal(blue.health_score, 100);
  expectDigest(blue.digest);
});

test("snapshot expone health y presupuesto de salida por ruta", () => {
  const report = runScenario("snapshot");
  const blue = routeByName(report, "blue-route");
  const treasury = routeByName(report, "treasury");

  assert.equal(blue.reserves, 3_200);
  assert.equal(blue.available, 3_140);
  assert.equal(blue.max_exit_budget, 2_880);
  assert.equal(blue.utilization_bps, 0);
  assert.equal(blue.health_score, 100);
  assert.equal(blue.open_cells, 4);
  assert.equal(treasury.reserves, 0);
  assert.equal(treasury.open_cells, 1);
  assert.equal(treasury.health_score, 100);
});
