/**
 * Build-time data generation. Runs the compiled C++ binaries and gathers the
 * figure data + highlighted source into web/data-static/ (statically imported
 * by pages, so there is no runtime cost) and copies SVG figures + the referat
 * PDF into web/public/.
 *
 * Run automatically before `next build` (the "prebuild" npm hook) and manually
 * before `next dev`:  node scripts/prebuild.mjs
 */
import { execFileSync } from "node:child_process";
import fs from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";
import { codeToHtml } from "shiki";

const HERE = path.dirname(fileURLToPath(import.meta.url));
const WEB = path.resolve(HERE, "..");
const PROJ = path.resolve(WEB, "..");
const OUT = path.join(WEB, "data-static");
const PUB = path.join(WEB, "public");

const isWin = process.platform === "win32";
const exe = (n) => path.join(PROJ, "build", isWin ? `${n}.exe` : n);
const DATADIR = path.join(PROJ, "data");

const NIST = ["data.e", "data.pi", "data.sqrt2", "data.sqrt3", "data.sha1"];

// The compiled referat is served under its title, not a generic "referat.pdf".
const PAPER_PDF = "Testul-Spectral-DFT-NIST-SP-800-22.pdf";

function ensure(dir) {
  fs.mkdirSync(dir, { recursive: true });
}

function writeJson(name, obj) {
  fs.writeFileSync(path.join(OUT, name), JSON.stringify(obj));
}

/** Per-file single-sequence result over the whole file (with spectrum). */
function buildNistResults() {
  const results = [];
  for (const f of NIST) {
    const out = execFileSync(exe("nist_test"), ["--json", "--spectrum", "-t", "dft", f], {
      cwd: DATADIR,
      maxBuffer: 64 * 1024 * 1024,
    }).toString();
    results.push({ file: f, ...JSON.parse(out) });
  }
  writeJson("nist-results.json", results);
  console.log(`[prebuild] nist-results.json (${results.length} files)`);
}

/** Multi-stream proportion + uniformity over all files. */
function buildAssessResults() {
  const args = ["--json", "-t", "dft", "-n", "10000", ...NIST];
  const out = execFileSync(exe("nist_assess"), args, {
    cwd: DATADIR,
    maxBuffer: 16 * 1024 * 1024,
  }).toString();
  writeJson("assess-results.json", JSON.parse(out));
  console.log("[prebuild] assess-results.json");
}

/** Copy the figure JSON + SVG + the referat PDF. */
function copyFigures() {
  const fdIn = path.join(PROJ, "docs", "figure-data");
  const figOut = path.join(OUT, "figures");
  ensure(figOut);
  for (const f of fs.readdirSync(fdIn)) {
    fs.copyFileSync(path.join(fdIn, f), path.join(figOut, f));
  }
  const svgIn = path.join(PROJ, "docs", "figures");
  const svgOut = path.join(PUB, "figures");
  ensure(svgOut);
  for (const f of fs.readdirSync(svgIn)) {
    if (f.endsWith(".svg")) {
      fs.copyFileSync(path.join(svgIn, f), path.join(svgOut, f));
    }
  }
  const pdf = path.join(PROJ, PAPER_PDF);
  if (fs.existsSync(pdf)) {
    fs.copyFileSync(pdf, path.join(PUB, PAPER_PDF));
  }
  console.log(`[prebuild] figures + ${PAPER_PDF} copied`);
}

/** Read + Shiki-highlight the C++/Python source for the in-app browser. */
async function buildSourceIndex() {
  const targets = [
    ...listDir(path.join(PROJ, "src")),
    ...listDir(path.join(PROJ, "tests")).filter((p) => /\.(cpp|py)$/.test(p)),
    path.join(PROJ, "CMakeLists.txt"),
    path.join(PROJ, "docs", "make_figures.py"),
  ].filter((p) => fs.existsSync(p));

  const files = [];
  for (const abs of targets) {
    const rel = path.relative(PROJ, abs).replace(/\\/g, "/");
    const code = fs.readFileSync(abs, "utf8");
    const lang = abs.endsWith(".py")
      ? "python"
      : abs.endsWith("CMakeLists.txt")
        ? "cmake"
        : "cpp";
    let html;
    try {
      html = await codeToHtml(code, { lang, theme: "github-dark" });
    } catch {
      html = `<pre>${code.replace(/[&<>]/g, (c) => ({ "&": "&amp;", "<": "&lt;", ">": "&gt;" })[c])}</pre>`;
    }
    files.push({ path: rel, lang, lines: code.split("\n").length, html });
  }
  writeJson("source-index.json", { files });
  console.log(`[prebuild] source-index.json (${files.length} files)`);
}

function listDir(dir) {
  if (!fs.existsSync(dir)) return [];
  return fs
    .readdirSync(dir)
    .filter((f) => /\.(hpp|cpp|py)$/.test(f))
    .map((f) => path.join(dir, f));
}

async function main() {
  ensure(OUT);
  ensure(PUB);
  copyFigures();
  try {
    buildNistResults();
    buildAssessResults();
  } catch (err) {
    console.warn("[prebuild] WARNING: binary run failed (build the C++ first):", err.message);
    // Write empty placeholders so static imports still resolve.
    if (!fs.existsSync(path.join(OUT, "nist-results.json"))) writeJson("nist-results.json", []);
    if (!fs.existsSync(path.join(OUT, "assess-results.json")))
      writeJson("assess-results.json", { files: [] });
  }
  await buildSourceIndex();
  console.log("[prebuild] done ->", OUT);
}

main();
