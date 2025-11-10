#!/usr/bin/env bash

set -euo pipefail

DEST="/tmp/test"

BINARIES=(
  bash
  ls
  cp
  xxd
   mv
   grep
   sed
   awk
   find
   cat
   ssh
   openssl
   ssh-keygen
)

rm -rf "${DEST}"

echo "Creating destination directory: ${DEST}"
mkdir -p "${DEST}"

for bin in "${BINARIES[@]}"; do
  echo "Processing binary: ${bin}"
  path="$(which "${bin}" 2>/dev/null || true)"
  if [[ -z "${path}" ]]; then
    echo "  → Warning: binary '${bin}' not found in PATH, skipping."
    continue
  fi
  echo "  → Found: ${path}"
  cp --preserve=mode,timestamps "${path}" "${DEST}/"
  echo "  → Copied ${path} → ${DEST}/"

  chmod +w "${DEST}/$(basename "${path}")"
  echo "  → Made executable: ${DEST}/$(basename "${path}")"
done

echo "Done."
