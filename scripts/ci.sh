#!/usr/bin/env bash
set -euo pipefail
bash scripts/tests.sh
node scripts/check-loc.mjs
