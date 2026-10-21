import assert from "node:assert/strict";
import test from "node:test";

import { expectDigest, routeByName, runScenario } from "../helpers/parallax-cli.js";

const scenarios = ["snapshot", "flow", "cancel", "fee-rotation"];

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
  const report = runScenario("flow");
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

test("el mismo escenario produce identidad de estado y secuencia reproducibles", () => {
  const first = runScenario("fee-rotation");
  const second = runScenario("fee-rotation");

  assert.equal(first.state_digest, second.state_digest);
  assert.deepEqual(first.receipts, second.receipts);
  assert.deepEqual(first.journal_tail, second.journal_tail);
});

test("la distribución de una liquidación conserva bruto, neto y comisión", () => {
  for (const name of ["flow", "fee-rotation"]) {
    const report = runScenario(name);
    const receipt = report.receipts[0];
    assert.equal(receipt.gross, receipt.net + receipt.fee);
    assert.equal(report.cells.beneficiary.settled_in, receipt.net);
    assert.equal(report.cells.treasury.settled_in, receipt.fee);
    assert.ok(report.totals.settled_out >= receipt.gross);
  }
});
