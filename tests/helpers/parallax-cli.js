import assert from "node:assert/strict";
import { closeSync, existsSync, mkdirSync, openSync, readFileSync, unlinkSync } from "node:fs";
import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";
import { spawnSync } from "node:child_process";

const root = dirname(dirname(dirname(fileURLToPath(import.meta.url))));
const runnerPath = join(root, "build", "runner.json");
const lockPath = join(root, "build", ".build.lock");
let built = false;

function shQuote(value) {
  return `'${String(value).replace(/'/g, `'\\''`)}'`;
}

function buildOnce() {
  if (built || existsSync(runnerPath)) {
    return;
  }
  mkdirSync(join(root, "build"), { recursive: true });

  let lockFd;
  try {
    lockFd = openSync(lockPath, "wx");
  } catch {
    const waitBuffer = new Int32Array(new SharedArrayBuffer(4));
    for (let attempt = 0; attempt < 600; attempt += 1) {
      if (existsSync(runnerPath)) {
        built = true;
        return;
      }
      Atomics.wait(waitBuffer, 0, 0, 100);
    }
    assert.fail("timed out waiting for ParallaxDTL build lock");
  }

  const result = spawnSync(process.execPath, ["scripts/build.mjs"], {
    cwd: root,
    encoding: "utf8",
    windowsHide: true,
  });
  try {
    assert.equal(
      result.status,
      0,
      `build failed\nstdout:\n${result.stdout}\nstderr:\n${result.stderr}`,
    );
    built = true;
  } finally {
    closeSync(lockFd);
    try {
      unlinkSync(lockPath);
    } catch {
      // Another process may have already cleaned up after a failed build.
    }
  }
}

export function runRaw(args = []) {
  buildOnce();
  const runner = JSON.parse(readFileSync(runnerPath, "utf8"));

  if (runner.mode === "wsl") {
    const command = [`cd ${shQuote(runner.unixRoot)}`, [runner.binary, ...args.map(shQuote)].join(" ")].join(
      " && ",
    );
    return spawnSync("wsl.exe", ["--exec", "sh", "-lc", command], {
      cwd: root,
      encoding: "utf8",
      windowsHide: true,
    });
  }

  return spawnSync(runner.binary, args, {
    cwd: root,
    encoding: "utf8",
    windowsHide: true,
  });
}

export function runScenario(name) {
  const args = name ? [name] : [];
  const result = runRaw(args);
  assert.equal(
    result.status,
    0,
    `scenario ${name ?? "(default)"} failed\nstdout:\n${result.stdout}\nstderr:\n${result.stderr}`,
  );
  return JSON.parse(result.stdout);
}

export function expectDigest(value) {
  assert.match(value, /^[0-9a-f]{64}$/);
}

export function expectCommon(report, scenario) {
  assert.equal(report.scenario, scenario);
  assert.equal(report.network_id, "parallax-localnet-ctf");
  assert.equal(report.conservation_ok, true);
  assert.equal(report.totals.global_ok, true);
  assert.equal(report.surface.policies, 2);
  assert.equal(report.surface.routes, 2);
  assert.equal(report.surface.cells, 5);
  expectDigest(report.state_digest);
}

export function routeByName(report, name) {
  const route = report.routes.find((entry) => entry.name === name);
  assert.ok(route, `route ${name} should exist`);
  return route;
}
