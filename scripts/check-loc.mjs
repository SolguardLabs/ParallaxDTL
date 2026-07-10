import { readFileSync, readdirSync, statSync } from "node:fs";
import { dirname, join, relative } from "node:path";
import { fileURLToPath } from "node:url";

const root = dirname(dirname(fileURLToPath(import.meta.url)));
const roots = ["src", "tests", "scripts"];
const docs = ["README.md", "vulnerability.md", "SECURITY.md", "Makefile", "package.json"];
const extensions = new Set([".c", ".h", ".js", ".mjs", ".md"]);

function extension(path) {
  const index = path.lastIndexOf(".");
  return index === -1 ? "" : path.slice(index);
}

function walk(path, output) {
  for (const entry of readdirSync(path)) {
    if (entry === "build" || entry === "node_modules") {
      continue;
    }
    const full = join(path, entry);
    const stat = statSync(full);
    if (stat.isDirectory()) {
      walk(full, output);
    } else if (extensions.has(extension(full))) {
      output.push(full);
    }
  }
}

const files = [];
for (const dir of roots) {
  walk(join(root, dir), files);
}
for (const file of docs) {
  files.push(join(root, file));
}

let total = 0;
const rows = [];
for (const file of files) {
  const content = readFileSync(file, "utf8");
  const lines = content.length === 0 ? 0 : content.split(/\r?\n/).length;
  total += lines;
  rows.push({ file: relative(root, file), lines });
}

rows.sort((a, b) => a.file.localeCompare(b.file));
for (const row of rows) {
  console.log(`${String(row.lines).padStart(5, " ")} ${row.file}`);
}
console.log(`${String(total).padStart(5, " ")} total`);
