import assert from "node:assert/strict";
import test from "node:test";

import { expectDigest, routeByName, runScenario } from "../helpers/parallax-cli.js";

const scenarios = ["snapshot", "flow", "exploit", "strict-exploit", "cancel", "fee-rotation"];

test("todos los escenarios reportan la ecuacion de reconciliacion global", () => {
  for (const name of scenarios) {
    const report = runScenario(name);
    const accounted =
      report.totals.cell_reserves + report.totals.external_withdrawals;

    assert.equal(
      accounted,
      report.totals.initial_supply,
      `${name} debe conservar reservas mas withdrawals`,
    );
    assert.equal(report.conservation_ok, true);
    assert.equal(report.totals.global_ok, true);
  }
});

test("los digests de celdas y receipts mantienen formato estable", () => {
  const report = runScenario("exploit");
  const blue = routeByName(report, "blue-route");

  expectDigest(report.state_digest);
  expectDigest(report.cells.origin.digest);
  expectDigest(report.cells.sponsor.digest);
  expectDigest(report.cells.beneficiary.digest);
  expectDigest(report.cells.treasury.digest);
  expectDigest(report.receipts[0].digest);
  expectDigest(report.receipts[0].route_digest);
  expectDigest(blue.digest);
});

test("la celda origen cambia solo en reserved_out cuando strict binding rechaza", () => {
  const report = runScenario("strict-exploit");

  assert.equal(report.cells.origin.reserve, 1_000);
  assert.equal(report.cells.origin.reserved_out, 700);
  assert.equal(report.cells.origin.available, 280);
  assert.equal(report.cells.origin.withdrawn, 0);
  assert.equal(report.cells.sponsor.reserve, 2_000);
  assert.equal(report.cells.sponsor.settled_out, 0);
  assert.equal(report.totals.reserved_out, 700);
});

test("el exploit y el camino estricto solo difieren en binding y efectos derivados", () => {
  const exploit = runScenario("exploit");
  const strict = runScenario("strict-exploit");

  assert.equal(exploit.receipts[0].gross, strict.receipts[0].gross);
  assert.equal(exploit.receipts[0].fee, strict.receipts[0].fee);
  assert.equal(exploit.receipts[0].net, strict.receipts[0].net);
  assert.equal(exploit.receipts[0].origin_cell, strict.receipts[0].origin_cell);
  assert.equal(exploit.receipts[0].beneficiary_cell, strict.receipts[0].beneficiary_cell);
  assert.equal(exploit.binding_ok, false);
  assert.equal(strict.binding_ok, true);
  assert.equal(exploit.cells.sponsor.reserve, 1_300);
  assert.equal(strict.cells.sponsor.reserve, 2_000);
  assert.equal(exploit.cells.origin.withdrawn, 980);
  assert.equal(strict.cells.origin.withdrawn, 0);
});

test("el journal tail conserva eventos suficientes para explicar el estado final", () => {
  const report = runScenario("flow");
  const blue = routeByName(report, "blue-route");
  const kinds = report.journal_tail.map((entry) => entry.kind);

  assert.ok(kinds.includes("receipt_issued"));
  assert.ok(kinds.includes("receipt_settled"));
  assert.ok(kinds.includes("withdrawal"));
  assert.equal(kinds.at(-1), "route_closed");
  assert.equal(blue.settled_gross, 700);
  assert.equal(blue.pending_gross, 0);
  for (const entry of report.journal_tail) {
    expectDigest(entry.digest);
  }
});
