#!/usr/bin/env bash
set -euo pipefail

# Destination directory
DEST="/tmp/test"

# List of binaries you consider “most-used” (adjust as needed)
BINARIES=(
  bash
  ls
  cp
  xxd
  # mv
  # grep
  # sed
  # awk
  # find
  # add more as desired
)

rm -rf "${DEST}"

echo "Creating destination directory: ${DEST}"
mkdir -p "${DEST}"

for bin in "${BINARIES[@]}"; do
  echo "Processing binary: ${bin}"
  # find full path
  path="$(which "${bin}" 2>/dev/null || true)"
  if [[ -z "${path}" ]]; then
    echo "  → Warning: binary '${bin}' not found in PATH, skipping."
    continue
  fi
  echo "  → Found: ${path}"
  # copy into DEST
  cp --preserve=mode,timestamps "${path}" "${DEST}/"
  echo "  → Copied ${path} → ${DEST}/"

  # ensure executable bit is set
  chmod +w "${DEST}/$(basename "${path}")"
  echo "  → Made executable: ${DEST}/$(basename "${path}")"
done

echo "Done."
