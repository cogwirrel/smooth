#!/bin/bash
# USAGE: exactly as that of create-experiment

# Source DB variables and check that they are set
source db.config
echo "${DBSERVER?}" "${DBPATH?}"

db/remote-exec.py --path "$DBPATH" "$DBSERVER" ./create-experiment.py "$@"

