import { execFileSync } from "node:child_process";
import { existsSync, readFileSync } from "node:fs";
import { dirname, extname, join } from "node:path";
import { fileURLToPath } from "node:url";

const root = dirname(dirname(fileURLToPath(import.meta.url)));
const textExtensions = new Set([
  ".c", ".h", ".js", ".mjs", ".md", ".json", ".yml", ".yaml", ".sh",
]);
const textNames = new Set([
  "Makefile", "LICENSE", ".editorconfig", ".gitignore", ".gitattributes",
]);

const files = execFileSync("git", ["ls-files", "-z"], { cwd: root })
  .toString("utf8")
  .split("\0")
  .filter(Boolean)
  .filter((path) => textExtensions.has(extname(path)) || textNames.has(path))
  .filter((path) => existsSync(join(root, path)))
  .sort();

let total = 0;
for (const path of files) {
  const source = readFileSync(join(root, path), "utf8");
  const count = source.split(/\r?\n/).filter((line) => line.trim().length > 0).length;
  total += count;
  console.log(`${String(count).padStart(5)} ${path}`);
}

console.log(`${String(total).padStart(5)} nonblank lines in ${files.length} versioned text files`);
