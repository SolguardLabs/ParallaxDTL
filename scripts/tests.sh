#!/usr/bin/env bash
set -euo pipefail
node scripts/build.mjs
node --test --test-concurrency=1 tests/node/*.test.js
