#!/usr/bin/env bash
# Quarterly / manual open splat source discovery and rate-limited import scaffold.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
CATALOG="$ROOT/assets/splat_catalog"
SOURCES="$CATALOG/sources"
LOG="$CATALOG/sync.log"
RATE_SECONDS="${SOS_SPLAT_SYNC_RATE:-5}"
QUERY='an open repository of gaussian splats from the world'

mkdir -p "$SOURCES" "$CATALOG/imported"
touch "$LOG"

log() { echo "[$(date -Iseconds)] $*" | tee -a "$LOG"; }

discover() {
  log "DISCOVER query=\"$QUERY\""
  log "DISCOVER seed sources in $SOURCES (manual Google/GitHub review each quarter)"
  for f in "$SOURCES"/*.json; do
    [[ -f "$f" ]] || continue
    log "DISCOVER known $(basename "$f")"
  done
  log "DISCOVER next: search \"$QUERY\" and add JSON under sources/ with license field"
}

import_one() {
  local id="${1:-}"
  if [[ -z "$id" ]]; then
    echo "Usage: $0 --import-one <source-id>" >&2
    exit 1
  fi
  local meta="$SOURCES/${id}.json"
  if [[ ! -f "$meta" ]]; then
    log "IMPORT fail missing $meta"
    exit 1
  fi
  log "IMPORT start id=$id rate=${RATE_SECONDS}s"
  sleep "$RATE_SECONDS"
  local policy
  policy="$(python3 -c "import json; print(json.load(open('$meta')).get('import_policy','metadata-only'))")"
  if [[ "$policy" == "reference-only" || "$policy" == "metadata-only" ]]; then
    log "IMPORT skip blob (policy=$policy) metadata recorded"
    exit 0
  fi
  log "IMPORT would download per metadata (not implemented — add URL + checksum to JSON first)"
}

case "${1:-}" in
  --discover) discover ;;
  --import-one) import_one "${2:-}" ;;
  *)
    echo "Usage: $0 --discover | --import-one <source-id>" >&2
    exit 1
    ;;
esac
