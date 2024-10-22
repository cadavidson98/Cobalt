#!/usr/bin/env bash

COBALT_SOURCES="/tmp/cobaltSources.txt"

find cobalt/ -regextype posix-extended -regex ".*\.(h|cpp)" > ${COBALT_SOURCES}

clang-format --verbose -i --style=file --files=${COBALT_SOURCES}
