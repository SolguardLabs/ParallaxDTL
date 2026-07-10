import assert from "node:assert/strict";
import test from "node:test";

import { expectCommon, expectDigest, runScenario } from "../helpers/parallax-cli.js";

test("snapshot inicializa celdas de liquidez sin receipts", () => {
  const report = runScenario("snapshot");

  expectCommon(report, "snapshot");
  assert.equal(report.binding_ok, true);
  assert.equal(report.ctf_vulnerability_triggered, false);
  assert.equal(report.surface.receipts, 0);
  assert.equal(report.totals.initial_supply, 3_200);
  assert.equal(report.totals.cell_reserves, 3_200);
  assert.equal(report.totals.external_withdrawals, 0);
  assert.equal(report.cells.origin.reserve, 1_000);
  assert.equal(report.cells.aux.reserve, 200);
  assert.equal(report.cells.sponsor.reserve, 2_000);
  assert.equal(report.cells.beneficiary.reserve, 0);
  assert.equal(report.cells.treasury.reserve, 0);
});

test("flow normal mueve, consolida, liquida, retira y cierra ruta", () => {
  const report = runScenario("flow");

  expectCommon(report, "flow");
  assert.equal(report.binding_ok, true);
  assert.equal(report.ctf_vulnerability_triggered, false);
  assert.equal(report.surface.receipts, 1);
  assert.equal(report.surface.route_closed, true);
  assert.equal(report.receipts[0].origin_cell, report.cells.origin.index);
  assert.equal(report.receipts[0].paid_by_cell, report.cells.origin.index);
  assert.equal(report.receipts[0].payer_matches_origin, true);
  assert.equal(report.receipts[0].gross, 700);
  assert.equal(report.receipts[0].fee, 14);
  assert.equal(report.receipts[0].net, 686);
  assert.equal(report.cells.origin.reserve, 220);
  assert.equal(report.cells.origin.withdrawn, 280);
  assert.equal(report.cells.aux.state, "closed");
  assert.equal(report.cells.aux.reserve, 0);
  assert.equal(report.cells.beneficiary.reserve, 686);
  assert.equal(report.cells.treasury.reserve, 14);
  assert.equal(report.totals.cell_reserves, 2_920);
  assert.equal(report.totals.external_withdrawals, 280);
  expectDigest(report.receipts[0].digest);
});

test("la invocacion por defecto ejecuta flow", () => {
  const report = runScenario();

  assert.equal(report.scenario, "flow");
  assert.equal(report.conservation_ok, true);
  assert.equal(report.binding_ok, true);
});
