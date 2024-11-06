#!/usr/bin/env bash

COBALT_SOURCES="/tmp/cobaltSources.txt"

find . -regextype posix-extended -regex ".*\.(h|cpp)" -not -path "./build/*" -not -path './extern/*' -not -path "./deprecate/*" > ${COBALT_SOURCES}

clang-format --verbose -i --style=file --files=${COBALT_SOURCES}
