# Splat Catalog

Metadata for discovered open Gaussian splat datasets and libraries. Blobs are imported slowly; see `.cursor/rules/gaussian-splat-sources.mdc`.

## Run sync manually

```bash
./scripts/splat_sources_sync.sh --discover    # log candidates, no downloads
./scripts/splat_sources_sync.sh --import-one sample_antimatter15  # rate-limited
```

## Structure

- `sources/*.json` — discovery records (URL, license, status)
- `imported/` — optional local copies (large files gitignored)
- `sync.log` — append-only audit trail
