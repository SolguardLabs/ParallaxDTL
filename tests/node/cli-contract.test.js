import assert from "node:assert/strict";
import test from "node:test";

import { runRaw } from "../helpers/parallax-cli.js";

test("help muestra escenarios disponibles", () => {
  const result = runRaw(["--help"]);

  assert.equal(result.status, 0);
  assert.match(result.stdout, /Usage:/);
  assert.match(result.stdout, /exploit/);
  assert.match(result.stdout, /strict-exploit/);
  assert.match(result.stdout, /cancel/);
  assert.match(result.stdout, /fee-rotation/);
});

test("escenario desconocido falla con error legible", () => {
  const result = runRaw(["unknown"]);

  assert.equal(result.status, 1);
  assert.match(result.stderr, /PDTL_ERR_ARGUMENT/);
  assert.match(result.stderr, /unknown scenario/);
  assert.equal(result.stdout, "");
});

test("demasiados argumentos devuelven codigo de uso", () => {
  const result = runRaw(["flow", "extra"]);

  assert.equal(result.status, 2);
  assert.match(result.stderr, /Usage:/);
});
