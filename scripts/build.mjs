import { existsSync, mkdirSync, writeFileSync } from "node:fs";
import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";
import { spawnSync } from "node:child_process";
import process from "node:process";

const root = dirname(dirname(fileURLToPath(import.meta.url)));
const buildDir = join(root, "build");
const src = [
  "src/main.c",
  "src/pdtl_hash.c",
  "src/pdtl_journal.c",
  "src/pdtl_ledger.c",
  "src/pdtl_json.c",
  "src/pdtl_scenarios.c",
];

function run(command, args, options = {}) {
  return spawnSync(command, args, {
    cwd: root,
    encoding: "utf8",
    windowsHide: true,
    ...options,
  });
}

function commandExists(command) {
  const probe =
    process.platform === "win32"
      ? run("where.exe", [command])
      : run("sh", ["-lc", `command -v ${command}`]);
  return probe.status === 0;
}

function shQuote(value) {
  return `'${String(value).replace(/'/g, `'\\''`)}'`;
}

function compileHost(compiler) {
  mkdirSync(buildDir, { recursive: true });
  const output = join(buildDir, process.platform === "win32" ? "parallaxdtl.exe" : "parallaxdtl");
  const args = [
    "-std=c11",
    "-Wall",
    "-Wextra",
    "-Wpedantic",
    "-O2",
    "-Isrc",
    ...src,
    "-o",
    output,
  ];
  const result = run(compiler, args);
  if (result.status !== 0) {
    throw new Error(`${compiler} failed\n${result.stdout}\n${result.stderr}`);
  }
  writeFileSync(
    join(buildDir, "runner.json"),
    JSON.stringify({ mode: "host", binary: output }, null, 2),
  );
  return output;
}

function compileMsvc() {
  mkdirSync(buildDir, { recursive: true });
  const output = join(buildDir, "parallaxdtl.exe");
  const args = [
    "/nologo",
    "/std:c11",
    "/W4",
    "/O2",
    "/I",
    "src",
    ...src,
    `/Fe:${output}`,
  ];
  const result = run("cl", args);
  if (result.status !== 0) {
    throw new Error(`cl failed\n${result.stdout}\n${result.stderr}`);
  }
  writeFileSync(
    join(buildDir, "runner.json"),
    JSON.stringify({ mode: "host", binary: output }, null, 2),
  );
  return output;
}

function wslAvailable() {
  if (process.platform !== "win32") {
    return false;
  }
  if (!commandExists("wsl.exe")) {
    return false;
  }
  const probe = run("wsl.exe", ["--exec", "sh", "-lc", "command -v gcc"]);
  return probe.status === 0;
}

function wslPath(windowsPath) {
  const result = run("wsl.exe", ["wslpath", "-a", windowsPath]);
  if (result.status !== 0) {
    throw new Error(`wslpath failed\n${result.stdout}\n${result.stderr}`);
  }
  return result.stdout.trim();
}

function compileWsl() {
  const unixRoot = wslPath(root);
  const command = [
    `cd ${shQuote(unixRoot)}`,
    "mkdir -p build",
    `gcc -std=c11 -Wall -Wextra -Wpedantic -O2 -Isrc ${src.map(shQuote).join(" ")} -o build/parallaxdtl`,
  ].join(" && ");
  const result = run("wsl.exe", ["--exec", "sh", "-lc", command]);
  if (result.status !== 0) {
    throw new Error(`wsl gcc failed\n${result.stdout}\n${result.stderr}`);
  }
  mkdirSync(buildDir, { recursive: true });
  writeFileSync(
    join(buildDir, "runner.json"),
    JSON.stringify({ mode: "wsl", unixRoot, binary: "./build/parallaxdtl" }, null, 2),
  );
  return `${unixRoot}/build/parallaxdtl`;
}

function main() {
  if (commandExists("gcc")) {
    console.log(compileHost("gcc"));
    return;
  }
  if (commandExists("clang")) {
    console.log(compileHost("clang"));
    return;
  }
  if (commandExists("cc")) {
    console.log(compileHost("cc"));
    return;
  }
  if (process.platform === "win32" && commandExists("cl")) {
    console.log(compileMsvc());
    return;
  }
  if (wslAvailable()) {
    console.log(compileWsl());
    return;
  }
  if (!existsSync(buildDir)) {
    mkdirSync(buildDir, { recursive: true });
  }
  throw new Error("No C compiler found. Install gcc/clang/cc, use a Visual Studio developer shell, or enable WSL with gcc.");
}

main();
